#ifndef BUTTON_H
#define BUTTON_H

#include "app_config.h"

/* =========================================================================
 * Button Driver Interface (SONiX SN8F5708 EVK)
 * ========================================================================= */

typedef enum
{
    BUTTON_ID_SW3 = 0,
    BUTTON_ID_SW6,
    BUTTON_ID_SW10,
    BUTTON_ID_SW16,
    BUTTON_ID_COUNT
} Button_Id_t;

typedef enum
{
    BUTTON_EVENT_NONE = 0,
    BUTTON_EVENT_SW3_CLICK,
    BUTTON_EVENT_SW6_CLICK,
    BUTTON_EVENT_SW10_CLICK,
    BUTTON_EVENT_SW16_CLICK
} Button_Event_t;

/* Initialize button GPIO pins with pull-ups */
void Button_Init(void);

/* Scan and debounce buttons (called periodically in 1ms/10ms system tick) */
void Button_Process(void);

/* Get the latest button click event (FIFO / Single-event queue) */
Button_Event_t Button_GetEvent(void);

/* Read raw button state: 1 = currently pressed, 0 = released */
unsigned char Button_IsPressed(Button_Id_t button_id);

#endif /* BUTTON_H */
