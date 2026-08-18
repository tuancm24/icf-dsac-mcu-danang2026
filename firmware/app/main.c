#include "board.h"
#include "button.h"
#include "display.h"
#include "buzzer.h"
#include "led.h"
#include "eeprom.h"
#include "clock.h"
#include "fsm.h"
#include "alarm.h"
#include "app_config.h"

/* =========================================================================
 * Production System Main Entry Point (Contract v2.7 Compliant)
 * Target MCU: SONiX SN8F5708 EVK
 * ========================================================================= */

void main(void)
{
    unsigned char saved_h = 0;
    unsigned char saved_m = 0;
    unsigned long last_clock_tick = 0;
    unsigned long last_display_scan_tick = 0;
    Button_Event_t btn_event;

    /* 1. Master Platform Initialization */
    Board_Init();

    /* 2. Driver Logical Initializations */
    Button_Init();
    Display_Init();
    Buzzer_Init();
    LED_Init();

    /* 3. Core Initializations */
    Clock_Init();
    FSM_Init();
    Alarm_Init();

    /* 4. Restore Persisted Alarm from 24C05 EEPROM (DEC-10) */
    if (EEPROM_ReadAlarm(&saved_h, &saved_m))
    {
        Alarm_SetTime(saved_h, saved_m);
    }

    /* 5. Initial Display Output */
    Display_SetTime(0, 0);
    Display_SetColon(1);
    Display_SetBlinkMode(DISPLAY_BLINK_NONE);
    LED_SetMode(LED_MODE_OFF);

    last_clock_tick = Timer_GetTickMs();
    last_display_scan_tick = Timer_GetTickMs();

    /* =====================================================================
     * 6. Cooperative Foreground Execution Loop (Section 7 Processing Order)
     * ===================================================================== */
    while (1)
    {
        unsigned long now = Timer_GetTickMs();

        /* Feed Watchdog (Platform wrapper) */
        Board_FeedWatchdog();

        /* Step A: Advance due Clock seconds */
        while ((unsigned long)(now - last_clock_tick) >= 1000UL)
        {
            last_clock_tick += 1000UL;
            Clock_Tick1Second();
        }

        /* Step B: Run Driver Foreground Process Routines */
        Button_Process();
        Buzzer_Process();
        LED_Process();
        Display_UpdateBlinkState();

        /* Step C: Multiplex 7-Segment Display (every 2ms) */
        if (Timer_HasElapsed(last_display_scan_tick, DISPLAY_SCAN_DIGIT_INTERVAL_MS))
        {
            last_display_scan_tick = now;
            Display_ScanRoutine();
        }

        /* Step D: Drain Button Events */
        btn_event = Button_GetEvent();
        if (btn_event != BUTTON_EVENT_NONE)
        {
            Buzzer_BeepShort();
        }
    }
}
