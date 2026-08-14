# EEPROM Driver Specification

## 1. Purpose

This document defines the requirements, architecture, behavior, and public interface of the I2C EEPROM Driver for the SONiX SN8F5708 EVK digital clock application.

The driver provides non-volatile storage and retrieval of confirmed alarm time (`hour` and `minute`) on an external I2C EEPROM (`24C02`), with data validation via a dedicated magic byte.

---

## 2. Scope

This specification covers:
- Hardware target: 24C02 I2C EEPROM on SONiX SN8F5708 EVK.
- Source files:
  - `firmware/drivers/eeprom.c` / `eeprom.h`
  - `firmware/platform/i2c.c` / `i2c.h`
- Verification files:
  - `docs/testing/eeprom/README.md`

---

## 3. Responsibilities

### EEPROM Driver Owns:
- Writing alarm hour (`0x00`), alarm minute (`0x01`), and magic byte (`0x02` = `0xA5`) to EEPROM.
- Reading and validating stored alarm parameters upon system startup.
- I2C bus transaction protocol execution (Start, Stop, Byte write, Random read, Acknowledge).

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
|  - Memory Map Management (0x00: Hour, 0x01: Min, 0x02: A5)|
|  - Data Range & Magic Byte Validation                     |
+-----------------------------------------------------------+
                              |
                              v
+-----------------------------------------------------------+
|                     Platform I2C Layer                    |
|          (I2C_Start, I2C_Stop, I2C_Write, I2C_Read)       |
+-----------------------------------------------------------+
                              |
                              v
+-----------------------------------------------------------+
|                 24C02 I2C EEPROM Hardware                 |
|                       (SCL, SDA)                          |
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

## 6. Functional Behavior

1. **Save Alarm Time (`EEPROM_SaveAlarm`):**
   - Writes `hour` to address `0x00`.
   - Writes `minute` to address `0x01`.
   - Writes Magic Byte `0xA5` to address `0x02`.
   - Returns `1` on success, `0` on I2C failure or invalid parameters.
2. **Read Alarm Time (`EEPROM_ReadAlarm`):**
   - Reads Magic Byte at `0x02`. If not `0xA5`, returns `0` (Unconfigured).
   - Reads `hour` and `minute`. If ranges are valid (`hour <= 23`, `minute <= 59`), populates pointers and returns `1`.

---

## 7. Initialization

Immediately after `EEPROM_Init()`:
- I2C bus lines (`SCL`, `SDA`) initialized to idle `HIGH` state.

---

## 8. Input / Output Behavior

| Function | Input | Stored Data | Return |
|---|---|---|---|
| `EEPROM_SaveAlarm(7, 30)` | `h=7, m=30` | `[0x00]=7, [0x01]=30, [0x02]=0xA5` | `1` (Success) |
| `EEPROM_SaveAlarm(25, 70)`| `h=25, m=70`| None | `0` (Rejected) |
| `EEPROM_ReadAlarm(&h, &m)`| Magic `0xA5` | `h=7, m=30` | `1` (Valid) |
| `EEPROM_ReadAlarm(&h, &m)`| Magic `0xFF` | None | `0` (Invalid) |

---

## 9. Boundary / Invalid Conditions

- Out-of-range hours (`> 23`) or minutes (`> 59`) rejected immediately.
- Corrupted magic byte indicates uninitialized memory, preventing spurious alarms.

---

## 10. Software Verification

Verified via Keil C51 Simulator:
- `TEST 1`: Save valid alarm time (`07:30`).
- `TEST 2`: Read back saved alarm time (`07:30`).
- `TEST 3`: Boundary values save and restore (`23:59`).
- `TEST 4`: Magic byte mismatch rejection.
- `TEST 5`: Out-of-range rejection.

---

## 11. Hardware Verification Status

> Hardware verification has not yet been performed because the SN8F5708 EVK is currently unavailable for individual testing. Current verification is limited to software/simulator-level behavior.

---

## 12. Integration Notes

- Write cycle delay (5-10ms) is automatically handled between sequential byte writes.

---

## 13. Known Limitations

- Standard 24C02 byte-write mode is used; page write is unnecessary for 3 bytes.
