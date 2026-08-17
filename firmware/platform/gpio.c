#include "gpio.h"
#include <SN8F5708.H>

/* =========================================================================
 * Platform GPIO Implementation for SONiX SN8F5708 EVK
 * Official SONiX Hardware Pinout & Multi-Column Support
 * ========================================================================= */

sfr P2UR_REG = 0xF3; /* Port 2 Pull-up Register */
sfr P4UR_REG = 0xF5; /* Port 4 Pull-up Register */

sbit PIN_HW_LED_D4  = P0^3; /* Status LED D4 (Silkscreen: P03) */
sbit PIN_HW_BUZZER  = P1^0; /* Buzzer PZ1 (Silkscreen: P10) */

/* I2C EEPROM Pins on Port 1 */
sbit PIN_HW_I2C_SCL = P1^4; /* Silkscreen: SCL_P14 */
sbit PIN_HW_I2C_SDA = P1^5; /* Silkscreen: SDA_P15 */

/* 7-Segment Digit Control Pins on Port 5 */
sbit PIN_HW_DIG1    = P5^0; /* Q1 (Hour Tens) */
sbit PIN_HW_DIG2    = P5^1; /* Q2 (Hour Units + Colon) */
sbit PIN_HW_DIG3    = P5^2; /* Q3 (Minute Tens) */
sbit PIN_HW_DIG4    = P5^3; /* Q4 (Minute Units) */

/* Button Matrix Pins on Port 2 (Inputs) and Port 4 (Scan Outputs) */
sbit PIN_HW_ROW0    = P2^4; /* Row 0: SW3, SW4, SW5, SW6 */
sbit PIN_HW_ROW1    = P2^5; /* Row 1: SW10 */
sbit PIN_HW_ROW3    = P2^7; /* Row 3: SW16 */

sbit PIN_HW_COL0    = P4^4; /* Col 0: SW3, SW16 */
sbit PIN_HW_COL1    = P4^5; /* Col 1: SW4 */
sbit PIN_HW_COL2    = P4^6; /* Col 2: SW5 */
sbit PIN_HW_COL3    = P4^7; /* Col 3: SW6, SW10 */

static unsigned char s_buzzer_state = 0;
static unsigned char s_led_d4_state = 0;
static unsigned char s_display_segments = 0;
static unsigned char s_active_digit = 0xFF;

/* Simulation State Flags */
static unsigned char s_sim_sw3_pressed = 0;
static unsigned char s_sim_sw6_pressed = 0;
static unsigned char s_sim_sw10_pressed = 0;
static unsigned char s_sim_sw16_pressed = 0;

static void delay_settle(void)
{
    volatile unsigned char d;
    for (d = 0; d < 30; d++);
}

void GPIO_Init(void)
{
    /* Configure Port Directions */
    P0M |= 0x08; /* P0.3 Output (LED D4) */
    P1M |= 0x31; /* P1.0 Output (Buzzer), P1.4, P1.5 I2C */
    P2M &= ~0xB0;/* P2.4, P2.5, P2.7 as Inputs */
    P3M  = 0xFF; /* Port 3 7-Segment Output */
    P4M |= 0xF0; /* P4.4, P4.5, P4.6, P4.7 as Outputs */
    P5M |= 0x0F; /* Port 5 Digit Select Output */

    /* Enable Pull-ups on Row Inputs (P2.4, P2.5, P2.7) */
    P2UR_REG |= 0xB0;

    /* Set Column Scan Outputs to idle HIGH */
    PIN_HW_COL0 = 1;
    PIN_HW_COL1 = 1;
    PIN_HW_COL2 = 1;
    PIN_HW_COL3 = 1;

    /* Set Row input lines High */
    PIN_HW_ROW0 = 1;
    PIN_HW_ROW1 = 1;
    PIN_HW_ROW3 = 1;

    /* Reset simulation flags */
    s_sim_sw3_pressed = 0;
    s_sim_sw6_pressed = 0;
    s_sim_sw10_pressed = 0;
    s_sim_sw16_pressed = 0;

    /* Set default inactive states for outputs */
    GPIO_SetBuzzer(PIN_STATE_LOW);
    GPIO_SetLED_D4(PIN_STATE_LOW);
    GPIO_SelectDisplayDigit(0xFF);
    GPIO_SetDisplaySegments(0x00);

    /* Set I2C idle HIGH */
    PIN_HW_I2C_SCL = 1;
    PIN_HW_I2C_SDA = 1;
}

void GPIO_SetBuzzer(Pin_State_t state)
{
    s_buzzer_state = (unsigned char)state;
    PIN_HW_BUZZER = (state == PIN_STATE_HIGH) ? 1 : 0; /* Active HIGH */
}

void GPIO_SetLED_D4(Pin_State_t state)
{
    s_led_d4_state = (unsigned char)state;
    /* Active LOW: state HIGH -> LED ON (pin=0), state LOW -> LED OFF (pin=1) */
    PIN_HW_LED_D4 = (state == PIN_STATE_HIGH) ? 0 : 1;
}

void GPIO_SetDisplaySegments(unsigned char segment_bitmap)
{
    s_display_segments = segment_bitmap;
    P3 = segment_bitmap;
}

void GPIO_SelectDisplayDigit(unsigned char digit_index)
{
    s_active_digit = digit_index;

    PIN_HW_DIG1 = 0;
    PIN_HW_DIG2 = 0;
    PIN_HW_DIG3 = 0;
    PIN_HW_DIG4 = 0;

    switch (digit_index)
    {
        case 0: PIN_HW_DIG1 = 1; break; /* Q1 ON */
        case 1: PIN_HW_DIG2 = 1; break; /* Q2 ON */
        case 2: PIN_HW_DIG3 = 1; break; /* Q3 ON */
        case 3: PIN_HW_DIG4 = 1; break; /* Q4 ON */
        default: break;
    }
}

/* =========================================================================
 * Button Scanning Routines with Multi-Column Robustness
 * ========================================================================= */

Pin_State_t GPIO_ReadButton_SW3(void)
{
    Pin_State_t state;
    if (s_sim_sw3_pressed) return PIN_STATE_LOW;

    /* Drive Col 0 (P4.4), Col 1 (P4.5), Col 2 (P4.6) LOW */
    PIN_HW_COL0 = 0;
    PIN_HW_COL1 = 0;
    PIN_HW_COL2 = 0;
    delay_settle();

    /* Read Row 0 (P2.4) */
    state = (PIN_HW_ROW0 == 0) ? PIN_STATE_LOW : PIN_STATE_HIGH;

    /* Restore Columns to HIGH */
    PIN_HW_COL0 = 1;
    PIN_HW_COL1 = 1;
    PIN_HW_COL2 = 1;

    return state;
}

Pin_State_t GPIO_ReadButton_SW6(void)
{
    Pin_State_t state;
    if (s_sim_sw6_pressed) return PIN_STATE_LOW;

    /* Drive Column 3 (P4.7) LOW */
    PIN_HW_COL3 = 0;
    delay_settle();

    /* Read Row 0 (P2.4) */
    state = (PIN_HW_ROW0 == 0) ? PIN_STATE_LOW : PIN_STATE_HIGH;

    /* Restore Column 3 to HIGH */
    PIN_HW_COL3 = 1;

    return state;
}

Pin_State_t GPIO_ReadButton_SW10(void)
{
    Pin_State_t state;
    if (s_sim_sw10_pressed) return PIN_STATE_LOW;

    /* Drive Column 3 (P4.7) LOW */
    PIN_HW_COL3 = 0;
    delay_settle();

    /* Read Row 1 (P2.5) */
    state = (PIN_HW_ROW1 == 0) ? PIN_STATE_LOW : PIN_STATE_HIGH;

    /* Restore Column 3 to HIGH */
    PIN_HW_COL3 = 1;

    return state;
}

Pin_State_t GPIO_ReadButton_SW16(void)
{
    Pin_State_t state;
    if (s_sim_sw16_pressed) return PIN_STATE_LOW;

    /* Drive Column 0 (P4.4) LOW */
    PIN_HW_COL0 = 0;
    delay_settle();

    /* Read Row 3 (P2.7) */
    state = (PIN_HW_ROW3 == 0) ? PIN_STATE_LOW : PIN_STATE_HIGH;

    /* Restore Column 0 to HIGH */
    PIN_HW_COL0 = 1;

    return state;
}

void GPIO_SimulateButtonPress(unsigned char button_id, unsigned char is_pressed)
{
    switch (button_id)
    {
        case 0: s_sim_sw3_pressed = is_pressed; break;
        case 1: s_sim_sw6_pressed = is_pressed; break;
        case 2: s_sim_sw10_pressed = is_pressed; break;
        case 3: s_sim_sw16_pressed = is_pressed; break;
        default: break;
    }
}

/* =========================================================================
 * I2C Bit-Banging Physical Pin Control
 * ========================================================================= */

void GPIO_SetI2C_SCL(Pin_State_t state)
{
    PIN_HW_I2C_SCL = (state == PIN_STATE_HIGH) ? 1 : 0;
}

void GPIO_SetI2C_SDA(Pin_State_t state)
{
    PIN_HW_I2C_SDA = (state == PIN_STATE_HIGH) ? 1 : 0;
}

Pin_State_t GPIO_ReadI2C_SDA(void)
{
    return (PIN_HW_I2C_SDA == 1) ? PIN_STATE_HIGH : PIN_STATE_LOW;
}

void GPIO_SetI2C_SDA_Mode(Pin_Mode_t mode)
{
    if (mode == PIN_MODE_INPUT_PULLUP || mode == PIN_MODE_INPUT_FLOATING)
    {
        /* Set P1.5 as Input (Bit 5 of P1M = 0) */
        P1M &= ~0x20;
        PIN_HW_I2C_SDA = 1; /* Set high for input / pull-up */
    }
    else
    {
        /* Set P1.5 as Output (Bit 5 of P1M = 1) */
        P1M |= 0x20;
    }
}
