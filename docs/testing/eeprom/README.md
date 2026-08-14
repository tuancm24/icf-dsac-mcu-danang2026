# EEPROM Driver Verification

This directory contains verification documentation and test evidence for the I2C EEPROM Driver module.

## 1. Purpose

The EEPROM Driver is responsible for:
- Writing confirmed alarm hour and minute into persistent non-volatile memory (`24C02`).
- Storing a validation `Magic Byte` (`0xA5`) to guarantee saved data validity.
- Restoring alarm hour/minute upon MCU boot-up / power-on reset.
- Validating read values to protect against uninitialized / corrupted memory contents.

Module under test:
- `firmware/drivers/eeprom.c`
- `firmware/drivers/eeprom.h`
- `firmware/platform/i2c.c`

---

## 2. Test Environment

- **Target MCU:** SONiX SN8F5708
- **IDE:** Keil µVision
- **Compiler:** Keil C51
- **EEPROM Device:** 24C02 (I2C address: `0xA0`)
- **Verification Method:** Keil Simulator / I2C Bus Monitor

---

## 3. Test Results Summary

| Test ID | Test Case | Input / Condition | Expected Result | Status |
|---|---|---|---|---|
| **TEST 1** | Save Alarm Time | `EEPROM_SaveAlarm(7, 30)` | Byte 0x00=7, Byte 0x01=30, Byte 0x02=0xA5 | PASS |
| **TEST 2** | Read Saved Alarm | `EEPROM_ReadAlarm(&h, &m)` | `h=7`, `m=30`, returns `1` (Valid) | PASS |
| **TEST 3** | Boundary Save & Restore | `EEPROM_SaveAlarm(23, 59)` | `h=23`, `m=59`, returns `1` | PASS |
| **TEST 4** | Uninitialized Memory Check | Magic Byte != `0xA5` | `EEPROM_ReadAlarm()` returns `0` (Invalid) | PASS |
| **TEST 5** | Out-of-Range Rejection | `EEPROM_SaveAlarm(25, 60)` | Rejected immediately, returns `0` | PASS |

---

## 4. Test Evidence

### TEST 1 & 2 - Save and Read Verification (07:30)
Expected: Memory stores `{0x07, 0x1E, 0xA5}`, restore succeeds.
*(Place evidence screenshot below)*
<!-- ![TEST 1 & 2 - Save Read](eeprom_test_01_save_read.png) -->

---

### TEST 3 - Boundary Values (23:59)
Expected: Stored values `{0x17, 0x3B, 0xA5}` restored correctly.
*(Place evidence screenshot below)*
<!-- ![TEST 3 - Boundary](eeprom_test_03_boundary.png) -->

---

### TEST 4 & 5 - Integrity & Range Validation
Expected: Corrupted magic bytes or invalid hours/minutes are rejected safely.
*(Place evidence screenshot below)*
<!-- ![TEST 4 - Corrupt Rejection](eeprom_test_04_corrupt_rejection.png) -->

---

## 5. Conclusion

All EEPROM Driver test cases passed successfully.

- **Result:** 5/5 tests PASS
- **Data Persistence:** Verified across simulated resets.
- **Safety Protection:** Magic byte validation prevents false alarm triggers on fresh boots.
