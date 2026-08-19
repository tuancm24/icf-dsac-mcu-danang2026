#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

/* =========================================================================
 * SONiX SN8F5708 EVK - Board Clock & Hardware Configuration
 * Target MCU: SN8F5708F (LQFP48)
 * Development Board: SONiX 5708_EVK-V1.0 (2024.02.26)
 * ========================================================================= */

/* 1. Oscillator Frequency */
#define FOSC                        (32000000UL) /* 32 MHz IHRC selected by OPTIONS_SN8F5708.A51 */
#define FCPU                        (FOSC)       /* CLKSEL = 0x07 => CPU Clock = Fosc / 1 */

/* 2. Timer 0 Periodic System Tick (1.000 ms) */
#define TIMER0_PRESCALER            (12UL)
#define TIMER0_CLOCK_HZ             (FCPU / TIMER0_PRESCALER) /* Timer0 T0CT=0 => Fcpu / 12 */
#define TIMER0_TICK_RATE_HZ         (1000UL)                  /* 1000 Hz = 1 ms nominal interval */
#define TIMER0_TICKS_PER_PERIOD     ((TIMER0_CLOCK_HZ + (TIMER0_TICK_RATE_HZ / 2UL)) / TIMER0_TICK_RATE_HZ)

/*
 * Timer 0 Mode 1 (16-bit Timer with Software Reload in ISR)
 * 32 MHz IHRC, Fcpu/12 => 2.666666 MHz timer clock.
 * Rounded counts/period = 2667; reload = 65536 - 2667 = 62869 = 0xF595.
 * Nominal period = 1.000125 ms (about +125 ppm).
 */
#define TIMER0_RELOAD_VALUE         (65536UL - TIMER0_TICKS_PER_PERIOD)
#define TIMER0_RELOAD_TH            ((unsigned char)((TIMER0_RELOAD_VALUE >> 8) & 0xFF)) /* 0xF5 */
#define TIMER0_RELOAD_TL            ((unsigned char)(TIMER0_RELOAD_VALUE & 0xFF))        /* 0x95 */

/* 3. Watchdog Configuration Policy (IMP-WDT-01) */
#ifndef BOARD_WDT_ENABLED
#define BOARD_WDT_ENABLED           (0) /* 1 = Enabled (feed WDTR), 0 = Disabled (safe no-op) */
#endif

#endif /* BOARD_CONFIG_H */
