#include "timer.h"
#include "app_config.h"

/* =========================================================================
 * Platform Timer Implementation for SONiX SN8F5708 EVK
 * Standard 8051 SFR Definitions & 1ms Periodic Interrupt
 * ========================================================================= */

sfr TCON_REG = 0x88; /* Timer Control Register */
sfr TMOD_REG = 0x89; /* Timer Mode Register */
sfr TL0_REG  = 0x8A; /* Timer 0 Low Byte */
sfr TH0_REG  = 0x8C; /* Timer 0 High Byte */
sfr IE_REG   = 0xA8; /* Interrupt Enable Register */

static volatile unsigned long s_system_tick_ms = 0;

void Timer_Init(void)
{
    s_system_tick_ms = 0;

    /*
     * Hardware Timer 0 Configuration on SN8F5708:
     * - TMOD: Timer 0 Mode 1 (16-bit Timer)
     * - Reload value: computed dynamically from FOSC in board_config.h
     * - IE Register (0xA8): Bit 7 = Global Interrupt Enable (EA), Bit 1 = Timer 0 Enable (ET0)
     * - TCON Register (0x88): Bit 4 = Timer 0 Run Control (TR0)
     */
    TMOD_REG &= 0xF0;
    TMOD_REG |= 0x01; /* Timer 0 Mode 1 (16-bit) */
    TH0_REG   = TIMER0_RELOAD_TH;
    TL0_REG   = TIMER0_RELOAD_TL;

    /* Enable Timer 0 Interrupt and Global Interrupt (Bit 7: EA=1, Bit 1: ET0=1) */
    IE_REG   |= 0x82;

    /* Start Timer 0 (Bit 4: TR0 = 1) */
    TCON_REG |= 0x10;
}

unsigned long Timer_GetTickMs(void)
{
    unsigned long tick;
    unsigned char ea_state = IE_REG & 0x80; /* Save previous interrupt state (EA) */

    IE_REG &= ~0x80; /* Mask global interrupt temporarily to avoid torn 32-bit read */
    tick = s_system_tick_ms;
    IE_REG |= ea_state; /* Restore previous interrupt state */

    return tick;
}

unsigned char Timer_HasElapsed(unsigned long start_tick, unsigned long duration_ms)
{
    unsigned long now = Timer_GetTickMs();
    return ((now - start_tick) >= duration_ms) ? 1 : 0;
}

void Timer_ISR_Handler(void)
{
    s_system_tick_ms++;
}

/* Hardware Interrupt Service Routine for Timer 0 (Vector 1 at 0x000B) */
void Timer0_ISR(void) interrupt 1
{
    TH0_REG = TIMER0_RELOAD_TH;
    TL0_REG = TIMER0_RELOAD_TL;
    Timer_ISR_Handler();
}

void Timer_DelayUs(unsigned int us)
{
    volatile unsigned int i;
    while (us--)
    {
        for (i = 0; i < 4; i++)
        {
            /* Calibrated loop for microsecond delay at 12MHz */
        }
    }
}
