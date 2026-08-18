#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

/* =========================================================================
 * SONiX SN8F5708 EVK - Board Clock & Hardware Configuration
 * Target MCU: SN8F5708F (LQFP48)
 * Development Board: SONiX 5708_EVK-V1.0 (2024.02.26)
 * ========================================================================= */

/* 1. Oscillator Frequency */
#define FOSC                        (12000000UL) /* 12.000 MHz External Crystal (Y1 on EVK) */
#define FCPU                        (FOSC)       /* CPU Clock = 12 MHz */

/* 2. Timer 0 Periodic System Tick (1.000 ms) */
#define TIMER0_PRESCALER            (12UL)
#define TIMER0_CLOCK_HZ             (FOSC / TIMER0_PRESCALER) /* 1 MHz -> 1 us per tick */
#define TIMER0_TICK_RATE_HZ         (1000UL)                  /* 1000 Hz = 1 ms interval */
#define TIMER0_TICKS_PER_PERIOD     (TIMER0_CLOCK_HZ / TIMER0_TICK_RATE_HZ) /* 1000 counts */

/*
 * Timer 0 Mode 1 (16-bit Timer with Software Reload in ISR)
 * Reload Value Calculation: 65536 - 1000 = 64536 = 0xFC18
 */
#define TIMER0_RELOAD_VALUE         (65536UL - TIMER0_TICKS_PER_PERIOD)
#define TIMER0_RELOAD_TH            ((unsigned char)((TIMER0_RELOAD_VALUE >> 8) & 0xFF)) /* 0xFC */
#define TIMER0_RELOAD_TL            ((unsigned char)(TIMER0_RELOAD_VALUE & 0xFF))        /* 0x18 */

/* 3. Watchdog Configuration Policy (IMP-WDT-01) */
#ifndef BOARD_WDT_ENABLED
#define BOARD_WDT_ENABLED           (0) /* 1 = Enabled (feed WDTR), 0 = Disabled (safe no-op) */
#endif

#endif /* BOARD_CONFIG_H */
