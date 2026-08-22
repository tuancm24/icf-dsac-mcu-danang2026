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
 * Target MCU: SONiX SN8F5708 EVK (INT11 & INT12 - Natural Alarm & 5s Buzzer)
 * ========================================================================= */

void main(void)
{
    unsigned char xdata saved_h = 0;
    unsigned char xdata saved_m = 0;
    unsigned char xdata alarm_valid;
    unsigned char xdata alarm_match_latched;
    unsigned long xdata last_clock_tick = 0;
    unsigned long xdata last_activity_tick = 0;
    unsigned char xdata inactivity_active = 0;
    Button_Event_t xdata btn_event;
    FSM_Event xdata fsm_event;
    FSM_State xdata state_before;
    FSM_State xdata state_after;
    Time_t xdata alarm_edit_time;
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

    /* 4. Restore Persisted Alarm from 24C05 EEPROM (DEC-10 / INT10) */
    if (EEPROM_ReadAlarm(&saved_h, &saved_m))
    {
        Alarm_SetTime(saved_h, saved_m);
        alarm_valid = 1;

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

        /*
         * Step A (INT02/11/12): Advance Clock and evaluate natural alarm occurrence.
         * Clock advance is paused while current-time editing is active (INT05).
         */
        if ((FSM_GetState() != FSM_SET_TIME_HOUR) &&
            (FSM_GetState() != FSM_SET_TIME_MINUTE))
        {
            while ((unsigned long)(now - last_clock_tick) >= 1000UL)
            {
                last_clock_tick += 1000UL;
                Clock_Tick1Second();

                /*
                 * INT11 & INT12: Natural Alarm Occurrence & One-Shot Latch.
                 * Evaluated strictly in the logical Clock-tick context.
                 */
                if (alarm_valid && Alarm_Check(Clock_GetTime()))
                {
                    if (!alarm_match_latched)
                    {
                        Buzzer_StartAlarm();
                        alarm_match_latched = 1;
                    }
                }
                else
                {
                    alarm_match_latched = 0; /* Re-arm latch upon minute mismatch */
                }
            }
        }

        /* Step B: Run Driver Foreground Process Routines */
        Button_Process();
        Buzzer_Process();
        LED_Process();

        /* Step C: Drain all accepted Button events in FIFO order (INT04 & INT08). */
        while ((btn_event = Button_GetEvent()) != BUTTON_EVENT_NONE)
        {
            state_before = FSM_GetState();

            switch (btn_event)
            {
                case BUTTON_EVENT_SW3_CLICK:
                    fsm_event = FSM_EVENT_SW3;
                    break;

                case BUTTON_EVENT_SW6_CLICK:
                    fsm_event = FSM_EVENT_SW6;
                    break;

                case BUTTON_EVENT_SW10_CLICK:
                    fsm_event = FSM_EVENT_SW10;
                    break;

                case BUTTON_EVENT_SW16_CLICK:
                    fsm_event = FSM_EVENT_SW16;
                    break;

                default:
                    fsm_event = FSM_EVENT_NONE;
                    break;
            }

            /* INT08 Key Feedback: Every accepted click requests 0.3s beep */
            Buzzer_BeepShort();

            /* Accepted SET click refreshes activity timestamp */
            if (state_before != FSM_NORMAL)
            {
                inactivity_active = 1;
                last_activity_tick = now;
            }

            /* INT06 transactional alarm-edit pre-transition actions. */
            if ((state_before == FSM_NORMAL) &&
                (fsm_event == FSM_EVENT_SW16))
            {
                alarm_edit_time = Alarm_GetTime();
                alarm_edit_time.second = 0;
            }
            else if ((state_before == FSM_SET_ALARM_HOUR) &&
                     (fsm_event == FSM_EVENT_SW6))
            {
                alarm_edit_time.hour++;
                if (alarm_edit_time.hour >= 24)
                {
                    alarm_edit_time.hour = 0;
                }
            }
            else if ((state_before == FSM_SET_ALARM_HOUR) &&
                     (fsm_event == FSM_EVENT_SW10))
            {
                if (alarm_edit_time.hour == 0)
                {
                    alarm_edit_time.hour = 23;
                }
                else
                {
                    alarm_edit_time.hour--;
                }
            }
            else if ((state_before == FSM_SET_ALARM_MINUTE) &&
                     (fsm_event == FSM_EVENT_SW6))
            {
                alarm_edit_time.minute++;
                if (alarm_edit_time.minute >= 60)
                {
                    alarm_edit_time.minute = 0;
                }
            }
            else if ((state_before == FSM_SET_ALARM_MINUTE) &&
                     (fsm_event == FSM_EVENT_SW10))
            {
                if (alarm_edit_time.minute == 0)
                {
                    alarm_edit_time.minute = 59;
                }
                else
                {
                    alarm_edit_time.minute--;
                }
            }
            else if ((state_before == FSM_SET_ALARM_MINUTE) &&
                     (fsm_event == FSM_EVENT_SW16))
            {
                Alarm_SetTime(alarm_edit_time.hour, alarm_edit_time.minute);
                alarm_valid = 1;

                if (Alarm_Check(Clock_GetTime()))
                {
                    alarm_match_latched = 1;
                }
                else
                {
                    alarm_match_latched = 0;
                }

                EEPROM_SaveAlarm(alarm_edit_time.hour, alarm_edit_time.minute);
            }

            if (fsm_event != FSM_EVENT_NONE)
            {
                FSM_HandleEvent(fsm_event);
            }

            state_after = FSM_GetState();

            /* Resulting state owns inactivity-session start/stop. */
            if (state_after == FSM_NORMAL)
            {
                inactivity_active = 0;
            }
            else if (state_before == FSM_NORMAL)
            {
                inactivity_active = 1;
                last_activity_tick = now;
            }

            /* INT05 current-time scheduler lifecycle. */
            if ((state_before == FSM_NORMAL) &&
                (state_after == FSM_SET_TIME_HOUR))
            {
                last_clock_tick = now;
            }
            else if (((state_before == FSM_SET_TIME_HOUR) ||
                      (state_before == FSM_SET_TIME_MINUTE)) &&
                     (state_after == FSM_NORMAL))
            {
                last_clock_tick = now;
            }
        }

        /*
         * Step D (INT09): 30-Second Inactivity Timeout Evaluation.
         * Evaluated strictly after draining all Button FIFO events.
         */
        if (inactivity_active && (FSM_GetState() != FSM_NORMAL))
        {
            if ((unsigned long)(now - last_activity_tick) >= TIMEOUT_INACTIVITY_MS)
            {
                state_before = FSM_GetState();

                /* Audible timeout feedback pip 0.3s (DEC-16 / INT08 closure) */
                Buzzer_BeepShort();

                /* Dispatch synthetic timeout event to FSM */
                FSM_HandleEvent(FSM_EVENT_TIMEOUT_30S);
                state_after = FSM_GetState();

                inactivity_active = 0;

                /*
                 * If timeout occurred from current-time setting, resume
                 * fresh ~1000ms clock anchor (INT05 closure).
                 */
                if ((state_before == FSM_SET_TIME_HOUR) ||
                    (state_before == FSM_SET_TIME_MINUTE))
                {
                    last_clock_tick = now;
                }

                /*
                 * If timeout occurred from alarm setting, temporary alarm_edit_time
                 * is discarded without committing to Alarm Core or saving to EEPROM.
                 */
            }
        }

        /*
         * Startup/current-occurrence latch bookkeeping:
         * once Clock no longer matches the confirmed alarm,
         * re-arm the latch for a future natural occurrence.
         */
        if (alarm_valid && alarm_match_latched)
        {
            if (!Alarm_Check(Clock_GetTime()))
            {
                alarm_match_latched = 0;
            }
        }

        /*
         * Stage G: Publish logical Display and LED output from current/resulting
         * FSM state (INT07). Display scan/blink and LED blink timing remain
         * Driver/Platform-owned.
         */
        Display_SetColon(1);

        switch (FSM_GetState())
        {
            case FSM_NORMAL:
                display_time = Clock_GetTime();
                Display_SetTime(display_time.hour, display_time.minute);
                Display_SetBlinkMode(DISPLAY_BLINK_NONE);
                LED_SetMode(LED_MODE_OFF);
                break;

            case FSM_SET_TIME_HOUR:
                display_time = Clock_GetTime();
                Display_SetTime(display_time.hour, display_time.minute);
                Display_SetBlinkMode(DISPLAY_BLINK_HOURS);
                LED_SetMode(LED_MODE_OFF);
                break;

            case FSM_SET_TIME_MINUTE:
                display_time = Clock_GetTime();
                Display_SetTime(display_time.hour, display_time.minute);
                Display_SetBlinkMode(DISPLAY_BLINK_MINUTES);
                LED_SetMode(LED_MODE_OFF);
                break;

            case FSM_SET_ALARM_HOUR:
                Display_SetTime(alarm_edit_time.hour, alarm_edit_time.minute);
                Display_SetBlinkMode(DISPLAY_BLINK_HOURS);
                LED_SetMode(LED_MODE_BLINK_ALARM_SETTING);
                break;

            case FSM_SET_ALARM_MINUTE:
                Display_SetTime(alarm_edit_time.hour, alarm_edit_time.minute);
                Display_SetBlinkMode(DISPLAY_BLINK_MINUTES);
                LED_SetMode(LED_MODE_BLINK_ALARM_SETTING);
                break;

            default:
                break;
        }

        /* Stage H: final foreground action. */
        Board_FeedWatchdog();
    }
}
