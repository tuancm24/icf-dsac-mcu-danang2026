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
    unsigned char alarm_valid;
    unsigned char alarm_match_latched;
    unsigned long last_clock_tick = 0;
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

    alarm_valid = 0;
    alarm_match_latched = 0;

    /* 4. Restore Persisted Alarm from 24C05 EEPROM (DEC-10) */
    if (EEPROM_ReadAlarm(&saved_h, &saved_m))
    {
        Alarm_SetTime(saved_h, saved_m);
        alarm_valid = 1;

        /*
         * Startup-match rule:
         * latch an already-existing startup occurrence without
         * starting the alarm buzzer.
         */
        if (Alarm_Check(Clock_GetTime()))
        {
            alarm_match_latched = 1;
        }
        else
        {
            alarm_match_latched = 0;
        }
    }
    else
    {
        /*
         * Invalid / empty / read-error persistence must not arm
         * Alarm Core's default 00:00 value.
         */
        alarm_valid = 0;
        alarm_match_latched = 0;
    }

    /* 5. Initial Display Output */
    Display_SetTime(0, 0);
    Display_SetColon(1);
    Display_SetBlinkMode(DISPLAY_BLINK_NONE);
    LED_SetMode(LED_MODE_OFF);

    last_clock_tick = Timer_GetTickMs();

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

        /* Step C: Drain Button Events */
        btn_event = Button_GetEvent();
        if (btn_event != BUTTON_EVENT_NONE)
        {
            Buzzer_BeepShort();
        }

        /*
         * Startup/current-occurrence latch bookkeeping:
         * once Clock no longer matches the confirmed alarm,
         * re-arm the latch for a future natural occurrence.
         *
         * Actual natural-occurrence triggering belongs to INT-11.
         */
        if (alarm_valid && alarm_match_latched)
        {
            if (!Alarm_Check(Clock_GetTime()))
            {
                alarm_match_latched = 0;
            }
        }
    }
}
