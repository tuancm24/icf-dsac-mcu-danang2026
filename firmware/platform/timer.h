#ifndef TIMER_H
#define TIMER_H

#include "board_config.h"

/* =========================================================================
 * Platform Timer & System Tick Interface (SONiX SN8F5708)
 * ========================================================================= */

/* Initialize System Timer for 1ms periodic interrupt */
void Timer_Init(void);

/* Get current system tick count in milliseconds */
unsigned long Timer_GetTickMs(void);

/* Check if a time interval has elapsed */
unsigned char Timer_HasElapsed(unsigned long start_tick, unsigned long duration_ms);

/* Timer 1ms ISR Handler (called by MCU Timer Interrupt Service Routine) */
void Timer_ISR_Handler(void);

/* Simple blocking microsecond delay (for I2C bus timing) */
void Timer_DelayUs(unsigned int us);

#endif /* TIMER_H */
