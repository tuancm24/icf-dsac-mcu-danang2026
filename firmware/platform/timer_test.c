#include "timer.h"
#include "gpio.h"
#include <SN8F5708.H>

/* =========================================================================
 * Module 1: Platform Timer Module Verification Test Runner
 * Target MCU: SONiX SN8F5708 EVK (5708_EVK-V1.0)
 *
 * Capabilities:
 * 1. Software Simulation Unit Tests (Watch Window inspection)
 * 2. Real Hardware Board Verification (1Hz LED D4 Blink & 1-Second Timer Ticking)
 * ========================================================================= */

/* Global watch variables for Keil Watch Window inspection */
volatile unsigned long test1_init_tick = 0xFFFFFFFF;
volatile unsigned long test2_tick_after_1000ms = 0;
volatile unsigned char test3_elapsed_true = 0;
volatile unsigned char test3_elapsed_false = 1;

void main(void)
{
    unsigned int i;
    unsigned long start_tick = 0;
    unsigned char led_state = 0;

    /* 1. Clear Watchdog and Initialize Hardware Peripherals */
    WDTR = 0x5A;
    GPIO_Init();
    Timer_Init(); /* Start 1ms hardware Timer 0 interrupt */

    /* =====================================================================
     * SECTION A: Software Simulation Unit Verification
     * ===================================================================== */
    /* TEST 1: Timer Initialization -> Expected: 0 */
    test1_init_tick = Timer_GetTickMs();

    /* TEST 2: Accumulate 1000 ISR ticks -> Expected: 1000ms */
    for (i = 0; i < 1000; i++)
    {
        WDTR = 0x5A;
        Timer_ISR_Handler();
    }
    test2_tick_after_1000ms = Timer_GetTickMs();

    /* TEST 3: Elapsed Time Verification */
    test3_elapsed_true  = Timer_HasElapsed(0, 500);  /* Expected: 1 */
    test3_elapsed_false = Timer_HasElapsed(0, 1500); /* Expected: 0 */

    /* TEST 4: Microsecond Delay */
    Timer_DelayUs(10);

    /* =====================================================================
     * SECTION B: Real Hardware Board Verification Loop
     * Continuously toggles LED D4 (P0.3) every 500ms (1-Second cycle)
     * using the 1ms Hardware Timer 0 Interrupt and Timer_HasElapsed().
     * ===================================================================== */
    start_tick = Timer_GetTickMs();

    while (1)
    {
        WDTR = 0x5A; /* Feed Watchdog timer */

        /* Toggle LED D4 every 500ms measured by hardware timer */
        if (Timer_HasElapsed(start_tick, 500))
        {
            start_tick = Timer_GetTickMs();
            led_state = !led_state;

            if (led_state)
            {
                GPIO_SetLED_D4(PIN_STATE_HIGH); /* LED D4 ON (P0.3 = 0) */
                GPIO_SetBuzzer(PIN_STATE_HIGH); /* Short 30ms tick sound */
            }
            else
            {
                GPIO_SetLED_D4(PIN_STATE_LOW);  /* LED D4 OFF (P0.3 = 1) */
                GPIO_SetBuzzer(PIN_STATE_LOW);
            }
        }

        /* Turn off buzzer tick sound after 30ms */
        if (Timer_HasElapsed(start_tick, 30))
        {
            GPIO_SetBuzzer(PIN_STATE_LOW);
        }
    }
}
