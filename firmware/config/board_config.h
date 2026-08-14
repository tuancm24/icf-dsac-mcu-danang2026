#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

/* =========================================================================
 * SONiX SN8F5708 EVK - Board Hardware Configuration
 * ========================================================================= */

/* System Oscillator Clock Frequency (Internal High-Speed Oscillator: 16 MHz) */
#ifndef FOSC
#define FOSC                        (16000000UL)
#endif

/* System Instruction Cycle / Machine Cycle */
#ifndef FCY
#define FCY                         (FOSC)
#endif

/* System Timer Tick Configuration (1ms per tick) */
#define SYS_TICK_PERIOD_MS          (1)
#define SYS_TICKS_PER_SEC           (1000 / SYS_TICK_PERIOD_MS)

#endif /* BOARD_CONFIG_H */
