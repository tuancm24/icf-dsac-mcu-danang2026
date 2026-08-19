#ifndef GPIO_H
#define GPIO_H

#include "pin_config.h"

/* =========================================================================
 * Platform GPIO Interface (SONiX SN8F5708 EVK)
 * ========================================================================= */

typedef enum
{
    PIN_STATE_LOW = 0,
    PIN_STATE_HIGH = 1
} Pin_State_t;

typedef enum
{
    PIN_MODE_INPUT_PULLUP = 0,
    PIN_MODE_INPUT_FLOATING,
    PIN_MODE_OUTPUT_PP,
    PIN_MODE_OUTPUT_OD
} Pin_Mode_t;

/* Initialize GPIO ports for all peripherals */
void GPIO_Init(void);

/* Status LED D4 control */
void GPIO_SetLED_D4(Pin_State_t state);

/* Buzzer PZ1 control */
void GPIO_SetBuzzer(Pin_State_t state);

/* 7-Segment Display control */
void GPIO_SetDisplaySegments(unsigned char segment_bitmap);
void GPIO_SelectDisplayDigit(unsigned char digit_index);

/* Button Matrix Reading (Strict Column-Isolated Scanning) */
Pin_State_t GPIO_ReadButton_SW3(void);
Pin_State_t GPIO_ReadButton_SW6(void);
Pin_State_t GPIO_ReadButton_SW10(void);
Pin_State_t GPIO_ReadButton_SW16(void);

#ifdef TEST_BUILD
/* Simulation / Unit Testing Button Injection */
void GPIO_SimulateButtonPress(unsigned char button_id, unsigned char is_pressed);
#endif

/* I2C EEPROM (24C05) Bit-Banging Bus Control */
void GPIO_SetI2C_SCL(Pin_State_t state);
void GPIO_SetI2C_SDA(Pin_State_t state);
Pin_State_t GPIO_ReadI2C_SDA(void);
void GPIO_SetI2C_SDA_Mode(Pin_Mode_t mode);

#endif /* GPIO_H */
