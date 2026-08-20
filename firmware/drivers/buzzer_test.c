#include "buzzer.h"
#include "timer.h"
#include "gpio.h"
#include <SN8F5708.H>

/* =========================================================================
 * Buzzer & LED D4 Module Hardware Verification & Test Runner
 * Target MCU: SONiX SN8F5708 EVK
 * ========================================================================= */

volatile unsigned char test1_init_busy = 0xFF;
volatile unsigned char test2_short_beep_pass = 0;
volatile unsigned char test3_alarm_5s_phases_pass = 0;
volatile unsigned char test4_short_refresh_pass = 0;
volatile unsigned char test5_alarm_preempt_short_pass = 0;
volatile unsigned char test6_alarm_protect_short_pass = 0;
volatile unsigned char test7_repeated_alarm_noop_pass = 0;

static void sim_advance_ms(unsigned int count)
{
    unsigned int k;
    for (k = 0; k < count; k++)
    {
        Timer_ISR_Handler();
        Buzzer_Process();
    }
}

void main(void)
{
    unsigned long start_tick;

    /* 1. Initialize Hardware Platform */
    WDTR = 0x5A;
    GPIO_Init();
    Timer_Init();
    Buzzer_Init();

    /* =====================================================================
     * SECTION A: Software Simulation Unit Verification
     * ===================================================================== */
    /* TEST 1: Initial state */
    test1_init_busy = (Buzzer_IsBusy() == 0) ? 0 : 1;

    /* TEST 2: Short Beep (300ms) */
    Buzzer_Init();
    Buzzer_BeepShort();
    if (Buzzer_IsBusy() == 1)
    {
        sim_advance_ms(299);
        if (Buzzer_IsBusy() == 1)
        {
            sim_advance_ms(2);
            test2_short_beep_pass = (Buzzer_IsBusy() == 0) ? 1 : 0;
        }
    }

    /* TEST 3: 5-Second Alarm Sequence (500ms ON / 500ms OFF phase transitions) */
    Buzzer_Init();
    Buzzer_StartAlarm();
    test3_alarm_5s_phases_pass = 1;
    if (Buzzer_IsBusy() != 1) test3_alarm_5s_phases_pass = 0;

    sim_advance_ms(250);  /* 250ms: In ON phase */
    if (P1^0 != 1) test3_alarm_5s_phases_pass = 0;

    sim_advance_ms(300);  /* 550ms: In OFF phase */
    if (P1^0 != 0) test3_alarm_5s_phases_pass = 0;

    sim_advance_ms(500);  /* 1050ms: In ON phase */
    if (P1^0 != 1) test3_alarm_5s_phases_pass = 0;

    sim_advance_ms(3950); /* Total 5000ms reached */
    if (Buzzer_IsBusy() != 0) test3_alarm_5s_phases_pass = 0;

    /* TEST 4: Rapid Short Beep Refresh (DEC-14) */
    Buzzer_Init();
    Buzzer_BeepShort();
    sim_advance_ms(200); /* 200ms elapsed */
    Buzzer_BeepShort();  /* Refresh timer at 200ms -> should stay busy for 300ms more */
    sim_advance_ms(200);
    if (Buzzer_IsBusy() == 1)
    {
        sim_advance_ms(105);
        test4_short_refresh_pass = (Buzzer_IsBusy() == 0) ? 1 : 0;
    }

    /* TEST 5: Alarm Start Preempts Short Beep Immediately */
    Buzzer_Init();
    Buzzer_BeepShort();
    sim_advance_ms(100);
    Buzzer_StartAlarm(); /* Preempts short beep */
    sim_advance_ms(200);
    test5_alarm_preempt_short_pass = (Buzzer_IsBusy() == 1) ? 1 : 0;

    /* TEST 6: Key Beep during Active Alarm does NOT cancel/shorten Alarm */
    Buzzer_Init();
    Buzzer_StartAlarm();
    sim_advance_ms(1000); /* 1s into alarm */
    Buzzer_BeepShort();   /* Keypress beep requested */
    sim_advance_ms(2000); /* 3s into alarm -> still busy */
    if (Buzzer_IsBusy() == 1)
    {
        sim_advance_ms(2005); /* 5005ms -> finishes naturally */
        test6_alarm_protect_short_pass = (Buzzer_IsBusy() == 0) ? 1 : 0;
    }

    /* TEST 7: Repeated Alarm Start during Active Alarm is a No-Op (DEC-15) */
    Buzzer_Init();
    Buzzer_StartAlarm();
    sim_advance_ms(2000); /* 2s into alarm */
    Buzzer_StartAlarm();  /* Redundant request */
    sim_advance_ms(3005); /* Total 5005ms from first start -> must finish, not extended */
    test7_repeated_alarm_noop_pass = (Buzzer_IsBusy() == 0) ? 1 : 0;

    /* =====================================================================
     * SECTION B: Hardware Board Real-Time Verification Loop
     * Demonstrates:
     * 1. Short keypress beep (0.3s) + LED D4 pulse
     * 2. 2-second pause
     * 3. 5-second continuous alarm pattern (500ms ON / 500ms OFF) + LED D4 blink
     * 4. 3-second silence before repeat
     * ===================================================================== */
    Buzzer_Init();

    while (1)
    {
        WDTR = 0x5A;

        /* Phase 1: Short Beep (0.3s = 300ms) */
        Buzzer_BeepShort();
        GPIO_SetLED_D4(PIN_STATE_HIGH);

        start_tick = Timer_GetTickMs();
        while (!Timer_HasElapsed(start_tick, 2000UL))
        {
            WDTR = 0x5A;
            Buzzer_Process();

            if (Timer_HasElapsed(start_tick, 300UL))
            {
                GPIO_SetLED_D4(PIN_STATE_LOW);
            }
        }

        /* Phase 2: 5-Second Alarm (500ms ON / 500ms OFF) */
        Buzzer_StartAlarm();
        start_tick = Timer_GetTickMs();

        while (!Timer_HasElapsed(start_tick, 8000UL)) /* 5s alarm + 3s pause */
        {
            unsigned long el;
            WDTR = 0x5A;
            Buzzer_Process();

            el = Timer_GetTickMs() - start_tick;
            if (el < 5000UL)
            {
                /* LED D4 nhấp nháy 1s (0.5s ON / 0.5s OFF) đồng bộ chu kỳ báo thức */
                unsigned int el_ms = (unsigned int)el;
                unsigned char led_phase = 1;
                while (el_ms >= 500U)
                {
                    led_phase ^= 1;
                    el_ms -= 500U;
                }
                GPIO_SetLED_D4(led_phase ? PIN_STATE_HIGH : PIN_STATE_LOW);
            }
            else
            {
                GPIO_SetLED_D4(PIN_STATE_LOW);
            }
        }
    }
}
