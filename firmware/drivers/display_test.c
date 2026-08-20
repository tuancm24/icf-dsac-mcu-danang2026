#include "display.h"
#include "timer.h"
#include "gpio.h"
#include <SN8F5708.H>

/* =========================================================================
 * 4-Digit 7-Segment Display Verification & Hardware Board Test Runner
 * Target MCU: SONiX SN8F5708 EVK (Real-Time Hardware Scanning)
 * ========================================================================= */

volatile unsigned char test1_init_pass = 0;
volatile unsigned char test2_normal_time_pass = 0;
volatile unsigned char test3_boundary_display_pass = 0;
volatile unsigned char test4_blink_hours_pass = 0;
volatile unsigned char test5_blink_minutes_pass = 0;
volatile unsigned char test6_colon_control_pass = 0;

static void advance_sim_ms(unsigned int ms)
{
    unsigned int k;
    for (k = 0; k < ms; k++)
    {
        Timer_ISR_Handler();
    }
}

void main(void)
{
    unsigned char t2_pass = 1;
    unsigned long last_scan_tick;
    unsigned long last_mode_tick;
    unsigned char demo_mode = 0;

    /* 1. Initialize hardware platform */
    WDTR = 0x5A;
    GPIO_Init();
    Timer_Init();
    Display_Init();

    /* =====================================================================
     * SECTION A: Software Unit Tests with Real Output Assertions (T-DSP-01)
     * ===================================================================== */
    /* TEST 1: Initial state */
    test1_init_pass = (P3 == 0x00) ? 1 : 0;

    /* TEST 2: Normal Time 12:34 with Colon -> Assert Exact Segment Bitmaps */
    Display_Init();
    Display_SetTime(12, 34);
    Display_SetColon(1);

    Display_ScanRoutine(); /* Digit 0: '1' -> 0x06 */
    if (P3 != 0x06 || P5^0 != 1) t2_pass = 0;

    Display_ScanRoutine(); /* Digit 1: '2' + DP -> 0x5B | 0x80 = 0xDB */
    if (P3 != 0xDB || P5^1 != 1) t2_pass = 0;

    Display_ScanRoutine(); /* Digit 2: '3' -> 0x4F */
    if (P3 != 0x4F || P5^2 != 1) t2_pass = 0;

    Display_ScanRoutine(); /* Digit 3: '4' -> 0x66 */
    if (P3 != 0x66 || P5^3 != 1) t2_pass = 0;

    test2_normal_time_pass = t2_pass;

    /* TEST 3: Boundary Display (00:00 and 23:59) */
    test3_boundary_display_pass = 0;
    Display_SetTime(0, 0);
    Display_ScanRoutine(); /* Digit 0: '0' -> 0x3F */
    if (P3 == 0x3F)
    {
        Display_SetTime(23, 59);
        Display_ScanRoutine(); /* Digit 1: '3' + DP -> 0xCF */
        if (P3 == 0xCF)
        {
            test3_boundary_display_pass = 1;
        }
    }

    /* TEST 4: Blink Hours (HH blank, MM visible during OFF phase) */
    Display_Init();
    Display_SetTime(12, 34);
    Display_SetBlinkMode(DISPLAY_BLINK_HOURS);
    advance_sim_ms(500); /* Advance into OFF phase (500ms) */
    Display_UpdateBlinkState();

    Display_ScanRoutine(); /* Digit 0 (H1) -> Must be blank 0x00 */
    if (P3 == 0x00)
    {
        Display_ScanRoutine(); /* Digit 1 (H0) -> Must be blank 0x00 */
        if (P3 == 0x00)
        {
            Display_ScanRoutine(); /* Digit 2 (M1) -> Must be visible 0x4F */
            if (P3 == 0x4F)
            {
                test4_blink_hours_pass = 1;
            }
        }
    }

    /* TEST 5: Blink Minutes (HH visible, MM blank during OFF phase) */
    Display_Init();
    Display_SetTime(12, 34);
    Display_SetBlinkMode(DISPLAY_BLINK_MINUTES);
    advance_sim_ms(500); /* Advance into OFF phase (500ms) */
    Display_UpdateBlinkState();

    Display_ScanRoutine(); /* Digit 0 (H1) -> Must be visible 0x06 */
    if (P3 == 0x06)
    {
        Display_ScanRoutine(); /* Digit 1 (H0) -> Must be visible 0xDB */
        Display_ScanRoutine(); /* Digit 2 (M1) -> Must be blank 0x00 */
        if (P3 == 0x00)
        {
            test5_blink_minutes_pass = 1;
        }
    }

    /* TEST 6: Colon Control */
    Display_Init();
    Display_SetTime(12, 34);
    Display_SetColon(0);
    Display_ScanRoutine(); /* Digit 0 */
    Display_ScanRoutine(); /* Digit 1: '2' WITHOUT DP -> 0x5B */
    test6_colon_control_pass = (P3 == 0x5B) ? 1 : 0;

    /* =====================================================================
     * SECTION B: Real-Time Hardware Board Multi-Mode Interactive Scanning
     * Automatically cycles through the 3 official competition modes:
     * - Phase 0 (3s): NORMAL (12.34 steady on)
     * - Phase 1 (3s): SET HOUR (12 blinks 1s, 34 steady on)
     * - Phase 2 (3s): SET MINUTE (34 blinks 1s, 12 steady on)
     * ===================================================================== */
    Display_Init();
    Display_SetTime(12, 34);
    Display_SetColon(1);
    Display_SetBlinkMode(DISPLAY_BLINK_NONE);

    last_scan_tick = Timer_GetTickMs();
    last_mode_tick = Timer_GetTickMs();
    demo_mode = 0;

    while (1)
    {
        WDTR = 0x5A;

        /* Quét màn hình LED 7 đoạn đúng chu kỳ 2ms thực tế */
        if (Timer_HasElapsed(last_scan_tick, 2))
        {
            last_scan_tick = Timer_GetTickMs();
            Display_ScanRoutine();
        }

        /* Cập nhật nhấp nháy theo Timer phần cứng */
        Display_UpdateBlinkState();

        /* Đổi chế độ đúng mỗi 3000ms (3 giây thực tế) */
        if (Timer_HasElapsed(last_mode_tick, 3000UL))
        {
            last_mode_tick = Timer_GetTickMs();
            demo_mode = (demo_mode + 1) % 3;

            switch (demo_mode)
            {
                case 0:
                    Display_SetBlinkMode(DISPLAY_BLINK_NONE);
                    break;
                case 1:
                    Display_SetBlinkMode(DISPLAY_BLINK_HOURS);
                    break;
                case 2:
                    Display_SetBlinkMode(DISPLAY_BLINK_MINUTES);
                    break;
                default:
                    break;
            }
        }
    }
}
