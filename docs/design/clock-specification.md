# Clock Core Specification

## 1. Purpose

This document defines the behavior and public interface of the Clock Core used by the digital clock application for the SONiX SN8F5708 EVK project.

The Clock Core is responsible for maintaining the current time in `HH:MM:SS` form and providing the basic operations required by the application state machine while setting the current hour and minute.

The module is implemented in:

- `firmware/app/clock.c`
- `firmware/app/clock.h`

Verification is implemented in:

- `firmware/app/clock_test.c`
- `docs/testing/clock/README.md`

---

## 2. Relation to System Requirements

The project specification requires the normal display to show current hour and minute in `HH.MM` format, start from `00.00` after power-up, increment the minute after every 60 seconds, wrap the minute from 59 to 00 while incrementing the hour, and keep the hour range from 00 to 23.

The project also requires the `+` and `-` controls to adjust the current hour or minute while the application is in the corresponding setting mode.

The Clock Core provides the timekeeping and adjustment operations needed by those higher-level behaviors.

Display blinking, button processing, timeout handling, buzzer behavior, EEPROM access, and application mode transitions are outside the Clock Core.

> **Specification boundary:** Requirements stated in the contest task are treated as system requirements. Behaviors that are observable only from the current `clock.c/.h` implementation are explicitly described as current implementation behavior rather than as new contest requirements.

---

## 3. Data Type

The Clock Core uses the following public time structure:

```c
typedef struct
{
    unsigned char hour;
    unsigned char minute;
    unsigned char second;
} Time_t;
```

### Valid operating ranges

| Field | Range |
|---|---|
| `hour` | `0` to `23` |
| `minute` | `0` to `59` |
| `second` | `0` to `59` |

The Clock Core maintains one internal instance:

```c
static Time_t current_time;
```

`current_time` is private to `clock.c`.

External modules must access the clock value through the public API.

`Time_t` is also reused by the current Alarm Core interface. This is an intentional shared data type in the present application architecture; ownership of the current clock value still remains inside the Clock Core.

---

## 4. Public Interface

### 4.1 `Clock_Init`

```c
void Clock_Init(void);
```

#### Purpose

Initialize the current clock time.

#### Post-condition

```text
hour   = 0
minute = 0
second = 0
```

Equivalent time:

```text
00:00:00
```

---

### 4.2 `Clock_Tick1Second`

```c
void Clock_Tick1Second(void);
```

#### Purpose

Advance the current time by exactly one second.

#### Behavior

1. Increment `second`.
2. If `second` reaches 60:
   - set `second = 0`
   - increment `minute`
3. If `minute` reaches 60:
   - set `minute = 0`
   - increment `hour`
4. If `hour` reaches 24:
   - set `hour = 0`

#### Examples

```text
00:00:00 + 1 second -> 00:00:01
00:00:59 + 1 second -> 00:01:00
12:59:59 + 1 second -> 13:00:00
23:59:59 + 1 second -> 00:00:00
```

`Clock_Tick1Second()` does not perform display refresh, timer configuration, or interrupt handling. A higher-level timing/platform layer is responsible for invoking this function once per elapsed second whenever the application intends the current clock to advance. Whether time continues running while the user is in a setting mode is an application-level policy and is not defined by the Clock Core.

---

### 4.3 `Clock_IncrementHour`

```c
void Clock_IncrementHour(void);
```

#### Purpose

Increase the current hour by one while setting the current time.

#### Behavior

```text
00 -> 01
...
22 -> 23
23 -> 00
```

#### Current implementation behavior

This operation changes only the `hour` field.

It does not modify `minute` or `second`.

The project requirement defines hour wraparound for manual adjustment; preserving the other fields is the behavior of the current Clock Core implementation.

---

### 4.4 `Clock_DecrementHour`

```c
void Clock_DecrementHour(void);
```

#### Purpose

Decrease the current hour by one while setting the current time.

#### Behavior

```text
23 -> 22
...
01 -> 00
00 -> 23
```

#### Current implementation behavior

This operation changes only the `hour` field.

It does not modify `minute` or `second`.

The project requirement defines hour wraparound for manual adjustment; preserving the other fields is the behavior of the current Clock Core implementation.

---

### 4.5 `Clock_IncrementMinute`

```c
void Clock_IncrementMinute(void);
```

#### Purpose

Increase the current minute by one while setting the current time.

#### Behavior

```text
00 -> 01
...
58 -> 59
59 -> 00
```

#### Current implementation behavior

This operation wraps the minute independently.

It does **not** increment the hour when `59 -> 00`.

It does not modify `hour` or `second`.

The project requirement specifies that manual minute increment wraps from 59 to 00; it does not require a carry into the hour. The no-carry behavior therefore matches both the stated requirement and the current implementation.

---

### 4.6 `Clock_DecrementMinute`

```c
void Clock_DecrementMinute(void);
```

#### Purpose

Decrease the current minute by one while setting the current time.

#### Behavior

```text
59 -> 58
...
01 -> 00
00 -> 59
```

#### Current implementation behavior

This operation wraps the minute independently.

It does **not** decrement the hour when `00 -> 59`.

It does not modify `hour` or `second`.

The project requirement specifies that manual minute decrement wraps from 00 to 59; it does not require borrowing from the hour.

---

### 4.7 `Clock_GetTime`

```c
Time_t Clock_GetTime(void);
```

#### Purpose

Return the current clock value.

#### Return value

A copy of the current `Time_t` value.

Example:

```text
current_time = 07:30:15
Clock_GetTime() -> {7, 30, 15}
```

The returned value does not expose direct write access to the internal `current_time`.

---

## 5. Functional Behavior

### 5.1 Initialization

Immediately after `Clock_Init()`:

```text
current_time = 00:00:00
```

This supports the project requirement that the clock starts from `00.00` after power-up.

---

### 5.2 Normal Timekeeping

A typical call relationship is:

```text
1-second timing event
        |
        v
Clock_Tick1Second()
        |
        v
current_time updated
        |
        v
Clock_GetTime()
        |
        v
Application / Display
```

The timing source itself is not implemented by the Clock Core. The Clock Core also does not decide whether ticking is enabled or paused in a particular application state.

---

### 5.3 Second Rollover

```text
HH:MM:59
   |
   | Clock_Tick1Second()
   v
HH:(MM+1):00
```

If the minute also reaches 60, the hour is incremented.

---

### 5.4 Minute Rollover During Normal Timekeeping

Example:

```text
10:59:59
   |
   | Clock_Tick1Second()
   v
11:00:00
```

This carry behavior belongs only to `Clock_Tick1Second()`.

Manual `Clock_IncrementMinute()` does not carry into the hour.

---

### 5.5 Full-Day Rollover

```text
23:59:59
   |
   | Clock_Tick1Second()
   v
00:00:00
```

This keeps the clock in 24-hour format.

---

## 6. Manual Time Adjustment

The Application FSM decides when manual adjustment operations are valid.

Typical mapping:

| Application State | Event | Clock Core Operation |
|---|---|---|
| `SET_TIME_HOUR` | SW6 | `Clock_IncrementHour()` |
| `SET_TIME_HOUR` | SW10 | `Clock_DecrementHour()` |
| `SET_TIME_MINUTE` | SW6 | `Clock_IncrementMinute()` |
| `SET_TIME_MINUTE` | SW10 | `Clock_DecrementMinute()` |

The Clock Core does not:

- read SW3, SW6, SW10, or SW16 directly
- debounce buttons
- determine the current FSM state
- generate buzzer feedback
- control 7-segment blinking
- manage timeout behavior

Those responsibilities belong to other modules.

---

## 7. Module Boundaries

### Clock Core owns

- Current `hour`
- Current `minute`
- Current `second`
- One-second time advancement
- Hour increment/decrement with wraparound
- Minute increment/decrement with wraparound
- Read access to the current time

### Clock Core does not own

- Hardware timer configuration
- Timer interrupt service routine
- 7-segment display
- Button/switch input
- Button debounce
- Application FSM
- Alarm time
- Alarm comparison
- EEPROM
- Buzzer
- LED
- Timeout management

---

## 8. Design Constraints

### 8.1 24-hour format

The hour is maintained within:

```text
00..23
```

No AM/PM mode is supported.

### 8.2 Independent manual adjustment

Manual hour and minute adjustment wraps only the selected field.

Examples:

```text
23:15:20 + manual hour increment -> 00:15:20
00:15:20 + manual hour decrement -> 23:15:20

10:59:20 + manual minute increment -> 10:00:20
10:00:20 + manual minute decrement -> 10:59:20
```

### 8.3 Seconds during manual setting

The existing Clock Core APIs for setting hour and minute do not reset or alter `second`.

Any future requirement to freeze or reset seconds while entering/exiting setup mode must be handled explicitly at application level or through a future Clock Core API revision.

### 8.4 No direct arbitrary setter

The current Clock Core does not expose an API such as:

```c
Clock_SetTime(hour, minute, second);
```

Current time is changed only through initialization, one-second ticking, and hour/minute increment/decrement operations.

---

## 9. Verification

The current Clock Core has been verified using Keil C51 Simulator.

| Test | Description | Expected Result |
|---|---|---|
| TEST 1 | Initialization | `00:00:00` |
| TEST 2 | 60-second rollover | `00:01:00` |
| TEST 3 | Hour decrement from 00 | `23:00:00` |
| TEST 4 | Hour increment from 23 | `00:00:00` |
| TEST 5 | Minute decrement from 00 | `00:59:00` |
| TEST 6 | Minute increment from 59 | `00:00:00` |
| TEST 7 | `23:59:59 + 1 second` | `00:00:00` |

Verification result:

```text
7/7 tests PASS
```

Detailed evidence is stored in:

```text
docs/testing/clock/
```

---

## 10. Integration Notes

The intended system relationship is:

```text
Timer / Interrupt
       |
       | 1-second event
       v
+------------------+
|    Clock Core    |
|------------------|
| current_time     |
| Tick1Second()    |
| IncrementHour()  |
| DecrementHour()  |
| IncrementMinute()|
| DecrementMinute()|
| GetTime()        |
+------------------+
       |
       | Time_t copy
       v
+-------------------------+
| Application Integration |
+-------------------------+
       |              |
       |              +------> Display
       |
       +---------------------> Alarm_Check(current_time)
```

The Application FSM may request the manual hour/minute adjustment APIs while the application is in the corresponding setup state.

The Clock Core should remain hardware-independent.

Hardware-specific timing logic should be placed in the timer/platform layer.

---

## 11. Known Limitations

The current implementation:

- assumes `Clock_Tick1Second()` is called exactly once per elapsed second
- does not configure or verify the hardware timer
- does not provide atomic access protection for interrupt/concurrent contexts
- does not support arbitrary time assignment
- does not reset seconds during manual hour/minute adjustment
- does not handle display behavior
- does not handle alarm behavior
- does not perform input validation because all internal operations maintain valid ranges

These limitations are acceptable for the current Clock Core scope. Items that affect real-time behavior or concurrency must be re-evaluated during hardware/system integration.

---

## 12. Acceptance Criteria

The Clock Core is considered correct for the current project stage when:

- `Clock_Init()` produces `00:00:00`
- `second` remains within `00..59` during normal ticking
- `minute` remains within `00..59`
- `hour` remains within `00..23`
- second rollover increments minute correctly
- minute rollover during normal ticking increments hour correctly
- full-day rollover produces `00:00:00`
- manual hour increment/decrement wraps correctly
- manual minute increment/decrement wraps correctly
- manual minute wrap does not modify hour
- all Clock Core unit tests pass

Current status:

```text
Clock Core verification: PASS
7/7 unit tests passed
```
