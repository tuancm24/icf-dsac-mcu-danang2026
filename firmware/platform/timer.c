#include "timer.h"
#include <SN8F5708.H>

/* =========================================================================
 * Platform Timer & System Tick Implementation
 * Target: SONiX SN8F5708 (Mode 1 16-bit Timer with Software Reload in ISR)
 * ========================================================================= */

sfr TH0_REG = 0x8C; /* SFR TH0 */
sfr TL0_REG = 0x8A; /* SFR TL0 */

static volatile unsigned long s_system_tick_ms = 0;

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

    /* 4. Enable Timer 0 Interrupt & Start Timer */
    ET0 = 1;            /* Enable Timer 0 interrupt */
    TR0 = 1;            /* Start Timer 0 */
    EA  = 1;            /* Global interrupt enable */
}

unsigned long Timer_GetTickMs(void)
{
    unsigned long tick;
    unsigned char ea_state;

    /*
     * Coherent 32-bit Atomic Snapshot (IMP-TMR-01):
     * Save previous EA interrupt state, disable interrupts during 4-byte copy,
     * then restore exact previous EA state.
     */
    ea_state = EA;
    EA = 0;
    tick = s_system_tick_ms;
    EA = ea_state;

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
