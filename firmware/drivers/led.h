#ifndef LED_H
#define LED_H

#include "app_config.h"

/* =========================================================================
 * Status LED Driver Interface (LED D4 on SONiX SN8F5708 EVK)
 * ========================================================================= */

typedef enum
{
    LED_MODE_OFF = 0,               /* LED permanently OFF (Normal / Time setup) */
    LED_MODE_ON,                    /* LED permanently ON */
    LED_MODE_BLINK_ALARM_SETTING    /* Blink 0.5s ON - 0.5s OFF during Alarm setup */
} LED_Mode_t;

/* Initialize LED GPIO pin */
void LED_Init(void);

/* Set operating mode of status LED D4 */
void LED_SetMode(LED_Mode_t mode);

/* Update LED blinking state machine (called in 1ms periodic system tick) */
void LED_Process(void);

#endif /* LED_H */
