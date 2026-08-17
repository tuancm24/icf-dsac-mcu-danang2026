#ifndef GPIO_H
#define GPIO_H

#include "board_config.h"

/* Pin Logic State Definitions */
typedef enum {
    PIN_STATE_LOW = 0,
    PIN_STATE_HIGH = 1
} Pin_State_t;

/* Pin Direction / Mode Definitions */
typedef enum {
    PIN_MODE_INPUT_FLOATING = 0,
    PIN_MODE_INPUT_PULLUP,
    PIN_MODE_OUTPUT_PP,
    PIN_MODE_OUTPUT_OD
} Pin_Mode_t;

/* Public Platform GPIO API */
void GPIO_Init(void);

/* Status Outputs */
void GPIO_SetBuzzer(Pin_State_t state);
void GPIO_SetLED_D4(Pin_State_t state);

/* 4-Digit 7-Segment Display Physical Lines */
void GPIO_SetDisplaySegments(unsigned char segment_bitmap);
void GPIO_SelectDisplayDigit(unsigned char digit_index);

/* Button Inputs (SW3, SW6, SW10, SW16) */
Pin_State_t GPIO_ReadButton_SW3(void);
Pin_State_t GPIO_ReadButton_SW6(void);
Pin_State_t GPIO_ReadButton_SW10(void);
Pin_State_t GPIO_ReadButton_SW16(void);

/* Simulation Helper */
void GPIO_SimulateButtonPress(unsigned char button_id, unsigned char is_pressed);

/* I2C Physical Lines for 24C08 EEPROM */
void GPIO_SetI2C_SCL(Pin_State_t state);
void GPIO_SetI2C_SDA(Pin_State_t state);
Pin_State_t GPIO_ReadI2C_SDA(void);
void GPIO_SetI2C_SDA_Mode(Pin_Mode_t mode);

#endif /* GPIO_H */
