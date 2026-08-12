# Application FSM Verification

This directory contains verification evidence for the Application FSM module.

## Test Environment

- Target MCU: SONiX SN8F5708
- Compiler: Keil C51
- IDE: Keil µVision
- Verification method: Keil Simulator
- Module under test:
  - `firmware/app/fsm.c`
  - `firmware/app/fsm.h`
- Test source:
  - `firmware/app/fsm_test.c`

## Test Results

| ID | Test Case | Expected Result | Result |
|---|---|---|---|
| TEST 1 | FSM initialization | `FSM_NORMAL` | PASS |
| TEST 2 | NORMAL + SW3 | `FSM_SET_TIME_HOUR` | PASS |
| TEST 3 | SET_TIME_HOUR + SW6 | Hour +1, state remains `FSM_SET_TIME_HOUR` | PASS |
| TEST 4 | SET_TIME_HOUR + SW10 | Hour -1, state remains `FSM_SET_TIME_HOUR` | PASS |
| TEST 5 | SET_TIME_HOUR + SW3 | `FSM_SET_TIME_MINUTE` | PASS |
| TEST 6 | SET_TIME_MINUTE + SW6 | Minute +1, state remains `FSM_SET_TIME_MINUTE` | PASS |
| TEST 7 | SET_TIME_MINUTE + SW10 | Minute -1, state remains `FSM_SET_TIME_MINUTE` | PASS |
| TEST 8 | SET_TIME_MINUTE + SW3 | `FSM_NORMAL` | PASS |
| TEST 9 | NORMAL + SW16 | `FSM_SET_ALARM_HOUR` | PASS |
| TEST 10 | SET_ALARM_HOUR + SW16 | `FSM_SET_ALARM_MINUTE` | PASS |
| TEST 11 | SET_ALARM_MINUTE + SW16 | `FSM_NORMAL` | PASS |
| TEST 12 | Timeout from SET_TIME_HOUR | `FSM_NORMAL` | PASS |
| TEST 13 | Timeout from SET_TIME_MINUTE | `FSM_NORMAL` | PASS |
| TEST 14 | Timeout from SET_ALARM_HOUR | `FSM_NORMAL` | PASS |
| TEST 15 | Timeout from SET_ALARM_MINUTE | `FSM_NORMAL` | PASS |
| TEST 16 | Undefined event: NORMAL + SW6 | State remains `FSM_NORMAL` | PASS |
| TEST 17 | Undefined event: NORMAL + SW10 | State remains `FSM_NORMAL` | PASS |

## Evidence

- `fsm-test-01-init-normal.png`
- `fsm-test-02-normal-to-set-time-hour.png`
- `fsm-test-03-set-time-hour-sw6-increment.png`
- `fsm-test-04-set-time-hour-sw10-decrement.png`
- `fsm-test-05-set-time-hour-to-minute.png`
- `fsm-test-06-set-time-minute-sw6-increment.png`
- `fsm-test-07-set-time-minute-sw10-decrement.png`
- `fsm-test-08-set-time-minute-to-normal.png`
- `fsm-test-09-normal-to-set-alarm-hour.png`
- `fsm-test-10-set-alarm-hour-to-minute.png`
- `fsm-test-11-set-alarm-minute-to-normal.png`
- `fsm-test-12-timeout-set-time-hour-to-normal.png`
- `fsm-test-13-timeout-set-time-minute-to-normal.png`
- `fsm-test-14-timeout-set-alarm-hour-to-normal.png`
- `fsm-test-15-timeout-set-alarm-minute-to-normal.png`
- `fsm-test-16-undefined-normal-sw6.png`
- `fsm-test-17-undefined-normal-sw10.png`

## Notes

- FSM state transitions were verified using `test_state` in the Keil Watch Window.
- Clock adjustment behavior was verified using `test_time`.
- Alarm Core and EEPROM integration are not verified by this test because they are not implemented yet.
- Hardware button integration will be verified during system integration.