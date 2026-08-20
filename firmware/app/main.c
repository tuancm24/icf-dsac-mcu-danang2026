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
    unsigned char xdata saved_h = 0;
    unsigned char xdata saved_m = 0;
    unsigned char xdata alarm_valid;
    unsigned char xdata alarm_match_latched;
    unsigned long xdata last_clock_tick = 0;
    Button_Event_t xdata btn_event;
    Time_t xdata display_time;

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
        unsigned long xdata now = Timer_GetTickMs();


        /* Step A: Advance all due logical Clock seconds. */
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

        /*
         * Stage G: Publish logical Display output from the current/resulting
         * FSM state. Display scan/blink timing remains Driver/Platform-owned.
         */
        Display_SetColon(1);

        switch (FSM_GetState())
        {
            case FSM_NORMAL:
                display_time = Clock_GetTime();
                Display_SetTime(display_time.hour, display_time.minute);
                Display_SetBlinkMode(DISPLAY_BLINK_NONE);
                break;

            case FSM_SET_TIME_HOUR:
                display_time = Clock_GetTime();
                Display_SetTime(display_time.hour, display_time.minute);
                Display_SetBlinkMode(DISPLAY_BLINK_HOURS);
                break;

            case FSM_SET_TIME_MINUTE:
                display_time = Clock_GetTime();
                Display_SetTime(display_time.hour, display_time.minute);
                Display_SetBlinkMode(DISPLAY_BLINK_MINUTES);
                break;

            default:
                /*
                 * SET_ALARM_* Display source belongs to INT06.
                 * Do not invent alarm-edit data during INT03.
                 */
                break;
        }

        /* Stage H: final foreground action. */
        Board_FeedWatchdog();
    }
}
