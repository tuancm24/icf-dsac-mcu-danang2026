#ifndef BOARD_H
#define BOARD_H

#include "board_config.h"
#include "pin_config.h"
#include "gpio.h"
#include "timer.h"
#include "i2c.h"

/* =========================================================================
 * Platform Board Initialization Interface (SONiX SN8F5708)
 * ========================================================================= */

/* Initialize all hardware platform peripherals (Clock, GPIO, Timer, I2C) */
void Board_Init(void);

#endif /* BOARD_H */
