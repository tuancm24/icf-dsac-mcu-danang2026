#ifndef DISPLAY_H
#define DISPLAY_H

#include "app_config.h"

/* =========================================================================
 * 4-Digit 7-Segment Display Driver Interface (SONiX SN8F5708 EVK)
 * ========================================================================= */

typedef enum
{
    DISPLAY_BLINK_NONE = 0,    /* Steady display without blinking */
    DISPLAY_BLINK_HOURS,       /* Blink 2 digits HH (0.5s ON - 0.5s OFF), MM visible */
    DISPLAY_BLINK_MINUTES,     /* Blink 2 digits MM (0.5s ON - 0.5s OFF), HH visible */
    DISPLAY_BLINK_ALL          /* Blink entire display */
} Display_BlinkMode_t;

/* Initialize 7-segment GPIO pins and digit control */
void Display_Init(void);

/* Update the time to be displayed (HH.MM format) */
void Display_SetTime(unsigned char hour, unsigned char minute);

/* Set decimal point / colon visibility: 1 = ON, 0 = OFF */
void Display_SetColon(unsigned char enable);

/* Set blinking mode for setup states */
void Display_SetBlinkMode(Display_BlinkMode_t mode);

/* Update blinking timer phase (called every 1ms/10ms system tick) */
void Display_UpdateBlinkState(void);

/* Scan one digit of the multiplexed display (called in 1ms/2ms Timer ISR) */
void Display_ScanRoutine(void);

/* Force display blank/clear */
void Display_Clear(void);

#endif /* DISPLAY_H */
