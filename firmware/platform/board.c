#include "board.h"
#include <SN8F5708.H>

/* =========================================================================
 * Platform Board Implementation (SONiX SN8F5708 EVK)
 * ========================================================================= */

void Board_Init(void)
{
    /* 1. Feed Watchdog immediately upon entry */
    Board_FeedWatchdog();

    /* 2. Initialize GPIO pin directions and default inactive states */
    GPIO_Init();

    /* 3. Initialize I2C Bus Peripheral */
    I2C_Init();

    /* 4. Initialize and start System Tick Timer 0 (1ms periodic interrupt) */
    Timer_Init();
}

void Board_FeedWatchdog(void)
{
    WDTR = 0x5A;
}
