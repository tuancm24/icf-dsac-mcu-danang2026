#include "eeprom.h"
#include "timer.h"
#include "gpio.h"
#include "i2c.h"
#include <SN8F5708.H>

/* =========================================================================
 * I2C EEPROM (24C05) Driver Verification & Hardware Board Test Runner
 * Target MCU: SONiX SN8F5708 EVK (Aligned for 32MHz IHRC)
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

static void delay_ms(unsigned char ms)
{
    volatile unsigned int d;
    while (ms--)
    {
        for (d = 0; d < 1600; d++); /* ~1ms delay loop @ 32MHz IHRC */
    }
}

void main(void)
{
    unsigned char h = 0, m = 0;
    unsigned char fault_pass = 1;
    unsigned char read_hour = 0;
    unsigned char read_minute = 0;
    unsigned char eeprom_ok = 0;
    unsigned char step;

    /* 1. Initialize Hardware Platform */
    WDTR = 0x5A;
    GPIO_Init();
    Timer_Init();
    EEPROM_Init();

    /* =====================================================================
     * SECTION A: Software Simulation Assertion-Based Unit Tests (T-EEP-01)
     * ===================================================================== */
#ifdef TEST_BUILD
    EEPROM_Test_SetMockMode(1);
    EEPROM_Test_InjectFailure(0);

    /* TEST 1: Save Alarm 06:45 */
    test1_save_alarm_pass = (EEPROM_SaveAlarm(6, 45) == 1) ? 1 : 0;

    /* TEST 2: Read Alarm and assert exact matching values */
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

    /* TEST 4: Corrupted Magic Byte Rejection (0x00, 0xFF) */
    EEPROM_Test_CorruptMagic(0x00);
    test4_magic_byte_reject_pass = (EEPROM_ReadAlarm(&h, &m) == 0) ? 1 : 0;
    EEPROM_Test_CorruptMagic(0xFF);
    if (EEPROM_ReadAlarm(&h, &m) != 0) test4_magic_byte_reject_pass = 0;

    /* TEST 5: Out of range and NULL pointer rejection */
    test5_out_of_range_reject_pass = (EEPROM_SaveAlarm(24, 0) == 0 &&
                                      EEPROM_SaveAlarm(0, 60) == 0 &&
                                      EEPROM_ReadAlarm(0, &m) == 0 &&
                                      EEPROM_ReadAlarm(&h, 0) == 0) ? 1 : 0;

    /* =====================================================================
     * SECTION B: Fault Injection Suite (T-EEP-02 / IMP-EEP-01)
     * ===================================================================== */
    fault_pass = 1;

    /* A. Establish baseline (10:20) */
    EEPROM_Test_InjectFailure(0);
    if (EEPROM_SaveAlarm(10, 20) != 1) fault_pass = 0;

    /* B. Stage 1: Failure before invalidate -> Baseline preserved */
    EEPROM_Test_InjectFailure(1);
    if (EEPROM_SaveAlarm(11, 22) != 0) fault_pass = 0;
    if (EEPROM_ReadAlarm(&h, &m) != 1 || h != 10 || m != 20) fault_pass = 0;

    /* C. Stages 2..4: Failure during write -> Invalidation causes Read to fail */
    for (step = 2; step <= 4; step++)
    {
        EEPROM_Test_InjectFailure(step);
        EEPROM_SaveAlarm(12, 30);
        if (EEPROM_ReadAlarm(&h, &m) != 0) fault_pass = 0;
    }

    /* D. Restore clean injection state */
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
    EEPROM_Test_SetMockMode(0); /* Switch to real physical I2C bus */
    EEPROM_Test_InjectFailure(0);
#endif

    delay_ms(50); /* 50ms stabilization delay */

    /* Save 07:30 to physical 24C05 on EVK */
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
        /* Bíp 0.3s báo hiệu 24C05 ghi và đọc thành công */
        GPIO_SetBuzzer(PIN_STATE_HIGH);
        delay_ms(300);
        GPIO_SetBuzzer(PIN_STATE_LOW);
        GPIO_SetLED_D4(PIN_STATE_HIGH);
    }
    else
    {
        GPIO_SetLED_D4(PIN_STATE_LOW);
    }

    while (1)
    {
        WDTR = 0x5A;
    }
}
