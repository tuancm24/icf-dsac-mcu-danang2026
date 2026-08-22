# Application FSM Specification

## 1. Purpose

This document defines the Finite State Machine (FSM) for the MCU digital clock application.

The FSM is derived from the official MCU preliminary-round requirements and is used as the application-level behavioral design reference for:

- Application state management
- Clock setting flow
- Alarm setting flow
- 7-segment display behavior associated with application states
- LED D4 behavior associated with application states
- Buzzer requests associated with button/alarm behavior
- 30-second timeout handling
- EEPROM save trigger
- FSM implementation and verification

The FSM specification defines required/target application behavior and module interactions. It does not imply that `fsm.c` directly owns the display, LED, buzzer, EEPROM, timer, or Alarm Core implementation.

Main development flow:

```text
Requirement
    ↓
FSM Specification
    ↓
fsm.h / fsm.c
    ↓
fsm_test.c
    ↓
Keil Simulator Verification
```

### 1.1 Specification and implementation status

This document intentionally describes the **target application behavior**, including interactions that are implemented outside the standalone FSM core.

At the current project stage:

- `fsm.c` implements the five FSM states, state transitions, current-clock adjustment calls, timeout transitions, and the current team rule for unspecified button combinations.
- Alarm-setting state transitions are implemented in `fsm.c`.
- Alarm edit-buffer manipulation, final `Alarm_SetTime(...)` confirmation, EEPROM save requests, display/D4 output, button buzzer requests, and alarm-trigger sequencing belong to later Application Integration and/or driver work.
- Therefore, a behavior shown in the transition table may be a target integration behavior even when the standalone `fsm.c` unit does not yet execute that action.

The verification section at the end of this document distinguishes currently verified FSM-core behavior from later integration verification.

---

## 2. FSM States

| State | Meaning |
|---|---|
| `NORMAL` | Normal clock display mode |
| `SET_TIME_HOUR` | Set current clock hour |
| `SET_TIME_MINUTE` | Set current clock minute |
| `SET_ALARM_HOUR` | Set alarm hour |
| `SET_ALARM_MINUTE` | Set alarm minute |

---

## 3. Detailed FSM State Transition Table

The table below is the **target application-level transition/output contract**. State changes and Clock Core calls that already exist in `fsm.c` are current FSM-core behavior. Alarm edit-buffer actions, EEPROM requests, and physical output behavior are integration responsibilities unless explicitly implemented in the FSM core.

| Current State | Event | Next State | Action / Data | 7SEG Display | LED D4 | Buzzer |
|---|---|---|---|---|---|---|
| `NORMAL` | `SW3` | `SET_TIME_HOUR` | Enter current-hour setting mode | Current **HH** blinks: 0.5s ON / 0.5s OFF. **MM** remains visible | OFF | Pip 0.3s |
| `SET_TIME_HOUR` | `SW3` | `SET_TIME_MINUTE` | Enter current-minute setting mode | Current **MM** blinks: 0.5s ON / 0.5s OFF. **HH** remains visible | OFF | Pip 0.3s |
| `SET_TIME_MINUTE` | `SW3` | `NORMAL` | Exit current-time setting mode | Normal `HH.MM` display | OFF | Pip 0.3s |
| `NORMAL` | `SW16` | `SET_ALARM_HOUR` | Enter alarm-hour setting mode; initialize temporary alarm edit value from confirmed alarm | Alarm **HH** blinks: 0.5s ON / 0.5s OFF. Alarm **MM** remains visible | Blink: 0.5s ON / 0.5s OFF | Pip 0.3s |
| `SET_ALARM_HOUR` | `SW16` | `SET_ALARM_MINUTE` | Enter alarm-minute setting mode | Alarm **MM** blinks: 0.5s ON / 0.5s OFF. Alarm **HH** remains visible | Blink: 0.5s ON / 0.5s OFF | Pip 0.3s |
| `SET_ALARM_MINUTE` | `SW16` | `NORMAL` | Confirm edited alarm hour/minute; request EEPROM save; return to normal mode | Return to normal `HH.MM` display | OFF | Pip 0.3s |
| `SET_TIME_HOUR` | `SW6` | `SET_TIME_HOUR` | Current hour +1; wrap `23 → 00` | Updated **HH** continues blinking | OFF | Pip 0.3s |
| `SET_TIME_HOUR` | `SW10` | `SET_TIME_HOUR` | Current hour -1; wrap `00 → 23` | Updated **HH** continues blinking | OFF | Pip 0.3s |
| `SET_TIME_MINUTE` | `SW6` | `SET_TIME_MINUTE` | Current minute +1; wrap `59 → 00` | Updated **MM** continues blinking | OFF | Pip 0.3s |
| `SET_TIME_MINUTE` | `SW10` | `SET_TIME_MINUTE` | Current minute -1; wrap `00 → 59` | Updated **MM** continues blinking | OFF | Pip 0.3s |
| `SET_ALARM_HOUR` | `SW6` | `SET_ALARM_HOUR` | Temporary alarm hour +1; wrap `23 → 00` | Updated alarm **HH** continues blinking | Blink: 0.5s ON / 0.5s OFF | Pip 0.3s |
| `SET_ALARM_HOUR` | `SW10` | `SET_ALARM_HOUR` | Temporary alarm hour -1; wrap `00 → 23` | Updated alarm **HH** continues blinking | Blink: 0.5s ON / 0.5s OFF | Pip 0.3s |
| `SET_ALARM_MINUTE` | `SW6` | `SET_ALARM_MINUTE` | Temporary alarm minute +1; wrap `59 → 00` | Updated alarm **MM** continues blinking | Blink: 0.5s ON / 0.5s OFF | Pip 0.3s |
| `SET_ALARM_MINUTE` | `SW10` | `SET_ALARM_MINUTE` | Temporary alarm minute -1; wrap `00 → 59` | Updated alarm **MM** continues blinking | Blink: 0.5s ON / 0.5s OFF | Pip 0.3s |
| `SET_TIME_HOUR` | `TIMEOUT_30S` | `NORMAL` | Exit setting mode | Normal `HH.MM` display | OFF | No pip |
| `SET_TIME_MINUTE` | `TIMEOUT_30S` | `NORMAL` | Exit setting mode | Normal `HH.MM` display | OFF | No pip |
| `SET_ALARM_HOUR` | `TIMEOUT_30S` | `NORMAL` | Discard temporary alarm edit; confirmed alarm remains unchanged | Return to normal `HH.MM` display | OFF | No pip |
| `SET_ALARM_MINUTE` | `TIMEOUT_30S` | `NORMAL` | Discard temporary alarm edit; confirmed alarm remains unchanged | Return to normal `HH.MM` display | OFF | No pip |

---

## 4. State Output Behavior

### `NORMAL`

- **7SEG:** normal `HH.MM`
- **LED D4:** OFF

### `SET_TIME_HOUR`

- **7SEG:** HH blinks 0.5s ON / 0.5s OFF
- **7SEG:** MM remains continuously visible
- **LED D4:** OFF

### `SET_TIME_MINUTE`

- **7SEG:** HH remains continuously visible
- **7SEG:** MM blinks 0.5s ON / 0.5s OFF
- **LED D4:** OFF

### `SET_ALARM_HOUR`

- **7SEG:** temporary alarm HH blinks 0.5s ON / 0.5s OFF
- **7SEG:** temporary alarm MM remains continuously visible
- **LED D4:** blinks 0.5s ON / 0.5s OFF

### `SET_ALARM_MINUTE`

- **7SEG:** temporary alarm HH remains continuously visible
- **7SEG:** temporary alarm MM blinks 0.5s ON / 0.5s OFF
- **LED D4:** blinks 0.5s ON / 0.5s OFF

---

## 5. FSM State Diagram

The MCU application uses **one FSM with five states**.

![Application FSM State Diagram](fsm-state-diagram.png)

### Self-loop adjustment events

In the four setting states, `SW6` and `SW10` modify the selected value but **do not change the current FSM state**:

```text
SET_TIME_HOUR
    ↻ SW6 / SW10

SET_TIME_MINUTE
    ↻ SW6 / SW10

SET_ALARM_HOUR
    ↻ SW6 / SW10

SET_ALARM_MINUTE
    ↻ SW6 / SW10
```

### Timeout return path

A 30-second period without a button event returns any setting state to `NORMAL`:

```text
SET_TIME_HOUR ──────┐
SET_TIME_MINUTE ────┤
SET_ALARM_HOUR ─────┼── TIMEOUT_30S ──→ NORMAL
SET_ALARM_MINUTE ───┘
```

---

## 6. Adjustment Actions

### Current Clock

```text
SET_TIME_HOUR + SW6
→ Clock_IncrementHour()

SET_TIME_HOUR + SW10
→ Clock_DecrementHour()

SET_TIME_MINUTE + SW6
→ Clock_IncrementMinute()

SET_TIME_MINUTE + SW10
→ Clock_DecrementMinute()
```

Clock rollover behavior:

```text
23 + 1 → 00
00 - 1 → 23

59 + 1 → 00
00 - 1 → 59
```

### Alarm Edit Buffer

Alarm-setting adjustments operate on a temporary edit value owned by Application Integration.

When entering alarm-setting mode:

```text
NORMAL + SW16
→ alarm_edit_time = Alarm_GetTime()
→ SET_ALARM_HOUR
```

Alarm edit actions:

```text
SET_ALARM_HOUR + SW6
→ alarm_edit_time.hour +1
→ wrap 23 → 00

SET_ALARM_HOUR + SW10
→ alarm_edit_time.hour -1
→ wrap 00 → 23

SET_ALARM_MINUTE + SW6
→ alarm_edit_time.minute +1
→ wrap 59 → 00

SET_ALARM_MINUTE + SW10
→ alarm_edit_time.minute -1
→ wrap 00 → 59
```

The confirmed Alarm Core value is not modified until the final SW16 confirmation in `SET_ALARM_MINUTE`.

---

## 7. Timeout Behavior

The following states use a 30-second inactivity timeout:

- `SET_TIME_HOUR`
- `SET_TIME_MINUTE`
- `SET_ALARM_HOUR`
- `SET_ALARM_MINUTE`

```text
Any SET state
     │
     │ 30 seconds without button event
     ▼
   NORMAL
```

Timeout exit behavior:

| Output | Behavior |
|---|---|
| State | `NORMAL` |
| 7SEG | Normal `HH.MM` |
| LED D4 | OFF |
| Buzzer | No button pip generated by timeout |

For alarm-setting states, the current team design uses a temporary edit buffer. On timeout:

```text
Discard alarm_edit_time
Keep confirmed Alarm Core alarm unchanged
Do not request EEPROM save
Do not create a new valid alarm configuration
```

The temporary-edit behavior is a team design decision used to prevent unfinished alarm edits from becoming active.

A `TIMEOUT_30S` event is not a button press. Therefore, the timeout transition itself does not request the 0.3-second button buzzer pip. The 0.3-second pip remains associated with presses of `SW3`, `SW6`, `SW10`, and `SW16` as defined in Section 8.

---

## 8. Button Buzzer Behavior

Every press of `SW3`, `SW6`, `SW10`, or `SW16` generates a 0.3-second buzzer pip.

This button beep is independent from the state transition logic.

---

## 9. Alarm Trigger Behavior

When the current clock reaches a valid stored/confirmed alarm time, the buzzer operates for 5 seconds:

```text
0.5s ON
0.5s OFF
0.5s ON
0.5s OFF
...
Total duration: 5s
```

Conceptually:

```text
alarm_valid == 1
AND
Alarm_Check(current_time) == 1
        ↓
Request buzzer alarm sequence
```

This behavior belongs to Alarm/Buzzer/Application Integration logic and does not add another setting state to this FSM.

One-shot/re-trigger protection is also handled outside the FSM so that repeated checks during the matching second do not restart the 5-second alarm sequence.

---

## 10. EEPROM Behavior

The required persistent alarm data consists of:

- Alarm hour
- Alarm minute

Alarm second is fixed to `0` by application/alarm logic.

Save trigger:

```text
SET_ALARM_MINUTE + SW16
→ Confirm alarm_edit_time through Alarm_SetTime(...)
→ Request save of alarm hour/minute to EEPROM
→ Mark alarm configuration valid
→ Return to NORMAL
```

The application-level FSM contract defines the save trigger and state transition. The standalone `fsm.c` currently implements the return-to-`NORMAL` transition; confirmation through `Alarm_SetTime(...)` and the EEPROM save request are integration responsibilities at the current project stage. EEPROM physical access and validation belong to the EEPROM/Application Integration layer.

---

## 11. Unspecified Requirement Cases

The official requirement does not explicitly define functional actions for:

- `NORMAL + SW6`
- `NORMAL + SW10`
- `SET_TIME_HOUR + SW16`
- `SET_TIME_MINUTE + SW16`
- `SET_ALARM_HOUR + SW3`
- `SET_ALARM_MINUTE + SW3`

Current team design rule:

- No state transition
- No clock/alarm value modification
- Button beep still occurs

> Note: the no-transition/no-modification behavior is a team design decision for unspecified combinations.

---

## 12. Alarm Setting Entry Behavior

The official requirement clearly specifies that `NORMAL + SW16` enters alarm-hour setting mode and causes the alarm HH digits to blink.

However, the official requirement does not explicitly define the initial alarm value shown when entering alarm-setting mode.

Current team design:

```text
NORMAL + SW16
        ↓
alarm_edit_time = Alarm_GetTime()
        ↓
SET_ALARM_HOUR
```

Therefore:

- The currently confirmed alarm value is copied into the temporary edit buffer.
- SW6/SW10 modify only `alarm_edit_time`.
- The confirmed Alarm Core value remains unchanged during editing.
- `SET_ALARM_MINUTE + SW16` commits the edited value through `Alarm_SetTime(...)`.
- A timeout discards the temporary edit and preserves the previously confirmed alarm.

This behavior keeps the FSM specification consistent with the Alarm Core specification and prevents unconfirmed edits from changing the active alarm.

---

## 13. Implementation Mapping

The mapping below shows the intended module ownership. Entries for drivers and Application Integration are architectural targets and may not exist as completed source modules yet.

```text
FSM Specification
       │
       ├── firmware/app/fsm.h
       ├── firmware/app/fsm.c
       │
       ├── Clock Core
       │   └── firmware/app/clock.c
       │
       ├── Alarm Core
       │   └── firmware/app/alarm.c
       │
       ├── Application Integration
       │   ├── alarm_edit_time
       │   ├── alarm_valid
       │   └── one-shot alarm trigger handling
       │
       ├── Display Driver
       │   └── firmware/drivers/display.c
       │
       ├── Buzzer Driver
       │   └── firmware/drivers/buzzer.c
       │
       └── EEPROM Driver
           └── firmware/drivers/eeprom.c
```

---

## 14. Verification Plan

FSM verification will use:

```text
firmware/app/fsm_test.c
```

Simulator evidence will be stored in:

```text
docs/testing/fsm/
```

Current standalone FSM-level verification covers:

- Power-on state = `NORMAL`
- SW3 current-time-setting state sequence
- SW16 alarm-setting state sequence
- SW6/SW10 current-clock adjustment behavior in `SET_TIME_HOUR` and `SET_TIME_MINUTE`
- 30-second timeout transitions
- Current team behavior for `NORMAL + SW6` and `NORMAL + SW10`

The existing `fsm_test.c` verifies the implemented FSM state transitions/event handling and the Clock Core calls used by current-time setting. It does **not** verify Alarm edit-buffer adjustment in the alarm-setting states, because that integration is not implemented in the standalone FSM core yet.

### Current implementation status

| Behavior | Current status |
|---|---|
| Five FSM states | Implemented |
| `NORMAL` initialization | Implemented and simulator-verified |
| SW3 time-setting state sequence | Implemented and simulator-verified |
| SW16 alarm-setting state sequence | Implemented and simulator-verified |
| Clock hour/minute adjustment through Clock Core | Implemented and simulator-verified |
| 30-second timeout state transitions | Implemented and simulator-verified |
| Current team rule for tested undefined NORMAL combinations | Implemented and simulator-verified |
| Alarm temporary edit buffer | Target integration behavior |
| Alarm hour/minute edit operations | Target integration behavior |
| Final `Alarm_SetTime(...)` confirmation | Target integration behavior |
| EEPROM save request/storage | Target integration behavior |
| Display and LED D4 physical outputs | Target integration/hardware behavior |
| Button buzzer pip | Target integration/hardware behavior |
| 5-second alarm sequence | Target integration/hardware behavior |
| Alarm-valid and one-shot protection | Target integration behavior |

The following behaviors require later Application Integration / hardware verification:

- Temporary alarm-edit buffer data handling
- Alarm hour/minute edit rollover
- Alarm confirmation through `Alarm_SetTime(...)`
- EEPROM save request and persistent storage
- Display / D4 physical output behavior
- Button buzzer pip
- 5-second alarm buzzer sequence
- Alarm-valid state
- Alarm one-shot/re-trigger protection
