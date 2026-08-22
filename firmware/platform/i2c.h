#ifndef I2C_H
#define I2C_H

#include "board_config.h"

/* =========================================================================
 * Platform I2C Bus Driver Interface (SONiX SN8F5708)
 * Bit-banging implementation for I2C EEPROM (24Cxx)
 * ========================================================================= */

/* Initialize I2C SCL & SDA lines (High impedance / Pull-up) */
void I2C_Init(void);

/* Generate I2C START condition */
void I2C_Start(void);

/* Generate I2C STOP condition */
void I2C_Stop(void);

/* Write one byte to I2C bus. Returns: 1 = ACK received, 0 = NACK */
unsigned char I2C_WriteByte(unsigned char data_byte);

/* Read one byte from I2C bus. ack: 1 = send ACK, 0 = send NACK */
unsigned char I2C_ReadByte(unsigned char send_ack);

#endif /* I2C_H */
