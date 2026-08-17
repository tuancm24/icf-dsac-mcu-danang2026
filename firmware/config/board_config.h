#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

/* =========================================================================
 * SONiX SN8F5708 EVK - Board Hardware Configuration
 * ========================================================================= */

/*
 * System Clock Frequency Configuration:
 * - 12 MHz External Crystal (Y1 on EVK Board Silkscreen / BOM)
 * - 16 MHz / 32 MHz Internal High-Speed RC Oscillator (IHRC)
 */
#ifndef FOSC
#define FOSC                        (12000000UL) /* 12 MHz Crystal on EVK */
#endif

/* System Instruction Cycle / Machine Cycle (Standard 8051 / 12T) */
#define MCU_TIMER_PRESCALER         (12UL)
#define TIMER_TICK_FREQ             (FOSC / MCU_TIMER_PRESCALER) /* 1 MHz at 12MHz FOSC */

/* 1ms Hardware Timer 0 Reload Value Calculation:
 * Counts per 1ms = TIMER_TICK_FREQ / 1000 = 1000 counts
 * 16-bit Auto-reload: 65536 - 1000 = 64536 = 0xFC18
 */
#define TIMER0_1MS_RELOAD_VALUE     (65536UL - (TIMER_TICK_FREQ / 1000UL))
#define TIMER0_RELOAD_TH            ((unsigned char)((TIMER0_1MS_RELOAD_VALUE >> 8) & 0xFF))
#define TIMER0_RELOAD_TL            ((unsigned char)(TIMER0_1MS_RELOAD_VALUE & 0xFF))

/* System Timer Tick Configuration (1ms per tick) */
#define SYS_TICK_PERIOD_MS          (1)
#define SYS_TICKS_PER_SEC           (1000 / SYS_TICK_PERIOD_MS)

#endif /* BOARD_CONFIG_H */
