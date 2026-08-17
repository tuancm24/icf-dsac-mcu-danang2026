# Application Integration Preparation Plan v2.6

**Project:** ICF-DSAC MCU Da Nang 2026
**Target:** SONiX SN8F5708 EVK
**Owner:** Tuấn — Application/Core/Integration
**Status:** REVIEWED / READY PREPARATION BASELINE v2.6 — aligned to ACTIVE / FROZEN Contract v2.7; production integration implementation waits for the reviewed Driver/Platform baseline in `develop`
**Contract reference:** `MCU-APPLICATION-DRIVER-INTEGRATION-CONTRACT-v2.7.md`

> This document is an Application-side preparation artifact. It does not freeze Trung's Driver/Platform implementation and does not replace the cross-team Integration Contract.

> **Baseline rule:** Contract v2.7 is ACTIVE / FROZEN and is the cross-team integration source-of-truth. Any later breaking boundary/semantic change requires controlled contract impact review; only affected integration stages are paused while unrelated stages remain valid.

> **Current workflow state:** the contract gate is complete; the remaining production-integration gate is the reviewed Driver/Platform baseline plus the production build/config checks in Section 3. Hardware-internal findings may be fixed by Trung without changing this Plan when the frozen Application boundary and observable semantics remain unchanged.


---

## 1. Objective

Prepare the Application Integration work so implementation can start immediately after the reviewed Driver/Platform baseline that conforms to frozen Contract v2.7 is merged into `develop`.

The preparation must:

- preserve the already verified Clock/FSM/Alarm Core;
- avoid guessing or depending on non-boundary Driver APIs;
- define Application-owned state and responsibilities;
- define the integration order;
- define review checkpoints and rollback points;
- identify the integration-test responsibilities that the separate Test Plan must cover before production integration code is written.

---

## 2. Current Application/Core Baseline

Application/Core already contains:

```text
firmware/app/
├── clock.c
├── clock.h
├── clock_test.c
├── fsm.c
├── fsm.h
├── fsm_test.c
├── alarm.c
├── alarm.h
└── alarm_test.c
```

Current verified responsibilities:

```text
Clock Core
- HH:MM:SS state
- 1-second rollover
- hour/minute increment/decrement wrap

FSM Core
- NORMAL
- SET_TIME_HOUR
- SET_TIME_MINUTE
- SET_ALARM_HOUR
- SET_ALARM_MINUTE
- current-time edit transitions/actions
- timeout transition to NORMAL

Alarm Core
- confirmed alarm HH:MM:00
- exact Alarm_Check(Time_t)
```

Application Integration must **compose** these modules. It must not move hardware behavior into the Core.

---

## 3. Preconditions Before Production Integration Code

Production integration implementation starts only after:

```text
P1  Trung reviews the v2.7 contract.
P2  Cross-team boundary APIs/semantics are accepted.
P3  Mandatory software/architecture blockers for PR #7 are closed.
P4  Final public headers are checked against the agreed contract.
P5  Both owners acknowledge the contract and it becomes ACTIVE / FROZEN.
P6  PR #7 is merged into `develop`.
P7  Tuấn pulls the reviewed `develop` baseline and records the baseline commit used to create `feature/application-integration`.
P8  Production target structure is ready: exactly one production `main()`, standalone test-runner mains excluded, and test-only instrumentation excluded from production behavior/API.
P9  Production build is structurally clean: 0 errors; warnings reviewed.
P10 Shared timing/config constants used by Application/Drivers are consistent with the frozen contract.
P11 Any required `firmware/project/` production-target changes are reviewed as Shared ownership before merge.
```

Hardware-only validation may remain explicitly `HW-PENDING` where allowed by the contract.

---

## 4. Application-Owned Integration State

The Application layer is expected to own integration state such as:

```c
/* Conceptual only — final names remain Application-internal. */

Time_t alarm_edit_time;

unsigned char alarm_valid;
unsigned char alarm_match_latched;

unsigned long last_clock_tick;
unsigned long last_activity_tick;

unsigned char timeout_active;
```

Notes:

- `alarm_edit_time` is temporary and exists only for transactional alarm editing.
- `alarm_valid` means a confirmed alarm is armed for the current runtime.
- `alarm_match_latched` prevents multiple starts for one logical alarm occurrence.
- latch lifecycle:
  - startup restore already matching current time -> latched for that existing occurrence;
  - alarm confirmation already matching current time -> latched for that existing occurrence;
  - natural Clock tick onto confirmed alarm with latch clear -> start alarm and set latch;
  - whenever required Application bookkeeping observes that the current Clock no longer matches the confirmed alarm -> clear latch/re-arm.
- latch clearing is therefore not restricted to the Clock-tick path; non-trigger configuration bookkeeping may also clear it when appropriate.
- persistence/debug status may be kept internally if useful, but is not a cross-team API requirement.

Application timing state must follow the Platform timing contract:

```text
elapsed checks use wrap-safe unsigned-delta semantics
(now - start) >= duration
```

or the frozen `Timer_HasElapsed()` wrapper. No Application timer may assume that the 32-bit tick never wraps.

---



## 5. Frozen v2.7 Responsibility Boundary for Application

Application may depend only on the final frozen boundary.

Frozen Application boundary:

Cross-boundary types/enums that Application code is allowed to rely on:

```text
Time_t

FSM_State
FSM_Event

Button_Event_t

Display_BlinkMode_t

LED_Mode_t
```

Expected functions:

```text
Clock / FSM / Alarm
    owned by Tuấn

Button
    Button_Init
    Button_Process
    Button_GetEvent

Display
    Display_Init
    Display_SetTime
    Display_SetColon
    Display_SetBlinkMode

Buzzer
    Buzzer_Init
    Buzzer_BeepShort
    Buzzer_StartAlarm
    Buzzer_Process

LED
    LED_Init
    LED_SetMode
    LED_Process

EEPROM
    EEPROM_SaveAlarm
    EEPROM_ReadAlarm

Platform
    Board_Init
    Board_FeedWatchdog
    Timer_GetTickMs
    Timer_HasElapsed
```

These are the accepted Application-facing dependencies from frozen v2.7. Production code must not expand this dependency set without controlled contract review.

A helper/support function appearing in Trung's headers does **not** automatically become an Application dependency. Application may depend only on the accepted Section-3 boundary.

Application must not depend on:

```text
GPIO_*
I2C_*
PIN_HW_*
raw MCU registers
WDTR
Timer_Init
Timer_ISR_Handler
Timer_DelayUs
Display_ScanRoutine
Display_UpdateBlinkState
Button_IsPressed
Buzzer_IsBusy
Buzzer_Stop
EEPROM_Init
```

---

## 6. Button-to-FSM Event Mapping

Preparation mapping:

```text
BUTTON_EVENT_SW3_CLICK
    -> FSM_EVENT_SW3

BUTTON_EVENT_SW6_CLICK
    -> FSM_EVENT_SW6

BUTTON_EVENT_SW10_CLICK
    -> FSM_EVENT_SW10

BUTTON_EVENT_SW16_CLICK
    -> FSM_EVENT_SW16

BUTTON_EVENT_NONE
    -> no FSM dispatch
```

The mapping function is Application-owned.

Undefined button/state combinations:

```text
state unchanged
Clock/alarm value unchanged
keypress feedback still requested
activity timer reset if currently in a SET session
```

Simultaneous/multi-key combinations are outside Application functional semantics in the v2.7 baseline. Application processes the debounced FIFO events supplied by the Button Driver; it does not invent a combined-key shortcut. Driver safety/memory integrity remains required even when multi-key behavior is not functionally accepted.

---

## 7. Application Event Dispatch Model

For every queued Button event:

```text
1. capture FSM state_before
2. map Button event -> FSM event
3. account for key feedback/activity
4. execute Application pre-transition action
5. call FSM_HandleEvent()
6. capture state_after
7. execute Application post-transition action
```

Key constraints:

- do not duplicate current-time `Clock_Increment*()` / `Clock_Decrement*()` already owned by the FSM Core;
- alarm temporary +/- belongs to Application Integration;
- final alarm confirmation must be recognized using `state_before`;
- state entry/exit controls timeout state and current-time Clock pause/resume;
- `NORMAL -> any SET` starts a fresh inactivity session in post-transition handling;
- an accepted click while already in SET refreshes activity before/through the transition;
- any transition to NORMAL disables the inactivity session.

Timeout is a separate synthetic event and is not inserted into the Button FIFO.

Buzzer responsibility rule:

```text
Application always issues the required short-feedback request for accepted button/timeout events.
Application does not query `Buzzer_IsBusy()` or suppress logical input handling.
Driver owns rapid short-beep refresh and alarm-over-feedback priority.
```

### Event action-phase matrix

The purpose of `state_before` is to make actions that would be lost after the FSM transition deterministic.

```text
NORMAL + SW3
    pre-transition:
        no Clock value edit
    FSM:
        -> SET_TIME_HOUR
    post-transition:
        pause Clock schedule
        discard partial sub-second phase
        start fresh inactivity session

NORMAL + SW16
    pre-transition:
        copy confirmed/default Alarm Core value -> alarm_edit_time
    FSM:
        -> SET_ALARM_HOUR
    post-transition:
        start fresh inactivity session

SET_ALARM_HOUR/MINUTE + SW6/SW10
    pre-transition:
        modify alarm_edit_time only
    FSM:
        state remains same
    post-transition:
        keep SET inactivity active/refreshed

SET_ALARM_MINUTE + SW16
    pre-transition:
        commit alarm exactly once
        apply confirmation-collision latch policy
        attempt EEPROM save
    FSM:
        -> NORMAL
    post-transition:
        disable inactivity session

SET_TIME_MINUTE + SW3
    pre-transition:
        no duplicate Clock edit
    FSM:
        -> NORMAL
    post-transition:
        disable inactivity session
        resume Clock with fresh one-second anchor

TIMEOUT from SET_ALARM_*
    timeout pre-action:
        discard temporary alarm edit
    FSM:
        -> NORMAL
    post-transition:
        disable inactivity session

TIMEOUT from SET_TIME_*
    timeout pre-action:
        keep live Clock edits already applied
    FSM:
        -> NORMAL
    post-transition:
        disable inactivity session
        resume Clock with fresh one-second anchor
```

This matrix is Application-internal sequencing guidance; it does not add public APIs.

---

## 8. Current-Time Setting Integration

Expected behavior:

```text
NORMAL
  + SW3
  -> SET_TIME_HOUR
  -> pause logical Clock second scheduling
  -> discard the partial sub-second scheduling phase

SET_TIME_HOUR
  + SW6 / SW10
  -> existing FSM adjusts Clock hour

SET_TIME_HOUR
  + SW3
  -> SET_TIME_MINUTE

SET_TIME_MINUTE
  + SW6 / SW10
  -> existing FSM adjusts Clock minute

SET_TIME_MINUTE
  + SW3
  -> NORMAL
  -> resume with a fresh 1-second scheduling anchor
```

Frozen v2.7 current-time scheduling rules:

```text
- seconds are preserved;
- paused setting time is not caught up later;
- current-time edits already made remain applied on timeout;
- EVERY exit from SET_TIME_* to NORMAL (SW3 completion or timeout)
  resumes from a fresh one-second scheduling anchor.
```

---

## 9. Alarm Setting Integration

Required state path:

```text
NORMAL + SW16
    -> SET_ALARM_HOUR

SET_ALARM_HOUR + SW16
    -> SET_ALARM_MINUTE

SET_ALARM_MINUTE + SW16
    -> confirm exactly once
    -> NORMAL
```

Alarm editing is transactional.

While `SET_ALARM_HOUR` / `SET_ALARM_MINUTE` is active:

```text
current Clock continues running
if alarm_valid == 1, the previous confirmed alarm remains armed
temporary edit is active for UI/editing only
temporary edit is NOT confirmed/armed and is NOT used for alarm triggering
```

Therefore, when `alarm_valid == 1`, if the running Clock naturally reaches the previous confirmed alarm during alarm editing, that old confirmed alarm may still trigger normally. If `alarm_valid == 0`, entering alarm-setting mode does not create an armed alarm.

On entry:

```text
NORMAL + SW16
    -> alarm_edit_time = Alarm_GetTime()
```

If no valid alarm exists, the Core default provides `00:00`.

Entering alarm-setting mode does **not** arm an alarm:

```text
NORMAL + SW16
    -> create/copy temporary edit value
    -> alarm_valid remains unchanged

Only:
    valid EEPROM restore
or  final SET_ALARM_MINUTE + SW16 confirmation
    -> may establish alarm_valid = 1
```

During edit:

```text
SET_ALARM_HOUR + SW6/SW10
    -> change only alarm_edit_time.hour

SET_ALARM_MINUTE + SW6/SW10
    -> change only alarm_edit_time.minute
```

Wrap:

```text
23 + 1 -> 00
00 - 1 -> 23

59 + 1 -> 00
00 - 1 -> 59
```

Final confirmation is a **pre-transition Application action** identified using `state_before`, so it cannot be lost when the FSM moves to NORMAL:

```text
SET_ALARM_MINUTE + SW16
    -> Alarm_SetTime(edit.hour, edit.minute)  /* confirmed alarm second remains 00 */
    -> alarm_valid = 1
    -> if current Clock already matches new alarm HH:MM:00:
           latch current occurrence WITHOUT starting buzzer
       else:
           clear/re-arm latch for the new confirmed alarm
    -> save_result = EEPROM_SaveAlarm(edit.hour, edit.minute) exactly once
    -> FSM then transitions to NORMAL
```

Persistence-result rule:

```text
save_result == 1
    -> new confirmed alarm is persistent

save_result == 0
    -> runtime confirmed alarm remains valid
    -> do NOT rollback Alarm_SetTime()
    -> do NOT invent an extra error beep/UI
    -> persistence is simply not guaranteed for reboot
```

`save_result` is conceptual Application-internal handling; the exact local variable name is not frozen.

Timeout:

```text
discard temporary edit
keep previous confirmed Alarm Core value
do not write EEPROM
return NORMAL
```

---

## 10. Clock Scheduling and Alarm Occurrence

Clock progression is driven from the Platform tick.

Conceptual scheduling:

```c
now = Timer_GetTickMs();

while ((unsigned long)(now - last_clock_tick) >= 1000UL)
{
    last_clock_tick += 1000UL;

    Clock_Tick1Second();

    /* Evaluate the confirmed alarm at this exact logical second. */
    evaluate_alarm_occurrence();
}
```

Important:

```text
Alarm_Check() = match predicate only.
```

A new alarm occurrence is generated only when a logical Clock tick advances to the confirmed `HH:MM:00`.

`Timer_HasElapsed()` remains available for ordinary elapsed-time checks such as inactivity/session timing. The Clock scheduler deliberately uses tick-delta/deadline logic because it must determine and process **how many** logical one-second steps are due, not merely whether one interval elapsed.

Do not generate a new alarm merely because:

```text
- boot restore matches current time;
- manual current-time editing creates a match;
- alarm confirmation equals current time.
```

Latch bookkeeping:

```text
natural logical Clock tick reaches confirmed alarm and latch == 0
    -> request Buzzer_StartAlarm()
    -> latch = 1

later logical Clock value no longer matches confirmed alarm
    -> latch = 0

startup/confirmation already equal to alarm
    -> latch current occurrence without starting buzzer
```

---

## 11. Inactivity Timeout

Session rules:

```text
NORMAL -> SET
    start fresh inactivity timer

accepted debounced button click while SET
    restart inactivity timer

return NORMAL
    disable inactivity timer
```

When a debounced click is consumed from the Button FIFO, Application refreshes the activity timestamp using the foreground cycle's coherent `now` snapshot from Section 14. The Driver does not need to expose event timestamps in the frozen boundary.

Timeout expiration uses wrap-safe elapsed-time semantics (`Timer_HasElapsed()` or equivalent unsigned subtraction).

At timeout:

```text
30 seconds without an accepted button click
    -> pre-timeout Application cleanup
    -> FSM_EVENT_TIMEOUT_30S
    -> request 300 ms timeout beep
```

Priority:

```text
queued valid Button event first
timeout second
```

A valid queued user event at the timeout boundary prevents a simultaneous timeout dispatch for that same processing cycle.

Per-state timeout cleanup:

```text
timeout from SET_TIME_HOUR / SET_TIME_MINUTE
    -> keep already-applied current-time edits
    -> transition NORMAL
    -> resume Clock from a fresh one-second anchor
    -> request timeout short beep

timeout from SET_ALARM_HOUR / SET_ALARM_MINUTE
    -> discard alarm_edit_time changes
    -> keep previous confirmed alarm/alarm_valid
    -> do NOT write EEPROM
    -> transition NORMAL
    -> request timeout short beep
```

---



## 12. Display and LED Requests

Application output policy:

| FSM State | Display Source | Blink | LED D4 |
|---|---|---|---|
| NORMAL | Clock HH.MM | none | OFF |
| SET_TIME_HOUR | Clock HH.MM | HH | OFF |
| SET_TIME_MINUTE | Clock HH.MM | MM | OFF |
| SET_ALARM_HOUR | temporary alarm HH.MM | HH | BLINK |
| SET_ALARM_MINUTE | temporary alarm HH.MM | MM | BLINK |

Application requests state only.

Expected Application request mapping:

```text
NORMAL
    Display_SetTime(Clock HH, MM)
    Display_SetColon(1)
    Display_SetBlinkMode(DISPLAY_BLINK_NONE)
    LED_SetMode(LED_MODE_OFF)

SET_TIME_HOUR
    Display_SetTime(Clock HH, MM)
    Display_SetColon(1)
    Display_SetBlinkMode(DISPLAY_BLINK_HOURS)
    LED_SetMode(LED_MODE_OFF)

SET_TIME_MINUTE
    Display_SetTime(Clock HH, MM)
    Display_SetColon(1)
    Display_SetBlinkMode(DISPLAY_BLINK_MINUTES)
    LED_SetMode(LED_MODE_OFF)

SET_ALARM_HOUR
    Display_SetTime(alarm_edit_time HH, MM)
    Display_SetColon(1)
    Display_SetBlinkMode(DISPLAY_BLINK_HOURS)
    LED_SetMode(LED_MODE_BLINK_ALARM_SETTING)

SET_ALARM_MINUTE
    Display_SetTime(alarm_edit_time HH, MM)
    Display_SetColon(1)
    Display_SetBlinkMode(DISPLAY_BLINK_MINUTES)
    LED_SetMode(LED_MODE_BLINK_ALARM_SETTING)
```

`Display_SetColon(1)` enables the `HH.MM` separator through the frozen Display API. The physical separator segment shape, wiring and polarity remain Driver-owned.

### Application output-update policy

Application must **not** rely on repeated identical Display setter calls to maintain blink timing.

**Recommended Application strategy:** use change-driven output requests:

```text
FSM state/blink requirement changes
    -> update Display_SetBlinkMode(...)

displayed HH/MM source value changes
    -> update Display_SetTime(...)

separator configuration changes / startup initialization
    -> update Display_SetColon(...)

LED requested mode changes
    -> update LED_SetMode(...)
```

The required invariant is that Application must not depend on repeated identical setter calls to maintain Driver-owned blink/scan timing. Change-driven requests are the recommended Application strategy, not a frozen implementation requirement. Driver-owned blink/scan services continue running independently between Application updates.

Application may implement this with state comparison/dirty flags or another internal strategy that preserves the same invariant; no new cross-team API is required. This remains Application-internal and must not be documented as an additional Driver API guarantee.

After startup initialization, the output cache/dirty state must force one complete initial synchronization (`00.00`, separator enabled, blink none, LED off) before normal change-driven suppression begins.

Driver/Platform owns:

```text
segment encoding
blink phase servicing
multiplex refresh
electrical polarity
```

---

## 13. Startup Preparation

Logical startup target:

```text
Board_Init()

Button_Init()
Display_Init()
Buzzer_Init()
LED_Init()

Clock_Init()
FSM_Init()
Alarm_Init()

alarm_valid = 0
alarm_match_latched = 0
timeout inactive
clock anchor initialized

EEPROM_ReadAlarm(...)
    valid:
        Alarm_SetTime(...)
        alarm_valid = 1
        apply startup-match latch rule
    invalid / empty / read error:
        keep alarm_valid = 0
        do not arm the Alarm Core default value

request initial outputs:
    FSM state = NORMAL
    Display current Clock as 00.00
    Display separator enabled
    Display blink mode = NONE
    LED OFF
    Buzzer remains OFF after Buzzer_Init()

enter foreground loop
```

No Application-level dependency on `EEPROM_Init()` is planned.

Startup asynchronous-service checkpoint:

```text
Board_Init() may start the system Timer before Driver logical initialization completes.
Any timer/ISR-driven Driver service must remain disabled/gated until the corresponding Driver state is initialized.
```

Application Integration must not assume that “Timer running” means “all Driver services are ready”. This is a Driver/Platform precondition that must be verified before full runtime integration.

---

## 14. Foreground Processing Order

Preparation order:

```text
A. snapshot one coherent current tick for the current foreground scheduling cycle
   -> use it consistently for Clock/timeout bookkeeping in that cycle unless a later operation explicitly requires a fresh timestamp

B. advance all due Clock logical seconds
   -> evaluate alarm after every second

C. service Application-called Drivers
   -> Button_Process
   -> Buzzer_Process
   -> LED_Process

D. drain Button FIFO
   -> dispatch each event in FIFO order

E. evaluate inactivity timeout

F. perform non-trigger alarm bookkeeping after configuration changes

G. apply any pending/change-driven Display/LED output requests

H. call Board_FeedWatchdog()
   -> every foreground cycle
   -> Application never accesses WDTR directly

I. repeat
```

Display scan/blink servicing remains Trung-owned and is not an Application scheduling dependency.

### Shared timing/config rule

Application must not introduce conflicting production literals for official timing behavior.

Agreed values:

```text
short feedback        300 ms
blink half-period     500 ms
alarm total          5000 ms
inactivity timeout  30000 ms
```

`firmware/config/` is shared ownership. Cross-impact timing/config changes require agreement before merge.

### Foreground latency / blocking rule

Application Integration must preserve the contract's cooperative-loop responsiveness:

```text
Button_Process()
Buzzer_Process()
LED_Process()
```

must continue to be serviced regularly.

EEPROM operations may execute in foreground, but integration must not treat long blocking as harmless. At `INT-10`, verify that EEPROM save/read behavior does not violate the observable:

```text
300 ms feedback
500 ms alarm/blink phase behavior
30 s inactivity behavior
```

If measured/simulated Driver blocking violates those behaviors, the fix belongs to the Driver/Platform implementation while preserving the frozen Application boundary.

Hardware-review findings such as button matrix scan direction, buzzer tone-generation method, exact EEPROM part identity, or hardware-I2C vs bit-bang implementation remain Driver/Platform-internal unless they change the frozen Application boundary or observable contract behavior. The Application Plan therefore does not prescribe those internal choices; it requires only the corresponding EVK behavior/evidence.

If the production watchdog is enabled, any blocking Driver operation must also remain compatible with the Platform watchdog policy. Application must not work around blocking by accessing `WDTR` directly; watchdog servicing remains behind `Board_FeedWatchdog()`.

---




## 15. Integration Implementation Order

After the accepted Driver/Platform baseline is available and, for the normal team workflow, merged into `develop`:

```text
INT-01  Production build / startup
        - exactly one production main()
        - test-runner mains excluded
        - Board_Init before Application uses Driver/Platform services
        - startup async-service safety verified
        - watchdog wrapper available
        - shared timing constants consistent
        - production build = 0 errors; warnings reviewed
        - test-only hooks/instrumentation excluded from production behavior/API
INT-02  Timer -> Clock
INT-03  Clock -> Display
INT-04  Button -> FSM
INT-05  Current-time setting
INT-06  Alarm temporary editing
INT-07  LED state policy
INT-08  Key feedback Buzzer
INT-09  30-second timeout
INT-10  EEPROM restore/save
        - runtime/persistence semantics correct
        - foreground blocking does not violate timing contract
INT-11  Alarm occurrence / one-shot
INT-12  5-second Buzzer alarm
INT-13  Full software regression
        - no cumulative 500 ms phase-drift regression is introduced across Display/LED/Buzzer integration
INT-14  Physical EVK validation
        - timer wall-clock accuracy/drift
        - physical button electrical behavior for SW3/SW6/SW10/SW16
        - physical display/multiplex quality and HH.MM separator
        - physical LED D4 OFF/blink behavior
        - audible Buzzer 0.3 s feedback and 5 s / 0.5 s ON-OFF alarm behavior
        - EEPROM/I2C ACK, write/read and reboot-restore behavior
```

Each stage must reach its required **software acceptance** before the next stage becomes the main debug target.

A stage may still carry an explicitly allowed `HW-PENDING` item when the contract says physical validation may wait. `HW-PENDING` is not a substitute for an unresolved software/architecture blocker.

---

## 16. Integration Checkpoints

Each integration stage must have a small review checkpoint before scope expands.

```text
CP-01  public boundary used is part of frozen contract
CP-02  no raw hardware/internal Driver API leaked into Application
CP-03  expected state/action flow matches v2.7 DEC/REQ rules
CP-04  relevant software test/evidence reaches SW-PASS/PASS as applicable
CP-05  any contract-allowed physical-only dependency is explicitly HW-PENDING
CP-06  failure ownership is known before code is changed
CP-07  any shared `firmware/config/` or `firmware/project/` change has both-owner review
CP-08  affected docs/spec/test terminology remains consistent with the frozen boundary
```

If a stage fails, keep the next stage out of scope until the current boundary is understood.

### Stage commit / rollback discipline

For implementation work after freeze:

```text
1. Start each INT-* stage from the last known passing baseline.
2. Keep the stage diff small and scoped to that integration boundary.
3. Run the stage checkpoint/tests.
4. After the stage reaches its required software acceptance (`SW-PASS` or `PASS`, as applicable), record a clean commit/checkpoint.
5. If the next stage fails, compare/reproduce against the last passing stage before broad refactoring.
6. Do not mix unrelated Driver/Core fixes into an integration-stage commit.
```

This gives the team an explicit rollback/reference point and makes later review/fix work cheaper.

---



## 17. Review / Debug Strategy

When an integration test fails:

```text
1. Identify expected public API input/output.
2. Reproduce at the ownership boundary.
3. Determine whether expected API semantics were met.
4. Assign the bug to the owning layer.
5. If ownership is still unclear, build the smallest boundary-level reproduction before changing production code.
```

Ownership:

```text
Clock/FSM/Alarm/Application policy
    -> Tuấn

Button/Display/Buzzer/LED/EEPROM
GPIO/I2C/Timer/ISR/hardware
    -> Trung
```

Do not silently edit the other owner's implementation to make a test pass.

---

## 18. Safe Work Before the Reviewed Driver Baseline

Contract v2.7 is already ACTIVE / FROZEN. Until the reviewed Driver/Platform baseline is accepted and merged into `develop`, safe Application-side work is:

```text
- keep this integration plan synchronized with the frozen contract;
- derive/review the separate Integration Test Plan;
- review existing Core regression;
- prepare event/state mapping and Application-owned pseudocode;
- prepare production build/checkpoint criteria;
- prepare expected evidence format;
- review Driver/Platform diffs and public headers without modifying Trung-owned implementation.
```

Do not do yet:

```text
- bind production Application code to an unreviewed Driver/Platform implementation;
- modify Trung's drivers/platform to make integration convenient;
- change shared config/project without both-owner agreement;
- mark Driver/HW behavior PASS without the required evidence;
- start full production integration before the reviewed Driver baseline gate is satisfied.
```

---

## 19. Contract-Deviation Contingency

Current baseline:

```text
Contract v2.7 = ACTIVE / FROZEN
-> this Plan proceeds against that frozen boundary
```

If Trung proposes a change:

```text
1. Identify the exact affected API / DEC / TIM / ownership rule.
2. Classify:
      internal implementation only
          -> no Plan semantic change; normal code update flows through the reviewed team branch/PR baseline
      breaking boundary / observable semantics
          -> impact-review required
3. Pause only affected INT-* stage(s).
4. Update contract first if both owners agree.
5. Update only impacted Plan sections/checkpoints.
6. Re-run affected Core/integration regression.
7. Resume unaffected integration stages normally.
```

Do not rewrite the whole plan for an internal Driver refactor that preserves the frozen boundary.

---

## 20. Future Test-Plan Coverage Handoff

File 2 must later derive executable tests from this Plan. File 1 only identifies the required coverage categories:

```text
TCOV-01  production build/startup and initial NORMAL outputs
TCOV-02  Timer -> Clock, wrap-safe elapsed handling and catch-up behavior
TCOV-03  Clock -> Display request mapping
TCOV-04  Button FIFO -> FSM mapping, pre/post action sequencing, undefined-input stability and multi-key out-of-scope safety boundary
TCOV-05  current-time setting, pause/resume and timeout exit
TCOV-06  transactional alarm editing and old-alarm-remains-armed behavior
TCOV-07  Display/LED state policy, change-driven output updates and Driver-owned blink independence
TCOV-08  key/timeout short-feedback requests and Driver priority boundary
TCOV-09  30-second inactivity and button-vs-timeout boundary
TCOV-10  EEPROM restore/save/failure integration semantics and foreground-latency impact
TCOV-11  natural alarm occurrence, latch/re-arm and non-trigger match cases
TCOV-12  alarm catch-up / processing-order boundaries
TCOV-13  watchdog/shared-config/shared-project/production-target integration checks
TCOV-14  full software regression, 500 ms phase-stability regression and EVK HW-PENDING handoff for buttons/display/LED/buzzer/EEPROM/timer
```

The separate Test Plan may expand these into detailed cases, fixtures and evidence rules, but must not change the Plan semantics.

---

## 21. File-1 Review Traceability

Before Git push, perform one final cross-check against the frozen v2.7 baseline:

```text
R1  Ownership matches contract Section 2.
R2  Application uses only Section-3 boundary APIs/types/enums; support/header helpers do not leak into dependencies.
R3  Current-time setting and wrap-safe scheduling match DEC-01/02 and TIM-02/03.
R4  Alarm editing/persistence matches DEC-03/06/08/10/12/13.
R5  Timeout/activity behavior and pre/post action sequencing match DEC-04/11/16 and Section 6.
R6  Alarm occurrence/latch behavior matches DEC-05/06/07/13.
R7  Startup ordering/safety matches Section 9 / TIM-11.
R8  Processing/output-service order matches Section 7 / TIM-03 / TIM-08 / TIM-09 / TIM-10 / TIM-12.
R9  Watchdog/shared-config/shared-project/build/test-only-instrumentation/HW-PENDING responsibilities match Sections 2/11/13/16.
R10 Contract-deviation handling matches Section 17 change control.
```

Any mismatch found here requires a new Plan revision before Git push.

---

## 22. Definition of Done — Preparation Task

This preparation task is DONE when:

```text
[ ] Integration responsibilities documented
[ ] Event mapping documented
[ ] State/action sequencing and pre/post transition phases documented
[ ] Clock/alarm scheduling and wrap-safe elapsed semantics documented
[ ] Timeout semantics documented
[ ] Display/LED output matrix and change-driven update policy documented
[ ] Integration order, phase-stability regression and software-vs-HW-PENDING progression documented
[ ] EVK handoff explicitly covers buttons, display, LED D4, audible buzzer, EEPROM/I2C and timer accuracy
[ ] Alarm-old-during-SET_ALARM behavior documented
[ ] Alarm latch lifecycle documented
[ ] Buzzer request/priority ownership documented
[ ] Startup asynchronous-service precondition documented
[ ] Watchdog foreground responsibility documented
[ ] Production target/build checkpoint documented
[ ] Shared timing/config/project ownership and foreground-latency constraint documented
[ ] Contract-deviation contingency documented
[ ] Stage commit/rollback discipline documented
[ ] Future integration-test responsibilities/coverage TCOV-01..14 identified
[ ] File-1 review traceability R1..R10 checked
[ ] No Driver/Platform implementation modified
[ ] No unaccepted API/type/enum dependency added to production code
```

Next documentation task after File 1 is READY:

```text
application-integration-test-plan.md
```

Production coding task after the reviewed Driver/Platform baseline is merged and the production preconditions in Section 3 are satisfied:

```text
feature/application-integration
```
