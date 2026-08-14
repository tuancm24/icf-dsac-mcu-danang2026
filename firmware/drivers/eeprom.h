#ifndef EEPROM_H
#define EEPROM_H

#include "app_config.h"

/* =========================================================================
 * EEPROM Driver Interface (24C02 / 24Cxx on SONiX SN8F5708 EVK)
 * ========================================================================= */

/* Initialize EEPROM / I2C peripheral interface */
void EEPROM_Init(void);

/* Save confirmed alarm time (hour, minute) to EEPROM */
/* Returns: 1 on success, 0 on I2C/EEPROM error */
unsigned char EEPROM_SaveAlarm(unsigned char hour, unsigned char minute);

/* Read saved alarm time from EEPROM */
/* Returns: 1 if valid alarm data exists, 0 if empty/corrupted/error */
unsigned char EEPROM_ReadAlarm(unsigned char *hour, unsigned char *minute);

#endif /* EEPROM_H */
