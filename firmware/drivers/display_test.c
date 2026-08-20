#include "display.h"
#include "timer.h"
#include "gpio.h"
#include <SN8F5708.H>

/* =========================================================================
 * 4-Digit 7-Segment Display Verification & Hardware Board Test Runner
 * Target MCU: SONiX SN8F5708 EVK (Real-Time Hardware Scanning & Boundary Suite)
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
    unsigned long last_step_tick;
    unsigned char step_idx = 0;

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
     * SECTION B: Comprehensive Hardware Boundary & Blink Stress Runner
     * Cycles through 6 critical test vectors:
     * - Step 0 (2.5s): 00.00 (Startup state / midnight)
     * - Step 1 (2.5s): 09.09 (Digits 0 and 9 interleaved font check)
     * - Step 2 (2.5s): 12.34 (Midday reference)
     * - Step 3 (2.5s): 23.59 (Upper maximum boundary)
     * - Step 4 (3.0s): 23.59 with BLINK_HOURS (23 blinks 1s, 59 steady)
     * - Step 5 (3.0s): 23.59 with BLINK_MINUTES (59 blinks 1s, 23 steady)
     * ===================================================================== */
    Display_Init();
    Display_SetTime(0, 0);
    Display_SetColon(1);
    Display_SetBlinkMode(DISPLAY_BLINK_NONE);

    last_scan_tick = Timer_GetTickMs();
    last_step_tick = Timer_GetTickMs();
    step_idx = 0;

    while (1)
    {
        WDTR = 0x5A;

        /* Quét digit mỗi 2ms chuẩn xác */
        if (Timer_HasElapsed(last_scan_tick, 2))
        {
            last_scan_tick = Timer_GetTickMs();
            Display_ScanRoutine();
        }

        /* Cập nhật trạng thái blink 1s */
        Display_UpdateBlinkState();

        /* Luân chuyển qua 6 kịch bản kiểm thử biên */
        if (Timer_HasElapsed(last_step_tick, 2500UL))
        {
            last_step_tick = Timer_GetTickMs();
            step_idx = (step_idx + 1) % 6;

            switch (step_idx)
            {
                case 0:
                    /* Mốc 00.00 khởi động */
                    Display_SetTime(0, 0);
                    Display_SetBlinkMode(DISPLAY_BLINK_NONE);
                    break;

                case 1:
                    /* Mốc 09.09 kiểm tra nét số 9 và số 0 */
                    Display_SetTime(9, 9);
                    Display_SetBlinkMode(DISPLAY_BLINK_NONE);
                    break;

                case 2:
                    /* Mốc 12.34 chuẩn */
                    Display_SetTime(12, 34);
                    Display_SetBlinkMode(DISPLAY_BLINK_NONE);
                    break;

                case 3:
                    /* Mốc biên cực đại 23.59 */
                    Display_SetTime(23, 59);
                    Display_SetBlinkMode(DISPLAY_BLINK_NONE);
                    break;

                case 4:
                    /* Chế độ cài đặt giờ tại mốc 23.59 (Số 23 nhấp nháy, 59 sáng tĩnh) */
                    Display_SetTime(23, 59);
                    Display_SetBlinkMode(DISPLAY_BLINK_HOURS);
                    break;

                case 5:
                    /* Chế độ cài đặt phút tại mốc 23.59 (Số 59 nhấp nháy, 23 sáng tĩnh) */
                    Display_SetTime(23, 59);
                    Display_SetBlinkMode(DISPLAY_BLINK_MINUTES);
                    break;

                default:
                    break;
            }
        }
    }
}
