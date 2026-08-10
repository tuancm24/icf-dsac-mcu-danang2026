# Clock Core Verification

This directory contains verification evidence for the Clock Core module.

## Test Environment

- Target MCU: SONiX SN8F5708
- Compiler: Keil C51
- IDE: Keil µVision
- Verification method: Keil Simulator
- Module under test:
  - `firmware/app/clock.c`
  - `firmware/app/clock.h`
- Test source:
  - `firmware/app/clock_test.c`

## Test Results

| ID | Test Case | Expected Result | Result |
|---|---|---|---|
| TEST 1 | Clock initialization | `00:00:00` | PASS |
| TEST 2 | 60 second rollover | `00:01:00` | PASS |
| TEST 3 | Hour decrement from 00 | `23:00:00` | PASS |
| TEST 4 | Hour increment from 23 | `00:00:00` | PASS |
| TEST 5 | Minute decrement from 00 | `00:59:00` | PASS |
| TEST 6 | Minute increment from 59 | `00:00:00` | PASS |
| TEST 7 | Full-day rollover: `23:59:59 + 1 second` | `00:00:00` | PASS |

## Evidence

- `test01_init_000000.png`
- `test02_60sec_000100.png`
- `test03_hour_decrement_230000.png`
- `test04_hour_increment_rollover_000000.png`
- `test05_minute_decrement_005900.png`
- `test06_minute_increment_rollover_000000.png`
- `boundary_before_235959.png`
- `boundary_after_000000.png`

> Keil Watch Window displays `unsigned char` values in hexadecimal.
> Therefore, `0x17 = 23` and `0x3B = 59`.