#ifndef BOARD_H
#define BOARD_H

#include "board_config.h"
#include "pin_config.h"
#include "gpio.h"
#include "timer.h"
#include "i2c.h"

/* =========================================================================
 * Platform Board Interface (SONiX SN8F5708 EVK)
 * Master Hardware Initializer & Watchdog Abstraction
 * ========================================================================= */

/* Master platform hardware initializer (GPIO, Timer 1ms ISR, I2C Bus) */
void Board_Init(void);

/* Frozen Watchdog Feed abstraction (Platform-owned WDT clear) */
void Board_FeedWatchdog(void);

#endif /* BOARD_H */
