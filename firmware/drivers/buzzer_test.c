#include "buzzer.h"
#include "timer.h"
#include "gpio.h"
#include <SN8F5708.H>

/* =========================================================================
 * Buzzer Driver Module Verification & Hardware Bring-Up Test Runner
 * Target MCU: SONiX SN8F5708 EVK
 * Verified Hardware Pins:
 * - Buzzer (PZ1): P1.0 (Active HIGH)
 * - LED D4:       P0.3 (Active LOW)
 * ========================================================================= */

/* Global watch variables for Keil Watch Window inspection */
volatile unsigned char test1_init_busy = 0xFF;
volatile unsigned char test2_beep_active_busy = 0;
volatile unsigned char test2_beep_finished_busy = 0xFF;
volatile unsigned char test3_alarm_active_busy = 0;
volatile unsigned char test3_alarm_finished_busy = 0xFF;
volatile unsigned char test4_stop_busy = 0xFF;

void main(void)
{
    unsigned int ms;

    /* 1. Clear Watchdog Timer and initialize hardware platform */
    WDTR = 0x5A;
    GPIO_Init();
    Timer_Init();
    Buzzer_Init();

    /* =====================================================================
     * SECTION A: Software Simulation Unit Verification
     * ===================================================================== */
    /* TEST 1: Buzzer Initialization -> Expected: 0 */
    test1_init_busy = Buzzer_IsBusy();

    /* TEST 2: Short Beep (0.3s = 300ms) on Keypress / Timeout */
    Buzzer_BeepShort();
    test2_beep_active_busy = Buzzer_IsBusy(); /* Expected: 1 */

    for (ms = 0; ms < 305; ms++)
    {
        WDTR = 0x5A;
        Timer_ISR_Handler();
        Buzzer_Process();
    }
    test2_beep_finished_busy = Buzzer_IsBusy(); /* Expected: 0 */

    /* TEST 3: 5-Second Alarm Sequence (0.5s ON - 0.5s OFF Pattern) */
    Buzzer_StartAlarm();
    test3_alarm_active_busy = Buzzer_IsBusy(); /* Expected: 1 */

    for (ms = 0; ms < 5005; ms++)
    {
        WDTR = 0x5A;
        Timer_ISR_Handler();
        Buzzer_Process();
    }
    test3_alarm_finished_busy = Buzzer_IsBusy(); /* Expected: 0 */

    /* TEST 4: Immediate Stop Control (Buzzer_Stop) */
    Buzzer_StartAlarm();
    Buzzer_Stop();
    test4_stop_busy = Buzzer_IsBusy(); /* Expected: 0 */

    /* =====================================================================
     * SECTION B: Hardware Board Bring-Up Execution Loop
     * Continuous 1-Second Cycle: Beep 0.3s + Blink LED D4
     * ===================================================================== */
    while (1)
    {
        unsigned long start_tick;

        WDTR = 0x5A; /* Clear Watchdog timer */

        /* Trigger non-blocking 0.3s beep and turn on LED D4 */
        Buzzer_BeepShort();
        GPIO_SetLED_D4(PIN_STATE_HIGH);

        start_tick = Timer_GetTickMs();
        while (!Timer_HasElapsed(start_tick, 1000))
        {
            WDTR = 0x5A;
            Buzzer_Process();

            /* Turn off LED D4 after 300ms */
            if (Timer_HasElapsed(start_tick, 300))
            {
                GPIO_SetLED_D4(PIN_STATE_LOW);
            }
        }
    }
}
