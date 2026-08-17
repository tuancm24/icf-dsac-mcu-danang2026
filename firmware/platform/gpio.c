#include "gpio.h"
#include <SN8F5708.H>

/* =========================================================================
 * Platform GPIO Implementation for SONiX SN8F5708 EVK
 * Verified Pinout from Silkscreen (5708_EVK-V1.0 2024.02.26):
 * - LED D4: P0.3 (Active LOW)
 * - Buzzer: P1.0 (Active HIGH)
 * - I2C EEPROM: SCL (P1.4), SDA (P1.5)
 * - 7-Segment: Segments (P3.0 -> P3.7), Digits Q1..Q4 (P5.0 -> P5.3)
 * - Button Matrix: Rows (P2.4, P2.5, P2.7), Cols (P4.4, P4.7)
 * ========================================================================= */

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

/* Button Matrix Pins on Port 2 (Rows) and Port 4 (Cols) */
sbit PIN_HW_ROW1    = P2^4; /* Row 1: SW3, SW4, SW5, SW6 */
sbit PIN_HW_ROW2    = P2^5; /* Row 2: SW7, SW8, SW9, SW10 */
sbit PIN_HW_ROW4    = P2^7; /* Row 4: SW16, SW15, SW17, SW18 */
sbit PIN_HW_COL1    = P4^4; /* Col 1: SW3, SW7, SW11, SW16 */
sbit PIN_HW_COL4    = P4^7; /* Col 4: SW6, SW10, SW14, SW18 */

static unsigned char s_buzzer_state = 0;
static unsigned char s_led_d4_state = 0;
static unsigned char s_display_segments = 0;
static unsigned char s_active_digit = 0xFF;

void GPIO_Init(void)
{
    /*
     * Port Mode Configurations:
     * - P0M: Bit 3 (0x08) = Output for LED D4
     * - P1M: Bit 0 (0x01) = Output for Buzzer, Bits 4,5 (0x30) = I2C Open-Drain / Output
     * - P2M: Bits 4,5,7 (0xB0) = Output for Button Matrix Rows
     * - P3M: 0xFF = Output for 7-Segment Segments (P3.0 -> P3.7)
     * - P4M: Bits 4,7 (0x00) = Input for Button Matrix Columns
     * - P5M: Bits 0..3 (0x0F) = Output for Digits Q1..Q4 (P5.0 -> P5.3)
     */
    P0M |= 0x08; /* P0.3 Output */
    P1M |= 0x31; /* P1.0 Output (Buzzer), P1.4, P1.5 I2C */
    P2M |= 0xB0; /* P2.4, P2.5, P2.7 Output for Matrix Rows */
    P3M  = 0xFF; /* Port 3 7-Segment Output */
    P4M &= ~0x90;/* P4.4, P4.7 Input for Matrix Columns */
    P5M |= 0x0F; /* Port 5 Digit Select Output */

    /* Set button rows to idle HIGH */
    PIN_HW_ROW1 = 1;
    PIN_HW_ROW2 = 1;
    PIN_HW_ROW4 = 1;

    /* Enable pull-ups / High level on input columns */
    PIN_HW_COL1 = 1;
    PIN_HW_COL4 = 1;

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
    P3 = segment_bitmap; /* Output segment pattern to Port 3 */
}

void GPIO_SelectDisplayDigit(unsigned char digit_index)
{
    s_active_digit = digit_index;

    /* Turn OFF all digits first (Active LOW on Port 5) */
    PIN_HW_DIG1 = 1;
    PIN_HW_DIG2 = 1;
    PIN_HW_DIG3 = 1;
    PIN_HW_DIG4 = 1;

    switch (digit_index)
    {
        case 0: PIN_HW_DIG1 = 0; break;
        case 1: PIN_HW_DIG2 = 0; break;
        case 2: PIN_HW_DIG3 = 0; break;
        case 3: PIN_HW_DIG4 = 0; break;
        default: break;
    }
}

/* =========================================================================
 * Button Matrix Scanning with Strict Row Isolation
 * Active ROW is driven LOW (0); Inactive ROWs remain HIGH (1).
 * ROW state is immediately restored to HIGH after reading.
 * ========================================================================= */

Pin_State_t GPIO_ReadButton_SW3(void)
{
    Pin_State_t state;
    /* Ensure all other rows are inactive */
    PIN_HW_ROW2 = 1;
    PIN_HW_ROW4 = 1;

    /* Activate Row 1 (SW3, SW6) */
    PIN_HW_ROW1 = 0;
    state = (PIN_HW_COL1 == 0) ? PIN_STATE_LOW : PIN_STATE_HIGH;
    PIN_HW_ROW1 = 1; /* Restore Row 1 to inactive HIGH */

    return state;
}

Pin_State_t GPIO_ReadButton_SW6(void)
{
    Pin_State_t state;
    PIN_HW_ROW2 = 1;
    PIN_HW_ROW4 = 1;

    /* Activate Row 1 (SW3, SW6) */
    PIN_HW_ROW1 = 0;
    state = (PIN_HW_COL4 == 0) ? PIN_STATE_LOW : PIN_STATE_HIGH;
    PIN_HW_ROW1 = 1; /* Restore Row 1 */

    return state;
}

Pin_State_t GPIO_ReadButton_SW10(void)
{
    Pin_State_t state;
    PIN_HW_ROW1 = 1;
    PIN_HW_ROW4 = 1;

    /* Activate Row 2 (SW10) */
    PIN_HW_ROW2 = 0;
    state = (PIN_HW_COL4 == 0) ? PIN_STATE_LOW : PIN_STATE_HIGH;
    PIN_HW_ROW2 = 1; /* Restore Row 2 */

    return state;
}

Pin_State_t GPIO_ReadButton_SW16(void)
{
    Pin_State_t state;
    PIN_HW_ROW1 = 1;
    PIN_HW_ROW2 = 1;

    /* Activate Row 4 (SW16) */
    PIN_HW_ROW4 = 0;
    state = (PIN_HW_COL1 == 0) ? PIN_STATE_LOW : PIN_STATE_HIGH;
    PIN_HW_ROW4 = 1; /* Restore Row 4 */

    return state;
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
