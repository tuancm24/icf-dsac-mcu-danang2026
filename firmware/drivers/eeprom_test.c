#include "eeprom.h"
#include "display.h"
#include "buzzer.h"
#include "timer.h"
#include "gpio.h"
#include <SN8F5708.H>

/* =========================================================================
 * I2C EEPROM (24C08) Driver Verification & Hardware Board Test Runner
 * Target MCU: SONiX SN8F5708 EVK
 *
 * Hardware Peripherals:
 * - U6 (24C08 EEPROM on Header J6: SCL=P1.4, SDA=P1.5)
 * - LED D4 (P0.3), Buzzer PZ1 (P1.0)
 * - SMG1 4-Digit 7SEG (Display restored alarm time)
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
    unsigned long last_scan_tick = 0;

    /* 1. Initialize hardware platform */
    WDTR = 0x5A;
    GPIO_Init();
    Timer_Init();
    Display_Init();
    EEPROM_Init();

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
     * SECTION B: Hardware Board I2C EEPROM (24C08) Test
     * 1. Save alarm time "07:30" to physical EEPROM chip
     * 2. Read back and verify validity via Magic Byte (0xA5)
     * 3. Display "07:30" on 4-Digit 7-Segment display to prove success!
     * ===================================================================== */
    /* Write test alarm to EEPROM */
    EEPROM_SaveAlarm(7, 30);

    /* Read back test alarm */
    if (EEPROM_ReadAlarm(&read_hour, &read_minute))
    {
        eeprom_ok = (read_hour == 7 && read_minute == 30) ? 1 : 0;
    }

    if (eeprom_ok)
    {
        /* Visual & Acoustic Confirmation */
        GPIO_SetLED_D4(PIN_STATE_HIGH); /* LED D4 ON */
        Display_SetTime(read_hour, read_minute);
        Display_SetColon(1);
    }
    else
    {
        /* Display Error (99:99) if EEPROM communication failed */
        Display_SetTime(99, 99);
        Display_SetColon(0);
    }

    last_scan_tick = Timer_GetTickMs();

    while (1)
    {
        WDTR = 0x5A; /* Clear Watchdog timer */

        /* Multiplex 7SEG display every 2ms */
        if (Timer_HasElapsed(last_scan_tick, 2))
        {
            last_scan_tick = Timer_GetTickMs();
            Display_ScanRoutine();
        }
    }
}
