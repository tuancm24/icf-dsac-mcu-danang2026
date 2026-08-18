# EEPROM Driver Verification

This directory contains verification documentation and test evidence for the I2C EEPROM Driver module.

## 1. Purpose

The EEPROM Driver is responsible for:
- Writing confirmed alarm hour and minute into persistent non-volatile memory (`24C05`).
- Storing a validation `Magic Byte` (`0xA5`) to guarantee saved data validity.
- Restoring alarm hour/minute upon MCU boot-up / power-on reset.
- Validating read values to protect against uninitialized / corrupted memory contents.
- Ensuring transaction-safety via a 4-step write sequence (invalidate marker -> hour -> minute -> valid marker).

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
- **EEPROM Device:** 24C05 (Fitted EVK part, I2C address: `0xA0`, SCL on `P1.4`, SDA on `P1.5`)
- **Verification Method:** Keil Simulator (SW-PASS) / Hardware EVK Board (HW-PENDING)

---

## 3. Test Results Summary

| Test ID | Test Case | Global Variable | Observed Value | Status |
|---|---|---|:---:|:---:|
| **T-EEP-01 (1)** | Save Alarm Time (06:45) | `test1_save_alarm_pass` | `0x01` | **SW-PASS** |
| **T-EEP-01 (2)** | Read Saved Alarm & Compare | `test2_read_alarm_pass` | `0x01` | **SW-PASS** |
| **T-EEP-01 (3)** | Boundary Vectors (00:00 & 23:59) | `test3_boundary_pass` | `0x01` | **SW-PASS** |
| **T-EEP-01 (4)** | Corrupted Magic Byte Rejection (0x00, 0xFF) | `test4_magic_byte_reject_pass` | `0x01` | **SW-PASS** |
| **T-EEP-01 (5)** | Out-of-Range & NULL Pointer Rejection | `test5_out_of_range_reject_pass` | `0x01` | **SW-PASS** |
| **T-EEP-02** | Transaction Fault Injection (Stages 1..4) | `test6_fault_injection_pass` | `0x01` | **SW-PASS** |

---

## 4. Test Evidence

### Software Simulation Verification Evidence
The Keil C51 Simulator confirms all assertions and fault-injection stages passed:

```text
Name                                Value     Type
------------------------------------------------------
test1_save_alarm_pass               0x01      uchar
test2_read_alarm_pass               0x01      uchar
test3_boundary_pass                 0x01      uchar
test4_magic_byte_reject_pass        0x01      uchar
test5_out_of_range_reject_pass      0x01      uchar
test6_fault_injection_pass          0x01      uchar
```

### Hardware Board Status
- **Target Hardware:** SONiX 5708_EVK-V1.0 Board (24C05 chip)
- **Status:** **HW-PENDING** (Pending final physical retention / reboot evidence logging).

---

## 5. Conclusion

- **Software Simulator Verification:** **SW-PASS (6/6 assertions passed - 100%)**
- **Hardware Board Verification:** **HW-PENDING**
