#include "display.h"
#include "gpio.h"
#include "timer.h"

/* Global Interrupt Enable Bit (IEN0.7 / EAL), local to this Driver module. */
sbit DISPLAY_EAL_BIT = 0xA8^7;

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

/* Display state shared with the asynchronous scan/blink service. */
static volatile unsigned char s_digits[4] = {0, 0, 0, 0};
static volatile unsigned char s_colon_enabled = 1;
/* Keep mode byte-sized so mode reads/writes are atomic on C51. */
static volatile unsigned char s_blink_mode = DISPLAY_BLINK_NONE;
static volatile unsigned char s_blink_phase = 1; /* 1 = Visible (ON), 0 = Hidden (OFF) */
static volatile unsigned long s_mode_start_tick = 0;
static volatile unsigned char s_scan_index = 0;

/* Timer may already run before Display_Init(); async Display service stays gated. */
static volatile unsigned char s_initialized = 0;

void Display_Init(void)
{
    /* Publish disabled state before rebuilding shared Display state. */
    s_initialized = 0;

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

    /* Publish ready only after all logical/GPIO Display state is coherent. */
    s_initialized = 1;
}

void Display_SetTime(unsigned char hour, unsigned char minute)
{
    unsigned char d0;
    unsigned char d1;
    unsigned char d2;
    unsigned char d3;
    unsigned char ea_state;

    if (hour > 23)   hour = 23;
    if (minute > 59) minute = 59;

    /* Perform formatting before the very short critical publication section. */
    d0 = hour / 10;
    d1 = hour % 10;
    d2 = minute / 10;
    d3 = minute % 10;

    /*
     * TIM-07: publish HH:MM atomically with respect to Timer ISR scanning.
     * Preserve and restore the previous global interrupt-enable state.
     */
    ea_state = DISPLAY_EAL_BIT;
    DISPLAY_EAL_BIT = 0;
    s_digits[0] = d0;
    s_digits[1] = d1;
    s_digits[2] = d2;
    s_digits[3] = d3;
    DISPLAY_EAL_BIT = ea_state;
}

void Display_SetColon(unsigned char enable)
{
    s_colon_enabled = enable ? 1 : 0;
}

void Display_SetBlinkMode(Display_BlinkMode_t mode)
{
    unsigned char requested_mode = (unsigned char)mode;

    if (s_blink_mode != requested_mode)
    {
        unsigned long start_tick = Timer_GetTickMs();
        unsigned char ea_state = DISPLAY_EAL_BIT;

        /*
         * TIM-07: mode, phase and 32-bit anchor become visible as one coherent
         * update to the asynchronous Display service.
         */
        DISPLAY_EAL_BIT = 0;
        s_blink_mode = requested_mode;
        s_blink_phase = 1; /* Reset phase to visible immediately */
        s_mode_start_tick = start_tick;
        DISPLAY_EAL_BIT = ea_state;
    }
}

static void Display_ApplyBlinkTick(unsigned long current_tick)
{
    if (!s_initialized)
    {
        return;
    }

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

#ifndef TEST_BUILD
void Display_UpdateBlinkStateAtTick(unsigned long current_tick)
{
    /* Production ISR path: tick is supplied by Timer0_ISR after the 1 ms step. */
    Display_ApplyBlinkTick(current_tick);
}
#else
void Display_UpdateBlinkState(void)
{
    /* Existing test path remains foreground-safe and uses the atomic getter. */
    Display_ApplyBlinkTick(Timer_GetTickMs());
}
#endif

void Display_Clear(void)
{
    GPIO_SelectDisplayDigit(0xFF);
    GPIO_SetDisplaySegments(0x00);
}

void Display_ScanRoutine(void)
{
    unsigned char seg_data = 0x00;
    unsigned char show_digit = 1;

    if (!s_initialized)
    {
        return;
    }

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
