#include "display.h"
#include "gpio.h"
#include "timer.h"

/* =========================================================================
 * 4-Digit 7-Segment Display Driver Implementation (SONiX SN8F5708 EVK)
 * Time Multiplexing & Segment Decoding (Contract v2.7)
 * ========================================================================= */

/* 7-Segment Font Table (0-9): Bit 0=A, 1=B, 2=C, 3=D, 4=E, 5=F, 6=G, 7=DP */
static const unsigned char s_font_7seg[10] = {
    0x3F, /* 0: a,b,c,d,e,f */
    0x06, /* 1: b,c */
    0x5B, /* 2: a,b,d,e,g */
    0x4F, /* 3: a,b,c,d,g */
    0x66, /* 4: b,c,f,g */
    0x6D, /* 5: a,c,d,f,g */
    0x7D, /* 6: a,c,d,e,f,g */
    0x07, /* 7: a,b,c */
    0x7F, /* 8: a,b,c,d,e,f,g */
    0x6F  /* 9: a,b,c,d,f,g */
};

/* Display buffer for 4 digits (0: H1, 1: H0, 2: M1, 3: M0) */
static unsigned char s_digits[4] = {0, 0, 0, 0};
static unsigned char s_colon_enabled = 1;
static Display_BlinkMode_t s_blink_mode = DISPLAY_BLINK_NONE;
static unsigned char s_blink_phase = 1; /* 1 = Visible (ON), 0 = Hidden (OFF) */
static unsigned long s_mode_start_tick = 0;
static unsigned char s_scan_index = 0;

void Display_Init(void)
{
    s_digits[0] = 0;
    s_digits[1] = 0;
    s_digits[2] = 0;
    s_digits[3] = 0;
    s_colon_enabled = 1;
    s_blink_mode = DISPLAY_BLINK_NONE;
    s_blink_phase = 1;
    s_mode_start_tick = 0;
    s_scan_index = 0;

    GPIO_SelectDisplayDigit(0xFF);
    GPIO_SetDisplaySegments(0x00);
}

void Display_SetTime(unsigned char hour, unsigned char minute)
{
    if (hour > 23)   hour = 23;
    if (minute > 59) minute = 59;

    s_digits[0] = hour / 10;
    s_digits[1] = hour % 10;
    s_digits[2] = minute / 10;
    s_digits[3] = minute % 10;
}

void Display_SetColon(unsigned char enable)
{
    s_colon_enabled = enable ? 1 : 0;
}

void Display_SetBlinkMode(Display_BlinkMode_t mode)
{
    if (s_blink_mode != mode)
    {
        s_blink_mode = mode;
        s_blink_phase = 1; /* Reset phase to visible immediately */
        s_mode_start_tick = Timer_GetTickMs();
    }
}

void Display_UpdateBlinkState(void)
{
    unsigned long current_tick = Timer_GetTickMs();

    if (s_blink_mode != DISPLAY_BLINK_NONE)
    {
        unsigned long elapsed = current_tick - s_mode_start_tick;
        /* TIM-12: Drift-free 500ms phase calculation */
        s_blink_phase = ((elapsed / BLINK_HALF_PERIOD_MS) % 2 == 0) ? 1 : 0;
    }
    else
    {
        s_blink_phase = 1;
    }
}

void Display_Clear(void)
{
    GPIO_SelectDisplayDigit(0xFF);
    GPIO_SetDisplaySegments(0x00);
}

void Display_ScanRoutine(void)
{
    unsigned char seg_data = 0x00;
    unsigned char show_digit = 1;

    /* 1. Turn off all digits to eliminate ghosting during transition */
    GPIO_SelectDisplayDigit(0xFF);

    /* 2. Check blinking suppression rules for current digit */
    if (s_blink_phase == 0)
    {
        if (s_blink_mode == DISPLAY_BLINK_HOURS && (s_scan_index == 0 || s_scan_index == 1))
        {
            show_digit = 0; /* Blank HH */
        }
        else if (s_blink_mode == DISPLAY_BLINK_MINUTES && (s_scan_index == 2 || s_scan_index == 3))
        {
            show_digit = 0; /* Blank MM */
        }
        else if (s_blink_mode == DISPLAY_BLINK_ALL)
        {
            show_digit = 0; /* Blank All */
        }
    }

    /* 3. Decode digit pattern if visible */
    if (show_digit)
    {
        unsigned char digit_val = s_digits[s_scan_index];
        if (digit_val < 10)
        {
            seg_data = s_font_7seg[digit_val];
        }

        /* Add colon / decimal point to Digit 1 (HH units) */
        if (s_scan_index == 1 && s_colon_enabled)
        {
            seg_data |= 0x80; /* Enable DP */
        }
    }

    /* 4. Output segment data to GPIO */
#if DISPLAY_COMMON_ANODE
    GPIO_SetDisplaySegments(~seg_data); /* Invert for Common Anode */
#else
    GPIO_SetDisplaySegments(seg_data);  /* Direct for Common Cathode (3461AS) */
#endif

    /* 5. Activate current digit */
    GPIO_SelectDisplayDigit(s_scan_index);

    /* 6. Advance to next digit index */
    s_scan_index = (s_scan_index + 1) % 4;
}
