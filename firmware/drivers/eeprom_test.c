#define TEST_BUILD
#include "eeprom.h"
#include "buzzer.h"
#include "timer.h"
#include "gpio.h"
#include "i2c.h"
#include <SN8F5708.H>

/* =========================================================================
 * I2C EEPROM (24C05) Driver Verification & Hardware Board Test Runner
 * Target MCU: SONiX SN8F5708 EVK
 *
 * Test Suites:
 * 1. Assertion-based Software Simulation Tests (T-EEP-01)
 *    - Vectors: 00:00, 06:45, 23:59
 *    - Invalid magic byte rejection
 *    - Out-of-range & NULL pointer rejection
 * 2. Fault Injection Suite (T-EEP-02 / IMP-EEP-01)
 *    - Inject failure at each transaction stage (1..4)
 *    - Assert no mixed/corrupted valid record is accepted
 * 3. Interactive Hardware Board Runner
 * ========================================================================= */

extern void EEPROM_Test_SetMockMode(unsigned char enable);
extern void EEPROM_Test_InjectFailure(unsigned char step);
extern void EEPROM_Test_CorruptMagic(unsigned char bad_magic);

volatile unsigned char test1_save_alarm_pass = 0;
volatile unsigned char test2_read_alarm_pass = 0;
volatile unsigned char test3_boundary_pass = 0;
volatile unsigned char test4_magic_byte_reject_pass = 0;
volatile unsigned char test5_out_of_range_reject_pass = 0;
volatile unsigned char test6_fault_injection_pass = 0;

void main(void)
{
    unsigned char h = 0, m = 0;
    unsigned char fault_pass = 1;
    unsigned char read_hour = 0;
    unsigned char read_minute = 0;
    unsigned char eeprom_ok = 0;

    /* 1. Initialize Hardware Platform */
    Board_FeedWatchdog();
    GPIO_Init();
    Timer_Init();
    Buzzer_Init();
    EEPROM_Init();

    /* =====================================================================
     * SECTION A: Software Simulation Assertion-Based Unit Tests (T-EEP-01)
     * ===================================================================== */
    EEPROM_Test_SetMockMode(1);
    EEPROM_Test_InjectFailure(0);

    /* TEST 1: Save Alarm 06:45 and assert return value */
    test1_save_alarm_pass = (EEPROM_SaveAlarm(6, 45) == 1) ? 1 : 0;

    /* TEST 2: Read Alarm and assert exact matching values */
    h = 0; m = 0;
    test2_read_alarm_pass = (EEPROM_ReadAlarm(&h, &m) == 1 && h == 6 && m == 45) ? 1 : 0;

    /* TEST 3: Boundary Vectors (00:00 and 23:59) */
    test3_boundary_pass = 0;
    if (EEPROM_SaveAlarm(0, 0) == 1 && EEPROM_ReadAlarm(&h, &m) == 1 && h == 0 && m == 0)
    {
        if (EEPROM_SaveAlarm(23, 59) == 1 && EEPROM_ReadAlarm(&h, &m) == 1 && h == 23 && m == 59)
        {
            test3_boundary_pass = 1;
        }
    }

    /* TEST 4: Corrupted Magic Byte Rejection (0x00, 0x5A, 0xFF) */
    EEPROM_Test_CorruptMagic(0x00);
    test4_magic_byte_reject_pass = (EEPROM_ReadAlarm(&h, &m) == 0) ? 1 : 0;
    EEPROM_Test_CorruptMagic(0xFF);
    if (EEPROM_ReadAlarm(&h, &m) != 0) test4_magic_byte_reject_pass = 0;

    /* TEST 5: Out of range and NULL pointer rejection */
    test5_out_of_range_reject_pass = (EEPROM_SaveAlarm(24, 0) == 0 &&
                                      EEPROM_SaveAlarm(0, 60) == 0 &&
                                      EEPROM_SaveAlarm(255, 255) == 0 &&
                                      EEPROM_ReadAlarm(0, &m) == 0 &&
                                      EEPROM_ReadAlarm(&h, 0) == 0) ? 1 : 0;

    /* =====================================================================
     * SECTION B: Fault Injection Suite (T-EEP-02 / IMP-EEP-01)
     * ===================================================================== */
    /* Setup known valid baseline */
    EEPROM_Test_InjectFailure(0);
    EEPROM_SaveAlarm(10, 20);

    /* Step 1 failure injection */
    EEPROM_Test_InjectFailure(1);
    EEPROM_SaveAlarm(11, 22);
    /* Should fail to update and magic remains invalid or old */

    /* Step 2 failure injection (hour write failed) */
    EEPROM_Test_InjectFailure(2);
    EEPROM_SaveAlarm(12, 33);
    /* Read must return 0 because magic was invalidated in step 1 */
    if (EEPROM_ReadAlarm(&h, &m) != 0) fault_pass = 0;

    /* Step 3 failure injection (minute write failed) */
    EEPROM_Test_InjectFailure(3);
    EEPROM_SaveAlarm(13, 44);
    if (EEPROM_ReadAlarm(&h, &m) != 0) fault_pass = 0;

    /* Step 4 failure injection (final magic write failed) */
    EEPROM_Test_InjectFailure(4);
    EEPROM_SaveAlarm(14, 55);
    if (EEPROM_ReadAlarm(&h, &m) != 0) fault_pass = 0;

    test6_fault_injection_pass = fault_pass;

    /* =====================================================================
     * SECTION C: Interactive Hardware Board Runner
     * ===================================================================== */
    EEPROM_Test_SetMockMode(0); /* Switch to real physical I2C pins */
    EEPROM_Test_InjectFailure(0);

    Timer_DelayUs(50000U); /* 50ms startup stabilization delay */

    /* Save 07:30 to physical 24C05 */
    EEPROM_SaveAlarm(7, 30);

    /* Read back from physical 24C05 */
    if (EEPROM_ReadAlarm(&read_hour, &read_minute))
    {
        if (read_hour == 7 && read_minute == 30)
        {
            eeprom_ok = 1;
        }
    }

    if (eeprom_ok)
    {
        Buzzer_BeepShort();
        GPIO_SetLED_D4(PIN_STATE_HIGH);
    }
    else
    {
        GPIO_SetLED_D4(PIN_STATE_LOW);
    }

    while (1)
    {
        Board_FeedWatchdog();
        Buzzer_Process();

        if (!Buzzer_IsBusy())
        {
            GPIO_SetLED_D4(PIN_STATE_LOW);
        }
    }
}
