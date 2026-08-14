#include "gpio.h"
#include <SN8F5708.H>

/* =========================================================================
 * Platform GPIO Implementation for SONiX SN8F5708 EVK
 * Verified Pinout from Silkscreen:
 * - LED D4: P0.3 (Active LOW)
 * - Buzzer: P1.0 (Active HIGH)
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

/* Button Matrix Pins */
sbit PIN_HW_ROW1    = P2^4; /* Row 1: SW3, SW6 */
sbit PIN_HW_ROW2    = P2^5; /* Row 2: SW10 */
sbit PIN_HW_ROW4    = P2^7; /* Row 4: SW16 */
sbit PIN_HW_COL1    = P4^4; /* Col 1: SW3, SW16 */
sbit PIN_HW_COL4    = P4^7; /* Col 4: SW6, SW10 */

static unsigned char s_buzzer_state = 0;
static unsigned char s_led_d4_state = 0;
static unsigned char s_display_segments = 0;
static unsigned char s_active_digit = 0xFF;

void GPIO_Init(void)
{
    /*
     * Port Mode Configurations:
     * - P0M: Bit 3 (0x08) = Output for LED D4
     * - P1M: Bit 0 (0x01) = Output for Buzzer, Bits 4,5 (0x30) = Open-Drain for I2C
     * - P2M: Bits 4,5,7 = Row outputs/inputs
     * - P3M: 0xFF = Output for 7-Segment Segments (P3.0 -> P3.7)
     * - P5M: 0x0F = Output for Digits Q1..Q4 (P5.0 -> P5.3)
     */
    P0M |= 0x08; /* P0.3 Output */
    P1M |= 0x31; /* P1.0 Output (Buzzer), P1.4, P1.5 I2C */
    P3M  = 0xFF; /* Port 3 7-Segment Output */
    P5M |= 0x0F; /* Port 5 Digit Select Output */

    /* Set default inactive states */
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

    /* Turn OFF all digits first (Active LOW) */
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

Pin_State_t GPIO_ReadButton_SW3(void)
{
    PIN_HW_ROW1 = 0;
    return (PIN_HW_COL1 == 0) ? PIN_STATE_LOW : PIN_STATE_HIGH;
}

Pin_State_t GPIO_ReadButton_SW6(void)
{
    PIN_HW_ROW1 = 0;
    return (PIN_HW_COL4 == 0) ? PIN_STATE_LOW : PIN_STATE_HIGH;
}

Pin_State_t GPIO_ReadButton_SW10(void)
{
    PIN_HW_ROW2 = 0;
    return (PIN_HW_COL4 == 0) ? PIN_STATE_LOW : PIN_STATE_HIGH;
}

Pin_State_t GPIO_ReadButton_SW16(void)
{
    PIN_HW_ROW4 = 0;
    return (PIN_HW_COL1 == 0) ? PIN_STATE_LOW : PIN_STATE_HIGH;
}

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
    mode = mode;
}
