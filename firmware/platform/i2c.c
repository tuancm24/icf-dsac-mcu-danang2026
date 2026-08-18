#include "i2c.h"
#include "gpio.h"
#include "timer.h"

/* =========================================================================
 * Platform I2C Bus Implementation (Bit-banging)
 * True Open-Drain / Bus-Release Timing for 24C05 EEPROM (IMP-I2C-01)
 * ========================================================================= */

#define I2C_DELAY_US    (5)

static void I2C_Delay(void)
{
    Timer_DelayUs(I2C_DELAY_US);
}

void I2C_Init(void)
{
    /* Release both SCL and SDA to High-Z (pulled HIGH by external resistors) */
    GPIO_SetI2C_SDA(PIN_STATE_HIGH);
    GPIO_SetI2C_SCL(PIN_STATE_HIGH);
    I2C_Delay();
}

void I2C_Start(void)
{
    /* Ensure bus is released first */
    GPIO_SetI2C_SDA(PIN_STATE_HIGH);
    GPIO_SetI2C_SCL(PIN_STATE_HIGH);
    I2C_Delay();

    /* SDA goes LOW while SCL is HIGH -> START condition */
    GPIO_SetI2C_SDA(PIN_STATE_LOW);
    I2C_Delay();

    /* Pull SCL LOW to hold bus for data transmission */
    GPIO_SetI2C_SCL(PIN_STATE_LOW);
    I2C_Delay();
}

void I2C_Stop(void)
{
    /* Pull SDA LOW while SCL is LOW */
    GPIO_SetI2C_SDA(PIN_STATE_LOW);
    I2C_Delay();

    /* Release SCL to HIGH */
    GPIO_SetI2C_SCL(PIN_STATE_HIGH);
    I2C_Delay();

    /* Release SDA to HIGH while SCL is HIGH -> STOP condition */
    GPIO_SetI2C_SDA(PIN_STATE_HIGH);
    I2C_Delay();
}

unsigned char I2C_WriteByte(unsigned char data_byte)
{
    unsigned char i;
    unsigned char ack;

    for (i = 0; i < 8; i++)
    {
        if (data_byte & 0x80)
        {
            /* Bit 1: Release SDA to High-Z (pulled HIGH by resistor) */
            GPIO_SetI2C_SDA(PIN_STATE_HIGH);
        }
        else
        {
            /* Bit 0: Actively drive SDA LOW */
            GPIO_SetI2C_SDA(PIN_STATE_LOW);
        }
        I2C_Delay();

        /* Pulse SCL HIGH to clock the bit */
        GPIO_SetI2C_SCL(PIN_STATE_HIGH);
        I2C_Delay();

        /* Pull SCL LOW */
        GPIO_SetI2C_SCL(PIN_STATE_LOW);
        data_byte <<= 1;
    }

    /* 9th Clock: Read Slave ACK (SDA released to High-Z) */
    GPIO_SetI2C_SDA(PIN_STATE_HIGH);
    I2C_Delay();

    GPIO_SetI2C_SCL(PIN_STATE_HIGH);
    I2C_Delay();

    /* Sample ACK (Active LOW from Slave) */
    ack = (GPIO_ReadI2C_SDA() == PIN_STATE_LOW) ? 1 : 0;

    GPIO_SetI2C_SCL(PIN_STATE_LOW);
    I2C_Delay();

    return ack;
}

unsigned char I2C_ReadByte(unsigned char send_ack)
{
    unsigned char i;
    unsigned char data_byte = 0;

    /* Release SDA to High-Z for reading from Slave */
    GPIO_SetI2C_SDA(PIN_STATE_HIGH);

    for (i = 0; i < 8; i++)
    {
        data_byte <<= 1;

        GPIO_SetI2C_SCL(PIN_STATE_HIGH);
        I2C_Delay();

        if (GPIO_ReadI2C_SDA() == PIN_STATE_HIGH)
        {
            data_byte |= 0x01;
        }

        GPIO_SetI2C_SCL(PIN_STATE_LOW);
        I2C_Delay();
    }

    /* Send Master ACK (Drive LOW) or NACK (Release HIGH) */
    if (send_ack)
    {
        GPIO_SetI2C_SDA(PIN_STATE_LOW); /* ACK: Drive LOW */
    }
    else
    {
        GPIO_SetI2C_SDA(PIN_STATE_HIGH); /* NACK: Release HIGH */
    }
    I2C_Delay();

    GPIO_SetI2C_SCL(PIN_STATE_HIGH);
    I2C_Delay();

    GPIO_SetI2C_SCL(PIN_STATE_LOW);
    GPIO_SetI2C_SDA(PIN_STATE_HIGH); /* Release SDA after byte */
    I2C_Delay();

    return data_byte;
}
