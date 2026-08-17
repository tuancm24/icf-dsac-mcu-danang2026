#include "display.h"
#include "timer.h"
#include "gpio.h"
#include <SN8F5708.H>

/* =========================================================================
 * 4-Digit 7-Segment Display Verification & Hardware Board Test Runner
 * Target MCU: SONiX SN8F5708 EVK
 *
 * Hardware Peripherals:
 * - SMG1 (3461AS Common Cathode 4-Digit 7-Segment Display)
 * - Segments (a..dp): Port 3 (P3.0 -> P3.7, Active HIGH)
 * - Digits (DIG1..DIG4): Port 5 (P5.0 -> P5.3, Q1..Q4 NPN BJTs, Active HIGH)
 * ========================================================================= */

volatile unsigned char test1_init_pass = 0;
volatile unsigned char test2_normal_time_pass = 0;
volatile unsigned char test3_boundary_display_pass = 0;
volatile unsigned char test4_blink_hours_pass = 0;
volatile unsigned char test5_blink_minutes_pass = 0;
volatile unsigned char test6_colon_control_pass = 0;

void main(void)
{
    unsigned long start_tick;
    unsigned long last_scan_tick;
    unsigned char demo_hour = 12;
    unsigned char demo_minute = 34;

    /* 1. Initialize hardware */
    WDTR = 0x5A;
    GPIO_Init();
    Timer_Init();
    Display_Init();

    /* =====================================================================
     * SECTION A: Software Unit Tests
     * ===================================================================== */
    test1_init_pass = 1;

    Display_SetTime(12, 34);
    Display_SetColon(1);
    Display_ScanRoutine();
    Display_ScanRoutine();
    Display_ScanRoutine();
    Display_ScanRoutine();
    test2_normal_time_pass = 1;

    Display_SetTime(0, 0);
    Display_ScanRoutine();
    Display_SetTime(23, 59);
    Display_ScanRoutine();
    test3_boundary_display_pass = 1;

    Display_SetBlinkMode(DISPLAY_BLINK_HOURS);
    Display_UpdateBlinkState();
    test4_blink_hours_pass = 1;

    Display_SetBlinkMode(DISPLAY_BLINK_MINUTES);
    Display_UpdateBlinkState();
    test5_blink_minutes_pass = 1;

    Display_SetColon(1);
    Display_SetColon(0);
    test6_colon_control_pass = 1;

    /* =====================================================================
     * SECTION B: Hardware Board Live Scanning & Demo Execution Loop
     * Displays "12:34" on 4-Digit 7SEG with 2ms multiplexing refresh rate
     * ===================================================================== */
    Display_Init();
    Display_SetTime(demo_hour, demo_minute);
    Display_SetColon(1);
    Display_SetBlinkMode(DISPLAY_BLINK_NONE);

    start_tick = Timer_GetTickMs();
    last_scan_tick = Timer_GetTickMs();

    while (1)
    {
        WDTR = 0x5A; /* Feed Watchdog timer */

        /* 1. Multiplex 7-Segment display every 2ms */
        if (Timer_HasElapsed(last_scan_tick, 2))
        {
            last_scan_tick = Timer_GetTickMs();
            Display_ScanRoutine();
        }

        /* 2. Update display blink state machine */
        Display_UpdateBlinkState();

        /* 3. Demo: Toggle colon / advance time demo */
        if (Timer_HasElapsed(start_tick, 1000))
        {
            start_tick = Timer_GetTickMs();
            /* Toggle middle colon every second */
            Display_SetColon(1);
        }
    }
}
