#include "display.h"
#include "timer.h"
#include "gpio.h"
#include <SN8F5708.H>

/* =========================================================================
 * 4-Digit 7-Segment Display Driver Verification Test Runner
 * Target MCU: SONiX SN8F5708 EVK
 * Verification Environment: Keil C51 Simulator
 * ========================================================================= */

volatile unsigned char test1_init_done = 0;
volatile unsigned char test2_set_time_done = 0;
volatile unsigned char test3_blink_hours_done = 0;
volatile unsigned char test4_blink_minutes_done = 0;

void main(void)
{
    /* 1. Initialize hardware */
    WDTR = 0x5A;
    GPIO_Init();
    Timer_Init();
    Display_Init();

    /* =====================================================================
     * TEST 1: Display Initialization
     * ===================================================================== */
    test1_init_done = 1;

    /* =====================================================================
     * TEST 2: Set Normal Time (12:34)
     * ===================================================================== */
    Display_SetTime(12, 34);
    Display_SetColon(1);
    test2_set_time_done = 1;

    /* Simulate 4-digit multiplexing scan */
    Display_ScanRoutine(); /* Digit 0 */
    Display_ScanRoutine(); /* Digit 1 */
    Display_ScanRoutine(); /* Digit 2 */
    Display_ScanRoutine(); /* Digit 3 */

    /* =====================================================================
     * TEST 3: Hours Blinking Mode (DISPLAY_BLINK_HOURS)
     * ===================================================================== */
    Display_SetBlinkMode(DISPLAY_BLINK_HOURS);
    Display_UpdateBlinkState();
    test3_blink_hours_done = 1;

    /* =====================================================================
     * TEST 4: Minutes Blinking Mode (DISPLAY_BLINK_MINUTES)
     * ===================================================================== */
    Display_SetBlinkMode(DISPLAY_BLINK_MINUTES);
    Display_UpdateBlinkState();
    test4_blink_minutes_done = 1;

    /* End of Test - Infinite loop for debugger inspection */
    while (1)
    {
        WDTR = 0x5A;
        /* Place breakpoint here (F9) */
    }
}
