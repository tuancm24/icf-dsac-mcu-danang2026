#ifndef EEPROM_H
#define EEPROM_H

#include "app_config.h"

/* =========================================================================
 * EEPROM Driver Interface (24C08 on SONiX SN8F5708 EVK)
 * ========================================================================= */

/* Initialize EEPROM / I2C peripheral interface (Idempotent) */
void EEPROM_Init(void);

/* Save confirmed alarm time (hour, minute) to EEPROM with transaction safety */
/* Returns: 1 on success, 0 on I2C/EEPROM error */
unsigned char EEPROM_SaveAlarm(unsigned char hour, unsigned char minute);

/* Read saved alarm time from EEPROM and verify Magic Byte (0xA5) */
/* Returns: 1 if valid alarm data exists, 0 if empty/corrupted/error */
unsigned char EEPROM_ReadAlarm(unsigned char *hour, unsigned char *minute);

#endif /* EEPROM_H */
