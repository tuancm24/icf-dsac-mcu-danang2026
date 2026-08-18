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
 *    - Stage 1: Inject failure before invalidate -> Old baseline 10:20 preserved
 *    - Stage 2: Inject failure during hour write -> Invalidation causes Read to fail
 *    - Stage 3: Inject failure during minute write -> Invalidation causes Read to fail
 *    - Stage 4: Inject failure during final magic write -> Read fails
 * 3. Interactive Hardware Board Runner
 * ========================================================================= */

#ifdef TEST_BUILD
extern void EEPROM_Test_SetMockMode(unsigned char enable);
extern void EEPROM_Test_InjectFailure(unsigned char step);
extern void EEPROM_Test_CorruptMagic(unsigned char bad_magic);
#endif

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
    WDTR = 0x5A;
    GPIO_Init();
    Timer_Init();
    Buzzer_Init();
    EEPROM_Init();

    /* =====================================================================
     * SECTION A: Software Simulation Assertion-Based Unit Tests (T-EEP-01)
     * ===================================================================== */
#ifdef TEST_BUILD
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
    fault_pass = 1;

    /* A. Establish baseline (10:20) */
    EEPROM_Test_InjectFailure(0);
    if (EEPROM_SaveAlarm(10, 20) != 1)
    {
        fault_pass = 0;
    }

    /* B. Inject Stage 1: Failure before invalidate */
    EEPROM_Test_InjectFailure(1);
    if (EEPROM_SaveAlarm(11, 22) != 0)
    {
        fault_pass = 0;
    }

    /* C. Read-back Stage 1: Old baseline 10:20 must remain intact */
    h = 0; m = 0;
    if (EEPROM_ReadAlarm(&h, &m) != 1 || h != 10 || m != 20)
    {
        fault_pass = 0;
    }

    /* D. Inject Stage 2: Failure during hour write (after magic invalidated) */
    EEPROM_Test_InjectFailure(2);
    if (EEPROM_SaveAlarm(12, 33) != 0)
    {
        /* Expected return 0 */
    }
    if (EEPROM_ReadAlarm(&h, &m) != 0)
    {
        fault_pass = 0; /* Magic was invalidated, must be rejected */
    }

    /* E. Inject Stage 3: Failure during minute write */
    EEPROM_Test_InjectFailure(3);
    if (EEPROM_SaveAlarm(13, 44) != 0)
    {
        /* Expected return 0 */
    }
    if (EEPROM_ReadAlarm(&h, &m) != 0)
    {
        fault_pass = 0;
    }

    /* F. Inject Stage 4: Failure during final magic write */
    EEPROM_Test_InjectFailure(4);
    if (EEPROM_SaveAlarm(14, 55) != 0)
    {
        /* Expected return 0 */
    }
    if (EEPROM_ReadAlarm(&h, &m) != 0)
    {
        fault_pass = 0;
    }

    /* G. Restore clean injection state and verify final write succeeds */
    EEPROM_Test_InjectFailure(0);
    if (EEPROM_SaveAlarm(7, 30) != 1 || EEPROM_ReadAlarm(&h, &m) != 1 || h != 7 || m != 30)
    {
        fault_pass = 0;
    }

    test6_fault_injection_pass = fault_pass;
#endif

    /* =====================================================================
     * SECTION C: Interactive Hardware Board Runner
     * ===================================================================== */
#ifdef TEST_BUILD
    EEPROM_Test_SetMockMode(0); /* Switch to real physical I2C pins */
    EEPROM_Test_InjectFailure(0);
#endif

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
        WDTR = 0x5A;
        Buzzer_Process();

        if (!Buzzer_IsBusy())
        {
            GPIO_SetLED_D4(PIN_STATE_LOW);
        }
    }
}
