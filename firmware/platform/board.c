#include "board.h"
#include <SN8F5708.H>

/* =========================================================================
 * Platform Board Implementation for SONiX SN8F5708 EVK
 * Master Hardware Initializer & Watchdog Abstraction (IMP-WDT-01)
 * ========================================================================= */

void Board_Init(void)
{
    /* 1. Feed Watchdog upon initial entry */
    Board_FeedWatchdog();

    /* 2. Initialize low-level MCU peripherals in deterministic sequence */
    GPIO_Init();
    I2C_Init();
    Timer_Init();
}

void Board_FeedWatchdog(void)
{
#if BOARD_WDT_ENABLED
    /* If Watchdog is enabled in production configuration, write clear sequence */
    WDTR = 0x5A;
#else
    /*
     * If Watchdog is disabled in configuration, this function is a safe no-op.
     * Application layer can call Board_FeedWatchdog() every cycle without knowing policy.
     */
#endif
}
