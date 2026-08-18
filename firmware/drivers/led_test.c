#include "led.h"
#include "timer.h"
#include "gpio.h"
#include "board.h"
#include <SN8F5708.H>

/* =========================================================================
 * Status LED (LED D4) Verification & Hardware Board Test Runner
 * Target MCU: SONiX SN8F5708 EVK
 * Verified Pin: P0.3 (Active LOW: 0 = ON, 1 = OFF)
 * ========================================================================= */

volatile unsigned char test1_init_off_pass = 0;
volatile unsigned char test2_mode_on_pass = 0;
volatile unsigned char test3_mode_off_pass = 0;
volatile unsigned char test4_blink_500ms_phases_pass = 0;
volatile unsigned char test5_idempotent_continuity_pass = 0;

static void advance_ms(unsigned int ms)
{
    unsigned int k;
    for (k = 0; k < ms; k++)
    {
        Timer_ISR_Handler();
        LED_Process();
    }
}

void main(void)
{
    /* 1. Initialize Hardware Platform */
    Board_FeedWatchdog();
    GPIO_Init();
    Timer_Init();
    LED_Init();

    /* =====================================================================
     * SECTION A: Software Simulation Unit Verification
     * ===================================================================== */
    /* TEST 1: Initial state -> OFF (P0.3 == 1) */
    test1_init_off_pass = (P0^3 == 1) ? 1 : 0;

    /* TEST 2: Static ON Mode -> (P0.3 == 0) */
    LED_SetMode(LED_MODE_ON);
    LED_Process();
    test2_mode_on_pass = (P0^3 == 0) ? 1 : 0;

    /* TEST 3: Static OFF Mode -> (P0.3 == 1) */
    LED_SetMode(LED_MODE_OFF);
    LED_Process();
    test3_mode_off_pass = (P0^3 == 1) ? 1 : 0;

    /* TEST 4: Blink Mode (0.5s ON / 0.5s OFF Phase Transitions) */
    LED_Init();
    LED_SetMode(LED_MODE_BLINK_ALARM_SETTING);
    advance_ms(250); /* 250ms: ON phase */
    if (P0^3 == 0)
    {
        advance_ms(300); /* 550ms: OFF phase */
        if (P0^3 == 1)
        {
            advance_ms(500); /* 1050ms: ON phase */
            if (P0^3 == 0)
            {
                test4_blink_500ms_phases_pass = 1;
            }
        }
    }

    /* TEST 5: Idempotence Continuity (IMP-LED-01) */
    /* Repeated LED_SetMode(same_mode) must NOT reset the 500ms phase timer */
    LED_Init();
    LED_SetMode(LED_MODE_BLINK_ALARM_SETTING);
    advance_ms(400); /* 400ms: still in ON phase */
    if (P0^3 == 0)
    {
        LED_SetMode(LED_MODE_BLINK_ALARM_SETTING); /* Redundant call at 400ms */
        advance_ms(150); /* Total 550ms: Must be in OFF phase if timer was not reset */
        if (P0^3 == 1)
        {
            test5_idempotent_continuity_pass = 1;
        }
    }

    /* =====================================================================
     * SECTION B: Hardware Board Live Blink Execution Loop
     * ===================================================================== */
    LED_Init();
    LED_SetMode(LED_MODE_BLINK_ALARM_SETTING);

    while (1)
    {
        Board_FeedWatchdog();
        LED_Process();
    }
}
