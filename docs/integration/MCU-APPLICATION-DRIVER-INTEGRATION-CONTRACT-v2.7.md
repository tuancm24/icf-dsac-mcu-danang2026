# MCU APPLICATION ↔ DRIVER INTEGRATION CONTRACT
## Integration Contract v2.7 — ACTIVE / FROZEN

**Project:** ICF-DSAC MCU Da Nang 2026
**Target:** SONiX SN8F5708 EVK
**Owners:** Tuấn — Application/Core/Integration; Trung — Drivers/Platform/Hardware
**Purpose:** One stable contract for PR review, implementation, integration, debugging, regression, and final report.

> **Freeze principle:** freeze only what crosses the Tuấn↔Trung boundary. Internal implementation remains owned by the module owner unless it violates an observable requirement, public API contract, timing contract, or safety/correctness rule.

---

# 0. STATUS AND SOURCE CLASSIFICATION

This document uses four labels:

- **[OFFICIAL]** — explicitly required by the competition task.
- **[EXISTING-CORE]** — already implemented/verified in Tuấn's Clock/FSM/Alarm baseline.
- **[TEAM]** — behavior not fully specified by the task; intentionally selected by the team for deterministic integration.
- **[IMPL]** — implementation requirement/quality gate; it must hold, but is not itself an Application-facing behavior.

The contract becomes **ACTIVE / FROZEN v2.7** only after:
1. Tuấn accepts the Application/Core and system decisions.
2. Trung accepts the Driver/Platform API and observable semantics.
3. Mandatory PR blockers are closed.
4. Final public headers are compared once more against this document.

After freeze, public API/observable behavior changes require both owners' approval and a contract revision.

---

# 1. OFFICIAL REQUIREMENTS BASELINE

| ID | Requirement | Source class |
|---|---|---|
| REQ-CLK-01 | Power-on normal mode displays `00.00`. | OFFICIAL |
| REQ-CLK-02 | Clock rolls minutes `59 -> 00` and increments hour; hours are `00..23`. | OFFICIAL |
| REQ-SET-01 | `NORMAL + SW3 -> SET_TIME_HOUR`; HH blinks 0.5 s ON / 0.5 s OFF. | OFFICIAL |
| REQ-SET-02 | `SET_TIME_HOUR + SW3 -> SET_TIME_MINUTE`; MM blinks. | OFFICIAL |
| REQ-SET-03 | `SET_TIME_MINUTE + SW3 -> NORMAL`. | OFFICIAL |
| REQ-ALMSET-01 | `NORMAL + SW16 -> SET_ALARM_HOUR`; HH blinks. | OFFICIAL |
| REQ-ALMSET-02 | `SET_ALARM_HOUR + SW16 -> SET_ALARM_MINUTE`; MM blinks. | OFFICIAL |
| REQ-ALMSET-03 | `SET_ALARM_MINUTE + SW16` saves alarm hour/minute to EEPROM and returns NORMAL. | OFFICIAL |
| REQ-PLUS-01 | SW6 increments selected hour/minute with `23->00` and `59->00`. | OFFICIAL |
| REQ-MINUS-01 | SW10 decrements selected hour/minute with `00->23` and `00->59`. | OFFICIAL |
| REQ-BUZ-01 | Every SW3/SW6/SW10/SW16 press produces a 0.3 s buzzer beep. | OFFICIAL |
| REQ-BUZ-02 | At alarm time, buzzer runs 5 s with 0.5 s ON / 0.5 s OFF. | OFFICIAL |
| REQ-TMO-01 | 30 s without button event in any SET state returns NORMAL. | OFFICIAL |
| REQ-TMO-02 | Timeout exit produces a 0.3 s buzzer beep. | OFFICIAL |
| REQ-LED-01 | D4 blinks 0.5 s ON / 0.5 s OFF only in alarm-setting states; otherwise OFF. | OFFICIAL |
| REQ-EEP-01 | EEPROM stores alarm hour/minute; alarm second is fixed at `00`. | OFFICIAL |

**Important:** the official task does not explicitly define rollback behavior, clock ticking during editing, boot-time `00:00` alarm collision, alarm confirmation that already matches the current second, simultaneous keys, rapid overlapping beeps, watchdog policy, or EEPROM-write failure UI. These behaviors are never presented as OFFICIAL unless explicitly supported by the task; where needed, they are identified as TEAM decisions.

---

# 2. OWNERSHIP — FROZEN

```text
firmware/app/       -> Tuấn
firmware/drivers/   -> Trung
firmware/platform/  -> Trung
firmware/config/    -> Shared; cross-impact changes require agreement
firmware/project/   -> Shared; build/target changes require agreement
docs/testing/<module>/ -> owner of that module/evidence
```

Architecture:

```text
Buttons / Timer
      |
      v
Application Integration (Tuấn)
      |
      +----> FSM / Clock / Alarm (Tuấn)
      |
      +----> public Driver APIs
                    |
                    v
           Drivers / Platform (Trung)
                    |
                    v
               SN8F5708
```

Application must not depend on raw GPIO, I2C transactions, physical pin registers, Timer registers, or watchdog SFRs.

---

# 3. PUBLIC API CONTRACT — FROZEN

## 3.0 Boundary rule

Only APIs/types that **Application Integration is allowed to depend on** are contract-frozen.

A module may expose additional helper/public functions for its own tests or internal scheduling. Those helpers are not automatically cross-team dependencies and may change without revising this contract, provided the frozen boundary remains compatible.

This prevents unnecessary coupling between Tuấn's Application and Trung's implementation.

---

## API-CLK — Clock Core [BOUNDARY]

```c
typedef struct
{
    unsigned char hour;
    unsigned char minute;
    unsigned char second;
} Time_t;

void Clock_Init(void);
void Clock_Tick1Second(void);
void Clock_IncrementHour(void);
void Clock_DecrementHour(void);
void Clock_IncrementMinute(void);
void Clock_DecrementMinute(void);
Time_t Clock_GetTime(void);
```

Observable contract:
- valid range always `00:00:00..23:59:59`;
- `Clock_Tick1Second()` performs correct rollover;
- hour/minute adjustment wraps correctly;
- no Driver/Platform dependency.

---

## API-FSM — Application FSM [BOUNDARY]

```c
typedef enum
{
    FSM_NORMAL = 0,
    FSM_SET_TIME_HOUR,
    FSM_SET_TIME_MINUTE,
    FSM_SET_ALARM_HOUR,
    FSM_SET_ALARM_MINUTE
} FSM_State;

typedef enum
{
    FSM_EVENT_NONE = 0,
    FSM_EVENT_SW3,
    FSM_EVENT_SW6,
    FSM_EVENT_SW10,
    FSM_EVENT_SW16,
    FSM_EVENT_TIMEOUT_30S
} FSM_Event;

void FSM_Init(void);
void FSM_HandleEvent(FSM_Event event);
FSM_State FSM_GetState(void);
```

Undefined state/event combinations:
```text
no state transition
no application value change
```
System-level key beep remains independent of whether the FSM performs an action.

---

## API-ALM — Alarm Core [BOUNDARY]

```c
void Alarm_Init(void);
void Alarm_SetTime(unsigned char hour, unsigned char minute);
Time_t Alarm_GetTime(void);
unsigned char Alarm_Check(Time_t current_time);
```

Precondition for `Alarm_SetTime()`:
```text
hour   0..23
minute 0..59
```

Alarm second is always `00`.

---

## API-BTN — Button Driver [BOUNDARY]

Exact event type required by Application:

```c
typedef enum
{
    BUTTON_EVENT_NONE = 0,
    BUTTON_EVENT_SW3_CLICK,
    BUTTON_EVENT_SW6_CLICK,
    BUTTON_EVENT_SW10_CLICK,
    BUTTON_EVENT_SW16_CLICK
} Button_Event_t;
```

Frozen Application-facing functions:

```c
void Button_Init(void);
void Button_Process(void);
Button_Event_t Button_GetEvent(void);
```

Behavior:
- one accepted debounced press produces one click event;
- holding a key does not repeatedly generate click events until release/re-press;
- `Button_GetEvent()` removes the oldest queued event and returns `BUTTON_EVENT_NONE` when empty;
- Application-level “button event” / inactivity activity means a debounced click event, not a raw bouncing GPIO edge;
- Application may call `Button_Process()` every foreground cycle. The Driver owns the configured 10 ms sampling gate and must not make correctness depend on the Application calling it at exactly 10 ms boundaries.

Current module also exposes `Button_Id_t` and `Button_IsPressed(...)`; Application Integration does **not** depend on them in v2.7, so they are not part of the required cross-team boundary.

---

## API-DSP — Display Driver [BOUNDARY]

Exact blink type required by Application:

```c
typedef enum
{
    DISPLAY_BLINK_NONE = 0,
    DISPLAY_BLINK_HOURS,
    DISPLAY_BLINK_MINUTES,
    DISPLAY_BLINK_ALL
} Display_BlinkMode_t;
```

Frozen Application-facing functions:

```c
void Display_Init(void);
void Display_SetTime(unsigned char hour, unsigned char minute);
void Display_SetColon(unsigned char enable);
void Display_SetBlinkMode(Display_BlinkMode_t mode);
```

Driver/Platform-owned service functions such as:

```c
Display_UpdateBlinkState();
Display_ScanRoutine();
Display_Clear();
```

are **not required Application dependencies** in v2.7.

`Display_UpdateBlinkState()` and `Display_ScanRoutine()` belong to Trung's Display service/timing implementation. Application selects the requested blink mode but does not own the blink phase update schedule. `Display_Clear()` is not needed by the official normal flow.

Application owns **what** HH/MM and blink mode to show. Trung owns segment encoding, multiplexing and electrical timing.

Application precondition:
```text
hour   0..23
minute 0..59
```
Application does not rely on the driver's current “clamp invalid value” behavior as a validation mechanism.

---

## API-BUZ — Buzzer Driver [BOUNDARY]

Frozen Application-facing functions:

```c
void Buzzer_Init(void);
void Buzzer_BeepShort(void);
void Buzzer_StartAlarm(void);
void Buzzer_Process(void);
```

Observable semantics:
- normal short beep = approximately 300 ms;
- alarm sequence = 5 s, 500 ms ON / 500 ms OFF;
- **any short-beep request** (button feedback or timeout feedback) must not cancel/restart/shorten an already-active alarm sequence;
- if an alarm occurrence begins while SHORT_BEEP is active, `Buzzer_StartAlarm()` immediately takes priority and starts the required alarm sequence;
- a short-beep request while SHORT_BEEP is already active refreshes the 300 ms timer per DEC-14;
- a repeated alarm-start request while ALARM is already active is ignored/no-op per DEC-15.

Current module may also expose `Buzzer_Stop()` and `Buzzer_IsBusy()`; v2.7 Application logic does not require those functions for the official behavior, so they are support APIs rather than frozen Application dependencies.

**Concurrent alarm + short-feedback conflict rule [TEAM]:** the official task independently requires the alarm waveform, button beeps, and timeout beep but does not define simultaneous acoustic conflicts. During an already-active 5 s alarm, preservation of the required alarm waveform has priority; the logical button/timeout event is still processed, but its 300 ms acoustic request may be absorbed/suppressed rather than corrupt the 500 ms alarm phases.

---

## API-LED — LED Driver [BOUNDARY]

Exact mode type required by Application:

```c
typedef enum
{
    LED_MODE_OFF = 0,
    LED_MODE_ON,
    LED_MODE_BLINK_ALARM_SETTING
} LED_Mode_t;
```

Frozen Application-facing functions:

```c
void LED_Init(void);
void LED_SetMode(LED_Mode_t mode);
void LED_Process(void);
```

`LED_SetMode(same_mode)` must be idempotent; repeated identical requests must not restart blink timing.

---

## API-EEP — EEPROM Driver [BOUNDARY]

Frozen Application-facing persistence functions:

```c
unsigned char EEPROM_SaveAlarm(unsigned char hour, unsigned char minute);
unsigned char EEPROM_ReadAlarm(unsigned char *hour, unsigned char *minute);
```

The current module may expose:

```c
void EEPROM_Init(void);
```

but v2.7 Application does **not** depend on that function. Low-level I2C initialization remains Board/Platform-owned. If `EEPROM_Init()` is retained, it must be harmless/idempotent and may be used internally by Trung's layer; it is not required in Tuấn's integration startup.

Return convention:
```text
1 = operation valid/successful
0 = invalid input / unavailable / communication or validation failure
```

Driver rejects `hour > 23` or `minute > 59`.

`EEPROM_ReadAlarm(hour, minute)` must return `0` for null output pointers and must not partially publish invalid data.

---

## API-BOARD/TMR — Platform [BOUNDARY]

Application-facing Platform functions:

```c
void Board_Init(void);
void Board_FeedWatchdog(void);

unsigned long Timer_GetTickMs(void);
unsigned char Timer_HasElapsed(unsigned long start_tick,
                               unsigned long duration_ms);
```

`Timer_Init()` remains Platform-owned initialization behind `Board_Init()` and is not an Application dependency.

**Watchdog freeze rule:** raw `WDTR` access is always Platform-owned. `Board_FeedWatchdog()` is frozen now so the Application API never changes later:
- if production WDT is enabled, the wrapper performs the required feed sequence;
- if production WDT is disabled, the wrapper is a documented safe no-op.

Application may call the wrapper every foreground cycle without knowing the MCU watchdog policy.

---

## 3.1 Explicit non-boundary APIs

Application Integration shall not call:

```text
GPIO_*
I2C_*
Timer_ISR_Handler()
Timer_DelayUs()
raw MCU Px/SFR registers
WDTR                 [raw register access forbidden; use Board_FeedWatchdog()]
PIN_HW_*
Display_UpdateBlinkState() [Driver/Platform display timing service]
Display_ScanRoutine()       [Driver/Platform refresh service]
Button_IsPressed()          [not needed by v2.7 Application]
```

This list is architectural, not a statement that those functions may not exist in the module headers.

# 4. TEAM DECISIONS — FROZEN SYSTEM SEMANTICS

## DEC-01 — Current-time editing

`SET_TIME_HOUR` and `SET_TIME_MINUTE` edit the real Clock Core through the existing FSM.

Clock second ticking is **paused** during these two states.

On entry:
```text
discard partial sub-second scheduling phase
```

During edit:
```text
do not accumulate missed seconds
```

On return NORMAL:
```text
resume with a fresh 1-second scheduling anchor
```

Hour/minute changes already made remain applied if timeout occurs. No rollback.

## DEC-02 — Seconds after current-time edit

Seconds are preserved.

Example:
```text
12:34:27
enter SET_TIME -> clock ticking pauses
edit MM to 40
exit
12:40:27
```

No new Clock API is required.

## DEC-03 — Alarm editing is transactional

On:
```text
NORMAL + SW16
```

copy confirmed alarm into:
```text
alarm_edit_time
```

SW6/SW10 in alarm-setting states modify only this temporary value.

On:
```text
SET_ALARM_MINUTE + SW16
```

commit exactly once:
```text
Alarm_SetTime(edit.hour, edit.minute)
alarm_valid = 1
EEPROM_SaveAlarm(edit.hour, edit.minute)
```

On timeout:
```text
discard temporary edit
keep previous confirmed alarm
do not write EEPROM
```

## DEC-04 — Inactivity timing

```text
NORMAL -> SET
    start fresh 30 s timer

any button event while still in SET
    restart 30 s timer

exit to NORMAL
    timeout timer inactive
```

No stale activity timestamp may carry into a new setting session.

## DEC-05 — Alarm match predicate and one-shot occurrence handling

`Alarm_Check(current_time)` is a **pure match predicate**. It answers only:

```text
does current_time equal the confirmed alarm HH:MM:00?
```

It does **not** itself define when a buzzer alarm occurrence is generated.

The Application may generate a new alarm occurrence only in the trigger context frozen by DEC-06: immediately after a logical `Clock_Tick1Second()` advances the Clock onto the confirmed alarm time.

Occurrence handling:

```c
/* Called in the logical Clock-tick trigger context only */
match = alarm_valid && Alarm_Check(now);

if (!match)
{
    alarm_match_latched = 0;
}
else if (!alarm_match_latched)
{
    Buzzer_StartAlarm();
    alarm_match_latched = 1;
}
```

Outside that trigger context — for example startup restore, manual current-time editing, or alarm confirmation — `Alarm_Check()` may be used only for latch/bookkeeping decisions required by DEC-07/DEC-13. It must not be treated as a generic “poll and fire whenever values match” API.

A new confirmed alarm updates the latch according to DEC-13; do not blindly force it to `0` if the new alarm already equals the current Clock.


## DEC-06 — Alarm remains armed during SET; trigger source is a logical Clock tick

The previous confirmed alarm remains armed until a new alarm is confirmed.

When `alarm_valid == 1`, the alarm may trigger in any FSM state **only when a logical one-second `Clock_Tick1Second()` advances the Clock onto the confirmed `HH:MM:00` occurrence**.

This is the frozen interpretation of the official phrase “khi đồng hồ chạy đến giờ hẹn”.

Consequences:

```text
natural Clock tick reaches alarm HH:MM:00
    -> alarm occurrence may trigger

manual SW6/SW10 current-time edit creates the same HH:MM:00 value
    -> DO NOT trigger merely because of the manual edit

alarm confirmation equals the current time
    -> DO NOT trigger merely because of confirmation (DEC-13)

EEPROM restore equals the power-on time
    -> DO NOT trigger merely because of restore (DEC-07)
```

In `SET_TIME_HOUR/MINUTE`, Clock ticking is paused, so no natural alarm occurrence advances while the user edits current time.

In `SET_ALARM_HOUR/MINUTE`, the current Clock continues ticking; the previously confirmed alarm therefore remains capable of triggering naturally until the new alarm is finally confirmed.

This removes ambiguous “manual setting caused an alarm” behavior while preserving the official running-clock behavior.


## DEC-07 — Boot collision at 00:00

A valid persisted `00:00` alarm shall not ring merely because the power-on Clock also starts at `00:00:00`.

After restore, if current time already equals the restored alarm:
```text
alarm_match_latched = 1
```

It rearms after the clock no longer matches and may trigger on a later genuine occurrence.

## DEC-08 — EEPROM failure

Alarm confirmation and EEPROM persistence are separate:

```text
confirmed alarm -> valid for current runtime
EEPROM save success -> new alarm is persistent across reboot
```

If save fails:

```text
runtime alarm remains valid
do not claim persistence success
no invented extra beep/error UI
```

The Application may keep an internal diagnostic flag such as `persistence_ok` for testing/debugging, but that flag is **not part of the frozen cross-team API or required runtime behavior**.

Persistence failure semantics do **not** promise that the previously stored EEPROM alarm survives a failed update. A transaction-safe implementation must guarantee “never accept a mixed/corrupted new record”; preserving the old record on every possible power/write failure is optional unless Trung chooses a stronger two-slot/journal scheme.

Thus:

```text
save success  -> new confirmed alarm is persistent
save failure  -> persistence is not guaranteed; runtime alarm remains valid
```


## DEC-09 — Simultaneous keys

Functional acceptance assumes one user button at a time. Multi-key combinations are not assigned Application semantics.

Driver must remain safe and must not corrupt state/memory, but ghosting/multi-key functionality is outside required acceptance unless the official task is later clarified.


## DEC-10 — EEPROM restore at startup

The official task explicitly requires storing alarm hour/minute but does not explicitly describe boot restore behavior. The team freezes this integration behavior:

```text
valid EEPROM record at boot
    -> Alarm_SetTime(saved_hour, saved_minute)
    -> alarm_valid = 1

invalid/empty/error record
    -> alarm_valid = 0
```

This is a TEAM integration decision consistent with the persistence goal; it is not mislabelled as an OFFICIAL requirement.

## DEC-11 — Button event wins over timeout at the same processing boundary

Application processes already-queued debounced button events **before** evaluating the 30-second timeout.

Therefore, if a valid button event is available at the boundary where the timeout would otherwise expire:

```text
process button event
restart/stop timeout according to resulting state
do not also dispatch TIMEOUT_30S for that same cycle
```

This avoids double-dispatch (`button + timeout`) for one scheduling instant.

The timeout is based on **debounced button events**, matching the task wording "không có sự kiện ấn nút", not on raw electrical transitions.

## DEC-12 — Alarm edit default when no valid alarm exists

After cold start or invalid EEPROM data:

```text
Alarm Core default = 00:00:00
alarm_valid = 0
```

If the user enters alarm setting before any valid alarm has been confirmed, the temporary edit value begins at:

```text
00:00
```

The Core default value alone does not arm the alarm. Only final confirmation or a valid EEPROM restore sets `alarm_valid = 1`.

## DEC-13 — Confirmation collision with current time

Confirming a new alarm is configuration, not a clock-tick occurrence.

Therefore, if the user confirms an alarm whose `HH:MM:00` is already equal to the current Clock:

```text
do not start the 5-second alarm as a side effect of confirmation
alarm_match_latched = 1 for the current equal-value occurrence
```

The latch clears once a later logical Clock value no longer matches. A future natural Clock tick onto the alarm time may trigger normally.

This is consistent with DEC-06 and DEC-07 and is a TEAM edge-case policy, not an OFFICIAL requirement.


## DEC-14 — Rapid short-beep overlap

The official task requires a 0.3 s beep for each button press but does not define presses that occur faster than the beep duration.

Team policy:

```text
new button press while SHORT_BEEP is active
    -> restart/refresh the 300 ms short-beep timer from the newest press
    -> do not queue multiple delayed acoustic beeps
```

This keeps input handling responsive and matches the existing non-blocking driver model.

If an ALARM sequence is active, DEC-14 does not apply; alarm priority from API-BUZ applies.

## DEC-15 — Repeated alarm-start request robustness

Application one-shot logic should prevent duplicate `Buzzer_StartAlarm()` requests. As a defensive Driver semantic, calling `Buzzer_StartAlarm()` while an alarm sequence is already active must not restart the 5-second timer.

This prevents a software defect outside the Driver from extending the alarm indefinitely.

## DEC-16 — Undefined button/state combinations

The official task specifies button actions only in the states where they are meaningful. For every other button/state combination, the team freezes:

```text
FSM state unchanged
Clock/alarm value unchanged
button event still counts as user activity
official 0.3 s key-feedback request still occurs
```

Examples:

```text
NORMAL + SW6
NORMAL + SW10
SET_TIME_HOUR + SW16
SET_TIME_MINUTE + SW16
SET_ALARM_HOUR + SW3
SET_ALARM_MINUTE + SW3
```

This extends the existing Core “undefined input = stable state” philosophy into a complete system-level rule and prevents future modules from inventing cross-mode shortcuts.

---



## 4.1 Business-logic invariants — review shortcuts

These invariants are intentionally short so future bug review can first ask “which invariant was violated?” before reading implementation details.

```text
INV-01  Only SW3 controls the current-time setting path.
INV-02  Only SW16 controls the alarm-setting path.
INV-03  SW6/SW10 change values only inside the matching SET context.
INV-04  Undefined button/state combinations never change mode or stored values.
INV-05  Every accepted button click counts as activity and requests key feedback.
INV-06  Timeout is generated only after 30 s with no accepted debounced click in a SET session.
INV-07  Current-time edits are live; alarm edits are temporary until final SW16 confirmation.
INV-08  Only confirmed alarm data may become runtime-valid/persistent.
INV-09  Alarm acoustic trigger comes from a natural logical Clock tick reaching confirmed HH:MM:00, not from configuration actions.
INV-10  One logical alarm occurrence starts at most one 5 s alarm sequence.
INV-11  Alarm sequence has acoustic priority over 300 ms feedback.
INV-12  Application never accesses raw MCU hardware/register APIs.
```

These invariants do not replace the detailed DEC/REQ/API sections; they are the first-line review checklist.

---

# 5. EVENT / STATE INTEGRATION MATRIX

Legend:
- `BEEP` = request 300 ms key beep.
- `RESET-TMO` = restart inactivity timer if resulting state remains SET.
- `START-TMO` = fresh inactivity timer on entry from NORMAL.
- `STOP-TMO` = timeout inactive in NORMAL.

| State before | Event | Core/System action | State after | Feedback/timer |
|---|---|---|---|---|
| NORMAL | SW3 | enter time-hour edit; pause Clock schedule | SET_TIME_HOUR | BEEP + START-TMO |
| SET_TIME_HOUR | SW3 | select minute edit | SET_TIME_MINUTE | BEEP + RESET-TMO |
| SET_TIME_MINUTE | SW3 | finish current-time edit; resume fresh 1 s anchor | NORMAL | BEEP + STOP-TMO |
| NORMAL | SW16 | copy confirmed alarm -> temp edit | SET_ALARM_HOUR | BEEP + START-TMO |
| SET_ALARM_HOUR | SW16 | select alarm-minute edit | SET_ALARM_MINUTE | BEEP + RESET-TMO |
| SET_ALARM_MINUTE | SW16 | commit alarm + apply DEC-13 latch + attempt EEPROM save | NORMAL | BEEP + STOP-TMO |
| SET_TIME_HOUR | SW6/SW10 | FSM changes Clock hour +/- with wrap | same | BEEP + RESET-TMO |
| SET_TIME_MINUTE | SW6/SW10 | FSM changes Clock minute +/- with wrap | same | BEEP + RESET-TMO |
| SET_ALARM_HOUR | SW6/SW10 | Application changes temp alarm hour +/- with wrap | same | BEEP + RESET-TMO |
| SET_ALARM_MINUTE | SW6/SW10 | Application changes temp alarm minute +/- with wrap | same | BEEP + RESET-TMO |
| any SET | TIMEOUT_30S | alarm edit discarded if applicable; current-time edits retained | NORMAL | timeout BEEP + STOP-TMO |
| any button/state combination not defined by the FSM | debounced button event | DEC-16: no state/value action | unchanged | BEEP; RESET-TMO if current state is SET |

---

# 6. EVENT DISPATCH ORDER — FROZEN

Application must preserve `state_before` and process queued button events in FIFO order.

The function names below are **pseudocode responsibilities**, not additional frozen APIs.

For each event drained from `Button_GetEvent()`:

```c
state_before = FSM_GetState();
event = map_button_event(button_event);

/* system feedback/activity accounting */
handle_button_feedback_and_activity(state_before, event);

/* actions requiring knowledge of the pre-transition state */
Application_HandlePreTransitionAction(state_before, event);

/* existing Core transition/current-time actions */
FSM_HandleEvent(event);

state_after = FSM_GetState();

/* entry/exit scheduling and committed post-actions */
Application_HandlePostTransitionAction(state_before, event, state_after);
```

Critical rules:
- do not duplicate Clock +/- in Application for current-time SET states;
- alarm temporary +/- belongs to Application because current FSM Core does not own the alarm edit buffer;
- final alarm confirmation must not be lost after FSM changes to NORMAL;
- state entry/exit controls timeout and Clock pause/resume anchors.

Timeout is a separate synthetic FSM event, not a Button FIFO event. After queued button events are processed, if DEC-11 does not suppress timeout and the SET inactivity interval has expired:

```c
state_before = FSM_GetState();
Application_HandleTimeoutPreAction(state_before);  /* discard alarm temp if needed */
FSM_HandleEvent(FSM_EVENT_TIMEOUT_30S);
Buzzer_BeepShort();
```

The names above remain pseudocode responsibilities, not public APIs.

---

# 7. SYSTEM PROCESSING ORDER — FROZEN

To remove ambiguity at timing boundaries, the production foreground cycle uses this logical priority. Function names are responsibilities, not new public APIs.

```text
A. Read one coherent `now = Timer_GetTickMs()`

B. Advance due Clock seconds if Clock is not paused
   - for EACH `Clock_Tick1Second()` performed:
       evaluate the currently confirmed alarm occurrence
   - never advance Clock during `SET_TIME_HOUR/MINUTE`

C. Run Application-required foreground Driver process functions
   - Button_Process()
   - Buzzer_Process()
   - LED_Process()

   Display blink/scan servicing remains Trung-owned and is not an Application scheduling dependency.

D. Drain all currently queued Button events in FIFO order
   - beep/activity accounting
   - pre-transition Application action
   - FSM_HandleEvent()
   - post-transition Application action

E. Evaluate timeout
   - only after queued button events
   - DEC-11: a valid queued button event wins over timeout for that processing boundary

F. Apply non-trigger alarm bookkeeping after foreground configuration changes
   - final alarm confirmation applies DEC-13
   - manual current-time edits do NOT create an alarm trigger (DEC-06)

G. Update Application-requested display value/blink mode and LED mode

H. Continue normal cooperative loop
```

### Why alarm is evaluated inside each Clock catch-up step

If the foreground loop is delayed and the Clock needs more than one catch-up tick:

```text
07:29:59
  -> 07:30:00   [alarm occurrence]
  -> 07:30:01
```

checking `Alarm_Check()` only after the final catch-up value would miss the `07:30:00` occurrence.

Therefore every logical one-second Clock advancement must have an alarm-occurrence evaluation before another second is applied.

`Alarm_Check()` is not used as a generic “poll and fire whenever values happen to match” after manual setting actions. Trigger generation is tied to logical Clock advancement per DEC-06.

### Same-cycle priority

If a due Clock second and a button event are both observed in one foreground cycle:

```text
due Clock second is accounted for first
then queued button input is processed
then timeout is evaluated
```

This avoids losing a full second when a button enters a paused time-setting state exactly at a one-second boundary.

Alarm output still has acoustic priority over a simultaneous short-beep request.

---

# 8. DISPLAY / LED OUTPUT MATRIX

| FSM state | Display source | Blink | LED D4 |
|---|---|---|---|
| NORMAL | `Clock_GetTime()` HH.MM | none | OFF |
| SET_TIME_HOUR | Clock HH.MM | HH | OFF |
| SET_TIME_MINUTE | Clock HH.MM | MM | OFF |
| SET_ALARM_HOUR | temporary alarm HH.MM | HH | 0.5 ON / 0.5 OFF |
| SET_ALARM_MINUTE | temporary alarm HH.MM | MM | 0.5 ON / 0.5 OFF |

Display separator:
```text
HH.MM
```

Driver must guarantee stable multiplex refresh independently enough that an EEPROM transaction does not visibly break the required display behavior.

Exact internal scheduling mechanism remains Trung-owned.

---

# 9. STARTUP CONTRACT

Required logical order:

```text
1. Board_Init()
2. Driver logical initialization:
      Button_Init()
      Display_Init()
      Buzzer_Init()
      LED_Init()
   (No Application-level `EEPROM_Init()` dependency; Board owns low-level I2C initialization.)
3. Clock_Init()
4. FSM_Init()
5. Alarm_Init()
6. Initialize Application integration state:
      alarm_valid = 0
      alarm_match_latched = 0
      timeout inactive
      clock scheduling anchor initialized
7. Apply DEC-10: EEPROM_ReadAlarm(...)
8. If valid:
      Alarm_SetTime(...)
      alarm_valid = 1
      apply DEC-07 startup-match latch policy
9. Display current Clock as 00.00
10. LED OFF
11. enter cooperative runtime
```

Initialization ownership:
- Board/Platform owns low-level GPIO/Timer/I2C initialization.
- Driver init owns driver logical state.
- If Trung retains `EEPROM_Init()`, it must be idempotent/logical and must not unexpectedly disrupt an already initialized shared I2C service; Tuấn's Application does not require it.


### Startup interrupt/service safety

Current `Board_Init()` starts the system Timer before Driver logical initialization. Therefore:

- the raw millisecond tick ISR may run immediately after `Board_Init()`;
- **no ISR/timer-driven Driver service may access Display/Button/Buzzer/LED state before that Driver has been initialized**.

If Trung implements deterministic display refresh from the Timer interrupt path, one of these must be true:

```text
A. display scan service is gated OFF until Display_Init() completes;
or
B. Driver initialization occurs before that service is enabled;
or
C. another equally safe initialization sequence is used.
```

No uninitialized Driver state may be consumed asynchronously during startup.

---

# 10. TIMING / CONCURRENCY CONTRACT

## TIM-01 — Tick
`Timer_GetTickMs()` must return a coherent/atomic snapshot on the 8-bit MCU.

If the implementation temporarily masks interrupts, it must restore the **previous interrupt-enable state**, not blindly enable interrupts afterward. `Timer_HasElapsed()` must obtain time through the same coherent snapshot mechanism or an equivalently safe implementation.

## TIM-02 — Wrap-safe elapsed checks
Elapsed-time logic uses unsigned subtraction:
```c
(now - start) >= duration
```
so 32-bit tick wrap remains safe for all configured project intervals, which are far below the 32-bit wrap period. No timing contract may assume elapsed intervals spanning an entire counter wrap.

## TIM-03 — Clock scheduler
Outside paused current-time SET states:

```c
now = Timer_GetTickMs();

while ((unsigned long)(now - last_clock_tick) >= 1000UL)
{
    last_clock_tick += 1000UL;
    Clock_Tick1Second();

    /* Required integration responsibility:
       evaluate the confirmed alarm occurrence at this exact logical second
       before applying another catch-up second. */
    evaluate_alarm_occurrence();
}
```

`evaluate_alarm_occurrence()` above is pseudocode, not a new public API.

When leaving paused current-time edit:
```text
last_clock_tick = current tick
```
and no paused-time catch-up occurs.

The production foreground loop must not be allowed to block for multi-second periods under normal operation. The catch-up loop exists for scheduler jitter/recovery, not as permission for long blocking tasks.

## TIM-04 — Foreground ownership
Unless explicitly redesigned and reviewed:
```text
Button_Process
FSM/Application
Buzzer_Process
LED_Process
EEPROM operations
```
are foreground operations in the v2.7 baseline.

## TIM-05 — Display scan
Display multiplex timing must be deterministic and resistant to foreground blocking. Internal mechanism is Driver/Platform-owned.

## TIM-06 — Timer accuracy
Physical 1 ms accuracy must be validated from the real oscillator/CPU/timer clock chain. Simulator/manual ISR calls prove logic, not wall-clock accuracy.


## TIM-07 — ISR/foreground shared-state safety
If Trung services `Display_UpdateBlinkState()` and/or `Display_ScanRoutine()` from an ISR or other asynchronous hardware-timed context while foreground code calls `Display_SetTime()`, `Display_SetBlinkMode()`, or `Display_SetColon()`, the implementation must prevent unsafe/torn shared display state.

Minimum observable requirement:
```text
no corrupted segment index
no invalid memory access
no visibly torn HH.MM update beyond an unavoidable sub-frame transition
```

Implementation may use byte-atomic design, a short critical section, double buffering, or another C51-safe method. The exact mechanism remains Trung-owned.

## TIM-08 — Periodic Driver process contract
`Button_Process()`, `Buzzer_Process()`, and `LED_Process()` may be called frequently from foreground; their timing behavior must derive from the system tick rather than from the exact foreground call count.

Display blink/scan servicing is Driver/Platform-owned and may use its own deterministic timing mechanism. Driver documentation must clearly distinguish Application-called foreground process functions from internal Display service functions.


## TIM-09 — No skipped alarm occurrence during Clock catch-up
Application must evaluate the confirmed alarm after every logical `Clock_Tick1Second()` step, not only after the final value of a multi-second catch-up loop.

This is an Application Integration correctness requirement and does not change the Alarm Core API.

## TIM-10 — Foreground service latency
Normal production code must keep the cooperative foreground loop responsive enough that Button/Buzzer/LED process functions are serviced regularly.

Blocking EEPROM activity is permitted only if it does not violate the observable 300 ms / 500 ms / 30 s behavior. If hardware measurement shows that blocking writes cause unacceptable timing distortion, Trung must improve EEPROM waiting (for example ACK polling/state-machine work) or move the affected timing-critical service behind an appropriate Platform/Driver mechanism without changing the frozen Application boundary.


## TIM-11 — Driver service activation after initialization
Any asynchronous/timer-driven Driver service must have an explicit initialized/enable condition. Starting Timer0 is not, by itself, permission to execute Driver logic whose `*_Init()` has not completed.

This rule is especially relevant if Display scanning is moved behind the Timer ISR/service path.


## TIM-12 — Periodic phase stability under foreground jitter
Periodic 500 ms behaviors must not accumulate phase drift merely because `*_Process()` is called a little late.

For Display blink, LED blink and Buzzer alarm phases, implementation should preserve the intended time base using one of these equivalent patterns:

```text
advance last deadline by whole configured periods
derive phase from elapsed time since sequence start
or another tick-based scheme with no cumulative scheduler drift
```

Do not repeatedly use `last_tick = current_tick` in a way that permanently shifts every future 500 ms boundary after each small scheduling delay.

Physical transitions may still be delayed by the finite foreground service latency; TIM-10 remains applicable.

---

# 11. DRIVER / PLATFORM MANDATORY QUALITY GATES

These are merge blockers if violated.

| ID | Owner | Gate |
|---|---|---|
| IMP-TMR-01 | Trung | Atomic 32-bit tick read; any temporary interrupt masking preserves/restores the previous interrupt state. |
| IMP-TMR-02 | Trung | Firmware clock-chain assumptions/configuration are internally consistent; Timer reload is derived from the verified configured CPU/timer input clock; docs match actual Mode 1/software reload behavior. |
| IMP-TMR-03 | Trung | `Timer_DelayUs()` not falsely claimed calibrated without proof. |
| IMP-TMR-HW-01 | Trung | Physical 1 ms timing accuracy/drift measured on EVK before final hardware PASS. May remain HW-PENDING at PR merge if software configuration is already coherent. |
| IMP-I2C-01 | Trung | Software I2C drive/release/ACK logic is electrically correct for open-drain-style operation; no unsafe push-pull HIGH assumption. |
| IMP-I2C-HW-01 | Trung | Actual EVK SDA/SCL pull-ups and physical bus behavior verified. May remain HW-PENDING if schematic/config evidence is documented and software logic is correct. |
| IMP-BTN-01 | Trung | Button input mode/pull-up configuration is explicitly correct in software/configuration. |
| IMP-BTN-HW-01 | Trung | Physical idle HIGH / pressed LOW behavior for SW3/SW6/SW10/SW16 verified on EVK. May remain HW-PENDING. |
| IMP-BTN-02 | Trung | Hold test proves first click AND no retrigger until release. |
| IMP-BTN-03 | Trung | Button event FIFO behavior/overflow policy remains documented and memory-safe; Application-facing enum names remain compatible. |
| IMP-LED-01 | Trung | `LED_SetMode(same_mode)` does not restart blink timing. |
| IMP-BUZ-01 | Trung | Short beep cannot cancel/restart/shorten active 5 s alarm sequence. |
| IMP-BUZ-02 | Trung | Test proves 0.5 s ON/OFF transitions, not only total busy duration. |
| IMP-BUZ-03 | Trung | Rapid short-beep refresh, alarm-preempts-short-beep, short-feedback-does-not-break-alarm, and repeated-alarm-start semantics match the frozen priority rules. |
| IMP-EEP-01 | Trung | Persistence update is transaction-safe: invalidate -> hour -> minute -> valid marker last, or equivalent robust scheme. |
| IMP-EEP-02 | Trung | 24C05 naming consistent across code/docs. |
| IMP-INIT-01 | Trung | `Board_Init()` owns low-level I2C initialization; any retained `EEPROM_Init()` is harmless/idempotent and not required by Application. |
| IMP-INIT-02 | Trung | Any ISR/timer-driven Driver service is disabled/gated until the corresponding Driver initialization is complete. |
| IMP-PIN-01 | Trung | `pin_config.h` and actual C51 hardware binding cannot silently disagree. |
| IMP-WDT-01 | Trung | Implement frozen `Board_FeedWatchdog()` abstraction: real feed when WDT enabled, documented safe no-op when disabled; Application never touches `WDTR`. |
| IMP-BUILD-01 | Shared | Production target contains one production `main()` and excludes standalone `*_test.c` mains. |
| IMP-API-01 | Both | Exact cross-boundary enum names and function prototypes in Section 3 match merged headers. |
| IMP-CFG-01 | Both | Official timing values have one agreed configuration source: key/timeout beep 300 ms, blink half-period 500 ms, alarm total 5000 ms, inactivity 30000 ms; no conflicting hard-coded production copies. |
| IMP-TIME-01 | Trung | Display/LED/Buzzer periodic phase logic does not accumulate scheduler-delay drift; timing is derived from stable tick/deadline semantics per TIM-12. |
| IMP-TEST-01 | Both | Test-only hooks/mocks/fault injection are allowed when needed for assertions, but are compile-time/test-only and never become Application boundary dependencies. |
| IMP-FSM-01 | Tuấn | All undefined button/state combinations obey DEC-16; no hidden cross-mode shortcuts are introduced during integration. |
| IMP-CONC-01 | Trung | Any ISR/foreground shared Display state is concurrency-safe per TIM-07. |
| IMP-DOC-01 | Trung | Driver specs/README scheduling statements match the final foreground-vs-ISR ownership in TIM-04/TIM-05/TIM-08. |
| IMP-DOC-02 | Trung | `Board_Init()` documentation does not claim system-clock initialization unless it actually configures the oscillator/CPU clock. |
| IMP-DOC-03 | Both | Report/workflow/API examples are updated to the frozen boundary; outdated illustrative APIs or “hardware must finish before integration” wording do not contradict the actual workflow. |

---

## 11.1 Verification instrumentation rule

Some Driver behavior cannot be proven by “function returned” alone because the relevant state is hardware-facing or private.

For simulator tests, the team may add test-only observability/fault-injection seams, for example:

```text
TEST BUILD ONLY
- read simulated buzzer logical output / driver state
- read display segment bitmap + active digit
- inject I2C ACK/NACK/write failure
- inspect queue count/state where necessary
```

Rules:

```text
1. guarded by a test build flag or otherwise excluded from production;
2. not called by Application Integration;
3. not added to the frozen boundary API in Section 3;
4. assertions compare actual observed state against expected state;
5. production behavior is unchanged when test hooks are absent.
```

This is preferred over unconditional `test_pass = 1` variables and makes future regression/fault-path testing reproducible.

---

# 12. TEST TRACEABILITY MATRIX

Status meanings:
```text
SW-PASS    verified in simulator/software
HW-PENDING requires physical EVK validation
PASS       final acceptance after applicable checks
FAIL       requirement not met
```

| Test ID | Requirement(s) | Test target | Owner | Minimum acceptance |
|---|---|---|---|---|
| T-CLK-01 | REQ-CLK-01/02 | Clock Core | Tuấn | init 00:00:00; 59/23 rollovers |
| T-SET-01 | REQ-SET-01..03 | FSM + Integration | Tuấn | exact SW3 state sequence + display blink mapping |
| T-ALMSET-01 | REQ-ALMSET-01..03 | FSM + Integration + EEPROM | Tuấn/Trung | temp edit; final commit once; persisted value correct |
| T-PLUS-01 | REQ-PLUS-01 | Core/Integration | Tuấn | hour/minute wrap + |
| T-MINUS-01 | REQ-MINUS-01 | Core/Integration | Tuấn | hour/minute wrap - |
| T-BTN-01 | REQ-BUZ-01 | Button + Buzzer + Integration | Both | each required button creates one event/beep |
| T-BUZ-01 | REQ-BUZ-01 | Buzzer | Trung | ~300 ms short beep |
| T-BUZ-02 | REQ-BUZ-02/IMP-TEST-01 | Buzzer | Trung | 5 s sequence; assert logical/physical output around 500 ms boundaries, not only busy duration |
| T-BUZ-03 | buzzer priority | Buzzer | Trung | key beep during alarm does not shorten/restart alarm |
| T-BUZ-04 | DEC-14/15 | Buzzer | Trung | rapid short beep refreshes 300 ms timer; repeated StartAlarm during ALARM does not restart sequence |
| T-BUZ-05 | buzzer priority | Buzzer | Trung | alarm start during active short beep preempts it; key/timeout short feedback during active alarm does not corrupt 5 s waveform |
| T-TMO-01 | REQ-TMO-01/02 | Integration | Tuấn | fresh 30 s inactivity, NORMAL + beep |
| T-TMO-02 | DEC-11 | Integration | Tuấn | queued valid key at timeout boundary is processed once; no simultaneous TIMEOUT dispatch |
| T-LED-01 | REQ-LED-01 | LED + Integration | Both | only alarm-setting states blink D4 |
| T-EEP-01 | REQ-EEP-01 | EEPROM | Trung | valid save/read + range rejection |
| T-EEP-02 | IMP-EEP-01/IMP-TEST-01 | EEPROM fault path | Trung | injected failure at each transaction stage cannot leave a mixed record accepted as valid |
| T-BOOT-01 | REQ-CLK-01 + DEC-07 + DEC-10 | Full system | Both | FSM NORMAL; Clock 00:00:00; display 00.00; LED OFF; buzzer OFF; valid EEPROM restore; restored matching 00:00 does not spuriously ring |
| T-BOOT-02 | DEC-10/12 | Integration | Tuấn/Trung | invalid/empty EEPROM leaves `alarm_valid=0`; first alarm edit begins from 00:00 |
| T-ALM-01 | REQ-BUZ-02 + REQ-EEP-01 | Full system | Both | confirmed alarm HH:MM:00 triggers once |
| T-ALM-02 | DEC-05 | Integration | Tuấn | same matching second does not retrigger |
| T-ALM-03 | DEC-13 | Integration | Tuấn | confirming an alarm equal to current `HH:MM:00` does not immediately trigger; latch rearms after mismatch |
| T-ALM-04 | DEC-06 | Integration | Tuấn | manual current-time +/- creating exact alarm `HH:MM:00` does not trigger; a later natural Clock tick occurrence does |
| T-EDIT-01 | DEC-01/02 | Integration | Tuấn | Clock pauses during time edit, seconds preserved, no catch-up burst |
| T-EDIT-02 | DEC-03 | Integration | Tuấn | alarm timeout discards temp and preserves old alarm |
| T-DSP-01 | display contract | Display | Trung | correct digits/blink logic in SW; physical multiplex HW-PENDING until board |
| T-DSP-02 | TIM-07 | Display concurrency | Trung | repeated foreground updates during scan service do not corrupt display state/memory |
| T-DSP-03 | IMP-TEST-01 | Display simulator | Trung | segment/digit/blink assertions observe real simulated output state; no unconditional PASS variables |
| T-DSP-BLINK-01 | REQ-SET-01/02 + REQ-ALMSET-01/02 | Display | Trung | HH/MM blink phase changes at the 500 ms half-period boundaries for the requested mode |
| T-TMR-01 | TIM-01/02 | Timer | Trung | coherent tick + wrap-safe elapsed |
| T-TMR-02 | TIM-06 + IMP-TMR-HW-01 | Hardware | Trung | measured timer interval/drift acceptable on EVK |
| T-ALM-CATCHUP-01 | TIM-09 | Integration | Tuấn | multi-second catch-up cannot skip an alarm occurrence in an intermediate second |
| T-ORDER-01 | Section 7 | Integration | Tuấn | due Clock tick + queued button + timeout boundary follow frozen priority without double-dispatch/lost second |
| T-UNDEF-01 | DEC-16 | Integration | Tuấn | undefined button/state combinations keep state/value stable while still producing activity/beep behavior |
| T-INIT-01 | TIM-11 | Startup/Driver | Trung | Timer/interrupt may start, but no asynchronous Driver service touches uninitialized Driver state |
| T-BTNQ-01 | API-BTN + DEC-11 | Integration | Both | queued events are consumed FIFO; no duplicate/lost event in normal human-input use |
| T-CFG-01 | IMP-CFG-01 | Build/static review | Both | timing constants used by Application/Drivers resolve to agreed official values without conflicting production literals |
| T-PHASE-01 | TIM-12/IMP-TIME-01 | Driver timing | Trung | injected foreground jitter does not accumulate phase drift across repeated Display/LED/Buzzer 500 ms periods |
| T-REG-01 | all | Full system | Both | full end-to-end regression |

---

# 13. APPLICATION COMPOSITION RESPONSIBILITY

Tuấn owns the production Application composition/root logic.

Recommended organization (not a cross-team filename freeze):

```text
firmware/app/application.c
firmware/app/application.h
firmware/app/main.c
```

Recommended logical entry points:

```c
void Application_Init(void);
void Application_Process(void);
```

These names/files are Tuấn-internal and may be refactored without a contract revision as long as the Driver/Platform boundary in Section 3 does not change.

Production requirements that **are** cross-team relevant:
- exactly one production `main()`;
- `Board_Init()` occurs before Application uses Driver/Platform services;
- test-runner `main()` files are excluded from the production target;
- Application does not manipulate raw hardware registers;
- the foreground loop calls `Board_FeedWatchdog()` through the frozen Platform abstraction; this is harmless when the Platform implements it as a no-op.

# 14. INTEGRATION ORDER — FROZEN

Integrate and test one boundary at a time:

```text
1. Production build + Board initialization
2. Timer -> Clock
3. Clock -> Display
4. Button -> FSM
5. FSM -> current-time setting
6. FSM -> temporary alarm setting
7. FSM -> LED policy
8. Button -> short buzzer feedback
9. 30 s timeout
10. EEPROM restore/save
11. Clock -> Alarm_Check -> one-shot
12. Alarm -> 5 s Buzzer sequence
13. Full regression
14. Physical EVK validation
```

Do not integrate all paths and debug them simultaneously.

---

# 15. DEBUG OWNERSHIP

```text
Clock/FSM/Alarm/Application logic
    -> Tuấn

Button/Display/Buzzer/LED/EEPROM
GPIO/I2C/Timer/ISR/pin/hardware
    -> Trung

Failure location unclear
    -> reproduce at public API boundary
    -> Tuấn + Trung inspect expected vs actual
    -> assign fix to owning side
```

No owner silently edits the other side's implementation merely to make a test pass.

---

# 16. PR #7 MERGE GATE

Before merge, review using this exact order:

```text
G1  public headers match Section 3
G2  no unapproved public API additions/removals
G3  all software/architecture quality gates in Section 11 CLOSED; only genuinely physical checks (pin electrical behavior, pull-ups, measured timing, physical multiplex quality) may remain HW-PENDING
G4  software tests assert real expected behavior
G5  hardware claims are not marked PASS without hardware evidence
G6  docs/spec/code/test/report/workflow terminology, API examples and scheduling ownership agree (especially Button/Buzzer/LED/Display/Timer)
G7  production Keil target is structurally ready
G8  build = 0 errors; warnings reviewed
G9  git diff contains no unrelated/generated junk
G10 test-only instrumentation is excluded from production behavior/API
G11 official timing constants/configuration are consistent
G12 both owners acknowledge the contract
G13 `Board_FeedWatchdog()` policy is implemented behind the already-frozen wrapper (no API decision remains open)
```

Only after G1..G13:
```text
PROPOSED -> ACTIVE / FROZEN v2.7
merge PR #7 -> develop
create feature/application-integration
```

---

# 17. CHANGE CONTROL AFTER FREEZE

A change is **breaking** if it alters:
- public function prototype/type/enum used across ownership boundary;
- return semantics;
- timing/priority behavior relied on by Application;
- event meaning;
- ownership boundary;
- initialization assumption.

Breaking change process:

```text
issue found
 -> identify requirement/test ID
 -> explain why current contract cannot satisfy it
 -> Tuấn + Trung agree
 -> update contract + affected spec/test in same change
 -> implement
 -> regression
```

Internal refactors that preserve the public contract do **not** require a contract revision.

---

# 18. FINAL SIGN-OFF BLOCK

```text
Contract: MCU Application <-> Driver Integration Contract
Version: 2.7 — ACTIVE / FROZEN

Tuấn:
[ ] Ownership accepted
[ ] Application/Core API accepted
[ ] TEAM decisions accepted
[ ] Test matrix accepted

Trung:
[ ] Ownership accepted
[ ] Driver/Platform boundary API + enum types accepted
[ ] Observable semantics accepted
[ ] TEAM conflict/edge-case decisions accepted
[ ] Startup/interrupt ownership accepted
[ ] Mandatory quality gates accepted

Final PR review:
[ ] Public headers match
[ ] Build clean
[ ] Software evidence valid and assertion-based
[ ] Test-only hooks excluded from production boundary
[ ] Timing/config constants consistent
[ ] HW-PENDING labels honest
[ ] No unresolved blocker

When all applicable boxes are checked:
STATUS = ACTIVE / FROZEN v2.7
```
