#include "eeprom.h"
#include "timer.h"
#include "gpio.h"
#include <SN8F5708.H>

/* =========================================================================
 * I2C EEPROM Driver Verification Test Runner
 * Target MCU: SONiX SN8F5708 EVK
 * Verification Environment: Keil C51 Simulator
 * ========================================================================= */

volatile unsigned char test1_init_done = 0;
volatile unsigned char test2_invalid_hour_rejected = 0;
volatile unsigned char test3_invalid_min_rejected = 0;

void main(void)
{
    /* 1. Initialize hardware */
    WDTR = 0x5A;
    GPIO_Init();
    Timer_Init();
    EEPROM_Init();

    /* =====================================================================
     * TEST 1: EEPROM Initialization
     * ===================================================================== */
    test1_init_done = 1;

    /* =====================================================================
     * TEST 2: Out-of-Range Hour Rejection (hour=25)
     * Expected: Returns 0 immediately without I2C transaction
     * ===================================================================== */
    test2_invalid_hour_rejected = (EEPROM_SaveAlarm(25, 30) == 0) ? 1 : 0; /* Expected: 1 */

    /* =====================================================================
     * TEST 3: Out-of-Range Minute Rejection (minute=65)
     * Expected: Returns 0 immediately without I2C transaction
     * ===================================================================== */
    test3_invalid_min_rejected = (EEPROM_SaveAlarm(12, 65) == 0) ? 1 : 0; /* Expected: 1 */

    /* End of Test - Infinite loop for debugger inspection */
    while (1)
    {
        WDTR = 0x5A;
        /* Place breakpoint here (F9) */
    }
}
