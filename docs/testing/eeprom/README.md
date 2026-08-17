# EEPROM Driver Verification

This directory contains verification documentation and test evidence for the I2C EEPROM Driver module.

## 1. Purpose

The EEPROM Driver is responsible for:
- Writing confirmed alarm hour and minute into persistent non-volatile memory (`24C08`).
- Storing a validation `Magic Byte` (`0xA5`) to guarantee saved data validity.
- Restoring alarm hour/minute upon MCU boot-up / power-on reset.
- Validating read values to protect against uninitialized / corrupted memory contents.

Module under test:
- `firmware/drivers/eeprom.c`
- `firmware/drivers/eeprom.h`
- `firmware/platform/i2c.c`

Test source:
- `firmware/drivers/eeprom_test.c`

---

## 2. Test Environment

- **Target MCU:** SONiX SN8F5708
- **IDE:** Keil µVision
- **Compiler:** Keil C51
- **EEPROM Device:** 24C08 (I2C address: `0xA0`)
- **Verification Method:** Keil Simulator / I2C Bus Monitor

---

## 3. Test Results Summary

| Test ID | Test Case | Input / Condition | Expected Result | Status |
|---|---|---|---|---|
| **TEST 1** | Save Alarm Time | `EEPROM_SaveAlarm(7, 30)` | Byte 0x00=7, Byte 0x01=30, Byte 0x02=0xA5 | PASS (Simulator) |
| **TEST 2** | Read Saved Alarm | `EEPROM_ReadAlarm(&h, &m)` | `h=7`, `m=30`, returns `1` (Valid) | PASS (Simulator) |
| **TEST 3** | Boundary Save & Restore | `EEPROM_SaveAlarm(23, 59)` | `h=23`, `m=59`, returns `1` | PASS (Simulator) |
| **TEST 4** | Uninitialized Memory Check | Magic Byte != `0xA5` | `EEPROM_ReadAlarm()` returns `0` (Invalid) | PASS (Simulator) |
| **TEST 5** | Out-of-Range Rejection | `EEPROM_SaveAlarm(25, 60)` | Rejected immediately, returns `0` | PASS (Simulator) |

---

## 4. Test Evidence

### Software Simulation Verification
- Test source: `firmware/drivers/eeprom_test.c`
- Input parameter bounds checking and byte map handling verified via Keil C51 simulation.

### Hardware Verification Status
- **Status:** **Pending hardware validation**
- **Note:** Physical I2C EEPROM (24C08) write cycle and data retention verification on board will be conducted during application integration.

---

## 5. Conclusion

- **Software Simulator Verification:** PASS (5/5 tests logic verified)
- **Hardware Verification:** Pending hardware validation
