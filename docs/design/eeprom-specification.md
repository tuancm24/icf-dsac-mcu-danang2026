# EEPROM Driver Specification

## 1. Purpose

This document defines the requirements, architecture, behavior, and public interface of the I2C EEPROM Driver for the SONiX SN8F5708 EVK digital clock application.

The driver provides non-volatile storage and retrieval of confirmed alarm time (`hour` and `minute`) on an external I2C EEPROM (`24C05`), with data validation via a dedicated magic byte and 4-step transaction safety.

---

## 2. Scope

This specification covers:
- Hardware target: 24C05 I2C EEPROM on SONiX SN8F5708 EVK (U6 / Header J6).
- Source files:
  - `firmware/drivers/eeprom.c` / `eeprom.h`
  - `firmware/platform/i2c.c` / `i2c.h`
- Verification files:
  - `docs/testing/eeprom/README.md`

---

## 3. Responsibilities

### EEPROM Driver Owns:
- Writing alarm hour (`0x00`), alarm minute (`0x01`), and magic byte (`0x02` = `0xA5`) to EEPROM using transaction-safe ordering.
- Reading and validating stored alarm parameters upon system startup.
- I2C bus transaction protocol execution (Start, Stop, Byte write, Random read, Acknowledge) with open-drain release semantics.

### EEPROM Driver Does NOT Own:
- Deciding when the user confirms an alarm (owned by Application FSM).
- Clock timekeeping or alarm matching (owned by Clock & Alarm Core).

---

## 4. Architecture

```text
+-----------------------------------------------------------+
|                     Application Layer                     |
|         (EEPROM_SaveAlarm, EEPROM_ReadAlarm)              |
+-----------------------------------------------------------+
                              |
                              v
+-----------------------------------------------------------+
|                    EEPROM Driver Layer                    |
|  - 4-step Transaction Safety (Invalidate -> Data -> A5)   |
|  - Data Range & Magic Byte Validation                     |
+-----------------------------------------------------------+
                              |
                              v
+-----------------------------------------------------------+
|                     Platform I2C Layer                    |
|       (I2C_Start, I2C_Stop, I2C_WriteByte, I2C_ReadByte)  |
+-----------------------------------------------------------+
                              |
                              v
+-----------------------------------------------------------+
|                 24C05 I2C EEPROM Hardware                 |
|                   (SCL: P1.4, SDA: P1.5)                  |
+-----------------------------------------------------------+
```

---

## 5. Public Interface

```c
#ifndef EEPROM_H
#define EEPROM_H

#include "app_config.h"

void EEPROM_Init(void);
unsigned char EEPROM_SaveAlarm(unsigned char hour, unsigned char minute);
unsigned char EEPROM_ReadAlarm(unsigned char *hour, unsigned char *minute);

#endif /* EEPROM_H */
```

---

## 6. Functional Behavior & Transaction Safety (IMP-EEP-01)

1. **Save Alarm Time (`EEPROM_SaveAlarm`):**
   - Step 1: Invalidate Magic Byte (writes `0x00` to `0x02`).
   - Step 2: Write `hour` to address `0x00`.
   - Step 3: Write `minute` to address `0x01`.
   - Step 4: Write Magic Byte `0xA5` to address `0x02`.
   - Returns `1` on success, `0` on I2C failure or invalid parameters.
2. **Read Alarm Time (`EEPROM_ReadAlarm`):**
   - Reads Magic Byte at `0x02`. If not `0xA5`, returns `0` (Unconfigured / Incomplete).
   - Reads `hour` and `minute`. If ranges are valid (`hour <= 23`, `minute <= 59`), populates output pointers and returns `1`.

---

## 7. Initialization

Immediately after `Board_Init()` / `EEPROM_Init()`:
- I2C bus lines (`SCL`, `SDA`) released to idle `HIGH` state (High-Z).
