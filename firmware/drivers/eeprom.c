#include "eeprom.h"
#include "i2c.h"
#include "timer.h"
#include "pin_config.h"
#include "app_config.h"

/* =========================================================================
 * EEPROM Driver Implementation (24C05 on SONiX SN8F5708 EVK)
 * Transaction-Safe Persistence (IMP-EEP-01 / Contract v2.7)
 * ========================================================================= */

#ifndef EEPROM_I2C_DEV_ADDR
#define EEPROM_I2C_DEV_ADDR         (0xA0)
#endif

#ifndef EEPROM_MAGIC_VALUE
#define EEPROM_MAGIC_VALUE          (0xA5)
#endif

#define EEPROM_DEV_ADDR_WRITE       (EEPROM_I2C_DEV_ADDR & 0xFE)
#define EEPROM_DEV_ADDR_READ        (EEPROM_I2C_DEV_ADDR | 0x01)
#define EEPROM_WRITE_DELAY_MS       (10)

/* Simulation / Test Mock Hooks (Active when TEST_BUILD is defined) */
#ifdef TEST_BUILD
static unsigned char s_mock_eeprom_storage[4] = {0xFF, 0xFF, 0xFF, 0xFF};
static unsigned char s_mock_enabled = 0;
static unsigned char s_inject_fail_step = 0; /* 0 = none, 1..4 = fail at step */

void EEPROM_Test_SetMockMode(unsigned char enable)
{
    s_mock_enabled = enable;
}

void EEPROM_Test_InjectFailure(unsigned char step)
{
    s_inject_fail_step = step;
}

void EEPROM_Test_CorruptMagic(unsigned char bad_magic)
{
    s_mock_eeprom_storage[EEPROM_ADDR_MAGIC_BYTE] = bad_magic;
}
#endif

static unsigned char EEPROM_WriteByte(unsigned char mem_addr, unsigned char data_val)
{
#ifdef TEST_BUILD
    if (s_mock_enabled)
    {
        if (mem_addr < 4)
        {
            s_mock_eeprom_storage[mem_addr] = data_val;
            return 1;
        }
        return 0;
    }
#endif

    {
        unsigned char ok = 1;

        I2C_Start();
        if (!I2C_WriteByte(EEPROM_DEV_ADDR_WRITE))
        {
            ok = 0;
        }
        if (ok && !I2C_WriteByte(mem_addr))
        {
            ok = 0;
        }
        if (ok && !I2C_WriteByte(data_val))
        {
            ok = 0;
        }
        I2C_Stop();

        /* Wait for EEPROM internal write cycle (tWR ≈ 5ms) */
        Timer_DelayUs(EEPROM_WRITE_DELAY_MS * 1000U);

        return ok;
    }
}

static unsigned char EEPROM_ReadByte(unsigned char mem_addr, unsigned char *data_out)
{
#ifdef TEST_BUILD
    if (s_mock_enabled)
    {
        if (mem_addr < 4 && data_out != 0)
        {
            *data_out = s_mock_eeprom_storage[mem_addr];
            return 1;
        }
        return 0;
    }
#endif

    {
        unsigned char ok = 1;

        I2C_Start();
        /* Dummy write to set internal word address */
        if (!I2C_WriteByte(EEPROM_DEV_ADDR_WRITE))
        {
            ok = 0;
        }
        if (ok && !I2C_WriteByte(mem_addr))
        {
            ok = 0;
        }

        if (ok)
        {
            /* Restart for read sequence */
            I2C_Start();
            if (!I2C_WriteByte(EEPROM_DEV_ADDR_READ))
            {
                ok = 0;
            }
            if (ok)
            {
                *data_out = I2C_ReadByte(0); /* NACK after single byte read */
            }
        }
        I2C_Stop();

        return ok;
    }
}

void EEPROM_Init(void)
{
    I2C_Init();
}

unsigned char EEPROM_SaveAlarm(unsigned char hour, unsigned char minute)
{
    /* Reject invalid time bounds */
    if (hour > 23 || minute > 59)
    {
        return 0;
    }

#ifdef TEST_BUILD
    if (s_inject_fail_step == 1) return 0;
#endif

    /* Step 1: Invalidate existing magic marker first */
    if (!EEPROM_WriteByte(EEPROM_ADDR_MAGIC_BYTE, 0x00))
    {
        return 0;
    }

#ifdef TEST_BUILD
    if (s_inject_fail_step == 2) return 0;
#endif

    /* Step 2: Write hour */
    if (!EEPROM_WriteByte(EEPROM_ADDR_ALARM_HOUR, hour))
    {
        return 0;
    }

#ifdef TEST_BUILD
    if (s_inject_fail_step == 3) return 0;
#endif

    /* Step 3: Write minute */
    if (!EEPROM_WriteByte(EEPROM_ADDR_ALARM_MINUTE, minute))
    {
        return 0;
    }

#ifdef TEST_BUILD
    if (s_inject_fail_step == 4) return 0;
#endif

    /* Step 4: Write valid magic marker (0xA5) last */
    if (!EEPROM_WriteByte(EEPROM_ADDR_MAGIC_BYTE, EEPROM_MAGIC_VALUE))
    {
        return 0;
    }

    return 1;
}

unsigned char EEPROM_ReadAlarm(unsigned char *hour, unsigned char *minute)
{
    unsigned char magic = 0;
    unsigned char h = 0;
    unsigned char m = 0;

    if (!hour || !minute)
    {
        return 0;
    }

    /* 1. Read magic validation byte */
    if (!EEPROM_ReadByte(EEPROM_ADDR_MAGIC_BYTE, &magic) || magic != EEPROM_MAGIC_VALUE)
    {
        return 0;
    }

    /* 2. Read saved hour and minute */
    if (!EEPROM_ReadByte(EEPROM_ADDR_ALARM_HOUR, &h) ||
        !EEPROM_ReadByte(EEPROM_ADDR_ALARM_MINUTE, &m))
    {
        return 0;
    }

    /* 3. Validate bounds */
    if (h > 23 || m > 59)
    {
        return 0;
    }

    *hour = h;
    *minute = m;
    return 1;
}
