#include "gpio.h"
#include <SN8F5708.H>

/* =========================================================================
 * Platform GPIO Implementation for SONiX SN8F5708 EVK
 * Official SONiX EVK Reference:
 * - Scan Outputs (Columns): Port 4 (P4.4..P4.7 driven LOW during scan)
 * - Key Inputs (Rows):      Port 2 (P2.4..P2.7 with P2UR pull-up enabled)
 * ========================================================================= */

sfr P2UR_REG = 0xF3; /* Port 2 Pull-up Register */
sfr P4UR_REG = 0xF5; /* Port 4 Pull-up Register */

sbit PIN_HW_LED_D4  = P0^3; /* Status LED D4 (Silkscreen: P03) */
sbit PIN_HW_BUZZER  = P1^0; /* Buzzer PZ1 (Silkscreen: P10) */

/* I2C EEPROM (24C05) Pins on Port 1 */
sbit PIN_HW_I2C_SCL = P1^4; /* Silkscreen: SCL_P14 */
sbit PIN_HW_I2C_SDA = P1^5; /* Silkscreen: SDA_P15 */

/* 7-Segment Digit Control Pins on Port 5 */
sbit PIN_HW_DIG1    = P5^0; /* Q1 (Hour Tens) */
sbit PIN_HW_DIG2    = P5^1; /* Q2 (Hour Units + Colon) */
sbit PIN_HW_DIG3    = P5^2; /* Q3 (Minute Tens) */
sbit PIN_HW_DIG4    = P5^3; /* Q4 (Minute Units) */

/* Button Matrix Pins: Rows on Port 2 (Inputs), Columns on Port 4 (Outputs) */
sbit PIN_HW_ROW0    = P2^4; /* Row 0: SW3, SW6 */
sbit PIN_HW_ROW1    = P2^5; /* Row 1: SW10 */
sbit PIN_HW_ROW2    = P2^6; /* Row 2: SW11..14 (Inactive) */
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
    volatile unsigned int d;
    for (d = 0; d < 100; d++); /* Settling time for matrix capacitance */
}

void GPIO_Init(void)
{
    /*
     * Port Mode Configurations:
     * - P0M: Bit 3 (0x08) = Output for LED D4
     * - P1M: Bit 0 (0x01) = Output for Buzzer, Bits 4,5 (0x00) = I2C released (Input/High-Z)
     * - P2M: Bits 4..7 (0x00) = Input for Button Matrix Rows (with P2UR Pull-up)
     * - P3M: 0xFF = Output for 7-Segment Segments (P3.0 -> P3.7)
     * - P4M: Bits 4..7 (0xF0) = Output for Button Matrix Columns (driven LOW during scan)
     * - P5M: Bits 0..3 (0x0F) = Output for Digits Q1..Q4 (P5.0 -> P5.3)
     */
    P0M |= 0x08; /* P0.3 Output (LED D4) */
    P1M |= 0x01; /* P1.0 Output (Buzzer) */
    P2M &= ~0xF0;/* P2.4..P2.7 as Inputs (Matrix Rows) */
    P3M  = 0xFF; /* Port 3 7-Segment Output */
    P4M |= 0xF0; /* P4.4..P4.7 as Outputs (Matrix Columns) */
    P5M |= 0x0F; /* Port 5 Digit Select Output */

    /* Enable 100k Pull-ups on Row Inputs (P2.4..P2.7 via P2UR: 0xF3) */
    P2UR_REG |= 0xF0;

    /* Set Column Scan Outputs to idle HIGH (Inactive) */
    P4 |= 0xF0;

    /* Set Row input lines latch HIGH */
    PIN_HW_ROW0 = 1;
    PIN_HW_ROW1 = 1;
    PIN_HW_ROW2 = 1;
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

    /* Release I2C Bus to High-Z */
    GPIO_SetI2C_SCL(PIN_STATE_HIGH);
    GPIO_SetI2C_SDA(PIN_STATE_HIGH);
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
    P3 = segment_bitmap; /* Output segment pattern to Port 3 */
}

void GPIO_SelectDisplayDigit(unsigned char digit_index)
{
    s_active_digit = digit_index;

    /* Turn OFF all digits first (NPN Base LOW = 0) */
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
 * Button Matrix Scanning (Official SONiX EVK Reference)
 * Col Output LOW -> Read Row Input with P2UR pull-up
 * ========================================================================= */

Pin_State_t GPIO_ReadButton_SW3(void)
{
    Pin_State_t state;
    if (s_sim_sw3_pressed) return PIN_STATE_LOW;

    /* Ensure all columns idle HIGH */
    P4 |= 0xF0;

    /* Drive Column 0 (P4.4) LOW */
    PIN_HW_COL0 = 0;
    delay_settle();

    /* Read Row 0 (P2.4) */
    state = (PIN_HW_ROW0 == 0) ? PIN_STATE_LOW : PIN_STATE_HIGH;

    /* Restore Column 0 to HIGH */
    PIN_HW_COL0 = 1;

    return state;
}

Pin_State_t GPIO_ReadButton_SW6(void)
{
    Pin_State_t state;
    if (s_sim_sw6_pressed) return PIN_STATE_LOW;

    P4 |= 0xF0;

    /* Drive Column 3 (P4.7) LOW */
    PIN_HW_COL3 = 0;
    delay_settle();

    /* Read Row 0 (P2.4) */
    state = (PIN_HW_ROW0 == 0) ? PIN_STATE_LOW : PIN_STATE_HIGH;

    PIN_HW_COL3 = 1;

    return state;
}

Pin_State_t GPIO_ReadButton_SW10(void)
{
    Pin_State_t state;
    if (s_sim_sw10_pressed) return PIN_STATE_LOW;

    P4 |= 0xF0;

    /* Drive Column 3 (P4.7) LOW */
    PIN_HW_COL3 = 0;
    delay_settle();

    /* Read Row 1 (P2.5) */
    state = (PIN_HW_ROW1 == 0) ? PIN_STATE_LOW : PIN_STATE_HIGH;

    PIN_HW_COL3 = 1;

    return state;
}

Pin_State_t GPIO_ReadButton_SW16(void)
{
    Pin_State_t state;
    if (s_sim_sw16_pressed) return PIN_STATE_LOW;

    P4 |= 0xF0;

    /* Drive Column 0 (P4.4) LOW */
    PIN_HW_COL0 = 0;
    delay_settle();

    /* Read Row 3 (P2.7) */
    state = (PIN_HW_ROW3 == 0) ? PIN_STATE_LOW : PIN_STATE_HIGH;

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
 * I2C EEPROM (24C05) Bit-Banging Physical Pin Control
 * True Open-Drain / Bus-Release Implementation (IMP-I2C-01):
 * - Driving LOW:  Set pin mode to OUTPUT, set pin latch = 0
 * - Driving HIGH: Set pin mode to INPUT (High-Z release), set pin latch = 1
 * ========================================================================= */

void GPIO_SetI2C_SCL(Pin_State_t state)
{
    if (state == PIN_STATE_HIGH)
    {
        P1M &= ~0x10;       /* Set P1.4 as Input / High-Z */
        PIN_HW_I2C_SCL = 1;
    }
    else
    {
        PIN_HW_I2C_SCL = 0;
        P1M |= 0x10;        /* Set P1.4 as Output */
    }
}

void GPIO_SetI2C_SDA(Pin_State_t state)
{
    if (state == PIN_STATE_HIGH)
    {
        P1M &= ~0x20;       /* Set P1.5 as Input / High-Z */
        PIN_HW_I2C_SDA = 1;
    }
    else
    {
        PIN_HW_I2C_SDA = 0;
        P1M |= 0x20;        /* Set P1.5 as Output */
    }
}

Pin_State_t GPIO_ReadI2C_SDA(void)
{
    return (PIN_HW_I2C_SDA == 1) ? PIN_STATE_HIGH : PIN_STATE_LOW;
}

void GPIO_SetI2C_SDA_Mode(Pin_Mode_t mode)
{
    if (mode == PIN_MODE_INPUT_PULLUP || mode == PIN_MODE_INPUT_FLOATING)
    {
        P1M &= ~0x20;       /* Set P1.5 as Input / High-Z */
        PIN_HW_I2C_SDA = 1;
    }
    else
    {
        P1M |= 0x20;        /* Set P1.5 as Output */
    }
}
