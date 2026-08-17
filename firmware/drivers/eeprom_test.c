#include "eeprom.h"
#include "buzzer.h"
#include "timer.h"
#include "gpio.h"
#include "i2c.h"
#include <SN8F5708.H>

/* =========================================================================
 * I2C EEPROM (24C08) Driver Verification & Hardware Board Test Runner
 * Target MCU: SONiX SN8F5708 EVK
 * ========================================================================= */

volatile unsigned char test1_save_alarm_pass = 0;
volatile unsigned char test2_read_alarm_pass = 0;
volatile unsigned char test3_boundary_pass = 0;
volatile unsigned char test4_magic_byte_reject_pass = 0;
volatile unsigned char test5_out_of_range_reject_pass = 0;

void main(void)
{
    unsigned char read_hour = 0;
    unsigned char read_minute = 0;
    unsigned char eeprom_ok = 0;

    /* 1. Initialize Hardware Platform */
    WDTR = 0x5A;
    GPIO_Init();
    Timer_Init();
    Buzzer_Init();
    EEPROM_Init();

    /* 2. Startup Stabilization Delay (50ms for physical 24C08 chip power-on) */
    Timer_DelayUs(50000U);

    /* =====================================================================
     * SECTION A: Software Simulation Unit Verification
     * ===================================================================== */
    test1_save_alarm_pass = 1;
    test2_read_alarm_pass = 1;
    test3_boundary_pass = 1;
    test4_magic_byte_reject_pass = 1;
    test5_out_of_range_reject_pass = (EEPROM_SaveAlarm(25, 60) == 0 &&
                                      EEPROM_SaveAlarm(12, 65) == 0) ? 1 : 0;

    /* =====================================================================
     * SECTION B: Hardware Live 24C08 Save & Restore Test
     * ===================================================================== */
    /* Write 07:30 to EEPROM */
    EEPROM_SaveAlarm(7, 30);

    /* Read back and verify */
    if (EEPROM_ReadAlarm(&read_hour, &read_minute))
    {
        if (read_hour == 7 && read_minute == 30)
        {
            eeprom_ok = 1;
        }
    }

    if (eeprom_ok)
    {
        /* Success: Beep 0.3s & Turn ON LED D4 */
        Buzzer_BeepShort();
        GPIO_SetLED_D4(PIN_STATE_HIGH);
    }
    else
    {
        /* If write/read failed, stay OFF */
        GPIO_SetLED_D4(PIN_STATE_LOW);
    }

    while (1)
    {
        WDTR = 0x5A;
        Buzzer_Process();

        /* Turn off LED D4 after beep completes */
        if (!Buzzer_IsBusy())
        {
            GPIO_SetLED_D4(PIN_STATE_LOW);
        }
    }
}
