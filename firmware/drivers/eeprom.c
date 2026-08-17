#include "eeprom.h"
#include "i2c.h"
#include "timer.h"
#include "pin_config.h"
#include "app_config.h"

/* =========================================================================
 * EEPROM Driver Implementation (24C08 on SONiX SN8F5708 EVK)
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

static unsigned char EEPROM_WriteByte(unsigned char mem_addr, unsigned char data_val)
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

static unsigned char EEPROM_ReadByte(unsigned char mem_addr, unsigned char *data_out)
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

void EEPROM_Init(void)
{
    I2C_Init();
}

unsigned char EEPROM_SaveAlarm(unsigned char hour, unsigned char minute)
{
    if (hour > 23 || minute > 59)
    {
        return 0; /* Reject invalid time bounds */
    }

    if (!EEPROM_WriteByte(EEPROM_ADDR_ALARM_HOUR, hour))
    {
        return 0;
    }
    if (!EEPROM_WriteByte(EEPROM_ADDR_ALARM_MINUTE, minute))
    {
        return 0;
    }
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

    /* Check magic byte first to verify validity */
    if (!EEPROM_ReadByte(EEPROM_ADDR_MAGIC_BYTE, &magic))
    {
        return 0;
    }
    if (magic != EEPROM_MAGIC_VALUE)
    {
        return 0; /* Uninitialized / corrupted EEPROM data */
    }

    if (!EEPROM_ReadByte(EEPROM_ADDR_ALARM_HOUR, &h))
    {
        return 0;
    }
    if (!EEPROM_ReadByte(EEPROM_ADDR_ALARM_MINUTE, &m))
    {
        return 0;
    }

    if (h > 23 || m > 59)
    {
        return 0;
    }

    *hour = h;
    *minute = m;

    return 1;
}
