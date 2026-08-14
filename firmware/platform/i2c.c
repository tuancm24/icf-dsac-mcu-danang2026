#include "i2c.h"
#include "gpio.h"
#include "timer.h"

/* =========================================================================
 * Platform I2C Bus Implementation (Bit-banging)
 * ========================================================================= */

#define I2C_DELAY_US    (5)

static void I2C_Delay(void)
{
    Timer_DelayUs(I2C_DELAY_US);
}

void I2C_Init(void)
{
    GPIO_SetI2C_SDA_Mode(PIN_MODE_OUTPUT_OPENDRAIN);
    GPIO_SetI2C_SCL(PIN_STATE_HIGH);
    GPIO_SetI2C_SDA(PIN_STATE_HIGH);
    I2C_Delay();
}

void I2C_Start(void)
{
    GPIO_SetI2C_SDA(PIN_STATE_HIGH);
    GPIO_SetI2C_SCL(PIN_STATE_HIGH);
    I2C_Delay();
    GPIO_SetI2C_SDA(PIN_STATE_LOW);
    I2C_Delay();
    GPIO_SetI2C_SCL(PIN_STATE_LOW);
    I2C_Delay();
}

void I2C_Stop(void)
{
    GPIO_SetI2C_SDA(PIN_STATE_LOW);
    GPIO_SetI2C_SCL(PIN_STATE_LOW);
    I2C_Delay();
    GPIO_SetI2C_SCL(PIN_STATE_HIGH);
    I2C_Delay();
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
            GPIO_SetI2C_SDA(PIN_STATE_HIGH);
        }
        else
        {
            GPIO_SetI2C_SDA(PIN_STATE_LOW);
        }
        data_byte <<= 1;
        I2C_Delay();
        GPIO_SetI2C_SCL(PIN_STATE_HIGH);
        I2C_Delay();
        GPIO_SetI2C_SCL(PIN_STATE_LOW);
        I2C_Delay();
    }

    /* Read ACK */
    GPIO_SetI2C_SDA(PIN_STATE_HIGH);
    GPIO_SetI2C_SDA_Mode(PIN_MODE_INPUT_PULLUP);
    I2C_Delay();
    GPIO_SetI2C_SCL(PIN_STATE_HIGH);
    I2C_Delay();

    ack = (GPIO_ReadI2C_SDA() == PIN_STATE_LOW) ? 1 : 0;

    GPIO_SetI2C_SCL(PIN_STATE_LOW);
    GPIO_SetI2C_SDA_Mode(PIN_MODE_OUTPUT_OPENDRAIN);
    I2C_Delay();

    return ack;
}

unsigned char I2C_ReadByte(unsigned char send_ack)
{
    unsigned char i;
    unsigned char data_byte = 0;

    GPIO_SetI2C_SDA(PIN_STATE_HIGH);
    GPIO_SetI2C_SDA_Mode(PIN_MODE_INPUT_PULLUP);

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

    /* Send ACK/NACK */
    GPIO_SetI2C_SDA_Mode(PIN_MODE_OUTPUT_OPENDRAIN);
    if (send_ack)
    {
        GPIO_SetI2C_SDA(PIN_STATE_LOW);  /* ACK */
    }
    else
    {
        GPIO_SetI2C_SDA(PIN_STATE_HIGH); /* NACK */
    }
    I2C_Delay();
    GPIO_SetI2C_SCL(PIN_STATE_HIGH);
    I2C_Delay();
    GPIO_SetI2C_SCL(PIN_STATE_LOW);
    GPIO_SetI2C_SDA(PIN_STATE_HIGH);
    I2C_Delay();

    return data_byte;
}
