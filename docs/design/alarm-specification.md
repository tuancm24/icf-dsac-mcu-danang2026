# Alarm Core Specification

## 1. Purpose

This document defines the Alarm Core behavior for the MCU digital clock application.

The Alarm Core is responsible for:

- Maintaining the confirmed alarm time
- Setting a confirmed alarm time
- Comparing the current clock time with the confirmed alarm time
- Providing the alarm match condition to the application integration layer
- Providing confirmed alarm hour/minute data for EEPROM storage

The Alarm Core is independent from:

- Application FSM state transitions
- Physical button handling
- 7-segment display driving
- Buzzer waveform generation
- EEPROM read/write implementation
- EEPROM restore policy
- Alarm-edit buffer management
- Alarm-valid / configured-state management
- Alarm one-shot / re-trigger protection
- 30-second inactivity timeout handling
- UI hour/minute increment/decrement behavior

Development flow:

```text
Official Requirement
        ↓
Alarm Core Specification
        ↓
alarm.h / alarm.c
        ↓
alarm_test.c
        ↓
Keil Simulator Verification
        ↓
FSM / EEPROM / Buzzer Integration
```

---

## 2. Alarm Data Model

The Alarm Core uses the existing `Time_t` type defined by the Clock Core.

```c
typedef struct
{
    unsigned char hour;
    unsigned char minute;
    unsigned char second;
} Time_t;
```

The Alarm Core maintains one confirmed alarm value:

```text
alarm_time.hour
alarm_time.minute
alarm_time.second
```

Valid ranges:

| Field | Range |
|---|---|
| Hour | `00` to `23` |
| Minute | `00` to `59` |
| Second | `00` |

The alarm second is fixed at:

```text
00
```

The user configures only the alarm hour and alarm minute.

The Alarm Core stores only the **confirmed** alarm value.

Temporary values edited while the user is inside `SET_ALARM_HOUR` or `SET_ALARM_MINUTE` belong to the Application Integration layer.

---

## 3. Initialization

Standalone Alarm Core initialization uses:

```c
Alarm_Init();
```

After initialization:

```text
hour   = 0
minute = 0
second = 0
```

Therefore, the standalone Alarm Core default value is:

```text
00:00:00
```

### Design Note

The official requirement does not explicitly define the initial alarm hour and minute before EEPROM/application integration.

Therefore, `00:00:00` is a **team design default for standalone Alarm Core verification**, not an official required alarm value.

The Alarm Core does not decide whether the current stored value is a valid user-configured alarm.

Alarm validity/configured-state management belongs to the Application Integration layer.

At power-on, the application must not trigger an alarm only because both the Clock Core and Alarm Core temporarily contain `00:00:00`.

EEPROM restore/load behavior and alarm-valid handling will be finalized during EEPROM/application integration.

---

## 4. Setting the Confirmed Alarm Time

The Alarm Core provides:

```c
void Alarm_SetTime(unsigned char hour, unsigned char minute);
```

Purpose:

- Commit a confirmed alarm hour/minute into the Alarm Core
- Support later EEPROM restore
- Keep `alarm_time.second` fixed at `0`

Expected behavior:

```text
Alarm_SetTime(7, 30)
        ↓
alarm_time = 07:30:00
```

### Input Assumption

The Application Integration layer is responsible for supplying valid values:

```text
0 <= hour <= 23
0 <= minute <= 59
```

The standalone Alarm Core implementation does not need complex range-clamping logic because values originate from controlled UI adjustment and later validated EEPROM integration.

The alarm second is always forced to:

```text
0
```

---

## 5. Alarm Time Access

The confirmed alarm time is obtained using:

```c
Time_t Alarm_GetTime(void);
```

The returned `Time_t` contains:

```text
hour
minute
second
```

The Alarm Core keeps its confirmed alarm data internal.

Higher-level modules shall use the public Alarm Core APIs instead of directly modifying `alarm_time`.

---

## 6. Alarm Match Condition

The Alarm Core provides:

```c
unsigned char Alarm_Check(Time_t current_time);
```

`Alarm_Check()` compares the current Clock Core time with the confirmed Alarm Core time.

The match condition is:

```text
current_time.hour   == alarm_time.hour
AND
current_time.minute == alarm_time.minute
AND
current_time.second == alarm_time.second
```

The Alarm Core invariant is:

```text
alarm_time.second = 0
```

Therefore, the alarm match can occur only at second `00`.

Return values:

| Return | Meaning |
|---|---|
| `0` | Current clock time does not match the alarm time |
| `1` | Current clock time matches the alarm time |

### Example: Match

```text
Current time = 07:30:00
Alarm time   = 07:30:00

Alarm_Check() = 1
```

### Example: Hour Mismatch

```text
Current time = 08:30:00
Alarm time   = 07:30:00

Alarm_Check() = 0
```

### Example: Minute Mismatch

```text
Current time = 07:31:00
Alarm time   = 07:30:00

Alarm_Check() = 0
```

### Example: Second Mismatch

```text
Current time = 07:30:01
Alarm time   = 07:30:00

Alarm_Check() = 0
```

### Important Integration Rule: Alarm Validity

`Alarm_Check()` performs only a time comparison.

It does **not** determine whether the alarm has been configured by the user or restored from valid EEPROM data.

Therefore, the Application Integration layer must request the alarm buzzer sequence only when:

```text
alarm_valid == 1
        AND
Alarm_Check(current_time) == 1
```

`alarm_valid` is intentionally not part of the Alarm Core public API.

### Important Integration Rule: Re-trigger Protection

`Alarm_Check()` represents a **level condition**, not a one-shot event.

If the main loop calls `Alarm_Check()` multiple times during the matching second, the function may return `1` multiple times.

Therefore, the Application Integration layer must prevent repeated alarm-sequence re-triggering during the same matching second.

The Alarm Core itself does not own this one-shot/re-trigger protection.

---

## 7. Alarm Editing Model

The confirmed Alarm Core value and the temporary alarm-edit value are separate concepts.

### Confirmed Alarm

Stored inside Alarm Core:

```text
alarm_time
```

This value represents the alarm that is currently confirmed for use by the application.

### Temporary Edit Value

Owned by Application Integration:

```text
alarm_edit_time
```

When entering alarm-setting mode, the application may initialize:

```text
alarm_edit_time = Alarm_GetTime()
```

SW6/SW10 while editing shall modify the temporary edit value rather than directly changing the confirmed Alarm Core value.

Hour adjustment rules:

```text
23 + 1 → 00
00 - 1 → 23
```

Minute adjustment rules:

```text
59 + 1 → 00
00 - 1 → 59
```

Minute rollover during manual editing does **not** automatically change the hour. Hour and minute are edited independently.

Conceptual flow:

```text
NORMAL
  │
  │ SW16
  ▼
SET_ALARM_HOUR
  │
  ├── SW6  → edit hour +1 with rollover
  ├── SW10 → edit hour -1 with rollover
  │
  │ SW16
  ▼
SET_ALARM_MINUTE
  │
  ├── SW6  → edit minute +1 with rollover
  ├── SW10 → edit minute -1 with rollover
  │
  │ SW16 confirm
  ▼
Alarm_SetTime(alarm_edit_time.hour,
              alarm_edit_time.minute)
  │
  ├── EEPROM save request
  ├── alarm_valid = 1
  ▼
NORMAL
```

This separation prevents an unconfirmed edit from changing the currently active alarm.

UI increment/decrement behavior belongs to the Application Integration layer, not the Alarm Core.

---

## 8. Timeout Behavior During Alarm Editing

The 30-second inactivity timeout belongs to the Application FSM.

If timeout occurs while configuring the alarm:

```text
SET_ALARM_HOUR
        or
SET_ALARM_MINUTE
        ↓
TIMEOUT_30S
        ↓
NORMAL
```

The Alarm Core does not implement or count the timeout.

### Team Design Decision

Because alarm editing uses a temporary edit buffer:

```text
- The temporary alarm_edit_time is discarded on timeout.
- The confirmed Alarm Core alarm_time remains unchanged.
- No EEPROM save is requested.
- No new valid alarm configuration is created.
```

This behavior ensures that only explicit confirmation with SW16 commits a new alarm value.

---

## 9. Buzzer Alarm Behavior

When:

```text
alarm_valid == 1
AND
Alarm_Check(current_time) == 1
```

the application shall request the buzzer alarm sequence.

The required buzzer behavior is:

```text
0.5 s ON
0.5 s OFF
0.5 s ON
0.5 s OFF
...
```

for a total duration of:

```text
5 seconds
```

The Alarm Core does **not** generate the buzzer waveform and does not directly control the buzzer GPIO.

The 5-second buzzer sequence and one-shot trigger handling belong to the Buzzer Driver / Application Integration logic.

---

## 10. EEPROM Behavior

The required persistent alarm data consists of:

```text
Alarm hour
Alarm minute
```

The alarm second is fixed to:

```text
0
```

by application logic.

The official requirement does not require the user to configure alarm seconds.

### Save Flow

The Application FSM defines the confirmation event:

```text
FSM_SET_ALARM_MINUTE + SW16
```

Expected integration flow:

```text
SET_ALARM_MINUTE
        │
        │ SW16
        ▼
Alarm_SetTime(edit_hour, edit_minute)
        │
        ▼
Alarm_GetTime()
        │
        ▼
Save alarm hour/minute to EEPROM
        │
        ▼
alarm_valid = 1
        │
        ▼
FSM_NORMAL
```

The Alarm Core does not directly access EEPROM.

### Restore Flow

EEPROM physical read behavior is outside the Alarm Core.

Conceptually, after successful validation of stored EEPROM data:

```text
EEPROM read hour/minute
        ↓
validate data
        ↓
Alarm_SetTime(hour, minute)
        ↓
alarm_valid = 1
```

Invalid or unavailable EEPROM data must not automatically create a valid alarm configuration.

---

## 11. FSM Integration

The Application FSM controls the alarm-setting state sequence:

```text
NORMAL
  │ SW16
  ▼
SET_ALARM_HOUR
  │ SW16
  ▼
SET_ALARM_MINUTE
  │ SW16
  ▼
NORMAL
```

The Application Integration layer manages `alarm_edit_time`.

### Alarm Hour Setting

```text
FSM_SET_ALARM_HOUR + SW6
→ increment alarm_edit_time.hour
→ wrap 23 → 00
```

```text
FSM_SET_ALARM_HOUR + SW10
→ decrement alarm_edit_time.hour
→ wrap 00 → 23
```

### Alarm Minute Setting

```text
FSM_SET_ALARM_MINUTE + SW6
→ increment alarm_edit_time.minute
→ wrap 59 → 00
```

```text
FSM_SET_ALARM_MINUTE + SW10
→ decrement alarm_edit_time.minute
→ wrap 00 → 59
```

### Alarm Confirmation

```text
FSM_SET_ALARM_MINUTE + SW16
        ↓
Alarm_SetTime(alarm_edit_time.hour,
              alarm_edit_time.minute)
        ↓
EEPROM save request
        ↓
alarm_valid = 1
        ↓
FSM_NORMAL
```

FSM state transitions remain the responsibility of the Application FSM.

Temporary edit-buffer management and `alarm_valid` management remain the responsibility of Application Integration.

---

## 12. Public Interface

Planned `alarm.h` interface:

```c
#ifndef ALARM_H
#define ALARM_H

#include "clock.h"

void Alarm_Init(void);

void Alarm_SetTime(unsigned char hour,
                   unsigned char minute);

Time_t Alarm_GetTime(void);

unsigned char Alarm_Check(Time_t current_time);

#endif /* ALARM_H */
```

No alarm enable/disable API is defined because the official requirement does not specify a user-controlled alarm enable/disable function.

Alarm validity is intentionally not part of the Alarm Core API.

Alarm UI increment/decrement functions are intentionally not part of the Alarm Core API because temporary editing belongs to Application Integration.

---

## 13. Module Boundaries

```text
                         Application FSM
                               │
                               ▼
                    Application Integration
                    ┌─────────────────────┐
                    │ alarm_edit_time     │
                    │ alarm_valid         │
                    │ edit rollover logic │
                    │ one-shot protection │
                    └──────────┬──────────┘
                               │ confirm
                               ▼
                          Alarm Core
                     ┌─────────────────┐
                     │ alarm_time      │
                     │ Alarm_SetTime   │
                     │ Alarm_GetTime   │
                     │ Alarm_Check     │
                     └───────┬─────────┘
                             │
                ┌────────────┴────────────┐
                ▼                         ▼
          EEPROM Integration        Buzzer Integration

Clock Core
    │
    │ current time
    └──────────────────────────────► Alarm Core
```

### Alarm Core owns

- Confirmed alarm hour
- Confirmed alarm minute
- Alarm second fixed at `00`
- Confirmed alarm time update through `Alarm_SetTime()`
- Alarm time access
- Alarm time comparison

### Alarm Core does not own

- Temporary alarm-edit buffer
- Alarm edit increment/decrement logic
- Alarm edit rollover logic
- Button GPIO handling
- Application FSM state transitions
- 7-segment display control
- LED D4 blinking
- EEPROM physical access
- EEPROM validation/restore policy
- Alarm-valid/configured-state management
- Buzzer GPIO control
- Buzzer 0.5-second timing
- Buzzer 5-second alarm duration
- Alarm one-shot/re-trigger protection
- 30-second inactivity timeout

---

## 14. Verification Plan

Alarm Core standalone verification will use:

```text
firmware/app/alarm_test.c
```

Keil Simulator evidence will be stored in:

```text
docs/testing/alarm/
```

Planned tests:

| ID | Test | Expected |
|---|---|---|
| TEST 1 | Alarm initialization | `00:00:00` |
| TEST 2 | `Alarm_SetTime(7, 30)` | `07:30:00` |
| TEST 3 | `Alarm_SetTime()` forces second | second = `00` |
| TEST 4 | Matching alarm time | `Alarm_Check() = 1` |
| TEST 5 | Hour mismatch | `Alarm_Check() = 0` |
| TEST 6 | Minute mismatch | `Alarm_Check() = 0` |
| TEST 7 | Second mismatch | `Alarm_Check() = 0` |

The `Alarm_SetTime()` tests verify that a confirmed hour/minute is stored correctly and that second remains fixed at `00`.

The match tests verify the Alarm Core comparator.

Alarm edit rollover, alarm-valid management, edit-buffer handling, EEPROM behavior, timeout behavior, and re-trigger protection are not part of standalone Alarm Core verification because they belong to Application Integration.

---

## 15. Integration Verification Deferred

The following behaviors are intentionally deferred from standalone Alarm Core verification:

- Temporary `alarm_edit_time`
- SW6/SW10 alarm edit adjustment
- Hour edit rollover `23 ↔ 00`
- Minute edit rollover `59 ↔ 00`
- EEPROM save implementation
- EEPROM restore/load and validation
- Alarm-valid/configured-state handling
- Application FSM calls/integration
- Buzzer 5-second alarm sequence
- 0.5-second buzzer ON/OFF timing
- Alarm one-shot/re-trigger protection
- Physical button integration
- 7-segment alarm-setting display behavior
- Hardware verification on the SN8F5708 EVK

These behaviors shall be implemented and verified during later application and hardware integration.

---

## 16. Design Summary

The Alarm Core is intentionally small and hardware-independent.

Its responsibilities are:

```text
Store confirmed alarm time
           +
Set confirmed alarm time
           +
Compare against Clock Core time
```

Higher-level behavior is separated into other modules:

```text
Application FSM         → setting-state workflow
Application Integration → edit buffer + rollover + alarm-valid + one-shot logic
EEPROM Driver           → persistent storage
Buzzer Driver           → physical alarm output
Clock Core              → current time source
```

This separation ensures that:

- Unconfirmed edits do not change the active alarm
- Timeout safely discards unfinished edits
- EEPROM restore can reuse `Alarm_SetTime()`
- Alarm UI adjustment does not bypass confirmation
- Alarm Core can be verified independently before full system integration
