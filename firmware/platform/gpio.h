#ifndef GPIO_H
#define GPIO_H

#include "pin_config.h"

/* =========================================================================
 * Platform GPIO Interface (SONiX SN8F5708)
 * ========================================================================= */

typedef enum
{
    PIN_MODE_INPUT_PULLUP = 0,
    PIN_MODE_INPUT_FLOATING,
    PIN_MODE_OUTPUT_PUSHPULL,
    PIN_MODE_OUTPUT_OPENDRAIN
} Pin_Mode_t;

typedef enum
{
    PIN_STATE_LOW = 0,
    PIN_STATE_HIGH = 1
} Pin_State_t;

/* Initialize all board GPIO pins */
void GPIO_Init(void);

/* Pin control helpers */
void GPIO_SetBuzzer(Pin_State_t state);
void GPIO_SetLED_D4(Pin_State_t state);

/* 7-Segment segment bus control (Segments A..G, DP) */
void GPIO_SetDisplaySegments(unsigned char segment_bitmap);

/* 7-Segment digit select control (digit_index: 0=H1, 1=H0, 2=M1, 3=M0, 0xFF=All Off) */
void GPIO_SelectDisplayDigit(unsigned char digit_index);

/* Button input pin readers */
Pin_State_t GPIO_ReadButton_SW3(void);
Pin_State_t GPIO_ReadButton_SW6(void);
Pin_State_t GPIO_ReadButton_SW10(void);
Pin_State_t GPIO_ReadButton_SW16(void);

/* I2C pin control (used by I2C bit-banging) */
void GPIO_SetI2C_SCL(Pin_State_t state);
void GPIO_SetI2C_SDA(Pin_State_t state);
Pin_State_t GPIO_ReadI2C_SDA(void);
void GPIO_SetI2C_SDA_Mode(Pin_Mode_t mode);

#endif /* GPIO_H */
