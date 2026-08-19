#include "timer.h"
#ifndef TEST_BUILD
#include "display.h"
#endif
#include <SN8F5708.H>

/* =========================================================================
 * Platform Timer & System Tick Implementation
 * Target: SONiX SN8F5708 (Mode 1 16-bit Timer with Software Reload in ISR)
 * ========================================================================= */

sfr TH0_REG = 0x8C; /* SFR TH0 */
sfr TL0_REG = 0x8A; /* SFR TL0 */
sbit EAL_BIT = 0xA8^7; /* Global Interrupt Enable Bit (IEN0.7 / EAL) */

static volatile unsigned long s_system_tick_ms = 0;

#ifndef TEST_BUILD
/* Production-only 2 ms divider for deterministic 4-digit Display scanning. */
static unsigned char s_display_scan_elapsed_ms = 0;
/* 10 ms keeps the existing tick-derived blink semantics with bounded ISR work. */
static unsigned char s_display_blink_elapsed_ms = 0;
#endif

void Timer_Init(void)
{
    /* 1. Configure Timer 0 in Mode 1 (16-bit up-counting timer) */
    TMOD &= 0xF0;       /* Clear T0 mode bits (T0M1, T0M0, T0CT, T0GATE) */
    TMOD |= 0x01;       /* T0M0 = 1, T0M1 = 0: Mode 1 (16-bit timer), clock from fcpu/12 */

    /* 2. Load 1ms Initial Reload Value (0xFC18 for 12MHz FOSC) */
    TH0_REG = TIMER0_RELOAD_TH;
    TL0_REG = TIMER0_RELOAD_TL;

    /* 3. Reset 32-bit System Tick Counter */
    s_system_tick_ms = 0;

#ifndef TEST_BUILD
    /* Reinitialization must also restart the Display scan cadence deterministically. */
    s_display_scan_elapsed_ms = 0;
    s_display_blink_elapsed_ms = 0;
#endif

    /* 4. Enable Timer 0 Interrupt & Start Timer */
    ET0 = 1;            /* Enable Timer 0 interrupt */
    TR0 = 1;            /* Start Timer 0 */
    EAL_BIT = 1;        /* Global interrupt enable */
}

unsigned long Timer_GetTickMs(void)
{
    unsigned long tick;
    unsigned char ea_state;

    /*
     * Coherent 32-bit Atomic Snapshot (IMP-TMR-01):
     * Save previous EA/EAL interrupt state, disable interrupts during 4-byte copy,
     * then restore exact previous state.
     */
    ea_state = EAL_BIT;
    EAL_BIT = 0;
    tick = s_system_tick_ms;
    EAL_BIT = ea_state;

    return tick;
}

unsigned char Timer_HasElapsed(unsigned long start_tick, unsigned long duration_ms)
{
    unsigned long current_tick = Timer_GetTickMs();
    /* Wrap-safe unsigned arithmetic */
    return ((current_tick - start_tick) >= duration_ms) ? 1 : 0;
}

void Timer_ISR_Handler(void)
{
    s_system_tick_ms++;
}

void Timer0_ISR(void) interrupt 1
{
    /* Software reload of 16-bit reload value in Mode 1 */
    TH0_REG = TIMER0_RELOAD_TH;
    TL0_REG = TIMER0_RELOAD_TL;

    /* Call platform tick handler */
    Timer_ISR_Handler();

#ifndef TEST_BUILD
    /*
     * TIM-05/TIM-11: production Display multiplexing is hardware-timed.
     * Display_ScanRoutine() self-gates until Display_Init() completes.
     * Keep Timer_ISR_Handler() pure so existing software tests retain their
     * one-call == one-millisecond tick semantics.
     */
    s_display_scan_elapsed_ms++;
    if (s_display_scan_elapsed_ms >= DISPLAY_SCAN_DIGIT_INTERVAL_MS)
    {
        s_display_scan_elapsed_ms = 0;
        Display_ScanRoutine();
    }

    /*
     * Driver-owned blink phase service. 10 ms divides the official 500 ms
     * half-period exactly and keeps Application independent of blink timing.
     * Display_UpdateBlinkState() retains its existing absolute-tick algorithm.
     */
    s_display_blink_elapsed_ms++;
    if (s_display_blink_elapsed_ms >= 10U)
    {
        s_display_blink_elapsed_ms = 0;
        Display_UpdateBlinkState();
    }
#endif
}

/*
 * Approximate unverified software loop for microsecond-range delays.
 * Used for basic bit-banging I2C bus cycle pacing.
 */
void Timer_DelayUs(unsigned int us)
{
    volatile unsigned int i;
    while (us--)
    {
        for (i = 0; i < 4; i++)
        {
            /* Approximate cycle delay */
        }
    }
}
