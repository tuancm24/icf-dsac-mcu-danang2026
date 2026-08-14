#include "board.h"

/* =========================================================================
 * Platform Board Initialization Implementation
 * ========================================================================= */

void Board_Init(void)
{
    /* 1. Initialize GPIO pin directions and default inactive states */
    GPIO_Init();

    /* 2. Initialize System Tick Timer (1ms periodic interrupt) */
    Timer_Init();

    /* 3. Initialize I2C Bus Peripheral */
    I2C_Init();
}
