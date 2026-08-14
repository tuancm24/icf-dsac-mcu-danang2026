#include "eeprom.h"
#include "i2c.h"
#include "timer.h"

/* =========================================================================
 * EEPROM Driver Implementation (24C02 / 24Cxx on SONiX SN8F5708 EVK)
 * ========================================================================= */

#define EEPROM_DEV_ADDR_WRITE   (EEPROM_I2C_DEV_ADDR & 0xFE)
#define EEPROM_DEV_ADDR_READ    (EEPROM_I2C_DEV_ADDR | 0x01)
#define EEPROM_WRITE_DELAY_MS   (10)

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
        else
        {
            *data_out = I2C_ReadByte(0); /* Read 1 byte with NACK */
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
        return 0;
    }

    /* 1. Save Hour */
    if (!EEPROM_WriteByte(EEPROM_ADDR_ALARM_HOUR, hour))
    {
        return 0;
    }

    /* 2. Save Minute */
    if (!EEPROM_WriteByte(EEPROM_ADDR_ALARM_MINUTE, minute))
    {
        return 0;
    }

    /* 3. Save Magic Byte to validate data persistence */
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

    if (hour == 0 || minute == 0)
    {
        return 0;
    }

    /* 1. Check Magic Byte */
    if (!EEPROM_ReadByte(EEPROM_ADDR_MAGIC_BYTE, &magic))
    {
        return 0;
    }
    if (magic != EEPROM_MAGIC_VALUE)
    {
        return 0; /* EEPROM has no valid configured alarm */
    }

    /* 2. Read Hour & Minute */
    if (!EEPROM_ReadByte(EEPROM_ADDR_ALARM_HOUR, &h) ||
        !EEPROM_ReadByte(EEPROM_ADDR_ALARM_MINUTE, &m))
    {
        return 0;
    }

    /* 3. Validate Range */
    if (h <= 23 && m <= 59)
    {
        *hour = h;
        *minute = m;
        return 1;
    }

    return 0;
}
