# Application Integration Test Plan — Draft v2.1

**Project:** ICF-DSAC MCU Da Nang 2026
**Target:** SONiX SN8F5708 EVK
**Owner:** Tuấn — Application/Core/Integration
**Baseline:** Application↔Driver Contract v2.7 + Application Integration Plan v2.6
**Status:** DRAFT — TCOV-01..14 reviewed; File-2 final-audit corrections applied; final diff-audit pending

> This Test Plan derives executable verification from the frozen Contract v2.7 and reviewed Integration Plan v2.6. It must not introduce new system semantics.

---

## 1. Test philosophy

A test result is accepted only when the evidence proves the expected result.

Use the Contract status vocabulary:

```text
SW-PASS    verified in simulator/software
HW-PENDING requires physical EVK validation
PASS       final acceptance after all applicable checks
FAIL       requirement not met
```

Evidence rules:

```text
1. Do not mark PASS from a function return alone when the behavior is hardware-facing/private.
2. Use assertion-based simulator evidence where software observability is possible.
3. Test-only hooks/mocks/fault injection must remain test-only and must not become Application boundary dependencies.
4. Hardware claims stay HW-PENDING until EVK evidence exists.
5. A failed test must identify the ownership boundary before production code is changed.
6. The Test Plan may expand coverage into detailed cases, but may not change Contract/Plan semantics.
```

---

## 2. Traceability format

Each test case records:

```text
Test ID
TCOV source
Contract / Plan trace
Purpose
Type
Owner
Preconditions
Stimulus / Procedure
Expected result
Required evidence
Acceptance
```

---

# 3. TCOV-01 — Production build / startup / initial NORMAL outputs

## TCOV-01 objective

Verify that the production target is structurally clean, starts through the agreed initialization boundary, restores alarm persistence safely, reaches the required NORMAL startup state, and does not allow asynchronous Driver service to touch uninitialized state.

TCOV-01 is complete only when every software-verifiable case below reaches SW-PASS and every physical-only item is explicitly carried as HW-PENDING until EVK validation.

---

## AITP-01-01 — Exactly one production `main()`

**Trace:** Plan P8 / INT-01; Contract IMP-BUILD-01 / Application composition requirements
**Type:** Build / static review
**Owner:** Shared

**Preconditions**
- Production Keil target is selected.
- Standalone unit-test targets/files remain available for tests but are not part of production execution.

**Procedure**
1. Inspect the production target/source list.
2. Search all files included in that target for definitions of `main()`.
3. Verify standalone `*_test.c` runners are excluded from the production target.

**Expected result**
- Exactly one production `main()` is linked.
- No standalone test-runner `main()` is linked into production.

**Required evidence**
- Keil target/source-list screenshot or exported build target listing.
- Search/result showing the single production `main()`.
- Production build log.

**Acceptance**
- `SW-PASS` when target structure is proven and build/link succeeds.
- `FAIL` if multiple production `main()` functions or test runners are linked.

---

## AITP-01-02 — Production build is structurally clean

**Trace:** Plan P9 / INT-01; Contract G7/G8
**Type:** Build
**Owner:** Shared

**Preconditions**
- AITP-01-01 target structure is correct.
- Frozen v2.7 public headers expected by Application are present.

**Procedure**
1. Clean the production target.
2. Rebuild the complete production target.
3. Record all compiler/linker warnings and errors.
4. Review every warning rather than accepting warning count blindly.

**Expected result**
- `0 errors`.
- Any warning is reviewed and shown not to indicate a correctness, target-structure, boundary, initialization, or memory problem.

**Required evidence**
- Complete clean-build log.
- Warning review notes if warning count is non-zero.

**Acceptance**
- `SW-PASS` only when build = 0 errors and warnings are reviewed.
- `FAIL` for any unresolved build error or correctness-relevant warning.

---

## AITP-01-03 — Startup initialization ordering

**Trace:** Plan Startup Preparation / INT-01; Contract Application composition responsibility
**Type:** Software integration / startup
**Owner:** Tuấn + Trung at ownership boundary

**Preconditions**
- Production initialization path is available.
- Driver initialization calls conform to frozen v2.7 boundary.

**Procedure**
1. Start from reset/cold-start path.
2. Observe or instrument initialization call ordering.
3. Verify `Board_Init()` occurs before Application uses Driver/Platform services.
4. Verify Driver init calls occur before their corresponding normal foreground service is relied upon.

**Expected result**
- `Board_Init()` is first at the cross-boundary platform startup.
- Application does not use uninitialized Driver/Platform services.
- No Application raw-register workaround is used.

**Required evidence**
- Startup call trace, simulator trace, or breakpoint sequence.
- Relevant production source excerpt/diff confirming order.

**Acceptance**
- `SW-PASS` when ordering is proven in production-equivalent software.
- `FAIL` if Application calls a service before its required initialization.

---

## AITP-01-04 — Asynchronous Driver service is gated until initialization

**Trace:** Contract TIM-11 / IMP-INIT-02 / T-INIT-01; Plan startup asynchronous-service checkpoint / INT-01
**Type:** Startup concurrency / Driver boundary
**Owner:** Trung

**Preconditions**
- Timer/interrupt may become active before all Driver logical initialization finishes.
- Any asynchronous Display or other Driver service used by the implementation is identifiable.

**Procedure**
1. Exercise startup with Timer/interrupt activation at the earliest production point.
2. Observe asynchronous Driver service before and during each corresponding `*_Init()`.
3. Use test-only instrumentation if private state must be observed.

**Expected result**
- Timer startup alone does not cause Driver logic to access uninitialized state.
- Each asynchronous service has an initialized/enable gate or equivalent safe mechanism.
- No invalid memory/state access occurs during startup.

**Required evidence**
- Assertion/trace from simulator or test build.
- Driver initialization/service code review evidence.

**Acceptance**
- `SW-PASS` when software safety is proven.
- Physical timing effects may remain `HW-PENDING` if they require EVK observation.
- `FAIL` if asynchronous service can run on uninitialized Driver state.

---

## AITP-01-05 — Core startup state and initial NORMAL outputs

**Trace:** REQ-CLK-01; Plan Startup Preparation; Contract T-BOOT-01
**Type:** Integration / observable startup
**Owner:** Both

**Preconditions**
- No startup fault prevents normal execution.
- Clock/FSM/Alarm Core initialize normally.

**Procedure**
1. Start/reset the system.
2. Observe Core state and initial output requests before user input.
3. Verify the first synchronized Application output state.

**Expected result**
- FSM = `NORMAL`.
- Clock = `00:00:00`.
- Display request = `00.00`.
- HH.MM separator enabled.
- Display blink = none.
- LED D4 request = OFF.
- Buzzer remains OFF after initialization.

**Required evidence**
- Simulator/watch trace for FSM/Clock/Application output requests.
- Display/LED/Buzzer physical appearance remains `HW-PENDING` until EVK validation.

**Acceptance**
- `SW-PASS` for logical/output-request behavior in software.
- Final physical output acceptance is completed under TCOV-14.
- `FAIL` if initial logical state/output request is incorrect.

---

## AITP-01-06 — Valid EEPROM restore at startup

**Trace:** Contract DEC-10 / T-BOOT-01; Plan Startup Preparation / TCOV-10 dependency
**Type:** Integration + persistence
**Owner:** Tuấn + Trung

**Preconditions**
- EEPROM test fixture contains a valid alarm record with hour/minute in range.
- `EEPROM_ReadAlarm()` can be observed or controlled in a test build.

**Procedure**
1. Arrange a valid persisted alarm value.
2. Cold-start Application initialization.
3. Observe `EEPROM_ReadAlarm()` result and Alarm/Application state after restore.

**Expected result**
- Valid persisted hour/minute are restored through `Alarm_SetTime(...)`.
- `alarm_valid = 1`.
- No extra/invented beep or UI behavior occurs merely because restore succeeded.

**Required evidence**
- Test fixture value.
- Call/state trace proving restored value and `alarm_valid`.
- Physical EEPROM storage remains `HW-PENDING` until EVK ACK/read/write/reboot evidence.

**Acceptance**
- `SW-PASS` for integration semantics using controlled persistence evidence.
- `HW-PENDING` for real EEPROM persistence until EVK validation.
- `FAIL` if a valid record is ignored, altered, or incorrectly leaves the alarm unarmed.

---

## AITP-01-07 — Restored alarm matching startup `00:00` does not spuriously ring

**Trace:** Contract DEC-07 / T-BOOT-01
**Type:** Integration edge case
**Owner:** Tuấn

**Preconditions**
- Valid persisted alarm = `00:00`.
- Power-on Clock = `00:00:00`.

**Procedure**
1. Arrange valid persisted alarm `00:00`.
2. Cold-start the system.
3. Observe alarm latch and buzzer-start requests during startup.
4. Advance Clock until it no longer matches, then later create a genuine future natural occurrence if needed by the fixture.

**Expected result**
- Startup restore does not request `Buzzer_StartAlarm()`.
- Existing startup occurrence is latched.
- Latch rearms only after Clock no longer matches.
- A later genuine natural occurrence may trigger normally.

**Required evidence**
- Trace of Clock value, `alarm_valid`, alarm-match latch, and buzzer-start request count.

**Acceptance**
- `SW-PASS` when no startup false trigger occurs and latch lifecycle is correct.
- `FAIL` if restored `00:00` rings immediately at power-on.

---

## AITP-01-08 — Invalid/empty/read-error EEPROM does not arm default alarm

**Trace:** Contract DEC-10 / DEC-12 / T-BOOT-02; Plan Startup Preparation
**Type:** Integration fault path
**Owner:** Tuấn + Trung

**Preconditions**
- Test fixture can produce invalid record, empty record, or `EEPROM_ReadAlarm()` failure.

**Procedure**
1. Execute startup separately for invalid/empty/read-error conditions.
2. Observe returned persistence status and Application alarm-valid state.
3. Enter alarm-setting mode before any valid alarm has been confirmed.

**Expected result**
- `alarm_valid = 0`.
- Core default `00:00` is not implicitly armed.
- First temporary alarm edit begins from `00:00`.
- No startup alarm is triggered.

**Required evidence**
- Fault-injection trace for each failure class used.
- Alarm state/value/latch observations.

**Acceptance**
- `SW-PASS` when invalid persistence never becomes an armed alarm.
- `FAIL` if default `00:00` becomes armed merely because restore failed.

---

## TCOV-01 exit gate

TCOV-01 may be marked software-complete only when:

```text
[ ] AITP-01-01 SW-PASS
[ ] AITP-01-02 SW-PASS
[ ] AITP-01-03 SW-PASS
[ ] AITP-01-04 SW-PASS
[ ] AITP-01-05 SW-PASS
[ ] AITP-01-06 SW-PASS (real EEPROM persistence may remain HW-PENDING)
[ ] AITP-01-07 SW-PASS
[ ] AITP-01-08 SW-PASS
```

TCOV-01 does **not** claim final hardware PASS for physical Display, LED, Buzzer, EEPROM, Button, or timer accuracy. Those are closed through later TCOV groups and TCOV-14 EVK validation.

### Cross-coverage references

The following concerns were intentionally removed from TCOV-01 to preserve one primary coverage owner:

```text
Watchdog abstraction / raw WDTR boundary
    -> primary owner: TCOV-13

Shared official timing/config consistency
    -> primary owner: TCOV-13

Test-only instrumentation / production-boundary cleanliness
    -> primary owner: TCOV-13
```

TCOV-13 may reuse build/startup evidence produced by TCOV-01, but should not duplicate the same acceptance decision under a second primary test ID.

EEPROM restore cases AITP-01-06..08 remain in TCOV-01 because they determine startup state. TCOV-10 will later test the broader persistence semantics and may reference these startup cases rather than duplicate them.

---

# 4. TCOV-02 — Timer -> Clock / elapsed-time / catch-up behavior

## TCOV-02 objective

Verify the Application-side Clock scheduling semantics defined by the frozen Contract v2.7 and Integration Plan v2.6: elapsed time is computed with wrap-safe unsigned arithmetic, logical one-second Clock ticks are generated from elapsed milliseconds, delayed foreground execution catches up deterministically, sub-second remainder is preserved, and paused current-time editing does not later replay elapsed setting time.

TCOV-02 verifies logical software timing semantics. Physical Timer accuracy and long-run EVK wall-clock accuracy remain `HW-PENDING` until hardware validation.

---

## AITP-02-01 — No Clock increment before one full logical second

**Trace:** Plan TCOV-02 / Timer -> Clock; Contract timing/catch-up semantics
**Type:** Software integration / timing boundary
**Owner:** Tuấn

**Preconditions**
- FSM is `NORMAL`.
- Clock scheduler anchor is known.
- Controlled `Timer_GetTickMs()` values are available in a test build.

**Procedure**
1. Set Clock to a known value, for example `12:34:56`.
2. Establish scheduler anchor at `t0`.
3. Present elapsed values strictly below one second, including `t0 + 1`, `t0 + 500`, and `t0 + 999 ms`.
4. Run the normal Application scheduling path after each value.

**Expected result**
- Clock remains `12:34:56`.
- No logical Clock tick is generated before accumulated elapsed time reaches one full second.

**Required evidence**
- Trace of `now_ms`, scheduler anchor/remainder state, Clock value, and logical tick count.

**Acceptance**
- `SW-PASS` when all sub-second boundary checks produce zero Clock ticks.
- `FAIL` if Clock advances before one full logical second.

---

## AITP-02-02 — Exactly one logical tick at the one-second boundary

**Trace:** Plan TCOV-02; Contract Timer -> Clock scheduling semantics
**Type:** Software integration / timing boundary
**Owner:** Tuấn

**Preconditions**
- FSM is `NORMAL`.
- Clock = `12:34:56`.
- Scheduler anchor = `t0`.

**Procedure**
1. Present `Timer_GetTickMs() = t0 + 1000 ms`.
2. Execute the normal scheduling path once.

**Expected result**
- Exactly one logical Clock tick is processed.
- Clock becomes `12:34:57`.
- No duplicate tick is generated from the same elapsed interval.

**Required evidence**
- Tick-count assertion and Clock before/after trace.

**Acceptance**
- `SW-PASS` for exactly one tick and correct Clock value.
- `FAIL` for zero, duplicate, or otherwise incorrect tick processing.

---

## AITP-02-03 — Delayed foreground execution catches up all whole seconds

**Trace:** Plan TCOV-02 / TCOV-12 dependency; Contract catch-up rule
**Type:** Software integration / delayed foreground
**Owner:** Tuấn

**Preconditions**
- FSM is `NORMAL`.
- Clock = `12:34:56`.
- Scheduler anchor = `t0`.
- Controlled elapsed time is available.

**Procedure**
1. Do not run the Clock scheduling path for `3500 ms`.
2. Present `now_ms = t0 + 3500 ms`.
3. Execute the normal scheduling path.

**Expected result**
- Exactly 3 logical one-second ticks are processed.
- Clock becomes `12:34:59`.
- `500 ms` remains as sub-second elapsed remainder for future scheduling.
- No elapsed whole second is silently discarded.

**Required evidence**
- Tick-count assertion.
- Clock before/after.
- Scheduler anchor/remainder trace.

**Acceptance**
- `SW-PASS` when 3 ticks are processed and the 500 ms remainder is preserved.
- `FAIL` if delayed foreground produces only one tick, loses whole seconds, or consumes the remainder incorrectly.

---

## AITP-02-04 — Preserved remainder completes the next logical second

**Trace:** Plan TCOV-02; Contract catch-up/remainder semantics
**Type:** Software integration / timing continuity
**Owner:** Tuấn

**Preconditions**
- Continue from AITP-02-03 with `500 ms` preserved remainder.
- Clock = `12:34:59`.

**Procedure**
1. Advance the controlled Timer by another `499 ms`.
2. Run the scheduler and observe Clock.
3. Advance by one additional millisecond.
4. Run the scheduler again.

**Expected result**
- After the first `499 ms`, no additional tick occurs.
- At the final `1 ms`, accumulated remainder reaches one second and exactly one tick occurs.
- Clock becomes `12:35:00`.

**Required evidence**
- Elapsed/remainder trace and tick-count assertions at both boundaries.

**Acceptance**
- `SW-PASS` when the preserved remainder contributes exactly to the next tick.
- `FAIL` if remainder is discarded, double-counted, or causes an early/late logical tick.

---

## AITP-02-05 — Unsigned millisecond wrap is handled correctly

**Trace:** Plan TCOV-02; Contract wrap-safe elapsed arithmetic
**Type:** Software integration / counter-wrap boundary
**Owner:** Tuấn

**Preconditions**
- Timer millisecond value uses the frozen unsigned counter type/semantics.
- Scheduler anchor is placed close to the maximum representable millisecond value.
- Clock is known.

**Procedure**
1. Use a reproducible full-second wrap vector, for example:
   - `last_clock_tick = 0xFFFFFF00UL`
   - `now_ms = 0x000003E8UL`
   - unsigned elapsed = `0x000004E8UL` = `1256 ms`
2. Execute the normal scheduler and record the number of logical Clock ticks plus the updated `last_clock_tick`.
3. Verify the same logical result against a non-wrap control interval of `1256 ms`.
4. Repeat with a reproducible sub-second wrap vector, for example:
   - `last_clock_tick = 0xFFFFFF00UL`
   - `now_ms = 0x00000100UL`
   - unsigned elapsed = `0x00000200UL` = `512 ms`
5. Execute the normal scheduler.

**Expected result**
- Elapsed time is interpreted modulo the unsigned 32-bit counter range.
- For the `1256 ms` vector, exactly one logical second is processed and the remaining `256 ms` phase is preserved by the scheduler anchor progression.
- The wrap case produces the same logical result as an equivalent non-wrap `1256 ms` interval.
- For the `512 ms` vector, no false Clock tick is generated.

**Required evidence**
- Exact pre-wrap and post-wrap counter values.
- Computed unsigned elapsed value for each vector.
- Tick-count, updated scheduler-anchor value, and Clock trace.
- Equivalent non-wrap control result for the full-second vector.

**Acceptance**
- `SW-PASS` when wrap-crossing behavior equals the same elapsed intervals without wrap.
- `FAIL` if wrap produces a huge false delay, freezes Clock, or changes logical tick count.

---

## AITP-02-06 — Multi-second catch-up preserves Clock rollover semantics

**Trace:** Plan TCOV-02; existing Clock Core regression dependency
**Type:** Integration / Clock rollover
**Owner:** Tuấn

**Preconditions**
- FSM is `NORMAL`.
- Scheduler anchor is known.
- Existing Clock Core behavior is already unit-tested.

**Procedure**
1. Set Clock near a rollover boundary, for example `23:59:58`.
2. Delay foreground Clock scheduling for `3000 ms`.
3. Execute catch-up.
4. Repeat around minute/hour boundaries as needed to prove integration does not bypass Core rollover behavior.

**Expected result**
- Three logical ticks are delegated through the normal Clock Core behavior.
- Example result: `23:59:58 -> 00:00:01`.
- Application scheduling does not implement a second competing rollover algorithm.

**Required evidence**
- Logical tick count.
- Clock before/after.
- Trace showing normal Clock Core tick path is used.

**Acceptance**
- `SW-PASS` when catch-up and Core rollover compose correctly.
- `FAIL` if Application-side catch-up corrupts or bypasses Clock Core rollover semantics.

---

## AITP-02-07 — SET_TIME pauses logical Clock progression

**Trace:** Plan TCOV-02 / TCOV-05 dependency; Contract current-time edit pause semantics
**Type:** Software integration / mode interaction
**Owner:** Tuấn

**Preconditions**
- Clock has a known value including seconds.
- System enters a current-time setting state through the normal FSM path.

**Procedure**
1. Enter `SET_TIME` at a known Clock value.
2. Advance controlled Timer by several seconds while remaining in current-time setting.
3. Continue executing the foreground loop as production would.
4. Observe Clock seconds throughout the paused interval.

**Expected result**
- Automatic one-second Clock progression is paused while current time is being edited.
- Seconds are preserved unless changed by an explicitly frozen Core operation; no hidden elapsed-time catch-up occurs while paused.

**Required evidence**
- FSM state, Clock value, Timer value, and logical tick-count trace.

**Acceptance**
- `SW-PASS` when no automatic Clock ticks are applied during SET_TIME.
- `FAIL` if Clock continues advancing automatically during current-time editing.

---

## AITP-02-08 — Leaving SET_TIME creates a fresh timing anchor with no catch-up

**Trace:** Plan TCOV-02 / TCOV-05; Contract fresh-anchor/no-catch-up rule
**Type:** Software integration / resume boundary
**Owner:** Tuấn

**Preconditions**
- System has remained in current-time setting for multiple real/logical Timer seconds.
- Clock seconds are preserved at a known value.

**Procedure**
1. Exit current-time setting to `NORMAL` through a normal accepted-button path.
2. Record the Timer value used for the fresh scheduling anchor.
3. Run the scheduler at `+999 ms`.
4. Run it again at `+1000 ms`.
5. Repeat the same check for timeout-driven exit from current-time setting.

**Expected result**
- Time spent in SET_TIME is not replayed as catch-up ticks.
- At `+999 ms` after the fresh anchor, no automatic tick occurs.
- At `+1000 ms`, exactly one automatic tick occurs.
- Button exit and timeout exit obey the same fresh-anchor rule.

**Required evidence**
- Exit reason/state trace.
- Fresh anchor value.
- Tick count at `+999/+1000 ms`.
- Clock before/after for both exit paths.

**Acceptance**
- `SW-PASS` when both exit paths resume from a fresh anchor without catch-up.
- `FAIL` if paused elapsed time is replayed or the first post-exit tick uses the stale pre-edit anchor.

---

## AITP-02-09 — Catch-up exposes logical ticks individually to higher-level alarm processing

**Trace:** Plan TCOV-02 with TCOV-11/TCOV-12 dependency; Contract processing-order/alarm occurrence semantics
**Type:** Software integration / sequencing
**Owner:** Tuấn

**Preconditions**
- FSM is `NORMAL`.
- A valid alarm can be arranged inside a multi-second catch-up interval.
- Test instrumentation can count logical Clock ticks and alarm-check opportunities without changing production semantics.

**Procedure**
1. Arrange Clock immediately before a future alarm occurrence.
2. Delay foreground execution so catch-up spans the alarm second and at least one additional second.
3. Execute normal catch-up processing.
4. Record each logical Clock value exposed to alarm occurrence processing.

**Expected result**
- Catch-up is not collapsed into a single final Clock value if doing so would skip per-tick higher-level semantics.
- Each logical tick required by the frozen processing model is observable in order.
- Detailed alarm trigger/latch acceptance remains owned by TCOV-11/TCOV-12.

**Required evidence**
- Ordered logical Clock-tick trace.
- Ordered alarm-processing opportunity trace.
- Cross-reference to later TCOV-11/12 evidence when available.

**Acceptance**
- `SW-PASS` when the catch-up mechanism preserves per-logical-tick processing opportunities.
- `FAIL` if catch-up jumps directly to the final time and can skip a natural alarm occurrence.

---

## TCOV-02 exit gate

TCOV-02 may be marked software-complete only when:

```text
[ ] AITP-02-01 SW-PASS
[ ] AITP-02-02 SW-PASS
[ ] AITP-02-03 SW-PASS
[ ] AITP-02-04 SW-PASS
[ ] AITP-02-05 SW-PASS
[ ] AITP-02-06 SW-PASS
[ ] AITP-02-07 SW-PASS
[ ] AITP-02-08 SW-PASS
[ ] AITP-02-09 SW-PASS
```

TCOV-02 establishes logical software timing correctness only. Physical Timer tick accuracy, oscillator-dependent drift, and long-run wall-clock accuracy remain `HW-PENDING` until EVK validation under TCOV-14.

---

# 5. TCOV-03 — Clock -> Display request mapping

## TCOV-03 objective

Verify that Application maps the current Clock value to the frozen Display boundary correctly in NORMAL and current-time setting states, requests the correct HH/MM blink mode for the active edit field, preserves the `HH.MM` separator policy, and does not take ownership of Driver-internal multiplex/blink timing.

TCOV-03 verifies Application-requested Display semantics. Physical segment wiring, multiplex quality, brightness, and real 500 ms blink phase remain Driver/hardware verification concerns and are not promoted to PASS here without the corresponding evidence.

---

## AITP-03-01 — NORMAL displays current Clock as HH.MM

**Trace:** Plan TCOV-03; Contract Display/LED Output Matrix
**Type:** Software integration / output mapping
**Owner:** Tuấn

**Preconditions**
- FSM = `NORMAL`.
- Clock is set to a known valid time.
- Display boundary calls/requests are observable in a test build.

**Procedure**
1. Set Clock to representative values such as `00:00`, `07:05`, `12:34`, and `23:59`.
2. Run the normal Application output-update path.
3. Record the value and display-mode requests sent through the frozen Display boundary.

**Expected result**
- Display value corresponds exactly to current Clock hour/minute.
- Application passes Clock `hour`/`minute` unchanged; the Driver representation must produce the required two-digit `HH.MM` presentation.
- Blink mode = none.
- `HH.MM` separator request = enabled.

**Required evidence**
- Clock value and Display request trace for each vector.
- No physical-segment PASS is claimed from this software trace alone.

**Acceptance**
- `SW-PASS` when every representative Clock value maps to the correct Display request.
- `FAIL` if Application requests stale/incorrect HH.MM data or an incorrect NORMAL blink/separator mode.

---

## AITP-03-02 — SET_TIME_HOUR displays live Clock and requests HH blink

**Trace:** Plan TCOV-03 / TCOV-05 dependency; Contract Output Matrix
**Type:** Software integration / state mapping
**Owner:** Tuấn

**Preconditions**
- FSM = `SET_TIME_HOUR`.
- Clock contains a known value.

**Procedure**
1. Enter `SET_TIME_HOUR` through the normal FSM path.
2. Run the Application output-update path.
3. Change current Clock hour through the frozen Core/FSM behavior.
4. Run output update again.

**Expected result**
- Display source remains the live current Clock.
- Updated hour is reflected in the Display request.
- Blink mode requests HH only.
- Separator remains enabled.
- Application does not implement its own blink phase/toggle loop.

**Required evidence**
- FSM state, Clock before/after, Display value request, blink-mode request, and separator request.

**Acceptance**
- `SW-PASS` when the live Clock value and HH blink request match the state.
- `FAIL` if Application uses an alarm/temp buffer, stale value, wrong blink field, or owns blink phase itself.

---

## AITP-03-03 — SET_TIME_MINUTE displays live Clock and requests MM blink

**Trace:** Plan TCOV-03 / TCOV-05 dependency; Contract Output Matrix
**Type:** Software integration / state mapping
**Owner:** Tuấn

**Preconditions**
- FSM = `SET_TIME_MINUTE`.
- Clock contains a known value.

**Procedure**
1. Enter `SET_TIME_MINUTE` through the normal FSM path.
2. Run output update.
3. Change current Clock minute through the frozen Core/FSM behavior.
4. Run output update again.

**Expected result**
- Display source remains the live current Clock.
- Updated minute is reflected in the Display request.
- Blink mode requests MM only.
- Separator remains enabled.

**Required evidence**
- FSM state, Clock before/after, Display value request, blink-mode request, and separator request.

**Acceptance**
- `SW-PASS` when current Clock and MM blink mapping are correct.
- `FAIL` for stale/incorrect value or incorrect blink selection.

---

## AITP-03-04 — Clock rollover is reflected in the next Display request

**Trace:** Plan TCOV-03 with TCOV-02 dependency; Contract NORMAL Display source
**Type:** Software integration / rollover mapping
**Owner:** Tuấn

**Preconditions**
- FSM = `NORMAL`.
- TCOV-02 logical Clock progression semantics are available.

**Procedure**
1. Arrange Clock immediately before representative boundaries:
   - `09:59:59`
   - `12:59:59`
   - `23:59:59`
2. Process the due logical Clock tick through the normal scheduler.
3. Run the normal output-update stage.
4. Record the resulting Display request.

**Expected result**
- Display request reflects the post-tick Clock HH.MM:
   - `09:59:59 -> 10.00`
   - `12:59:59 -> 13.00`
   - `23:59:59 -> 00.00`
- No stale pre-tick display value is retained by Application semantics.

**Required evidence**
- Ordered Clock-tick and Display-request trace.

**Acceptance**
- `SW-PASS` when post-rollover Clock values map correctly.
- `FAIL` if Application presents stale or independently calculated time.

---

## TCOV-03 exit gate

TCOV-03 may be marked software-complete only when:

```text
[ ] AITP-03-01 SW-PASS
[ ] AITP-03-02 SW-PASS
[ ] AITP-03-03 SW-PASS
[ ] AITP-03-04 SW-PASS
```

TCOV-03 proves the Clock-to-Display request path only. Physical seven-segment correctness, multiplex stability, brightness, electrical pin behavior, and measured 500 ms blink phase remain outside this software-only exit gate and require their designated Driver/EVK evidence.

### Cross-coverage references

The following draft cases were intentionally removed from TCOV-03 because their primary coverage owner is TCOV-07:

```text
Full FSM Display/LED state-output matrix
    -> primary owner: TCOV-07

Resulting-state/change-driven output update behavior
    -> primary owner: TCOV-07

Driver-owned blink independence under repeated/irregular Application updates
    -> primary owner: TCOV-07
```

TCOV-07 may reuse Clock-to-Display evidence from TCOV-03, but it must own the acceptance decision for state policy, change-driven update behavior, and Driver-owned blink independence.

---

# 6. TCOV-04 — Button FIFO -> FSM mapping / action sequencing / undefined-input safety

## TCOV-04 objective

Verify the Application-side button-event boundary frozen by Contract v2.7 and Plan v2.6: debounced Driver events map one-to-one to the expected FSM events, `BUTTON_EVENT_NONE` causes no dispatch, queued events are consumed in FIFO order, `state_before` is preserved for pre-transition responsibilities, post-transition responsibilities use the resulting state, undefined button/state combinations do not modify state or Clock/alarm values, and multi-key input does not create an invented combined-key Application shortcut.

TCOV-04 verifies the Application integration boundary for already-produced debounced Button events. Physical switch wiring, debounce timing, matrix electrical behavior, and real simultaneous-key/ghosting behavior remain Driver/EVK verification responsibilities unless they alter the frozen event boundary.

---

## AITP-04-01 — Frozen Button event -> FSM event mapping

**Trace:** Plan Section 6 / TCOV-04; Contract API-BTN / API-FSM
**Type:** Software integration / mapping
**Owner:** Tuấn

**Preconditions**
- Application mapping function/responsibility is observable in a test build.
- No state-specific action is required to prove the mapping itself.

**Procedure**
1. Supply each frozen Button event separately:
   - `BUTTON_EVENT_SW3_CLICK`
   - `BUTTON_EVENT_SW6_CLICK`
   - `BUTTON_EVENT_SW10_CLICK`
   - `BUTTON_EVENT_SW16_CLICK`
2. Capture the FSM event passed to the Core.
3. Compare against the frozen mapping.

**Expected result**

```text
BUTTON_EVENT_SW3_CLICK  -> FSM_EVENT_SW3
BUTTON_EVENT_SW6_CLICK  -> FSM_EVENT_SW6
BUTTON_EVENT_SW10_CLICK -> FSM_EVENT_SW10
BUTTON_EVENT_SW16_CLICK -> FSM_EVENT_SW16
```

- One Button event maps to exactly one corresponding FSM event.
- Application does not map a physical/raw button identifier or GPIO state directly.

**Required evidence**
- Table of Button event input -> FSM event output.
- Call-count assertion proving one mapping/dispatch per consumed event.

**Acceptance**
- `SW-PASS` when all four mappings are exact and one-to-one.
- `FAIL` for any swapped, missing, duplicated, or raw-hardware-dependent mapping.

---

## AITP-04-02 — `BUTTON_EVENT_NONE` causes no FSM dispatch

**Trace:** Plan Section 6; Contract API-BTN
**Type:** Software integration / empty-queue boundary
**Owner:** Tuấn

**Preconditions**
- `Button_GetEvent()` returns `BUTTON_EVENT_NONE`.

**Procedure**
1. Execute the Application Button-drain path with an empty queue.
2. Record FSM dispatch count and Application pre/post action calls.

**Expected result**
- No `FSM_HandleEvent(...)` call is made for `BUTTON_EVENT_NONE`.
- No button-derived pre-transition or post-transition action is executed.
- No invented user activity or key-feedback event is generated from an empty queue.

**Required evidence**
- Empty-queue trace with dispatch/action call counts.

**Acceptance**
- `SW-PASS` when the empty queue produces zero button dispatches.
- `FAIL` if `BUTTON_EVENT_NONE` is converted into a real FSM/user event.

---

## AITP-04-03 — Queued Button events are consumed in FIFO order

**Trace:** Contract API-BTN / Section 6 / T-BTNQ-01; Plan TCOV-04
**Type:** Software integration / queue ordering
**Owner:** Both at Button/Application boundary

**Preconditions**
- Test fixture can preload or emulate an ordered debounced event queue.
- Application uses only `Button_GetEvent()` at the frozen boundary.

**Procedure**
1. Arrange the queue in a known order, for example:
   `SW3 -> SW6 -> SW10 -> SW16`.
2. Execute the normal Application Button-drain phase.
3. Record the sequence returned by `Button_GetEvent()` and the sequence dispatched to the FSM.
4. Repeat with another non-symmetric sequence, for example:
   `SW16 -> SW3 -> SW16`.

**Expected result**
- Events are observed and dispatched oldest-first.
- No event is reordered, duplicated, or silently dropped by Application.
- Application does not inspect Driver-internal queue storage.

**Required evidence**
- Ordered queue-input/event-output/dispatch trace.
- Dispatch count equals number of supplied queued events.

**Acceptance**
- `SW-PASS` when every supplied sequence is preserved exactly.
- `FAIL` for reordering, duplication, loss, or internal Driver queue dependency.

---

## AITP-04-04 — Event dispatch preserves pre-transition and post-transition phases

**Trace:** Plan Section 7 / action-phase matrix; Contract Section 6 EVENT DISPATCH ORDER
**Type:** Software integration / sequencing
**Owner:** Tuấn

**Preconditions**
- Instrumentation can record:
  `state_before`, mapped FSM event, pre-transition phase, FSM dispatch, `state_after`, post-transition phase.
- Test-only instrumentation does not change production semantics.

**Procedure**
1. Use a representative transition that changes state, such as:
   `NORMAL + SW3 -> SET_TIME_HOUR`.
2. Execute one queued event through the normal Application dispatch path.
3. Record the exact action order.
4. Repeat with a transition whose pre-transition identity matters, such as final alarm confirmation, but assert only sequencing/phase ownership here; detailed alarm-commit semantics remain TCOV-06.

**Expected result**

```text
capture state_before
-> map Button event
-> feedback/activity accounting
-> pre-transition Application action
-> FSM_HandleEvent(...)
-> capture state_after
-> post-transition Application action
```

- `state_before` is captured before FSM mutation.
- `state_after` is captured after FSM dispatch.
- Actions that require the old state are not deferred until that information is lost.

**Required evidence**
- Ordered call/action trace with `state_before` and `state_after`.

**Acceptance**
- `SW-PASS` when the frozen phase order is preserved.
- `FAIL` if FSM transition occurs before a required pre-transition responsibility or if post-transition logic uses an incorrect stale state.

---

## AITP-04-05 — Defined current-time +/- events are not duplicated by Application

**Trace:** Plan Section 7 key constraint; Contract Section 6 critical rules
**Type:** Software integration / ownership boundary
**Owner:** Tuấn

**Preconditions**
- FSM is in `SET_TIME_HOUR` or `SET_TIME_MINUTE`.
- Clock value is known.
- One accepted SW6 or SW10 event is queued.

**Procedure**
1. Process one SW6 event in `SET_TIME_HOUR`.
2. Record calls/changes made by FSM/Core and Application.
3. Repeat with SW10 and with minute-setting state.

**Expected result**
- The existing FSM/Core performs the current-time increment/decrement exactly once.
- Application does not invoke an additional duplicate `Clock_Increment*()` / `Clock_Decrement*()` operation for the same event.
- Resulting Clock change therefore occurs once per accepted event.

**Required evidence**
- Clock before/after.
- Core/Application call trace showing no duplicate current-time edit.

**Acceptance**
- `SW-PASS` when each accepted +/- event causes exactly one Core-owned current-time edit.
- `FAIL` if Application duplicates the edit and causes a double increment/decrement.

---

## AITP-04-06 — Undefined button/state combinations preserve state and values

**Trace:** Contract DEC-16 / IMP-FSM-01 / T-UNDEF-01; Plan Section 6 / TCOV-04
**Type:** Table-driven software integration / negative behavior
**Owner:** Tuấn

**Preconditions**
- Clock and confirmed/temp alarm values are initialized to distinguishable known values.
- Test fixture can enter the required FSM states through valid transitions.

**Procedure**
1. Exercise representative undefined combinations, including at minimum:

```text
NORMAL + SW6
NORMAL + SW10
SET_TIME_HOUR + SW16
SET_TIME_MINUTE + SW16
SET_ALARM_HOUR + SW3
SET_ALARM_MINUTE + SW3
```

2. Capture FSM state and relevant Clock/alarm values before and after each event.

**Expected result**
- FSM state is unchanged.
- Clock value is unchanged.
- Confirmed alarm and temporary alarm values are unchanged.
- The event still reaches the system feedback/activity accounting path; detailed buzzer-duration and inactivity-timer acceptance remain TCOV-08/TCOV-09.

**Required evidence**
- Table of `state_before`, event, state_after, Clock/alarm before/after.
- Trace proving feedback/activity accounting was requested without claiming its later timing behavior here.

**Acceptance**
- `SW-PASS` when every undefined combination is functionally inert except for the frozen feedback/activity accounting.
- `FAIL` if any undefined combination changes mode or stored/edit values.

---

## AITP-04-07 — Undefined event in a SET session still reaches activity accounting

**Trace:** Contract DEC-16; Plan Section 6 / Section 7; TCOV-09 dependency
**Type:** Software integration / cross-coverage boundary
**Owner:** Tuấn

**Preconditions**
- System is in a SET state.
- Inactivity session is active.
- A button event that is undefined for that state is queued.

**Procedure**
1. Record the current inactivity activity timestamp/session state.
2. Process the undefined debounced button event.
3. Observe whether the Application activity-accounting responsibility is invoked.
4. Do not test the 30-second expiration boundary here; that remains TCOV-09.

**Expected result**
- State/value behavior remains unchanged per DEC-16.
- The accepted debounced click is still treated as user activity.
- TCOV-04 proves the accounting path is invoked; TCOV-09 owns the resulting timeout timing correctness.

**Required evidence**
- State/value trace plus activity-accounting invocation/timestamp trace.

**Acceptance**
- `SW-PASS` when the undefined SET-state click remains functionally inert but is still accounted as activity.
- `FAIL` if the click is ignored completely for inactivity accounting.

---

## AITP-04-08 — Multi-key input does not create a combined-key Application semantic

**Trace:** Plan Section 6 multi-key boundary / TCOV-04; Contract DEC-09
**Type:** Software integration / out-of-scope safety boundary
**Owner:** Both at Button/Application boundary

**Preconditions**
- Application receives only frozen debounced FIFO events.
- Test fixture can supply multiple queued events without inventing a new public enum value.

**Procedure**
1. Supply two or more valid Button events in the FIFO in a deterministic test order.
2. Process them through the normal Button-drain path.
3. Inspect Application mapping/dispatch logic for any synthetic combined-key event or shortcut.
4. Separately record that physical simultaneous-key/ghosting behavior is not claimed as functionally accepted by this software test.

**Expected result**
- Application processes the FIFO events individually in the supplied order.
- No new combined-key FSM event or cross-mode shortcut is invented.
- No memory/state corruption is observed at the software boundary.
- Physical matrix behavior under truly simultaneous presses remains Driver/EVK evidence and is not falsely marked PASS here.

**Required evidence**
- Ordered per-event dispatch trace.
- Static/code evidence showing no combined-key public/Application semantic.
- Explicit `HW-PENDING` note for physical simultaneous-key/ghosting behavior when applicable.

**Acceptance**
- `SW-PASS` when Application remains within the frozen per-event FIFO semantics.
- `FAIL` if Application invents a combined-key action or corrupts state when multiple events are queued.

---

## AITP-04-09 — Button FIFO drain terminates only when queue reports NONE

**Trace:** Contract API-BTN / Section 6; Plan TCOV-04
**Type:** Software integration / queue-drain completeness
**Owner:** Tuấn

**Preconditions**
- Queue contains a known number of events followed by empty state.

**Procedure**
1. Preload/emulate `N` queued events.
2. Run the normal drain phase.
3. Count `Button_GetEvent()` results and FSM dispatches until `BUTTON_EVENT_NONE` is returned.

**Expected result**
- All `N` real events are processed exactly once.
- Drain ends when `BUTTON_EVENT_NONE` is observed.
- `BUTTON_EVENT_NONE` itself is not dispatched.

**Required evidence**
- Getter-return sequence and dispatch-count trace.

**Acceptance**
- `SW-PASS` when `N` queued events produce exactly `N` real dispatches and clean termination.
- `FAIL` for early termination, extra dispatch, or infinite/repeated processing of an empty queue.

---

## TCOV-04 exit gate

TCOV-04 may be marked software-complete only when:

```text
[ ] AITP-04-01 SW-PASS
[ ] AITP-04-02 SW-PASS
[ ] AITP-04-03 SW-PASS
[ ] AITP-04-04 SW-PASS
[ ] AITP-04-05 SW-PASS
[ ] AITP-04-06 SW-PASS
[ ] AITP-04-07 SW-PASS
[ ] AITP-04-08 SW-PASS
[ ] AITP-04-09 SW-PASS
```

TCOV-04 proves the Application event-boundary, FIFO, mapping, sequencing, and undefined-input semantics. It does not claim physical debounce quality, switch electrical correctness, matrix ghosting behavior, 300 ms acoustic timing, or the complete 30-second inactivity boundary; those remain with their designated Driver/TCOV/EVK evidence.

---

# 7. TCOV-05 — Current-time setting / pause-resume / timeout exit

## TCOV-05 objective

Verify the frozen current-time editing semantics across the complete SW3 setting path: entry from NORMAL pauses automatic Clock-second scheduling and discards the old partial scheduling phase; SW6/SW10 edit the real Clock Core with the required hour/minute wrap behavior; SW3 advances hour -> minute -> NORMAL; seconds remain preserved; time spent in current-time setting is not accumulated for later catch-up; normal exit and timeout exit both retain edits already applied and resume automatic Clock progression from a fresh one-second anchor.

TCOV-05 owns the end-to-end current-time-setting behavior. TCOV-02 remains the primary owner of generic Timer/Clock catch-up arithmetic, while TCOV-09 remains the primary owner of exact 30-second inactivity timing and the button-vs-timeout race boundary.

---

## AITP-05-01 — SW3 enters current-time hour setting from NORMAL

**Trace:** REQ-SET-01; DEC-01; event/state integration matrix; Plan TCOV-05
**Type:** Software integration / mode entry
**Owner:** Tuấn

**Preconditions**
- FSM = `NORMAL`.
- Clock is a known non-boundary value including seconds, for example `12:34:27`.
- Scheduler has a known current anchor/partial phase.

**Procedure**
1. Arrange a non-zero partial sub-second Clock scheduling phase.
2. Queue one accepted `BUTTON_EVENT_SW3_CLICK`.
3. Process it through the normal Application event path.
4. Record state, Clock value, and current-time scheduling state after entry.

**Expected result**
- FSM becomes `SET_TIME_HOUR`.
- Clock hour/minute/second remain unchanged on entry.
- Automatic Clock-second scheduling is paused.
- The old partial sub-second scheduling phase is discarded rather than preserved for later catch-up.
- Inactivity-session entry responsibility is started; exact 30-second timing remains TCOV-09.

**Required evidence**
- `state_before -> event -> state_after`.
- Clock before/after including seconds.
- Scheduler pause/anchor-phase evidence.
- Inactivity-session start invocation/state.

**Acceptance**
- `SW-PASS` when entry semantics match the frozen current-time edit decision.
- `FAIL` if entry modifies Clock value, leaves automatic ticking active, or preserves stale partial phase for later replay.

---

## AITP-05-02 — SW6/SW10 edit current-time hour with 23/00 wrap

**Trace:** REQ-PLUS-01 / REQ-MINUS-01; event/state matrix; Plan TCOV-05
**Type:** Software integration / value editing
**Owner:** Tuấn

**Preconditions**
- FSM = `SET_TIME_HOUR`.
- Automatic Clock ticking is paused.

**Procedure**
1. Set Clock to `22:15:27`; process one SW6 click.
2. Set Clock to `23:15:27`; process one SW6 click.
3. Set Clock to `01:15:27`; process one SW10 click.
4. Set Clock to `00:15:27`; process one SW10 click.

**Expected result**

```text
22 -> 23
23 -> 00
01 -> 00
00 -> 23
```

- Only the hour field changes.
- Minute and second fields remain unchanged.
- Each accepted event causes exactly one Core-owned edit.
- FSM remains `SET_TIME_HOUR`.

**Required evidence**
- Clock before/after for all four vectors.
- FSM state after each event.
- Call trace sufficient to prove no duplicate Application edit.

**Acceptance**
- `SW-PASS` when all vectors and field-preservation checks pass.
- `FAIL` for wrong wrap, wrong field modification, duplicate edit, or unintended state transition.

---

## AITP-05-03 — SW3 advances hour setting to minute setting without resuming Clock

**Trace:** REQ-SET-02; DEC-01; event/state matrix
**Type:** Software integration / mode transition
**Owner:** Tuấn

**Preconditions**
- FSM = `SET_TIME_HOUR`.
- Clock contains a known edited value.
- Automatic Clock scheduling is paused.

**Procedure**
1. Queue one SW3 click.
2. Process the event normally.
3. Observe FSM state, Clock value, and scheduler pause state.

**Expected result**
- FSM becomes `SET_TIME_MINUTE`.
- Clock value is unchanged by the field-selection event.
- Automatic Clock ticking remains paused.
- Current-time edit session continues.
- No elapsed paused time is applied to the Clock merely because the selected edit field changes.
- SET-state activity accounting is refreshed; exact timeout timing remains TCOV-09.

**Required evidence**
- State/Clock/scheduler trace before and after SW3.

**Acceptance**
- `SW-PASS` when field selection changes only the FSM edit substate and preserves the frozen paused-Clock behavior.
- `FAIL` if Clock resumes automatically, the Clock value changes unexpectedly, or field selection causes elapsed paused time to be applied.

---

## AITP-05-04 — SW6/SW10 edit current-time minute with 59/00 wrap

**Trace:** REQ-PLUS-01 / REQ-MINUS-01; event/state matrix; Plan TCOV-05
**Type:** Software integration / value editing
**Owner:** Tuấn

**Preconditions**
- FSM = `SET_TIME_MINUTE`.
- Automatic Clock ticking is paused.

**Procedure**
1. Set Clock to `12:58:27`; process one SW6 click.
2. Set Clock to `12:59:27`; process one SW6 click.
3. Set Clock to `12:01:27`; process one SW10 click.
4. Set Clock to `12:00:27`; process one SW10 click.

**Expected result**

```text
58 -> 59
59 -> 00
01 -> 00
00 -> 59
```

- Only the minute field changes.
- Hour and second fields remain unchanged.
- FSM remains `SET_TIME_MINUTE`.

**Required evidence**
- Clock before/after for all vectors.
- FSM state after each event.

**Acceptance**
- `SW-PASS` when minute edits and wrap behavior are exact.
- `FAIL` for wrong wrap, wrong field modification, or unintended state transition.

---

## AITP-05-05 — Normal SW3 exit retains edits and resumes from a fresh one-second anchor

**Trace:** REQ-SET-03; DEC-01; event/state matrix; TCOV-02 dependency
**Type:** Software integration / normal exit
**Owner:** Tuấn

**Preconditions**
- FSM = `SET_TIME_MINUTE`.
- Current-time edits have already changed the real Clock.
- System has spent multiple Timer seconds in the current-time edit session.

**Procedure**
1. Record the edited Clock value immediately before final SW3.
2. Process final SW3 to return to NORMAL.
3. Record the fresh scheduling anchor established on exit.
4. Run the scheduler at `+999 ms` and then `+1000 ms` relative to that fresh anchor.

**Expected result**
- FSM becomes `NORMAL`.
- All current-time edits already applied remain in the real Clock.
- Seconds are preserved from the edited Clock; no reset to `00` is introduced by exit.
- No paused-time catch-up burst occurs.
- At `+999 ms`, no automatic Clock tick occurs.
- At `+1000 ms`, exactly one automatic Clock tick occurs.
- Current SET inactivity session is stopped.

**Required evidence**
- Edited Clock before exit and Clock after exit.
- Fresh anchor value.
- Tick count at `+999/+1000 ms`.
- Inactivity-session stop evidence.

**Acceptance**
- `SW-PASS` when normal exit retains edits and restarts scheduling from a fresh anchor.
- `FAIL` for rollback, second reset, stale-anchor catch-up, or continued SET timeout activity.

---

## AITP-05-06 — Long current-time edit does not accumulate missed Clock seconds

**Trace:** DEC-01; T-EDIT-01; Plan TCOV-05
**Type:** Software integration / pause-duration behavior
**Owner:** Tuấn

**Preconditions**
- System is in a current-time SET state.
- Clock is known.
- Controlled Timer advancement is available.

**Procedure**
1. Record Clock.
2. Advance the Timer by a substantial interval while remaining in `SET_TIME_HOUR/MINUTE`, but keep the inactivity session from expiring by supplying accepted activity as required.
3. Execute normal foreground cycles throughout.
4. Exit normally to `NORMAL`.
5. Observe Clock immediately after exit and during the first new logical second.

**Expected result**
- No automatic Clock seconds are accumulated during the edit interval.
- Only explicit SW6/SW10 edits alter hour/minute while paused.
- Exit does not replay the elapsed edit duration.
- First automatic post-exit tick occurs from the fresh anchor.

**Required evidence**
- Timer/Clock/state trace spanning the edit interval.
- Explicit user-edit events distinguished from automatic Clock ticks.
- Post-exit anchor/tick evidence.

**Acceptance**
- `SW-PASS` when elapsed setting time produces no hidden catch-up.
- `FAIL` if missed seconds are accumulated or replayed.

---

## AITP-05-07 — Timeout from SET_TIME_HOUR retains current-time edits and returns NORMAL

**Trace:** REQ-TMO-01; DEC-01; event/state matrix; TCOV-09 dependency
**Type:** Software integration / timeout exit semantics
**Owner:** Tuấn

**Preconditions**
- FSM = `SET_TIME_HOUR`.
- At least one current-time hour edit has already been applied.
- Test fixture can invoke the timeout path at an already-determined expired boundary without using raw button state.

**Procedure**
1. Record edited Clock including seconds.
2. Invoke the normal timeout handling path once the inactivity condition is considered expired.
3. Observe resulting state, Clock value, and scheduling anchor.

**Expected result**
- `FSM_EVENT_TIMEOUT_30S` returns the system to `NORMAL`.
- Current-time edits already applied remain applied; there is no rollback.
- Seconds remain preserved.
- Clock scheduling resumes from a fresh one-second anchor with no paused-time catch-up.
- Timeout short-feedback is requested once; acoustic 300 ms correctness remains TCOV-08.
- Exact 30-second expiration arithmetic remains TCOV-09.

**Required evidence**
- State/Clock before and after timeout.
- Timeout dispatch count.
- Fresh scheduling anchor.
- Short-feedback request count.

**Acceptance**
- `SW-PASS` when timeout exit preserves current-time edits and cleanly resumes NORMAL scheduling.
- `FAIL` for rollback, stale-anchor catch-up, duplicate timeout dispatch, or missing timeout feedback request.

---

## AITP-05-08 — Timeout from SET_TIME_MINUTE retains current-time edits and returns NORMAL

**Trace:** REQ-TMO-01; DEC-01; event/state matrix; TCOV-09 dependency
**Type:** Software integration / timeout exit semantics
**Owner:** Tuấn

**Preconditions**
- FSM = `SET_TIME_MINUTE`.
- Hour/minute edits have already been applied.
- Timeout condition is supplied through the normal Application timeout path.

**Procedure**
1. Record the edited Clock.
2. Execute the timeout path.
3. Observe state, Clock value, scheduling anchor, and timeout-feedback request.

**Expected result**
- FSM becomes `NORMAL`.
- All already-applied current-time hour/minute edits remain.
- Seconds remain preserved.
- No catch-up of time spent editing occurs.
- Scheduling resumes from a fresh anchor.
- One timeout short-feedback request is issued.

**Required evidence**
- State/Clock before and after.
- Anchor and feedback-request trace.

**Acceptance**
- `SW-PASS` when minute-setting timeout has the same frozen retain/resume semantics.
- `FAIL` for rollback, Clock catch-up, or incorrect timeout exit.

---

## AITP-05-09 — Current-time setting does not create an alarm trigger by manual edit alone

**Trace:** DEC-06; current-time setting semantics; TCOV-11 dependency
**Type:** Software integration / non-trigger boundary
**Owner:** Tuấn

**Preconditions**
- A confirmed valid alarm exists.
- FSM enters a current-time SET state.
- Clock can be manually edited to equal the confirmed alarm `HH:MM:00`.

**Procedure**
1. Arrange a confirmed alarm, for example `07:30:00`.
2. Enter current-time setting with Clock seconds at `00`.
3. Use SW6/SW10 edits to create `07:30:00` manually.
4. Observe alarm-start request/latch behavior caused by the edit itself.
5. Do not attempt to close the later natural-occurrence/re-arm behavior here; TCOV-11 owns that complete alarm acceptance.

**Expected result**
- Manual current-time editing alone does not call `Buzzer_StartAlarm()` merely because the resulting Clock equals the alarm.
- No natural Clock occurrence is processed while current-time ticking is paused.
- Later natural alarm behavior remains subject to TCOV-11.

**Required evidence**
- Clock/alarm values.
- FSM state.
- Alarm-start request count during manual edit.

**Acceptance**
- `SW-PASS` when manual equality does not create an alarm trigger.
- `FAIL` if the manual current-time edit itself starts the alarm.

---

## TCOV-05 exit gate

TCOV-05 may be marked software-complete only when:

```text
[ ] AITP-05-01 SW-PASS
[ ] AITP-05-02 SW-PASS
[ ] AITP-05-03 SW-PASS
[ ] AITP-05-04 SW-PASS
[ ] AITP-05-05 SW-PASS
[ ] AITP-05-06 SW-PASS
[ ] AITP-05-07 SW-PASS
[ ] AITP-05-08 SW-PASS
[ ] AITP-05-09 SW-PASS
```

TCOV-05 proves current-time edit behavior and exit semantics. Generic elapsed/catch-up arithmetic remains primarily TCOV-02; exact 30-second inactivity timing and same-boundary button priority remain TCOV-09; detailed alarm occurrence/latch/re-arm behavior remains TCOV-11; physical buzzer timing remains Driver/EVK evidence.

---

# 8. TCOV-06 — Transactional alarm editing / confirmation / previous-alarm continuity

## TCOV-06 objective

Verify the frozen transactional alarm-setting semantics across the complete SW16 path: entering alarm setting creates a temporary edit value without changing alarm validity; SW6/SW10 modify only the temporary hour/minute with the required wrap behavior; the previously confirmed alarm remains armed while editing; final SW16 confirmation is recognized using the pre-transition state and commits the edited alarm exactly once before the FSM returns to NORMAL; confirmation updates latch state correctly when the new alarm already equals the current Clock; EEPROM save is attempted exactly once after runtime confirmation; save failure does not roll back the runtime alarm or invent new UI; and timeout discards the temporary edit without changing the previously confirmed alarm or writing EEPROM.

TCOV-06 owns transactional alarm-edit integration semantics. TCOV-10 remains the primary owner of EEPROM Driver transaction correctness, persistence validation, injected write/read failures, and foreground-latency effects. TCOV-11 remains the primary owner of full natural alarm occurrence/latch/re-arm behavior.

---

## AITP-06-01 — Entering alarm setting copies the current confirmed/default Alarm Core value into the temporary edit buffer

**Trace:** REQ-ALMSET-01; DEC-03 / DEC-12; Plan Section 9 / TCOV-06
**Type:** Software integration / transactional entry
**Owner:** Tuấn

**Preconditions**
- FSM = `NORMAL`.
- Test is executed for two startup/runtime conditions:
  1. `alarm_valid = 1` with a known confirmed alarm, for example `06:45`.
  2. `alarm_valid = 0` with Alarm Core default `00:00`.

**Procedure**
1. Record confirmed Alarm Core value and `alarm_valid`.
2. Queue one accepted SW16 click.
3. Process the normal Application event path.
4. Observe FSM state, temporary alarm edit value, confirmed Alarm Core value, and `alarm_valid`.

**Expected result**
- FSM becomes `SET_ALARM_HOUR`.
- If a valid alarm exists, `alarm_edit_time` starts from the current confirmed alarm value.
- If no valid alarm exists, the temporary edit starts from the Alarm Core default `00:00`.
- Entering alarm-setting mode does not change the confirmed Alarm Core value.
- Entering alarm-setting mode does not change `alarm_valid`.

**Required evidence**
- State, `alarm_valid`, confirmed alarm, and temporary edit before/after entry for both conditions.

**Acceptance**
- `SW-PASS` when entry creates the correct temporary edit without arming or replacing the confirmed alarm.
- `FAIL` if entry changes `alarm_valid`, modifies the confirmed alarm, or initializes the temporary edit from an unsupported value.

---

## AITP-06-02 — SW6/SW10 edit only temporary alarm hour with 23/00 wrap

**Trace:** REQ-PLUS-01 / REQ-MINUS-01; DEC-03; Plan Section 9
**Type:** Software integration / temporary edit
**Owner:** Tuấn

**Preconditions**
- FSM = `SET_ALARM_HOUR`.
- Confirmed alarm and temporary edit values are distinguishable.
- Current Clock is also distinguishable from both values.

**Procedure**
1. Set temporary hour to `22`; process one SW6 click.
2. Set temporary hour to `23`; process one SW6 click.
3. Set temporary hour to `01`; process one SW10 click.
4. Set temporary hour to `00`; process one SW10 click.
5. Observe confirmed alarm and current Clock throughout.

**Expected result**

```text
22 -> 23
23 -> 00
01 -> 00
00 -> 23
```

- Only `alarm_edit_time.hour` changes.
- Temporary minute remains unchanged.
- Confirmed Alarm Core value remains unchanged.
- Current Clock remains governed by normal Clock scheduling; the alarm edit action does not modify it.
- FSM remains `SET_ALARM_HOUR`.

**Required evidence**
- Temporary edit, confirmed alarm, Clock, and FSM state before/after each vector.

**Acceptance**
- `SW-PASS` when hour editing is isolated to the temporary alarm buffer with exact wrap behavior.
- `FAIL` if the confirmed alarm or Clock is edited, the wrong temporary field changes, or wrap behavior is incorrect.

---

## AITP-06-03 — SW16 advances alarm-hour edit to alarm-minute edit without committing

**Trace:** REQ-ALMSET-02; DEC-03; Plan Section 9
**Type:** Software integration / transactional phase transition
**Owner:** Tuấn

**Preconditions**
- FSM = `SET_ALARM_HOUR`.
- Temporary alarm hour has been edited to a value different from the confirmed alarm.

**Procedure**
1. Record temporary edit, confirmed alarm, `alarm_valid`, and EEPROM-save call count.
2. Process one SW16 click.
3. Observe resulting state and all recorded values/counters.

**Expected result**
- FSM becomes `SET_ALARM_MINUTE`.
- Temporary alarm edit remains intact.
- Confirmed Alarm Core value is still unchanged.
- `alarm_valid` is unchanged.
- No `EEPROM_SaveAlarm(...)` call occurs at this intermediate SW16.
- No final alarm commit occurs yet.

**Required evidence**
- State/value trace and Alarm/EEPROM call counts.

**Acceptance**
- `SW-PASS` when SW16 changes only the selected alarm-edit field/state at this phase.
- `FAIL` if the new alarm is confirmed or persisted before final SW16.

---

## AITP-06-04 — SW6/SW10 edit only temporary alarm minute with 59/00 wrap

**Trace:** REQ-PLUS-01 / REQ-MINUS-01; DEC-03; Plan Section 9
**Type:** Software integration / temporary edit
**Owner:** Tuấn

**Preconditions**
- FSM = `SET_ALARM_MINUTE`.
- Temporary edit, confirmed alarm, and current Clock are distinguishable.

**Procedure**
1. Set temporary minute to `58`; process one SW6 click.
2. Set temporary minute to `59`; process one SW6 click.
3. Set temporary minute to `01`; process one SW10 click.
4. Set temporary minute to `00`; process one SW10 click.

**Expected result**

```text
58 -> 59
59 -> 00
01 -> 00
00 -> 59
```

- Only `alarm_edit_time.minute` changes.
- Temporary hour remains unchanged.
- Confirmed Alarm Core value remains unchanged.
- Current Clock is not edited by the alarm-setting action.
- FSM remains `SET_ALARM_MINUTE`.

**Required evidence**
- Temporary edit, confirmed alarm, Clock, and FSM state before/after each vector.

**Acceptance**
- `SW-PASS` when minute editing is isolated to the temporary alarm buffer with exact wrap behavior.
- `FAIL` for confirmed-alarm mutation, wrong-field edit, Clock edit, or incorrect wrap.

---

## AITP-06-05 — Previous confirmed alarm remains armed while alarm editing is in progress

**Trace:** DEC-06; Plan Section 9 / TCOV-06; TCOV-11 dependency
**Type:** Software integration / old-alarm continuity
**Owner:** Tuấn

**Preconditions**
- `alarm_valid = 1`.
- Previous confirmed alarm is set to a known near-future value.
- FSM is in `SET_ALARM_HOUR` or `SET_ALARM_MINUTE`.
- Temporary edit has been changed to a different time.

**Procedure**
1. Allow the current Clock to continue through normal logical one-second ticks while alarm editing remains active.
2. Arrange a natural Clock tick onto the previous confirmed alarm time before final confirmation of the temporary edit.
3. Observe which alarm value is used for occurrence evaluation and whether an alarm-start request is generated.
4. Keep detailed buzzer waveform acceptance outside this test.

**Expected result**
- Current Clock continues running during alarm-setting states.
- Temporary edit is not used as the armed/triggering alarm.
- The previous confirmed alarm remains armed until final confirmation.
- A genuine natural Clock tick onto that previous confirmed alarm may generate the normal alarm occurrence request.
- Detailed one-shot/latch/waveform acceptance remains TCOV-11/Driver coverage.

**Required evidence**
- Ordered Clock value, confirmed alarm, temporary edit, `alarm_valid`, and alarm-start request trace.

**Acceptance**
- `SW-PASS` when alarm editing does not disarm or replace the previously confirmed alarm before final confirmation.
- `FAIL` if temporary edit becomes armed early or the previous confirmed alarm is silently disabled during edit.

---

## AITP-06-06 — Final SW16 commits the edited alarm exactly once before returning NORMAL

**Trace:** REQ-ALMSET-03; DEC-03; event dispatch pre-transition rule; T-ALMSET-01
**Type:** Software integration / commit boundary
**Owner:** Tuấn + Trung at persistence boundary

**Preconditions**
- FSM = `SET_ALARM_MINUTE`.
- Temporary alarm edit contains a known valid value different from the currently confirmed alarm.
- Alarm/EEPROM boundary calls are observable.

**Procedure**
1. Record `state_before`, confirmed alarm, `alarm_valid`, and relevant call counts.
2. Process one final SW16 click.
3. Record the ordering and count of:
   - runtime alarm confirmation,
   - `alarm_valid` establishment,
   - confirmation-collision latch bookkeeping,
   - `EEPROM_SaveAlarm(edit.hour, edit.minute)`,
   - FSM transition to NORMAL.

**Expected result**
- Final confirmation is recognized using the pre-transition `SET_ALARM_MINUTE` context.
- The edited hour/minute become the confirmed Alarm Core value exactly once.
- Confirmed alarm second remains `00`.
- `alarm_valid = 1`.
- `EEPROM_SaveAlarm(edit.hour, edit.minute)` is attempted exactly once.
- FSM then reaches `NORMAL`.
- The commit is not lost merely because the FSM changes state.

**Required evidence**
- Ordered action trace with `state_before/state_after`.
- Alarm confirmation call/value count.
- EEPROM-save call/value count.
- Final Alarm Core value and `alarm_valid`.

**Acceptance**
- `SW-PASS` when one final SW16 produces exactly one runtime commit and one persistence attempt with the edited HH/MM.
- `FAIL` for lost, duplicate, premature, or wrong-value commit/save.

---

## AITP-06-07 — Confirmation equal to current Clock does not trigger immediately and latches the existing occurrence

**Trace:** DEC-13; DEC-05/06; Plan Section 9; TCOV-11 dependency
**Type:** Software integration / confirmation collision
**Owner:** Tuấn

**Preconditions**
- FSM = `SET_ALARM_MINUTE`.
- Current Clock equals the temporary alarm value at `HH:MM:00`.
- Alarm-start requests and `alarm_match_latched` are observable in a test build.

**Procedure**
1. Arrange current Clock = temporary alarm `HH:MM:00`.
2. Process final SW16 confirmation.
3. Observe runtime alarm value, alarm-start request count, and occurrence latch state immediately after confirmation.

**Expected result**
- New alarm is confirmed normally.
- Confirmation itself does not call `Buzzer_StartAlarm()`.
- `alarm_match_latched = 1` for the already-equal current occurrence.
- Full later mismatch/re-arm/future-natural-occurrence behavior remains primarily TCOV-11.

**Required evidence**
- Current Clock and confirmed alarm after commit.
- Alarm-start request count.
- Latch state immediately after confirmation.

**Acceptance**
- `SW-PASS` when equal-time confirmation is treated as configuration, not a natural alarm occurrence.
- `FAIL` if confirmation immediately starts the 5-second alarm or leaves the current equal occurrence incorrectly unlatched.

---

## AITP-06-08 — Successful persistence result leaves the newly confirmed runtime alarm active

**Trace:** DEC-03 / DEC-08; Plan Section 9; TCOV-10 dependency
**Type:** Software integration / persistence-result handling
**Owner:** Tuấn + Trung at EEPROM boundary

**Preconditions**
- Final confirmation path is ready.
- Test persistence fixture returns success (`1`) for the one `EEPROM_SaveAlarm(...)` attempt.

**Procedure**
1. Confirm a known temporary alarm.
2. Return success from the EEPROM boundary.
3. Observe runtime confirmed alarm, `alarm_valid`, and Application-visible behavior.

**Expected result**
- Newly confirmed runtime alarm remains the edited value.
- `alarm_valid = 1`.
- Application may regard the new alarm as persistent for reboot semantics.
- No extra success UI/beep is invented beyond the normal required key feedback.
- EEPROM physical write/read correctness remains TCOV-10/EVK evidence.

**Required evidence**
- Save return value.
- Runtime alarm/`alarm_valid` after confirmation.
- Feedback/UI request trace sufficient to show no invented success behavior.

**Acceptance**
- `SW-PASS` when persistence success does not alter the frozen runtime confirmation semantics.
- `FAIL` if success causes an unsupported additional system behavior or runtime alarm inconsistency.

---

## AITP-06-09 — EEPROM save failure does not roll back runtime confirmation or invent error UI

**Trace:** DEC-08; Plan Section 9; TCOV-10 dependency
**Type:** Software integration / persistence failure handling
**Owner:** Tuấn + Trung at EEPROM boundary

**Preconditions**
- FSM = `SET_ALARM_MINUTE`.
- Temporary edit is valid.
- Test persistence fixture returns failure (`0`) from `EEPROM_SaveAlarm(...)`.

**Procedure**
1. Process final SW16 confirmation.
2. Force the single EEPROM save attempt to return failure.
3. Observe confirmed Alarm Core value, `alarm_valid`, state, and feedback/UI requests.

**Expected result**
- Runtime confirmed alarm remains the newly edited value.
- `alarm_valid = 1`.
- Application does not roll back to the previous confirmed alarm.
- FSM returns to `NORMAL`.
- Persistence across reboot is not claimed.
- No additional error beep or error UI is invented.
- This case does not claim that the old EEPROM record survives every failed physical transaction.

**Required evidence**
- Save call count/value and returned failure.
- Runtime alarm/`alarm_valid` after failure.
- State and feedback/UI request trace.

**Acceptance**
- `SW-PASS` when persistence failure affects persistence guarantee only, not the newly confirmed runtime alarm.
- `FAIL` for runtime rollback, invalidation, duplicate save, or invented error feedback.

---

## AITP-06-10 — Timeout discards temporary alarm edits and preserves the previous confirmed alarm

**Trace:** REQ-TMO-01; DEC-03; Plan Section 9; TCOV-09/10 dependencies
**Type:** Software integration / transactional abort
**Owner:** Tuấn

**Preconditions**
- Execute separately from `SET_ALARM_HOUR` and `SET_ALARM_MINUTE`.
- A previous confirmed alarm exists and `alarm_valid` is known.
- Temporary edit differs from the confirmed alarm.
- Test fixture invokes the normal timeout path after the inactivity condition is already considered expired.

**Procedure**
1. Record confirmed alarm, `alarm_valid`, temporary edit, and EEPROM-save call count.
2. Execute the normal timeout path.
3. Observe state, confirmed alarm, `alarm_valid`, and persistence-call count.

**Expected result**
- Temporary alarm edit is discarded.
- Previous confirmed Alarm Core value is preserved.
- `alarm_valid` remains unchanged.
- No EEPROM save is attempted because timeout is not confirmation.
- FSM returns to `NORMAL`.
- One timeout short-feedback request is generated; exact 300 ms behavior remains TCOV-08.
- Exact 30-second boundary/priority arithmetic remains TCOV-09.

**Required evidence**
- Before/after confirmed alarm, `alarm_valid`, temp edit, state, and EEPROM-save count.
- Timeout-feedback request count.

**Acceptance**
- `SW-PASS` when timeout behaves as a transactional abort with no persistence write.
- `FAIL` if temp edits leak into the confirmed alarm, `alarm_valid` changes unexpectedly, EEPROM is written, or the system remains in alarm-setting state.

---

## AITP-06-11 — Entering alarm setting with no valid alarm does not arm the default `00:00`

**Trace:** DEC-12 / DEC-03; Plan Section 9; startup dependency from TCOV-01
**Type:** Software integration / no-valid-alarm boundary
**Owner:** Tuấn

**Preconditions**
- `alarm_valid = 0`.
- Alarm Core default is `00:00`.
- FSM = `NORMAL`.

**Procedure**
1. Enter alarm setting with SW16.
2. Observe temporary edit, confirmed/default Alarm Core value, and `alarm_valid`.
3. Remain in alarm editing without final confirmation.

**Expected result**
- Temporary edit begins at `00:00`.
- `alarm_valid` remains `0`.
- Merely entering or editing the default value does not create an armed alarm.
- Only valid EEPROM restore or final confirmation may establish `alarm_valid = 1`.

**Required evidence**
- State, temporary edit, Alarm Core value, and `alarm_valid` trace.

**Acceptance**
- `SW-PASS` when default `00:00` is available for editing but remains unarmed until a frozen arming event occurs.
- `FAIL` if entry alone arms the default alarm.

---

## TCOV-06 exit gate

TCOV-06 may be marked software-complete only when:

```text
[ ] AITP-06-01 SW-PASS
[ ] AITP-06-02 SW-PASS
[ ] AITP-06-03 SW-PASS
[ ] AITP-06-04 SW-PASS
[ ] AITP-06-05 SW-PASS
[ ] AITP-06-06 SW-PASS
[ ] AITP-06-07 SW-PASS
[ ] AITP-06-08 SW-PASS
[ ] AITP-06-09 SW-PASS
[ ] AITP-06-10 SW-PASS
[ ] AITP-06-11 SW-PASS
```

TCOV-06 proves alarm-edit transaction semantics and the Application-side persistence boundary. EEPROM transaction safety/read-back/hardware persistence remain TCOV-10; full natural alarm occurrence/latch/re-arm behavior remains TCOV-11; physical buzzer waveform remains Driver/EVK evidence.

---

# 9. TCOV-07 — Display/LED state policy / output-update invariant / Driver-owned blink independence

## TCOV-07 objective

Verify the frozen Application output policy across all FSM states: the correct Display source, blink request, separator request, and LED D4 mode are selected from the current/resulting state; the first post-startup output synchronization establishes the required logical output state; repeated foreground execution does not make correctness depend on repeated identical setter calls; Application does not own Display/LED blink phase timing; and Driver-side repeated same-mode LED requests are idempotent so they cannot restart the 500 ms blink phase.

TCOV-07 verifies output-state policy and ownership boundaries. The Plan recommends change-driven output requests, but does **not** freeze a specific dirty-flag/cache implementation or an exact setter-call count. Physical segment wiring, multiplex quality, LED polarity, and measured 500 ms blink timing remain Driver/EVK verification concerns.

---

## AITP-07-01 — Full FSM Display/LED output matrix matches the frozen policy

**Trace:** Plan Section 12 / TCOV-07; Contract Display/LED output policy
**Type:** Table-driven software integration
**Owner:** Tuấn

**Preconditions**
- Test fixture can enter each frozen FSM state through valid transitions.
- Current Clock and temporary alarm values are intentionally different so source selection is observable.
- Display/LED boundary requests are observable in a test build.

**Procedure**
1. Arrange:
   - current Clock = a known HH:MM value,
   - temporary alarm = a different known HH:MM value.
2. Visit each state:
   - `NORMAL`
   - `SET_TIME_HOUR`
   - `SET_TIME_MINUTE`
   - `SET_ALARM_HOUR`
   - `SET_ALARM_MINUTE`
3. Capture requested Display source/value, blink mode, separator request, and LED mode.
4. Compare against the frozen matrix.

**Expected result**

```text
NORMAL
    Display source = Clock HH.MM
    blink          = DISPLAY_BLINK_NONE
    separator      = enabled
    LED D4         = LED_MODE_OFF

SET_TIME_HOUR
    Display source = Clock HH.MM
    blink          = DISPLAY_BLINK_HOURS
    separator      = enabled
    LED D4         = LED_MODE_OFF

SET_TIME_MINUTE
    Display source = Clock HH.MM
    blink          = DISPLAY_BLINK_MINUTES
    separator      = enabled
    LED D4         = LED_MODE_OFF

SET_ALARM_HOUR
    Display source = temporary alarm HH.MM
    blink          = DISPLAY_BLINK_HOURS
    separator      = enabled
    LED D4         = LED_MODE_BLINK_ALARM_SETTING

SET_ALARM_MINUTE
    Display source = temporary alarm HH.MM
    blink          = DISPLAY_BLINK_MINUTES
    separator      = enabled
    LED D4         = LED_MODE_BLINK_ALARM_SETTING
```

**Required evidence**
- State-by-state table containing FSM state, Clock value, temporary alarm value, Display request, blink request, separator request, and LED mode request.

**Acceptance**
- `SW-PASS` only when every matrix row matches the frozen policy.
- `FAIL` for any incorrect source, blink selection, separator state, or LED mode.

---

## AITP-07-02 — Output update uses the resulting/current FSM state after a transition

**Trace:** Plan processing order / Section 12 / TCOV-07; TCOV-04 dependency
**Type:** Software integration / state-transition boundary
**Owner:** Tuấn

**Preconditions**
- Application foreground processing order is observable.
- Output requests can be captured after event processing.

**Procedure**
1. Start from `NORMAL`.
2. Process SW3 and capture:
   `state_before -> event -> state_after -> output request`.
3. Continue through the SW3 current-time path.
4. Repeat the same observation through the SW16 alarm-setting path.
5. Include the final transitions back to `NORMAL`.

**Expected result**
- The output-update stage reflects the resulting/current FSM state after event processing.
- Entry into a SET state requests the correct new blink/source/LED policy without an extra logical stale-state decision.
- Return to `NORMAL` requests Clock HH.MM, blink none, separator enabled, and LED off.

**Required evidence**
- Ordered transition/output trace for every tested path.

**Acceptance**
- `SW-PASS` when each post-transition output request corresponds to `state_after`.
- `FAIL` if Application applies the old state's logical output policy after the transition has completed.

---

## AITP-07-03 — First post-startup synchronization establishes the complete initial logical output state

**Trace:** Plan Section 12 output-update policy / startup rule; TCOV-01 dependency
**Type:** Software integration / initial synchronization
**Owner:** Tuấn

**Preconditions**
- Application output-cache/dirty state, if any, starts from its production initialization condition.
- Startup Core state has already been established by TCOV-01.

**Procedure**
1. Start/reset the production-equivalent Application.
2. Execute the first output-update stage after initialization.
3. Capture all Display/LED boundary requests necessary to establish the logical startup state.
4. Do not assume any specific internal cache implementation.

**Expected result**
- Before normal suppression/optimization can omit unchanged requests, the system establishes:
  - Display value `00.00`,
  - separator enabled,
  - blink none,
  - LED D4 off.
- No stale/uninitialized cached output state prevents the first complete synchronization.

**Required evidence**
- First synchronization trace showing the required logical requests/state.
- TCOV-01 startup-state evidence may be reused.

**Acceptance**
- `SW-PASS` when the complete logical startup output state is established once production output handling begins.
- `FAIL` if an output cache/optimization prevents one or more required initial states from being established.

---

## AITP-07-04 — Application correctness does not depend on repeated identical Display setter calls

**Trace:** Plan Section 12 Application output-update policy / TCOV-07
**Type:** Software integration / invariant test
**Owner:** Tuấn

**Preconditions**
- System is held in a stable FSM state with unchanged displayed HH/MM and unchanged blink/separator requirements.
- Driver blink/scan servicing is available independently of Application requests.

**Procedure**
1. Establish a correct Display logical request for a SET state.
2. Continue foreground execution for multiple cycles while the logical Display request remains unchanged.
3. Exercise the production output-update strategy as implemented.
4. Observe whether correctness requires repeated identical `Display_SetTime()`, `Display_SetBlinkMode()`, or `Display_SetColon()` calls.
5. Do **not** require a particular dirty-flag/cache design or exact setter-call count.

**Expected result**
- Display logical state remains valid even if the Application does not repeatedly issue identical setters.
- Application does not use repeated identical setter calls as a timing mechanism for blink/scan behavior.
- An implementation that legitimately repeats idempotent setters may still pass if correctness does not depend on the repetition.

**Required evidence**
- Output-request trace under unchanged logical state.
- Code/test evidence demonstrating that blink/scan timing is not advanced by Application setter-call cadence.

**Acceptance**
- `SW-PASS` when correctness is independent of repeated identical Display setter calls.
- `FAIL` if blink/scan behavior requires Application to keep reissuing the same Display request.

---

## AITP-07-05 — Changed Display source/value propagates to the Driver boundary

**Trace:** Plan Section 12 recommended change-driven strategy / required output invariant; TCOV-03 dependency
**Type:** Software integration / output-value change
**Owner:** Tuấn

**Preconditions**
- FSM state remains valid.
- Display source/value can change through normal Clock progression or temporary alarm editing.

**Procedure**
1. In `NORMAL`, allow Clock HH/MM to change through a legitimate Clock update and observe the next Application output stage.
2. In an alarm-setting state, modify `alarm_edit_time` through a legitimate SW6/SW10 event and observe the next output stage.
3. Record the Display value requested after each logical source-value change.

**Expected result**
- The Driver boundary eventually receives the new required HH/MM value at the normal output-update stage.
- `NORMAL` uses the updated Clock value.
- Alarm-setting states use the updated temporary alarm value.
- Application does not leave a stale displayed value after its logical source has changed.

**Required evidence**
- Source-value-before/after and corresponding Display request trace.

**Acceptance**
- `SW-PASS` when every logical value change is reflected at the Display boundary.
- `FAIL` if output suppression/cache logic leaves stale HH/MM data.

---

## AITP-07-06 — Changed FSM blink/LED requirements propagate without coupling to physical phase

**Trace:** Plan Section 12 / TCOV-07; Contract API-DSP/API-LED ownership
**Type:** Software integration / mode-change boundary
**Owner:** Tuấn

**Preconditions**
- System can transition between states with different blink/LED requirements.

**Procedure**
1. Exercise transitions whose logical requirements change:
   - `NORMAL -> SET_TIME_HOUR`
   - `SET_TIME_HOUR -> SET_TIME_MINUTE`
   - `NORMAL -> SET_ALARM_HOUR`
   - `SET_ALARM_HOUR -> SET_ALARM_MINUTE`
   - any SET state -> `NORMAL`
2. Capture the requested Display blink mode and LED mode after each transition.
3. Do not inspect or constrain the instantaneous physical blink phase at the transition instant.

**Expected result**
- Logical mode requests update to the new state's frozen requirements.
- Application selects only the desired mode; it does not force a visible ON/OFF phase or manually toggle segments/LED output.
- Physical phase continuity/restart behavior is judged only where the Driver contract explicitly defines it.

**Required evidence**
- State transition -> blink-mode/LED-mode request trace.
- Static/code evidence showing no Application-owned physical phase toggle loop.

**Acceptance**
- `SW-PASS` when logical mode changes propagate while phase ownership remains Driver-side.
- `FAIL` if Application directly owns/toggles the blink phase or requests the wrong mode.

---

## AITP-07-07 — Display blink/scan servicing remains independent of irregular Application foreground update cadence

**Trace:** Contract API-DSP / TIM-05 / TIM-07 / TIM-08; Plan TCOV-07
**Type:** Boundary / Driver-independence integration
**Owner:** Tuấn + Trung boundary

**Preconditions**
- A Display blink mode is active.
- Test environment can vary Application foreground update cadence without disabling the Driver's intended display service mechanism.

**Procedure**
1. Establish a stable SET state with a frozen blink request.
2. Execute Application foreground cycles at irregular intervals within a software test/simulator arrangement.
3. Observe that Application does not call or depend on:
   - `Display_UpdateBlinkState()`
   - `Display_ScanRoutine()`
4. Verify the Driver's internal display service remains the owner of blink/scan progression.
5. Leave measured 500 ms timing and physical multiplex quality to Driver/TCOV-14 evidence.

**Expected result**
- Application requires only the frozen Display setter boundary.
- Driver blink/scan servicing remains independent of the exact number/timing of Application output-update calls.
- No Driver-internal display timing helper becomes an Application dependency.

**Required evidence**
- Application dependency/static search.
- Simulator/service trace showing Driver-owned blink/scan progression under irregular foreground update cadence.

**Acceptance**
- `SW-PASS` when cross-team ownership remains clean and Application cadence is not the Display timing source.
- `FAIL` if Application must invoke Driver-internal blink/scan helpers or exact foreground setter cadence is required for correct servicing.

---

## AITP-07-08 — Repeated identical LED mode requests cannot restart alarm-setting blink timing

**Trace:** Contract API-LED / IMP-LED-01; Plan TCOV-07
**Type:** Driver-boundary integration / idempotence
**Owner:** Trung, observed at Application/Driver boundary

**Preconditions**
- `LED_MODE_BLINK_ALARM_SETTING` is active.
- Driver blink state/timing is observable with test-only instrumentation or equivalent evidence.

**Procedure**
1. Enter an alarm-setting state and request `LED_MODE_BLINK_ALARM_SETTING`.
2. Allow blink timing to progress for a non-zero interval shorter than or spanning a half-period.
3. Issue the **same** `LED_MODE_BLINK_ALARM_SETTING` request again.
4. Observe the Driver's existing blink timing state/deadline/phase behavior.
5. Repeat identical same-mode requests at irregular foreground times.

**Expected result**
- `LED_SetMode(same_mode)` is idempotent.
- Repeated identical requests do not restart/reset the LED blink timing sequence.
- Application may therefore remain correct whether it suppresses identical requests or occasionally repeats them.
- Measured physical 500 ms half-period remains Driver/TCOV-14 evidence.

**Required evidence**
- Driver logical mode plus timing/phase/deadline trace before and after same-mode requests.
- Evidence must show continuity rather than only the final LED state.

**Acceptance**
- `SW-PASS` when same-mode requests preserve ongoing Driver blink timing.
- `FAIL` if an identical request restarts or resets the blink timer/phase.

---

## AITP-07-09 — LED D4 is never logically requested to blink outside alarm-setting states

**Trace:** REQ-LED-01; Plan Section 12 / TCOV-07
**Type:** Table-driven software integration / negative policy
**Owner:** Tuấn

**Preconditions**
- Test fixture can visit all frozen FSM states.

**Procedure**
1. Visit `NORMAL`, `SET_TIME_HOUR`, and `SET_TIME_MINUTE`.
2. Capture LED mode requests.
3. Visit `SET_ALARM_HOUR` and `SET_ALARM_MINUTE`.
4. Capture LED mode requests.

**Expected result**
- `NORMAL`, `SET_TIME_HOUR`, `SET_TIME_MINUTE` -> `LED_MODE_OFF`.
- `SET_ALARM_HOUR`, `SET_ALARM_MINUTE` -> `LED_MODE_BLINK_ALARM_SETTING`.
- Application never requests alarm-setting blink mode in any non-alarm-setting state.

**Required evidence**
- FSM-state -> LED-mode request table.

**Acceptance**
- `SW-PASS` when LED D4 logical policy is exclusive to alarm-setting states.
- `FAIL` if D4 blink mode is requested in NORMAL/current-time setting or omitted in alarm setting.

---

## TCOV-07 exit gate

TCOV-07 may be marked software-complete only when:

```text
[ ] AITP-07-01 SW-PASS
[ ] AITP-07-02 SW-PASS
[ ] AITP-07-03 SW-PASS
[ ] AITP-07-04 SW-PASS
[ ] AITP-07-05 SW-PASS
[ ] AITP-07-06 SW-PASS
[ ] AITP-07-07 SW-PASS
[ ] AITP-07-08 SW-PASS
[ ] AITP-07-09 SW-PASS
```

TCOV-07 proves logical Display/LED state policy, Application output-update invariants, and blink/scan ownership boundaries. It does not claim physical segment correctness, brightness, multiplex quality, LED electrical polarity, or measured 500 ms phase accuracy; those require Driver/EVK evidence and TCOV-14 handoff.

---

# 10. TCOV-08 — Key/timeout short-feedback requests / Buzzer priority boundary

## TCOV-08 objective

Verify the frozen short-feedback and Buzzer-priority semantics at the Application/Driver boundary: every accepted debounced button event requests one short feedback action even when the FSM action is undefined; timeout exit requests one short feedback action; short feedback is approximately 300 ms under normal Driver operation; a new short-beep request while SHORT_BEEP is active refreshes the 300 ms interval from the newest request rather than queueing delayed beeps; an alarm occurrence preempts an active short beep; short-feedback requests during an active alarm do not cancel, restart, shorten, or corrupt the 5-second 500 ms ON/OFF alarm sequence; and repeated alarm-start requests while ALARM is active are defensive no-ops.

TCOV-08 owns request/priority behavior across the Application/Buzzer boundary. Exact 30-second timeout detection remains TCOV-09. Natural alarm occurrence generation/latch semantics remain TCOV-11/12. Physical acoustic loudness and final EVK waveform validation remain Driver/TCOV-14 evidence.

---

## AITP-08-01 — Every accepted debounced button event requests one short feedback action

**Trace:** REQ-BUZ-01; Contract event/state matrix; T-BTN-01; Plan TCOV-08
**Type:** Software integration / feedback mapping
**Owner:** Tuấn + Trung boundary

**Preconditions**
- Button FIFO supplies one accepted debounced event at a time.
- `Buzzer_BeepShort()` request count is observable.
- No alarm sequence is active.

**Procedure**
1. Exercise representative defined button actions for SW3, SW6, SW10, and SW16 in states where each is meaningful.
2. For each consumed debounced event, record:
   - Button event,
   - FSM/system action,
   - short-feedback request count.
3. Repeat with several events across different FSM states.

**Expected result**
- Each accepted debounced button event produces exactly one logical short-feedback request.
- Feedback request is independent of whether the button changes state or changes a value.
- Application does not use raw GPIO transitions as the feedback source.

**Required evidence**
- Event -> FSM/system action -> `Buzzer_BeepShort()` request-count table.

**Acceptance**
- `SW-PASS` when every accepted event produces one logical short-feedback request.
- `FAIL` for missing, duplicate, or raw-transition-driven feedback requests.

---

## AITP-08-02 — Undefined button/state combinations still request short feedback

**Trace:** DEC-16 / T-UNDEF-01; Plan TCOV-08 with TCOV-04 dependency
**Type:** Software integration / negative-state feedback
**Owner:** Tuấn

**Preconditions**
- Representative undefined button/state combinations from TCOV-04 are available.
- No alarm sequence is active.

**Procedure**
1. Exercise undefined combinations such as:
   - `NORMAL + SW6`
   - `NORMAL + SW10`
   - `SET_TIME_HOUR + SW16`
   - `SET_ALARM_HOUR + SW3`
2. Record state/value before and after plus short-feedback request count.

**Expected result**
- State/value behavior remains unchanged according to DEC-16.
- Each accepted undefined debounced click still requests one short feedback action.

**Required evidence**
- Undefined-combination table with state/value stability and feedback request count.

**Acceptance**
- `SW-PASS` when undefined actions remain functionally inert but still request feedback.
- `FAIL` if accepted undefined clicks produce no feedback or create extra feedback requests.

---

## AITP-08-03 — Timeout exit requests one short feedback action

**Trace:** REQ-TMO-02; event/state integration matrix; T-TMO-01; Plan TCOV-08 with TCOV-09 dependency
**Type:** Software integration / timeout feedback boundary
**Owner:** Tuấn

**Preconditions**
- System is in any SET state.
- Test fixture enters the normal timeout handling path after timeout expiration is already considered true.
- No alarm sequence is active for the base case.

**Procedure**
1. Trigger the normal timeout path from each SET-state family.
2. Record timeout dispatch and `Buzzer_BeepShort()` request count.
3. Do not re-test exact 30-second arithmetic here.

**Expected result**
- Each actual timeout dispatch requests one short feedback action.
- No extra second timeout-feedback request is generated for the same timeout event.
- Exact timeout boundary/priority remains TCOV-09.

**Required evidence**
- Timeout dispatch count and short-feedback request count.

**Acceptance**
- `SW-PASS` when each timeout dispatch maps to exactly one logical short-feedback request.
- `FAIL` for missing or duplicate timeout feedback requests.

---

## AITP-08-04 — Normal short beep lasts approximately 300 ms without blocking Application semantics

**Trace:** REQ-BUZ-01 / T-BUZ-01; shared timing config; Plan TCOV-08
**Type:** Driver-boundary software timing
**Owner:** Trung

**Preconditions**
- No alarm sequence is active.
- Short-beep logical state/output is observable in simulator/test instrumentation.
- Agreed short-feedback duration is 300 ms.

**Procedure**
1. Issue one `Buzzer_BeepShort()` request.
2. Service `Buzzer_Process()` through the normal Driver scheduling model.
3. Observe Buzzer logical/output state around the 300 ms boundary.
4. Repeat with foreground servicing jitter that remains within the supported cooperative-loop model.

**Expected result**
- Short-beep logical output becomes active after the request.
- It remains active for approximately the required 300 ms interval.
- It returns inactive without requiring a blocking 300 ms Application delay.
- Final physical/acoustic timing remains subject to EVK evidence.

**Required evidence**
- Tick/time trace around short-beep start and end.
- Driver process/output-state evidence.
- Explicit distinction between software timing evidence and physical acoustic PASS.

**Acceptance**
- `SW-PASS` when logical non-blocking 300 ms behavior is demonstrated.
- `HW-PENDING` for final audible duration until EVK validation.
- `FAIL` if behavior is blocking or software timing does not match the frozen short-feedback duration.

---

## AITP-08-05 — Rapid short-beep request refreshes the 300 ms interval instead of queueing delayed beeps

**Trace:** DEC-14 / T-BUZ-04; Plan TCOV-08
**Type:** Driver-boundary timing / overlap
**Owner:** Trung

**Preconditions**
- No alarm sequence is active.
- Driver short-beep state/timing is observable.

**Procedure**
1. Issue a short-beep request at `t0`.
2. Before the first 300 ms interval expires, issue another short-beep request at a known later time `t1`.
3. Continue servicing the Driver beyond both the original and refreshed deadlines.
4. Repeat with more than two rapid requests.

**Expected result**
- The newest request refreshes/restarts the short-beep 300 ms timer from the newest press/request.
- The buzzer does not stop at the old `t0 + 300 ms` deadline if a newer request has refreshed the interval.
- Multiple delayed acoustic beeps are not queued for later playback.
- The final short beep ends after the refreshed newest-request interval.

**Required evidence**
- Request timestamps and short-beep deadline/output-state trace.
- Evidence that no delayed-beep queue is emitted after the final refreshed interval.

**Acceptance**
- `SW-PASS` when rapid feedback behaves as one refreshed active short-beep interval.
- `FAIL` if requests are queued as delayed beeps or the newest request fails to refresh the active interval.

---

## AITP-08-06 — Alarm start preempts an active short beep

**Trace:** API-BUZ priority rule / T-BUZ-05; Plan TCOV-08 with TCOV-11 dependency
**Type:** Driver-boundary priority
**Owner:** Trung

**Preconditions**
- SHORT_BEEP is active.
- Test fixture can issue a legitimate `Buzzer_StartAlarm()` request without redefining alarm-occurrence semantics.
- Alarm waveform state is observable.

**Procedure**
1. Start a normal short beep.
2. Before the short beep expires, issue one `Buzzer_StartAlarm()` request.
3. Observe Buzzer mode/output transition and alarm timing origin.

**Expected result**
- Alarm mode takes priority immediately.
- Active short-beep behavior does not delay the alarm start.
- The required 5-second alarm sequence begins from the alarm-start request according to Driver semantics.
- Natural occurrence generation remains outside this test.

**Required evidence**
- Ordered SHORT_BEEP -> alarm-start request -> ALARM mode/output trace.
- Alarm-start timestamp/reference point.

**Acceptance**
- `SW-PASS` when alarm start preempts active short feedback.
- `FAIL` if the alarm waits for the short beep to finish or short-feedback state corrupts alarm startup.

---

## AITP-08-07 — Button short-feedback request during active ALARM cannot corrupt the alarm sequence

**Trace:** API-BUZ concurrent conflict rule / T-BUZ-03 / T-BUZ-05; Plan TCOV-08
**Type:** Driver-boundary priority / conflict
**Owner:** Tuấn + Trung boundary

**Preconditions**
- A 5-second ALARM sequence is already active.
- Application processes an accepted debounced button event.
- Alarm phase/output is observable.

**Procedure**
1. Start/arrange an active alarm sequence.
2. At representative points in the alarm sequence, process a button event that normally requests short feedback.
3. Record:
   - logical button processing,
   - short-feedback request,
   - alarm timer/phase before and after the request.
4. Continue until the original alarm sequence completes.

**Expected result**
- The logical button event is still processed normally.
- The corresponding short-feedback request may be absorbed/suppressed acoustically while ALARM is active.
- The active alarm is not cancelled.
- The 5-second alarm timer is not restarted or shortened.
- The 500 ms ON/OFF alarm phase sequence is not corrupted by the short-feedback request.

**Required evidence**
- Button event/feedback request trace.
- Alarm start reference, phase transitions, and completion reference before/after feedback requests.

**Acceptance**
- `SW-PASS` when button feedback cannot alter the active alarm sequence.
- `FAIL` if feedback cancels, restarts, shortens, or corrupts the alarm waveform state machine.

---

## AITP-08-08 — Timeout short-feedback request during active ALARM obeys the same alarm priority

**Trace:** API-BUZ concurrent conflict rule / T-BUZ-05; Plan TCOV-08 with TCOV-09 dependency
**Type:** Driver-boundary priority / timeout conflict
**Owner:** Tuấn + Trung boundary

**Preconditions**
- ALARM sequence is active.
- System timeout path legitimately dispatches a timeout event.
- Exact timeout-expiration arithmetic is supplied by the TCOV-09 fixture.

**Procedure**
1. Arrange an active alarm.
2. Execute one legitimate timeout dispatch that requests timeout short feedback.
3. Observe logical timeout handling and alarm timer/phase continuity.

**Expected result**
- Timeout logic/state transition still occurs.
- Timeout short-feedback request may be acoustically absorbed/suppressed while ALARM is active.
- Alarm sequence is not cancelled, restarted, shortened, or phase-corrupted.

**Required evidence**
- Timeout dispatch/feedback request trace.
- Alarm phase/timer trace across the timeout event.

**Acceptance**
- `SW-PASS` when timeout feedback respects active-alarm priority.
- `FAIL` if timeout feedback alters the alarm sequence.

---

## AITP-08-09 — Repeated `Buzzer_StartAlarm()` while ALARM is active is a no-op for alarm duration

**Trace:** DEC-15 / T-BUZ-04; Plan TCOV-08
**Type:** Driver robustness / priority
**Owner:** Trung

**Preconditions**
- ALARM sequence is active.
- Alarm start timestamp/timer is observable.

**Procedure**
1. Start one alarm sequence and record its start reference.
2. While ALARM is active, issue one or more additional `Buzzer_StartAlarm()` calls.
3. Continue servicing the Driver through the original completion point.

**Expected result**
- Repeated alarm-start requests do not restart the 5-second timer.
- Repeated requests do not extend the alarm indefinitely.
- The sequence completes according to the original alarm start.
- This is defensive Driver behavior; Application one-shot logic is still expected to avoid duplicate requests in normal operation.

**Required evidence**
- Original alarm-start reference.
- Repeated request timestamps.
- Alarm phase/completion trace.

**Acceptance**
- `SW-PASS` when repeated alarm-start requests are duration-neutral no-ops during active ALARM.
- `FAIL` if a repeated request restarts or extends the alarm sequence.

---

## TCOV-08 exit gate

TCOV-08 may be marked software-complete only when:

```text
[ ] AITP-08-01 SW-PASS
[ ] AITP-08-02 SW-PASS
[ ] AITP-08-03 SW-PASS
[ ] AITP-08-04 SW-PASS (physical acoustic duration may remain HW-PENDING)
[ ] AITP-08-05 SW-PASS
[ ] AITP-08-06 SW-PASS
[ ] AITP-08-07 SW-PASS
[ ] AITP-08-08 SW-PASS
[ ] AITP-08-09 SW-PASS
```

TCOV-08 proves short-feedback request mapping and Buzzer priority/overlap semantics. Exact inactivity expiration and same-boundary button-vs-timeout behavior remain TCOV-09; alarm occurrence/latch generation remains TCOV-11/12; measured audible timing and final physical waveform remain Driver/TCOV-14 evidence.

---

# 11. TCOV-09 — 30-second inactivity / wrap-safe expiration / button-vs-timeout boundary

## TCOV-09 objective

Verify the frozen SET-session inactivity semantics: every `NORMAL -> SET` transition starts a fresh 30-second inactivity session; every accepted debounced button click while the resulting state remains SET refreshes activity; return to NORMAL disables timeout; expiration is based on accepted debounced events and wrap-safe elapsed-time arithmetic; no timeout occurs before the 30-second interval has elapsed; timeout dispatch occurs once when the interval has expired; and queued valid Button events are drained before timeout evaluation so a valid queued click at the same processing boundary wins and prevents simultaneous `TIMEOUT_30S` dispatch.

TCOV-09 owns inactivity-session timing and the button-vs-timeout scheduling boundary. Per-state timeout cleanup is already covered by TCOV-05/06, while timeout short-feedback mapping/priority is covered by TCOV-08. TCOV-12 later combines timeout with due Clock/alarm catch-up and other same-cycle ordering boundaries.

---

## AITP-09-01 — Every NORMAL -> SET entry starts a fresh inactivity session

**Trace:** REQ-TMO-01; DEC-04; Plan Section 11 / TCOV-09
**Type:** Software integration / session start
**Owner:** Tuấn

**Preconditions**
- FSM = `NORMAL`.
- Inactivity state/timestamp is observable in a test build.
- Coherent foreground `now` can be controlled.

**Procedure**
1. At known `now = t0`, enter `SET_TIME_HOUR` with an accepted SW3 event.
2. Observe inactivity active state and activity reference.
3. Return to `NORMAL`.
4. Advance time substantially.
5. At known `now = t1`, enter `SET_ALARM_HOUR` with an accepted SW16 event.
6. Observe the new inactivity session.

**Expected result**
- Each `NORMAL -> SET` transition starts a fresh inactivity session using the current foreground-cycle time reference.
- No stale activity timestamp from the previous SET session carries into the new session.
- The second session's expiration is based on `t1`, not on `t0` or any prior session.

**Required evidence**
- State transition, coherent `now`, timeout-active state, and activity reference for both sessions.

**Acceptance**
- `SW-PASS` when each SET entry creates an independent fresh 30-second session.
- `FAIL` if a prior session timestamp can shorten or otherwise contaminate a later SET session.

---

## AITP-09-02 — Accepted debounced click while SET refreshes the inactivity interval

**Trace:** DEC-04; INV-05/06; Plan Section 11
**Type:** Software integration / activity refresh
**Owner:** Tuấn

**Preconditions**
- FSM is any SET state.
- Inactivity session is active.
- Button FIFO can supply accepted debounced events at controlled foreground times.

**Procedure**
1. Start a SET session at `t0`.
2. Before expiration, consume one accepted debounced click at `t1`.
3. Keep the resulting FSM state in SET.
4. Observe the activity reference and timeout behavior relative to `t1`.
5. Repeat with:
   - a value-changing click,
   - a SET-substate transition click,
   - an undefined button/state combination that leaves the state/value unchanged.

**Expected result**
- Every accepted debounced click consumed while the resulting state remains SET refreshes activity from the foreground cycle's coherent `now`.
- Refresh is not limited to value-changing or state-changing actions.
- Undefined accepted clicks still count as activity.
- Timeout is measured from the newest accepted click.

**Required evidence**
- Event/state/resulting-state, coherent `now`, and activity-reference trace for each click category.

**Acceptance**
- `SW-PASS` when all accepted clicks that leave the system in SET refresh the inactivity session.
- `FAIL` if undefined/inert accepted clicks are ignored for activity or the old deadline remains in effect.

---

## AITP-09-03 — Button that exits SET to NORMAL disables the inactivity session

**Trace:** DEC-04; Plan Section 11; event/state matrix
**Type:** Software integration / session stop
**Owner:** Tuấn

**Preconditions**
- Execute final normal exits from:
  - `SET_TIME_MINUTE` via SW3,
  - `SET_ALARM_MINUTE` via SW16.

**Procedure**
1. Record inactivity active state before each final button.
2. Process the accepted exit button.
3. Observe resulting FSM state and inactivity active state.
4. Advance time beyond 30 seconds while remaining in NORMAL.

**Expected result**
- Final button is processed normally.
- Resulting FSM state is `NORMAL`.
- Inactivity session becomes inactive rather than being refreshed as a continuing SET session.
- No later timeout is generated while the system remains NORMAL.

**Required evidence**
- State-before/event/state-after/timeout-active trace.
- Evidence of no timeout dispatch after remaining NORMAL beyond 30 seconds.

**Acceptance**
- `SW-PASS` when every normal SET exit disables inactivity.
- `FAIL` if timeout remains active after returning NORMAL.

---

## AITP-09-04 — No timeout dispatch before 30 seconds of inactivity

**Trace:** REQ-TMO-01; INV-06; Plan Section 11
**Type:** Software integration / lower timing boundary
**Owner:** Tuấn

**Preconditions**
- A fresh SET inactivity session begins at known `t0`.
- No accepted debounced clicks occur after `t0`.

**Procedure**
1. Evaluate the normal timeout path at elapsed values below 30,000 ms, including a value immediately below the boundary such as 29,999 ms where the test tick resolution permits.
2. Record timeout-dispatch count and FSM state.

**Expected result**
- No `FSM_EVENT_TIMEOUT_30S` is dispatched while elapsed inactivity is less than 30 seconds.
- FSM remains in the SET state unless changed by another legitimate event.
- No timeout cleanup or timeout-feedback request occurs before expiration.

**Required evidence**
- `t0`, current `now`, unsigned elapsed value, state, and timeout-dispatch count.

**Acceptance**
- `SW-PASS` when all `< 30 s` observations remain non-expired.
- `FAIL` for any early timeout dispatch.

---

## AITP-09-05 — Timeout dispatches once when the 30-second inactivity interval has expired

**Trace:** REQ-TMO-01/02; T-TMO-01; Plan Section 11
**Type:** Software integration / expiration boundary
**Owner:** Tuấn

**Preconditions**
- Fresh SET session begins at known `t0`.
- No accepted debounced clicks occur.
- No queued Button event exists at the expiration processing boundary.

**Procedure**
1. Advance the coherent foreground `now` until wrap-safe elapsed inactivity reaches the configured 30-second interval.
2. Execute timeout evaluation.
3. Observe timeout dispatch, resulting state, timeout-active state, and feedback request.
4. Execute later foreground cycles while remaining NORMAL.

**Expected result**
- Once the 30-second interval has elapsed, one `FSM_EVENT_TIMEOUT_30S` is dispatched.
- Normal per-state timeout cleanup executes.
- FSM reaches `NORMAL`.
- Inactivity becomes inactive.
- One timeout short-feedback request is issued.
- Subsequent NORMAL cycles do not redispatch the same timeout.

**Required evidence**
- Elapsed-time calculation around expiration.
- Timeout dispatch count.
- State/timeout-active transition.
- Timeout-feedback request count.

**Acceptance**
- `SW-PASS` when expiration produces exactly one timeout transition/feedback action and then disables the session.
- `FAIL` for missed, repeated, or early timeout behavior.

---

## AITP-09-06 — Timeout elapsed-time check is correct across 32-bit tick wrap

**Trace:** DEC-04; TIM-01/02 wrap-safe timing requirement; Plan Section 11
**Type:** Software integration / wrap boundary
**Owner:** Tuấn + Trung timer boundary

**Preconditions**
- Inactivity timing uses the Platform tick type/semantics frozen by the Contract.
- Test fixture can place the activity reference near unsigned tick wrap.

**Procedure**
1. Start/refresh activity at a tick value near `UINT32_MAX`.
2. Advance `now` across wrap by less than 30 seconds and evaluate timeout.
3. Advance until total unsigned elapsed inactivity reaches the 30-second interval and evaluate again.
4. Record the wrap-safe elapsed values and dispatch behavior.

**Expected result**
- Unsigned wrap does not cause an immediate false timeout.
- `< 30 s` across wrap remains non-expired.
- Expiration occurs when the wrap-safe elapsed interval reaches the configured 30 seconds.
- No signed absolute-time comparison is required by the test.

**Required evidence**
- Activity tick, wrapped `now`, unsigned elapsed calculation, and timeout-dispatch trace.

**Acceptance**
- `SW-PASS` when inactivity expiration is correct across tick wrap.
- `FAIL` for false early timeout, missed timeout, or wrap-dependent corruption.

---

## AITP-09-07 — Queued valid Button event at timeout boundary wins; no simultaneous timeout dispatch

**Trace:** DEC-11; T-TMO-02; frozen processing order D -> E
**Type:** Software integration / same-boundary priority
**Owner:** Tuấn

**Preconditions**
- FSM is a SET state.
- Inactivity interval would otherwise be expired at the current foreground cycle.
- At least one valid accepted debounced Button event is already queued before the Button-drain stage.
- Coherent `now` is fixed for the foreground cycle.

**Procedure**
1. Arrange elapsed inactivity at the timeout boundary/expired condition.
2. Queue one valid accepted Button event before the cycle's Button-drain stage.
3. Execute the frozen foreground order:
   - drain queued Button events,
   - perform activity/state handling,
   - then evaluate timeout.
4. Record button processing count, state transition/value action, activity handling, timeout-dispatch count, and feedback requests.

**Expected result**
- Queued Button event is processed exactly once before timeout evaluation.
- Its normal logical action and key-feedback request occur.
- Activity/timeout state is updated according to the resulting state:
  - if resulting state remains SET, the inactivity session is refreshed;
  - if resulting state becomes NORMAL, inactivity is disabled.
- `TIMEOUT_30S` is **not** also dispatched for that same processing cycle.
- No timeout short-feedback request is generated for a timeout that was suppressed by DEC-11.

**Required evidence**
- Ordered trace showing Button stage before timeout stage.
- Button-event count/action.
- State-before/state-after.
- Activity/timeout-active state.
- Timeout dispatch and key/timeout feedback request counts.

**Acceptance**
- `SW-PASS` when the queued valid Button event wins cleanly with no double-dispatch.
- `FAIL` if both Button and timeout are dispatched for the same boundary or the Button event is lost/duplicated.

---

## AITP-09-08 — Multiple currently queued Button events are drained before timeout evaluation

**Trace:** DEC-11; frozen processing order; API-BTN FIFO dependency; TCOV-04
**Type:** Software integration / queued-event boundary
**Owner:** Tuấn

**Preconditions**
- FSM begins in a SET state near/at timeout expiration.
- Multiple accepted debounced Button events are already queued in FIFO order before stage D.

**Procedure**
1. Queue a small sequence of valid events whose ordered effects are observable.
2. Execute one foreground cycle through the Button-drain and timeout-evaluation stages.
3. Record FIFO event order, resulting FSM state, final activity/timeout state, and timeout-dispatch count.

**Expected result**
- All currently queued Button events are processed in FIFO order before timeout evaluation.
- Activity/state handling follows each event and its resulting state.
- Timeout evaluation occurs only after the queue-drain stage.
- A valid queued event at that processing boundary prevents simultaneous timeout dispatch for that cycle.
- This test does not redefine queue capacity/overflow policy.

**Required evidence**
- FIFO ordered event trace.
- State transitions and activity handling.
- Timeout-dispatch count after queue drain.

**Acceptance**
- `SW-PASS` when queue draining and timeout priority follow the frozen processing order.
- `FAIL` if timeout is evaluated between already-queued events or a simultaneous timeout is dispatched contrary to DEC-11.

---

## AITP-09-09 — Raw electrical activity without an accepted debounced event does not refresh inactivity

**Trace:** DEC-11; INV-06; Plan Section 11
**Type:** Integration boundary / debounce ownership
**Owner:** Tuấn + Trung boundary

**Preconditions**
- FSM is in a SET state with an active inactivity session.
- Button Driver test fixture can present raw/bounce activity that does not produce an accepted FIFO event.

**Procedure**
1. Start a fresh inactivity session.
2. Inject raw electrical transitions/bounce that the Button Driver rejects and does not enqueue.
3. Observe Application activity reference.
4. Continue until the original inactivity interval expires.

**Expected result**
- Rejected/raw transitions do not refresh Application inactivity timing.
- Only an accepted debounced Button event consumed from the FIFO counts as activity.
- If no accepted event occurs, timeout remains based on the original accepted activity/entry reference.

**Required evidence**
- Raw-transition stimulus.
- Evidence that no Button FIFO event was produced.
- Activity-reference and timeout trace.

**Acceptance**
- `SW-PASS` when raw/rejected activity cannot postpone the timeout.
- `FAIL` if Application inactivity is refreshed without an accepted debounced event.

---

## TCOV-09 exit gate

TCOV-09 may be marked software-complete only when:

```text
[ ] AITP-09-01 SW-PASS
[ ] AITP-09-02 SW-PASS
[ ] AITP-09-03 SW-PASS
[ ] AITP-09-04 SW-PASS
[ ] AITP-09-05 SW-PASS
[ ] AITP-09-06 SW-PASS
[ ] AITP-09-07 SW-PASS
[ ] AITP-09-08 SW-PASS
[ ] AITP-09-09 SW-PASS
```

TCOV-09 proves inactivity-session lifetime, exact lower/expiration boundary behavior, wrap-safe timing, accepted-event activity semantics, and DEC-11 button-before-timeout priority. Per-state cleanup semantics remain cross-covered by TCOV-05/06, timeout feedback priority by TCOV-08, and combined due-Clock/button/timeout ordering by TCOV-12.

---

# 12. TCOV-10 — EEPROM restore/save/failure integration semantics / transaction safety / foreground-latency impact

## TCOV-10 objective

Verify the frozen persistence boundary and its integration semantics: startup restore accepts only a valid EEPROM record and establishes the runtime alarm accordingly; invalid/empty/error restore leaves `alarm_valid = 0`; final alarm confirmation attempts exactly one save of the confirmed HH/MM; save success establishes persistence for reboot while save failure does not roll back the runtime-confirmed alarm or invent error UI; the EEPROM Driver rejects invalid public inputs; persistence updates are transaction-safe so an interrupted/mixed update cannot later be accepted as a valid alarm record; and foreground EEPROM activity does not violate the observable 300 ms feedback, 500 ms periodic behavior, or 30 s inactivity contract.

TCOV-10 owns EEPROM restore/save/failure integration semantics and persistence-boundary quality. TCOV-01 may provide startup-state evidence, and TCOV-06 may provide alarm-confirmation call-order evidence, but TCOV-10 owns the persistence acceptance decision. Physical I2C ACK/electrical behavior, real EEPROM retention across power cycles, and EVK reboot-restore remain `HW-PENDING` until the designated Driver/TCOV-14 evidence exists.

---

## AITP-10-01 — Valid EEPROM record restores confirmed alarm at startup

**Trace:** DEC-10; API-EEP; T-BOOT-01; Plan INT-10 / TCOV-10
**Type:** Software integration / startup restore
**Owner:** Tuấn + Trung boundary

**Preconditions**
- Startup sequence follows the frozen initialization order.
- `EEPROM_ReadAlarm(&hour, &minute)` can be controlled to return success with a valid HH/MM pair.
- Alarm Core and `alarm_valid` are observable.

**Procedure**
1. Arrange a valid persisted record, for example `06:45`.
2. Start the production-equivalent initialization path.
3. Return success from `EEPROM_ReadAlarm(...)` with the arranged HH/MM.
4. Observe calls/state after restore.

**Expected result**
- `Alarm_SetTime(06, 45)` is applied to the confirmed Alarm Core.
- `alarm_valid = 1`.
- The restored alarm second remains fixed at `00`.
- Startup then applies the existing startup-match latch bookkeeping rather than treating restore as a generic alarm trigger.
- This software case does not claim physical EEPROM retention.

**Required evidence**
- EEPROM read return/value trace.
- Alarm Core value after restore.
- `alarm_valid`.
- Startup latch bookkeeping result where applicable.

**Acceptance**
- `SW-PASS` when a valid record becomes the confirmed runtime alarm exactly through the frozen restore path.
- `FAIL` if valid restore is ignored, misapplied, or leaves the runtime alarm invalid.

---

## AITP-10-02 — Invalid/empty/error EEPROM restore leaves no valid runtime alarm

**Trace:** DEC-10 / DEC-12; T-BOOT-02; Plan TCOV-10
**Type:** Software integration / restore failure
**Owner:** Tuấn + Trung boundary

**Preconditions**
- Execute separately for representative read-failure/invalid-record outcomes exposed as `EEPROM_ReadAlarm(...) == 0`.

**Procedure**
1. Start from cold-start Application state with `alarm_valid = 0`.
2. Make `EEPROM_ReadAlarm(...)` return failure.
3. Complete startup.
4. Observe Alarm Core/default state and `alarm_valid`.

**Expected result**
- `alarm_valid` remains `0`.
- Application does not treat unread/invalid data as a confirmed alarm.
- Alarm Core default may remain `00:00:00`, but that default alone is not armed.
- If alarm setting is entered later, its default edit behavior remains governed by DEC-12/TCOV-06.

**Required evidence**
- Read result.
- `alarm_valid`.
- Alarm Core/default state after startup.

**Acceptance**
- `SW-PASS` when invalid/empty/error restore cannot arm a runtime alarm.
- `FAIL` if failed restore publishes or arms invalid persistence data.

---

## AITP-10-03 — EEPROM public boundary rejects invalid save ranges

**Trace:** API-EEP; T-EEP-01
**Type:** Driver-boundary validation
**Owner:** Trung

**Preconditions**
- `EEPROM_SaveAlarm(hour, minute)` is callable in an isolated Driver test fixture.

**Procedure**
1. Call save with representative invalid inputs:
   - `hour > 23`,
   - `minute > 59`,
   - both invalid.
2. Observe return value and persistence-write behavior.
3. Include valid boundary controls such as `00:00` and `23:59`.

**Expected result**
- Invalid range input returns `0`.
- Invalid input is not accepted as a valid persisted alarm.
- Valid boundary HH/MM is permitted to proceed according to normal Driver behavior.

**Required evidence**
- Input/return-value table.
- Evidence that invalid values do not become a valid record.

**Acceptance**
- `SW-PASS` when invalid ranges are rejected and valid range boundaries remain supported.
- `FAIL` if an out-of-range alarm can be accepted as a valid persisted value.

---

## AITP-10-04 — EEPROM read boundary rejects null output pointers without partial publication

**Trace:** API-EEP
**Type:** Driver-boundary validation / safety
**Owner:** Trung

**Preconditions**
- Driver read function can be exercised in a test fixture.

**Procedure**
1. Call `EEPROM_ReadAlarm(NULL, &minute)`.
2. Call `EEPROM_ReadAlarm(&hour, NULL)`.
3. If supported by the test environment, call with both pointers null.
4. Observe return values and any caller-visible output mutation.

**Expected result**
- Null-output-pointer calls return `0`.
- Driver does not partially publish one field as if a valid alarm was returned.
- No invalid memory access occurs.

**Required evidence**
- Call/return table.
- Sentinel values proving no partial valid-data publication.

**Acceptance**
- `SW-PASS` when null-pointer handling is safe and non-partial.
- `FAIL` for success return, unsafe access, or partial publication of a supposedly valid record.

---

## AITP-10-05 — Final alarm confirmation attempts exactly one EEPROM save of confirmed HH/MM

**Trace:** DEC-03; API-EEP; T-ALMSET-01; TCOV-06 dependency
**Type:** Software integration / persistence call boundary
**Owner:** Tuấn + Trung boundary

**Preconditions**
- FSM = `SET_ALARM_MINUTE`.
- Temporary edit contains a valid value different from the previously confirmed alarm.
- EEPROM save calls are observable.

**Procedure**
1. Process the final SW16 confirmation once.
2. Record runtime confirmation and `EEPROM_SaveAlarm(...)` calls.
3. Continue later foreground cycles without another alarm confirmation.

**Expected result**
- Runtime alarm is confirmed exactly once.
- Application attempts exactly one `EEPROM_SaveAlarm(edit.hour, edit.minute)` for that confirmation.
- The saved HH/MM equals the newly confirmed runtime alarm.
- No periodic/repeated save occurs in later foreground cycles merely because the alarm remains valid.

**Required evidence**
- Confirmation count.
- EEPROM save count and arguments.
- Confirmed Alarm Core value.

**Acceptance**
- `SW-PASS` when one confirmation produces one matching persistence attempt.
- `FAIL` for missing, duplicate, delayed-repeat, or wrong-value save.

---

## AITP-10-06 — Save success establishes reboot persistence semantics without changing runtime confirmation

**Trace:** DEC-08; API-EEP; Plan INT-10
**Type:** Software integration / save-success semantics
**Owner:** Tuấn + Trung boundary

**Preconditions**
- Final alarm confirmation has produced a valid runtime alarm.
- `EEPROM_SaveAlarm(...)` returns `1`.

**Procedure**
1. Confirm a known alarm.
2. Return success from the EEPROM save boundary.
3. Observe runtime alarm, `alarm_valid`, and persistence status/evidence.
4. Where a software EEPROM model exists, perform a subsequent read in a fresh test instance to verify the saved HH/MM is the value published as valid.

**Expected result**
- Runtime confirmed alarm remains valid.
- Save success is the condition under which the new alarm may be regarded as persistent across reboot.
- Application does not invent additional success UI/beep.
- Software-model read-back, if used, must return the confirmed saved HH/MM.
- Real power-cycle retention remains `HW-PENDING`.

**Required evidence**
- Save return value and arguments.
- Runtime alarm/`alarm_valid`.
- Software read-back evidence when available.
- Explicit separation from EVK power-cycle PASS.

**Acceptance**
- `SW-PASS` when success semantics are consistent from runtime confirmation through the persistence boundary.
- `HW-PENDING` for real nonvolatile retention/reboot restore.
- `FAIL` if save success causes runtime inconsistency or publishes a different alarm value.

---

## AITP-10-07 — Save failure does not roll back runtime alarm and does not claim persistence success

**Trace:** DEC-08; Plan INT-10; TCOV-06 dependency
**Type:** Software integration / save-failure semantics
**Owner:** Tuấn + Trung boundary

**Preconditions**
- Final alarm confirmation is ready.
- EEPROM save result can be forced to `0`.

**Procedure**
1. Confirm a new alarm.
2. Make the one EEPROM save attempt return failure.
3. Observe runtime alarm, `alarm_valid`, state, and feedback/UI behavior.
4. Inspect any test/debug persistence status only as internal diagnostic evidence.

**Expected result**
- Newly confirmed runtime alarm remains active and `alarm_valid = 1`.
- Application does not roll back to the previous alarm.
- Persistence success is not claimed.
- No extra error beep/UI is invented.
- Failure does not imply or guarantee that the previous EEPROM record survived the failed update.

**Required evidence**
- Save result.
- Runtime alarm/`alarm_valid`.
- State and feedback/UI trace.
- Diagnostic persistence flag if one exists, without treating it as a cross-team API.

**Acceptance**
- `SW-PASS` when save failure affects persistence guarantee only, not runtime confirmation.
- `FAIL` for runtime rollback/invalidation or invented user-visible error behavior.

---

## AITP-10-08 — Interrupted/faulted persistence update cannot leave a mixed record later accepted as valid

**Trace:** IMP-EEP-01 / IMP-TEST-01; T-EEP-02
**Type:** Driver fault injection / transaction safety
**Owner:** Trung

**Preconditions**
- Test-only fault injection can interrupt/fail the EEPROM update at meaningful transaction stages.
- Test-only instrumentation remains excluded from production behavior/API.

**Procedure**
1. Begin saving a new valid alarm record.
2. Inject failure/interruption at each meaningful write stage supported by the implementation, including the validity/publication stage.
3. After each injected failure, perform `EEPROM_ReadAlarm(...)` using production validation logic.
4. Compare the returned record with the fully old and fully new values.

**Expected result**
- A partially updated/mixed record is never returned as a valid new alarm.
- Transaction safety follows the frozen rule:
  `invalidate -> hour -> minute -> valid marker last`, or an equivalent robust scheme.
- The implementation may preserve the old record or may leave persistence invalid depending on its chosen scheme; the Contract does not require old-record survival in every failure mode.
- Any record returned as valid must be internally consistent.

**Required evidence**
- Fault stage matrix.
- Raw/logical persistence state sufficient for test verification.
- `EEPROM_ReadAlarm(...)` result after every injected failure.
- Proof that no mixed/corrupt value is accepted as valid.

**Acceptance**
- `SW-PASS` only when every injected transaction-stage failure satisfies the no-mixed-valid-record rule.
- `FAIL` if any interrupted update can later be accepted as a mixed/corrupted valid alarm.

---

## AITP-10-09 — EEPROM foreground activity does not violate 300 ms / 500 ms / 30 s observable timing

**Trace:** TIM-10; Plan foreground latency/blocking rule; INT-10
**Type:** Integration performance / foreground latency
**Owner:** Tuấn + Trung boundary

**Preconditions**
- Production-equivalent EEPROM read/save behavior can be executed with measured or simulated foreground latency.
- Button/Buzzer/LED foreground service behavior and Timer tick progression are observable.
- Relevant TCOV-08/09 timing fixtures are available for reuse.

**Procedure**
1. Measure or simulate the foreground duration of representative EEPROM save/read operations.
2. Exercise EEPROM activity while timing-sensitive behaviors are active or near a boundary, including representative combinations of:
   - short-feedback servicing,
   - 500 ms Buzzer/LED periodic behavior,
   - inactivity timing near its 30 s boundary.
3. Observe whether cooperative foreground service latency changes the frozen observable behavior beyond what the Contract permits.
4. Do not “fix” excessive blocking by adding raw hardware access in Application.

**Expected result**
- EEPROM activity does not make the integrated system violate the observable:
  - ~300 ms short-feedback behavior,
  - required 500 ms periodic alarm/blink behavior,
  - 30 s inactivity semantics.
- If blocking latency is shown to violate those behaviors, TCOV-10 is not allowed to PASS merely because EEPROM data itself is correct.
- The implementation fix belongs to Driver/Platform while preserving the frozen Application boundary.

**Required evidence**
- EEPROM-operation duration/latency trace.
- Reused timing evidence from affected Buzzer/LED/timeout behavior.
- Before/during/after service timestamps sufficient to identify blocking impact.

**Acceptance**
- `SW-PASS` when software/simulator evidence shows no observable timing-contract violation for the tested production-equivalent latency.
- `HW-PENDING` where real EEPROM write-cycle/EVK timing is still needed.
- `FAIL` if measured/simulated blocking demonstrably breaks a frozen observable timing requirement.

---

## AITP-10-10 — Application does not depend on `EEPROM_Init()` or low-level I2C APIs

**Trace:** API-EEP / IMP-INIT-01; architectural boundary
**Type:** Static/integration boundary review
**Owner:** Tuấn + Trung boundary

**Preconditions**
- Production Application dependencies and startup calls can be inspected.

**Procedure**
1. Search production Application code for:
   - `EEPROM_Init()`,
   - `I2C_*`,
   - raw EEPROM/I2C hardware access.
2. Inspect startup sequence against the frozen ownership rule.

**Expected result**
- Application uses only the frozen persistence boundary:
  - `EEPROM_ReadAlarm(...)`
  - `EEPROM_SaveAlarm(...)`
- Low-level I2C initialization remains behind Board/Platform ownership.
- If `EEPROM_Init()` still exists in the Driver module, Application does not require it; any retained implementation is harmless/idempotent within the owning layer.

**Required evidence**
- Static dependency/search result.
- Startup-call trace or build dependency evidence.

**Acceptance**
- `SW-PASS` when Application remains independent of low-level EEPROM/I2C initialization details.
- `FAIL` if production Application requires `EEPROM_Init()` or directly uses I2C/raw hardware to satisfy persistence.

---

## AITP-10-11 — Valid EEPROM Save -> Read round-trip returns the exact saved HH/MM

**Trace:** T-EEP-01; API-EEP; REQ-EEP-01
**Type:** Driver-boundary software round-trip
**Owner:** Trung

**Preconditions**
- `EEPROM_SaveAlarm(...)` and `EEPROM_ReadAlarm(...)` are exercised against the same production-equivalent Driver persistence model/path.
- This software test does not claim real EVK retention across power removal.

**Procedure**
1. For each valid vector below:
   - `00:00`
   - `06:45`
   - `23:59`
2. Call `EEPROM_SaveAlarm(hour, minute)`.
3. Require save return value `1`.
4. Initialize read-back variables to sentinel values different from the saved data.
5. Call `EEPROM_ReadAlarm(&read_hour, &read_minute)`.
6. Require read return value `1`.
7. Compare read-back HH/MM with the exact value previously saved.

**Expected result**

```text
save 00:00 -> 1
read       -> 1, exactly 00:00

save 06:45 -> 1
read       -> 1, exactly 06:45

save 23:59 -> 1
read       -> 1, exactly 23:59
```

- A successful valid save is readable through the frozen public read boundary.
- Read-back HH/MM exactly matches the saved HH/MM.
- No stale or mixed record is accepted as the current valid record.
- Alarm second is not persisted through this boundary; system semantics keep alarm second fixed at `00`.
- Real I2C electrical behavior, write-cycle timing, nonvolatile retention, and reboot persistence remain `HW-PENDING`.

**Required evidence**
- Per-vector save input and return value.
- Sentinel-before/read-after values.
- Read return value.
- Exact saved-vs-read HH/MM comparison.

**Acceptance**
- `SW-PASS` only when every valid vector saves successfully and is then read back successfully with an exact HH/MM match.
- `HW-PENDING` remains for physical EVK retention/reboot proof.
- `FAIL` if a valid save cannot be read back, read returns failure, or returned HH/MM differs from the saved value.

---

## TCOV-10 exit gate

TCOV-10 may be marked software-complete only when:

```text
[ ] AITP-10-01 SW-PASS
[ ] AITP-10-02 SW-PASS
[ ] AITP-10-03 SW-PASS
[ ] AITP-10-04 SW-PASS
[ ] AITP-10-05 SW-PASS
[ ] AITP-10-06 SW-PASS (real retention/reboot may remain HW-PENDING)
[ ] AITP-10-07 SW-PASS
[ ] AITP-10-08 SW-PASS
[ ] AITP-10-09 SW-PASS (real EEPROM latency impact may retain explicit HW-PENDING)
[ ] AITP-10-10 SW-PASS
[ ] AITP-10-11 SW-PASS (real retention/reboot remains HW-PENDING)
```

TCOV-10 proves persistence restore/save/failure semantics, public-boundary validation, transaction safety, and software-visible foreground-latency compatibility. Real I2C electrical behavior, ACK/write-cycle behavior, nonvolatile retention, and EVK reboot-restore remain Driver/TCOV-14 evidence until physically verified.

---

# 13. TCOV-11 — Natural alarm occurrence / one-shot latch / re-arm / non-trigger match cases

## TCOV-11 objective

Verify the frozen alarm-occurrence semantics: `Alarm_Check()` remains a pure match predicate; a new alarm occurrence may be generated only when a natural logical `Clock_Tick1Second()` advances the Clock onto a valid confirmed `HH:MM:00`; one matching occurrence starts at most one alarm request and sets the occurrence latch; repeated evaluation while the same logical occurrence remains matched does not retrigger; a later non-matching logical Clock value clears/re-arms the latch; a future natural occurrence may trigger again; and configuration/startup actions that merely create an equal Clock/alarm value do not themselves generate an alarm.

TCOV-11 owns natural alarm occurrence, one-shot latch/re-arm, and non-trigger match cases. TCOV-12 owns multi-second catch-up and combined same-cycle processing-order boundaries. TCOV-08/14 own Buzzer priority and detailed 5-second/500 ms waveform acceptance.

---

## AITP-11-01 — Natural logical Clock tick onto confirmed alarm triggers one alarm request and latches the occurrence

**Trace:** DEC-05 / DEC-06; T-ALM-01; Plan Section 10 / TCOV-11
**Type:** Software integration / natural occurrence
**Owner:** Tuấn

**Preconditions**
- `alarm_valid = 1`.
- Confirmed alarm = `07:30:00`.
- Clock = `07:29:59`.
- `alarm_match_latched = 0`.
- Alarm-start request count is observable.

**Procedure**
1. Process exactly one natural logical `Clock_Tick1Second()`.
2. Immediately evaluate alarm occurrence through the production Application path.
3. Record resulting Clock, match result, alarm-start request count, and latch state.

**Expected result**
- Clock becomes `07:30:00`.
- The confirmed alarm matches at this exact logical second.
- `Buzzer_StartAlarm()` is requested exactly once.
- `alarm_match_latched` becomes `1`.
- This case proves occurrence generation only; detailed 5-second waveform timing remains TCOV-08/14.

**Required evidence**
- Clock before/after.
- `alarm_valid`, confirmed alarm, match result.
- Alarm-start request count.
- Latch before/after.

**Acceptance**
- `SW-PASS` when one natural tick onto the confirmed valid alarm produces exactly one alarm-start request and sets the latch.
- `FAIL` for missing, duplicate, or non-natural trigger behavior.

---

## AITP-11-02 — Re-evaluating the same matched occurrence does not retrigger

**Trace:** DEC-05; T-ALM-02
**Type:** Software integration / one-shot latch
**Owner:** Tuấn

**Preconditions**
- Clock = confirmed alarm `HH:MM:00`.
- `alarm_valid = 1`.
- `alarm_match_latched = 1` from the already-accepted occurrence.

**Procedure**
1. Invoke the same occurrence-bookkeeping/evaluation responsibility again without advancing the Clock to a new logical second.
2. Repeat more than once if the test harness allows.
3. Record alarm-start request count and latch state.

**Expected result**
- No additional `Buzzer_StartAlarm()` request is generated for the same already-latched occurrence.
- Latch remains `1` while the current logical value still matches.

**Required evidence**
- Clock/alarm equality.
- Alarm-start request count before/after repeated evaluation.
- Latch state.

**Acceptance**
- `SW-PASS` when the same matching logical occurrence remains one-shot.
- `FAIL` if repeated evaluation of the same occurrence generates another alarm-start request.

---

## AITP-11-03 — A later non-matching logical Clock value clears the latch

**Trace:** DEC-05; DEC-13 re-arm rule; Plan Section 10
**Type:** Software integration / re-arm bookkeeping
**Owner:** Tuấn

**Preconditions**
- Alarm occurrence has already been latched at `07:30:00`.
- `alarm_match_latched = 1`.

**Procedure**
1. Advance the Clock naturally to a later logical value that no longer matches, for example `07:30:01`.
2. Run the required alarm bookkeeping.
3. Observe match result and latch state.

**Expected result**
- Current Clock no longer matches confirmed `HH:MM:00`.
- `alarm_match_latched` becomes `0`.
- No new alarm-start request is generated merely by clearing the latch.

**Required evidence**
- Clock before/after.
- Match result.
- Latch before/after.
- Alarm-start request count.

**Acceptance**
- `SW-PASS` when mismatch clears the occurrence latch without creating a new trigger.
- `FAIL` if latch remains stuck or mismatch itself starts an alarm.

---

## AITP-11-04 — After re-arm, a future natural occurrence may trigger again

**Trace:** DEC-05 / DEC-06; T-ALM-01 / T-ALM-02
**Type:** Software integration / future occurrence
**Owner:** Tuấn

**Preconditions**
- A previous occurrence has triggered and later mismatch has cleared `alarm_match_latched`.
- `alarm_valid = 1`.
- Confirmed alarm remains unchanged.

**Procedure**
1. Arrange the Clock immediately before a later genuine occurrence of the same confirmed alarm.
2. Process the natural logical tick onto `HH:MM:00`.
3. Observe alarm-start request count and latch state.

**Expected result**
- The new natural occurrence is eligible to trigger because the latch was re-armed after mismatch.
- Exactly one new alarm-start request is generated for this later occurrence.
- Latch becomes `1` again.

**Required evidence**
- Proof of prior latch clear.
- Clock before/after the future occurrence.
- Alarm-start request count and latch state.

**Acceptance**
- `SW-PASS` when re-arm enables a later genuine occurrence exactly once.
- `FAIL` if latch never re-arms or the later occurrence triggers more than once.

---

## AITP-11-05 — `alarm_valid = 0` prevents a trigger even if Alarm Core/default value matches the Clock

**Trace:** DEC-05 / DEC-10 / DEC-12; T-BOOT-02
**Type:** Software integration / validity gate
**Owner:** Tuấn

**Preconditions**
- `alarm_valid = 0`.
- Alarm Core/default HH:MM may equal the current Clock.
- `alarm_match_latched` state is known.

**Procedure**
1. Arrange a logical Clock tick that results in a value equal to the Alarm Core/default `HH:MM:00`.
2. Run normal alarm-occurrence evaluation.
3. Observe alarm-start request count and latch bookkeeping.

**Expected result**
- Invalid/unarmed Alarm Core data does not create an alarm occurrence.
- No `Buzzer_StartAlarm()` request is generated.
- The match predicate alone is insufficient without `alarm_valid`.

**Required evidence**
- `alarm_valid`.
- Clock/alarm values.
- Alarm-start request count.
- Latch result.

**Acceptance**
- `SW-PASS` when an unarmed alarm cannot trigger.
- `FAIL` if equality alone bypasses `alarm_valid`.

---

## AITP-11-06 — Manual current-time edit creating exact alarm equality does not trigger

**Trace:** DEC-06; T-ALM-04; TCOV-05 dependency
**Type:** Software integration / non-trigger configuration action
**Owner:** Tuấn

**Preconditions**
- `alarm_valid = 1`.
- Confirmed alarm = a known HH:MM:00.
- System is in current-time setting.
- Current Clock can be manually edited to exactly equal the confirmed alarm.

**Procedure**
1. Use accepted SW6/SW10 current-time edits to create exact `HH:MM:00` equality.
2. Observe alarm-start request count during/after the manual edit action.
3. Do not advance a natural logical Clock second as part of the edit action.

**Expected result**
- Manual current-time editing does not generate a new alarm occurrence merely because equality is created.
- No `Buzzer_StartAlarm()` request is generated by the configuration action.
- A later natural logical Clock occurrence remains governed by normal latch/re-arm rules.

**Required evidence**
- Clock/alarm values before/after edit.
- FSM state.
- Alarm-start request count.

**Acceptance**
- `SW-PASS` when manual equality is non-triggering.
- `FAIL` if the edit action itself starts the alarm.

---

## AITP-11-07 — Confirming an alarm equal to current `HH:MM:00` does not trigger and latches the current occurrence

**Trace:** DEC-13; T-ALM-03; TCOV-06 dependency
**Type:** Software integration / confirmation collision
**Owner:** Tuấn

**Preconditions**
- FSM = `SET_ALARM_MINUTE`.
- Temporary alarm equals current Clock `HH:MM:00`.
- Alarm-start requests and latch are observable.

**Procedure**
1. Process final SW16 confirmation.
2. Observe confirmed alarm, `alarm_valid`, alarm-start request count, and latch state.
3. Advance later to a non-matching logical Clock value and observe re-arm bookkeeping.

**Expected result**
- Confirmation establishes the new valid alarm.
- Confirmation itself generates no alarm-start request.
- `alarm_match_latched = 1` for the current already-equal occurrence.
- Once the Clock later no longer matches, latch clears to `0`.
- A future natural occurrence may then trigger normally.

**Required evidence**
- Current Clock and confirmed alarm.
- Alarm-start request count at confirmation.
- Latch state at confirmation and after later mismatch.

**Acceptance**
- `SW-PASS` when equal-time confirmation is non-triggering and latch/re-arm behavior matches DEC-13.
- `FAIL` for immediate alarm start, incorrect current-occurrence latch, or failure to re-arm after mismatch.

---

## AITP-11-08 — Startup restore equal to power-on Clock does not trigger and latches the existing occurrence

**Trace:** DEC-07 / DEC-10; T-BOOT-01; TCOV-01/10 dependency
**Type:** Software integration / startup collision
**Owner:** Tuấn + Trung boundary

**Preconditions**
- Power-on Clock = `00:00:00`.
- EEPROM restore returns a valid confirmed alarm `00:00`.
- Startup alarm-start request count and latch are observable.

**Procedure**
1. Execute the frozen startup restore path.
2. Restore valid `00:00`.
3. Observe alarm-start request count and `alarm_match_latched`.
4. Advance the Clock naturally to a non-matching value and observe re-arm.

**Expected result**
- Restored alarm becomes runtime-valid.
- Startup equality does not call `Buzzer_StartAlarm()`.
- `alarm_match_latched = 1` for the already-equal power-on occurrence.
- After a later non-matching logical Clock value, latch clears and a future genuine occurrence may trigger.

**Required evidence**
- Restore result/value.
- Clock/alarm values.
- Alarm-start request count.
- Latch before/after later mismatch.

**Acceptance**
- `SW-PASS` when startup collision is latched without a spurious alarm.
- `FAIL` if boot restore itself starts the alarm or leaves the equal startup occurrence incorrectly unlatched.

---

## AITP-11-09 — Previous confirmed alarm can trigger naturally while alarm-setting mode is active

**Trace:** DEC-06; Plan Section 9/10; TCOV-06 dependency
**Type:** Software integration / cross-state natural occurrence
**Owner:** Tuấn

**Preconditions**
- `alarm_valid = 1`.
- Previous confirmed alarm is known.
- FSM = `SET_ALARM_HOUR` or `SET_ALARM_MINUTE`.
- Temporary alarm edit differs from the previous confirmed alarm.
- Clock continues normal logical ticking.

**Procedure**
1. Remain in alarm-setting mode without final confirmation.
2. Allow a natural logical Clock tick to reach the previous confirmed alarm `HH:MM:00`.
3. Observe which value is used for occurrence evaluation and the alarm-start request count.

**Expected result**
- Temporary alarm edit is not the armed trigger source.
- Previous confirmed alarm remains armed until final confirmation.
- Natural tick onto the previous confirmed alarm may generate exactly one alarm-start request and latch that occurrence.
- Alarm-setting state itself does not suppress the valid natural occurrence.

**Required evidence**
- FSM state.
- Previous confirmed alarm and temporary edit.
- Clock before/after.
- Alarm-start request count and latch.

**Acceptance**
- `SW-PASS` when natural alarm semantics remain valid during `SET_ALARM_*`.
- `FAIL` if temporary edit becomes armed early or alarm-setting mode silently disarms the previous confirmed alarm.

---

## AITP-11-10 — `Alarm_Check()` is not used as a generic poll-and-fire trigger outside logical Clock-tick context

**Trace:** DEC-05 / DEC-06; architectural trigger-context rule
**Type:** Static + behavioral integration review
**Owner:** Tuấn

**Preconditions**
- Production Application alarm-occurrence call sites can be inspected.
- Test harness can distinguish configuration bookkeeping from natural Clock-tick occurrence handling.

**Procedure**
1. Inspect production Application code for `Alarm_Check(...)` usage.
2. Classify each call site as:
   - logical Clock-tick trigger context, or
   - non-trigger bookkeeping context allowed by DEC-07/DEC-13.
3. Exercise startup restore, manual current-time edit, and alarm confirmation equality cases.
4. Verify none of those non-trigger contexts call `Buzzer_StartAlarm()` merely because `Alarm_Check()` returns match.

**Expected result**
- Alarm occurrence generation is tied to the logical Clock-tick trigger context.
- `Alarm_Check()` may support required latch/bookkeeping outside that context, but is not a generic “values match -> fire” API.
- Startup/configuration equality cases remain non-triggering.

**Required evidence**
- Alarm-check call-site/static trace.
- Behavioral request counts from non-trigger contexts.

**Acceptance**
- `SW-PASS` when production trigger ownership matches DEC-05/06.
- `FAIL` if any generic foreground poll can start an alarm solely because current values happen to match.

---

## TCOV-11 exit gate

TCOV-11 may be marked software-complete only when:

```text
[ ] AITP-11-01 SW-PASS
[ ] AITP-11-02 SW-PASS
[ ] AITP-11-03 SW-PASS
[ ] AITP-11-04 SW-PASS
[ ] AITP-11-05 SW-PASS
[ ] AITP-11-06 SW-PASS
[ ] AITP-11-07 SW-PASS
[ ] AITP-11-08 SW-PASS
[ ] AITP-11-09 SW-PASS
[ ] AITP-11-10 SW-PASS
```

TCOV-11 proves natural alarm occurrence, one-shot latch/re-arm, validity gating, and non-trigger configuration/startup matches. Multi-second catch-up and due-Clock/button/timeout combined ordering remain TCOV-12; detailed 5-second/500 ms Buzzer waveform and physical acoustic validation remain TCOV-08/14.

---

# 14. TCOV-12 — Alarm catch-up / same-cycle processing-order boundaries

## TCOV-12 objective

Verify the frozen foreground scheduling and ordering semantics under timing backlog and same-cycle contention: one coherent foreground `now` is captured; all due logical Clock seconds are advanced before Button/timeout handling; alarm occurrence is evaluated after **every** logical `Clock_Tick1Second()` step so an intermediate alarm time cannot be skipped during multi-second catch-up; Application-called Driver process functions run before Button FIFO draining; all currently queued Button events are drained before timeout evaluation; due Clock work is accounted for before a queued button may enter paused current-time setting; and alarm acoustic priority remains intact when the same cycle also produces key-feedback demand.

TCOV-12 owns combined catch-up/processing-order boundaries. TCOV-02 owns generic Clock catch-up arithmetic, TCOV-09 owns standalone button-vs-timeout priority, TCOV-11 owns standalone natural alarm occurrence/latch semantics, and TCOV-08 owns Buzzer priority semantics. TCOV-12 proves that these already-frozen behaviors compose correctly in the same foreground cycle.

---

## AITP-12-01 — Multi-second Clock catch-up evaluates alarm after every logical second

**Trace:** TIM-09; T-ALM-CATCHUP-01; Plan Section 10 / TCOV-12
**Type:** Software integration / multi-step catch-up
**Owner:** Tuấn

**Preconditions**
- `alarm_valid = 1`.
- Confirmed alarm = `07:30:00`.
- Clock = `07:29:58`.
- Scheduler has at least 3 logical seconds due in one foreground cycle.
- `alarm_match_latched = 0`.
- Alarm-start request count is observable.

**Procedure**
1. Capture the foreground cycle's coherent `now`.
2. Run the normal Clock catch-up loop for all due logical seconds.
3. Record each `Clock_Tick1Second()` result and the alarm-evaluation result immediately following that step.
4. Continue catch-up past the alarm second to the final due Clock value.

**Expected result**

```text
07:29:58
    -> tick 1: 07:29:59 -> evaluate alarm: no occurrence
    -> tick 2: 07:30:00 -> evaluate alarm: occurrence, StartAlarm once, latch=1
    -> tick 3: 07:30:01 -> evaluate alarm: mismatch, latch clears
```

- Alarm evaluation occurs after each logical second, not only after the final catch-up value.
- The intermediate `07:30:00` occurrence is not skipped even though the final Clock value no longer matches.
- Exactly one alarm-start request is generated for that occurrence.

**Required evidence**
- Ordered per-tick Clock/evaluation trace.
- Alarm-start request count.
- Latch state across each logical step.

**Acceptance**
- `SW-PASS` when intermediate alarm occurrence is observed exactly once during catch-up.
- `FAIL` if evaluation occurs only after the final catch-up value or the intermediate occurrence is lost.

---

## AITP-12-02 — Catch-up with no intermediate alarm match produces no alarm request

**Trace:** TIM-09 / DEC-05; TCOV-11 dependency
**Type:** Software integration / negative catch-up
**Owner:** Tuấn

**Preconditions**
- `alarm_valid = 1`.
- Confirmed alarm does not occur within the due logical-second interval.
- Multiple Clock seconds are due.

**Procedure**
1. Execute normal multi-second catch-up.
2. Record each logical Clock value and alarm evaluation.
3. Observe alarm-start request count.

**Expected result**
- Every logical second is still evaluated.
- No `Buzzer_StartAlarm()` request is generated when no due logical second reaches the confirmed alarm occurrence.
- Catch-up itself does not manufacture an alarm merely because multiple seconds were processed.

**Required evidence**
- Per-step Clock/evaluation trace.
- Alarm-start request count.

**Acceptance**
- `SW-PASS` when all steps are evaluated and none falsely triggers.
- `FAIL` for a false alarm occurrence during non-matching catch-up.

---

## AITP-12-03 — Due Clock second is processed before a queued button enters SET_TIME

**Trace:** Same-cycle priority; T-ORDER-01; Plan Section 14
**Type:** Software integration / due-Clock + Button boundary
**Owner:** Tuấn

**Preconditions**
- FSM = `NORMAL`.
- One logical Clock second is due in the current foreground cycle.
- A valid SW3 click is already queued.
- Clock value makes the one-second advancement observable.
- Current-time setting pauses future automatic Clock progression after entry.

**Procedure**
1. Start one foreground cycle with both conditions already true:
   - one due Clock second,
   - queued SW3.
2. Record ordered execution from Clock stage through Button stage.
3. Observe Clock value and FSM state after the cycle.

**Expected result**
- The due logical Clock second is advanced first.
- Alarm occurrence, if relevant to that logical second, is evaluated before Button processing.
- Only afterward is SW3 processed and FSM enters `SET_TIME_HOUR`.
- The due second is not lost merely because the button enters a paused current-time state in the same cycle.

**Required evidence**
- Ordered trace: due Clock tick/evaluation -> Driver service -> Button event/FSM transition.
- Clock before/after.
- FSM before/after.

**Acceptance**
- `SW-PASS` when the due second is fully accounted for before the SET_TIME transition.
- `FAIL` if Button processing pauses Clock early and loses the already-due logical second.

---

## AITP-12-04 — Due Clock alarm occurrence is generated before same-cycle key-feedback request

**Trace:** Same-cycle priority; DEC-06; API-BUZ priority; TCOV-08/11 dependencies
**Type:** Software integration / alarm + Button contention
**Owner:** Tuấn + Trung boundary

**Preconditions**
- `alarm_valid = 1`.
- Clock is one logical second before confirmed alarm.
- One Clock second is due.
- A valid Button event is already queued.
- Alarm-start and short-feedback requests are observable.

**Procedure**
1. Execute the normal foreground cycle.
2. Record:
   - Clock advancement,
   - alarm occurrence request,
   - queued Button processing,
   - short-feedback request,
   - Buzzer priority outcome.

**Expected result**
- Due Clock tick reaches the confirmed alarm and generates the alarm occurrence before Button-stage feedback.
- The Button event is still processed normally afterward.
- Its logical short-feedback request still occurs.
- Acoustic output remains governed by alarm priority; the same-cycle key feedback cannot cancel/restart/shorten/corrupt the alarm.

**Required evidence**
- Ordered alarm-start vs Button-feedback request trace.
- Buzzer mode/priority trace sufficient to show alarm dominance.

**Acceptance**
- `SW-PASS` when logical ordering is Clock/alarm first, Button feedback second, with alarm acoustic priority preserved.
- `FAIL` if Button feedback suppresses the legitimate alarm occurrence or corrupts the alarm sequence.

---

## AITP-12-05 — Application-called Driver process functions run before Button FIFO drain

**Trace:** Plan Section 14 stage C -> D; Contract foreground order
**Type:** Integration scheduling / ordering
**Owner:** Tuấn

**Preconditions**
- Test instrumentation can record calls to:
  `Button_Process()`, `Buzzer_Process()`, `LED_Process()`, and `Button_GetEvent()`/FIFO drain.

**Procedure**
1. Execute representative foreground cycles with and without queued events.
2. Record the ordering of Application-called Driver process functions and FIFO draining.

**Expected result**
- `Button_Process()`, `Buzzer_Process()`, and `LED_Process()` run in the Driver-service stage before Button FIFO draining begins.
- Display blink/scan servicing is not introduced as an Application scheduling dependency.
- The test does not require exact micro-order among internal Driver-owned asynchronous services beyond the frozen Application stage ownership.

**Required evidence**
- Ordered call trace for stages C and D.
- Static dependency evidence for Display service ownership where useful.

**Acceptance**
- `SW-PASS` when stage C precedes stage D according to the frozen foreground sequence.
- `FAIL` if production Application drains Button events before the required foreground Driver-service stage.

---

## AITP-12-06 — All currently queued Button events are drained before timeout evaluation

**Trace:** DEC-11; T-ORDER-01; Plan Section 14 stages D -> E; TCOV-09 dependency
**Type:** Software integration / Button + timeout composition
**Owner:** Tuấn

**Preconditions**
- FSM starts in a SET state.
- Timeout would otherwise be expired at this cycle's coherent `now`.
- Multiple valid debounced Button events are already queued before stage D.

**Procedure**
1. Execute one normal foreground cycle.
2. Record all FIFO events and state/activity effects.
3. Record the point at which timeout evaluation occurs.

**Expected result**
- All currently queued events are drained in FIFO order before timeout evaluation.
- Activity/timeout state is updated after each event according to the resulting state.
- Timeout evaluation does not interleave between events already queued at the start of the drain stage.
- DEC-11 prevents simultaneous timeout dispatch for a valid queued boundary event where the resulting activity/state suppresses the timeout.

**Required evidence**
- Full stage-D event trace.
- Stage-E timeout evaluation trace.
- Timeout dispatch count.

**Acceptance**
- `SW-PASS` when Button queue drain completes before timeout evaluation.
- `FAIL` if timeout is evaluated between already-queued events or double-dispatch violates DEC-11.

---

## AITP-12-07 — Due Clock + queued Button + expired timeout follow B -> D -> E without lost second or double-dispatch

**Trace:** T-ORDER-01; DEC-11; Plan Section 14
**Type:** Full same-cycle integration boundary
**Owner:** Tuấn

**Preconditions**
- A SET-capable scenario is arranged so that in one foreground cycle:
  - at least one Clock second is due where Clock scheduling is active,
  - a valid Button event is already queued,
  - inactivity timeout would otherwise be expired at the coherent `now`.
- Event/state choice makes the resulting timeout action observable.

**Procedure**
1. Execute the full production foreground cycle.
2. Record:
   - coherent `now`,
   - due Clock advancement(s) and alarm evaluation(s),
   - Driver-service stage,
   - queued Button event/action,
   - activity/timeout-state update,
   - timeout evaluation/dispatch.
3. Verify no work item is lost or executed twice.

**Expected result**

```text
A: snapshot coherent now
B: account all due Clock second(s) + alarm evaluation per step
C: service Application-called Drivers
D: process queued Button event(s)
E: evaluate timeout using resulting activity/state
```

- Due Clock work is not lost.
- Button event is processed exactly once.
- Timeout is not simultaneously dispatched when DEC-11 says the queued valid event wins.
- No duplicate Clock tick, Button dispatch, or timeout dispatch occurs.

**Required evidence**
- One complete ordered foreground-cycle trace.
- Counts for Clock ticks, alarm evaluations, Button dispatches, and timeout dispatches.

**Acceptance**
- `SW-PASS` when all three competing responsibilities compose exactly in frozen order.
- `FAIL` for lost second, lost/duplicate Button event, or forbidden double timeout dispatch.

---

## AITP-12-08 — One coherent foreground `now` is used consistently for Clock and timeout bookkeeping in the cycle

**Trace:** Plan Section 14 stage A; frozen foreground scheduling rule
**Type:** Software integration / coherent timestamp
**Owner:** Tuấn

**Preconditions**
- Timer source can advance during a foreground cycle.
- Test instrumentation can identify timestamp reads used for Clock due-work and timeout bookkeeping.

**Procedure**
1. Begin a foreground cycle near a relevant timing boundary.
2. Allow the raw underlying timer to advance during later parts of the cycle.
3. Observe the timestamp/reference used by Clock scheduling and timeout bookkeeping.
4. Exclude later operations that explicitly require a fresh timestamp by contract.

**Expected result**
- One coherent current-tick snapshot is captured for the foreground scheduling cycle.
- Clock/timeout bookkeeping in that cycle uses the coherent snapshot consistently unless a later operation explicitly requires a fresh timestamp.
- The system does not create ordering ambiguity by independently sampling slightly different `now` values for competing stage-B/stage-E decisions.

**Required evidence**
- Timestamp-read/use trace.
- Clock/timeout decision inputs.

**Acceptance**
- `SW-PASS` when the coherent-snapshot rule is respected.
- `FAIL` if inconsistent same-cycle timestamps change the frozen scheduling outcome.

---

## AITP-12-09 — No logical Clock advancement occurs in SET_TIME even when Timer backlog exists

**Trace:** DEC-01/02; Contract foreground stage B; TCOV-05 dependency
**Type:** Software integration / paused-state backlog boundary
**Owner:** Tuấn

**Preconditions**
- FSM = `SET_TIME_HOUR` or `SET_TIME_MINUTE`.
- Raw Timer tick advances enough that multiple nominal one-second intervals pass.
- Current-time edit session remains active.

**Procedure**
1. Execute repeated foreground cycles while the Timer advances.
2. Observe Clock tick count and alarm occurrence evaluation count.
3. Exit to NORMAL using the frozen fresh-anchor behavior.
4. Observe first post-exit logical Clock tick.

**Expected result**
- No `Clock_Tick1Second()` occurs while in current-time setting.
- No natural alarm occurrence is generated from the paused Timer backlog.
- Elapsed edit time is not replayed as Clock catch-up after exit.
- Post-exit Clock progression resumes from the fresh anchor already owned by TCOV-05.

**Required evidence**
- Timer/Clock/state trace.
- Clock-tick and alarm-evaluation counts during edit.
- Post-exit anchor/tick evidence reused from TCOV-05 where appropriate.

**Acceptance**
- `SW-PASS` when Timer backlog cannot leak into paused current-time Clock progression.
- `FAIL` if paused elapsed time creates hidden Clock/alarm catch-up.

---

## TCOV-12 exit gate

TCOV-12 may be marked software-complete only when:

```text
[ ] AITP-12-01 SW-PASS
[ ] AITP-12-02 SW-PASS
[ ] AITP-12-03 SW-PASS
[ ] AITP-12-04 SW-PASS
[ ] AITP-12-05 SW-PASS
[ ] AITP-12-06 SW-PASS
[ ] AITP-12-07 SW-PASS
[ ] AITP-12-08 SW-PASS
[ ] AITP-12-09 SW-PASS
```

TCOV-12 proves multi-second alarm-safe catch-up and the combined foreground scheduling order under same-cycle contention. Standalone Clock arithmetic remains TCOV-02, standalone timeout priority TCOV-09, standalone natural-alarm semantics TCOV-11, Buzzer overlap TCOV-08, and final phase-stability/full regression TCOV-14.

---

# 15. TCOV-13 — Watchdog / shared config / shared project / production target / test-instrumentation integration checks

## TCOV-13 objective

Verify the frozen production-structure and shared-integration gates that are not ordinary functional FSM tests: Application uses the frozen `Board_FeedWatchdog()` Platform abstraction and never raw `WDTR`; the wrapper is safe for both enabled-WDT and disabled-WDT production policies; the production foreground loop reaches the watchdog wrapper once per normal foreground cycle; official timing values resolve from an agreed shared configuration without conflicting production literals; cross-boundary public headers match Contract v2.7; the production target contains exactly one production `main()` and excludes standalone test-runner mains; test-only hooks/mocks/fault injection are excluded from production behavior/API; Application does not depend on raw hardware/non-boundary APIs; shared `firmware/config/` and `firmware/project/` changes receive the required shared-owner review; and the production Keil target is structurally clean with zero errors and reviewed warnings.

TCOV-13 owns static/build/project/config/watchdog integration quality. It does not replace TCOV-01 startup behavioral testing or TCOV-14 final regression/EVK evidence. Internal Application filenames or helper names are not frozen and must not be tested as contract requirements.

---

## AITP-13-01 — Application uses `Board_FeedWatchdog()` and never accesses raw `WDTR`

**Trace:** API-BOARD/TMR; IMP-WDT-01; Application composition requirement
**Type:** Static architecture review
**Owner:** Tuấn + Trung boundary

**Preconditions**
- Production Application and Platform sources/headers are available for inspection.

**Procedure**
1. Search production Application code for:
   - `Board_FeedWatchdog()`,
   - `WDTR`,
   - direct watchdog SFR/register manipulation.
2. Inspect the Platform boundary declaration.
3. Verify raw watchdog implementation details remain Platform-owned.

**Expected result**
- Application watchdog interaction occurs only through `Board_FeedWatchdog()`.
- Production Application contains no direct `WDTR` access and no duplicated raw watchdog feed sequence.
- `Board_FeedWatchdog()` remains the frozen cross-team abstraction.

**Required evidence**
- Static search result.
- Public Platform header/prototype evidence.
- Application call-site evidence.

**Acceptance**
- `SW-PASS` when Application is completely isolated from raw watchdog hardware.
- `FAIL` if Application accesses `WDTR` or another raw watchdog register/path directly.

---

## AITP-13-02 — Watchdog wrapper supports enabled-WDT and disabled-WDT policies without changing Application API

**Trace:** API-BOARD/TMR watchdog freeze rule; IMP-WDT-01
**Type:** Platform boundary / policy verification
**Owner:** Trung

**Preconditions**
- Platform implementation and build configuration can represent/document the production watchdog policy.

**Procedure**
1. Inspect/test the wrapper under the production WDT-enabled policy when that policy is used.
2. Inspect/test the wrapper under the WDT-disabled policy or corresponding compile/config path.
3. Confirm Application call sites are unchanged between policies.

**Expected result**
- If WDT is enabled, `Board_FeedWatchdog()` performs the Platform-owned required feed behavior.
- If WDT is disabled, the same function is a documented safe no-op.
- No Application API or call-site change is required merely because watchdog policy changes.
- This test does not prescribe the raw register sequence beyond Platform correctness.

**Required evidence**
- Platform implementation/config evidence for the applicable policy paths.
- Same public prototype and Application call-site evidence.

**Acceptance**
- `SW-PASS` when the frozen wrapper safely represents both policy outcomes without leaking raw hardware ownership.
- `FAIL` if Application must know the watchdog policy or if disabled-WDT behavior is unsafe/undefined.

---

## AITP-13-03 — Normal production foreground cycle reaches watchdog feed stage once

**Trace:** Plan Section 14 stage H; Application composition requirement
**Type:** Software integration / scheduling structure
**Owner:** Tuấn

**Preconditions**
- Production-equivalent `Application_Process()`/foreground root is observable.
- Test instrumentation can count wrapper calls without becoming a production dependency.

**Procedure**
1. Execute representative normal foreground cycles across several FSM states.
2. Count `Board_FeedWatchdog()` calls per completed foreground cycle.
3. Include cycles with:
   - no queued input,
   - queued input,
   - due Clock work,
   - normal SET-state processing.
4. Keep long blocking-driver compatibility as a separate latency concern already covered where applicable.

**Expected result**
- Each normally completed foreground cycle reaches the frozen watchdog feed stage exactly once.
- Functional state does not bypass the watchdog stage.
- Application still calls the same wrapper when Platform implements it as a no-op.

**Required evidence**
- Per-cycle ordered call/count trace.
- Evidence from more than one FSM/foreground condition.

**Acceptance**
- `SW-PASS` when normal foreground composition consistently reaches one wrapper call per completed cycle.
- `FAIL` if ordinary state/event paths silently omit or duplicate the Application watchdog-stage call.

---

## AITP-13-04 — Official timing values resolve from one agreed configuration without conflicting production copies

**Trace:** IMP-CFG-01; T-CFG-01; Plan shared timing/config rule
**Type:** Build/static configuration review
**Owner:** Both

**Preconditions**
- Production config headers/sources and Application/Driver timing references are available.

**Procedure**
1. Trace the production definitions/references for:
   - key/timeout short feedback = `300 ms`,
   - blink half-period = `500 ms`,
   - alarm total = `5000 ms`,
   - inactivity timeout = `30000 ms`.
2. Search production source for conflicting hard-coded copies of these official timing behaviors.
3. Verify Application and Driver code resolve to the agreed configuration values.
4. Distinguish harmless test vectors/comments from production behavioral constants.

**Expected result**

```text
short feedback       300 ms
blink half-period    500 ms
alarm total         5000 ms
inactivity         30000 ms
```

- Production behavior has one agreed configuration source/meaning for these values.
- No conflicting production literal silently overrides the shared contract values.
- Shared config ownership remains respected.

**Required evidence**
- Definition/reference map.
- Static search for competing production literals.
- Build/preprocessor evidence where needed.

**Acceptance**
- `SW-PASS` when all four official values resolve consistently across Application/Drivers.
- `FAIL` if conflicting production constants can produce different contract behavior.

---

## AITP-13-05 — Cross-boundary public headers match frozen Contract v2.7

**Trace:** IMP-API-01; PR gate G1/G2
**Type:** Static API compatibility review
**Owner:** Both

**Preconditions**
- Merged production public headers are available.
- Contract Section 3 is the source of truth.

**Procedure**
1. Compare the public cross-boundary function prototypes, types, and enums used by Application against Contract Section 3.
2. Check for missing required boundary declarations.
3. Check for unapproved additions/removals that Application now depends on.
4. Ignore owner-internal helpers that are not Application dependencies.

**Expected result**
- Required cross-boundary function prototypes/types/enums match the frozen contract.
- Application does not depend on an unapproved new public helper to make integration work.
- Internal implementation helpers do not accidentally become a de facto Application boundary.

**Required evidence**
- Contract-to-header comparison.
- Application dependency/static include/call evidence.

**Acceptance**
- `SW-PASS` when the merged boundary is contract-compatible.
- `FAIL` for a breaking prototype/type/enum mismatch or an unapproved Application dependency.

---

## AITP-13-06 — Production target contains exactly one production `main()` and excludes standalone test-runner mains

**Trace:** IMP-BUILD-01; Plan P8; Application composition requirement
**Type:** Keil/project structural review
**Owner:** Shared

**Preconditions**
- Production Keil target/project file and compiled source membership are available.

**Procedure**
1. Enumerate every source file included in the production target that defines `main()`.
2. Identify standalone `*_test.c`, simulator runner, or other test-runner entry points.
3. Inspect production target membership/exclusion.
4. Build the production target.

**Expected result**
- Exactly one production `main()` is linked.
- Standalone test-runner `main()` files are excluded from the production target.
- Test targets may still contain their own entry points outside production.

**Required evidence**
- Production target source membership.
- Search/list of `main()` definitions.
- Link/build evidence.

**Acceptance**
- `SW-PASS` when production has exactly one entry point and test runners are structurally excluded.
- `FAIL` for zero/multiple production mains or an included standalone test-runner main.

---

## AITP-13-07 — Test-only hooks/mocks/fault injection are excluded from production behavior and frozen API

**Trace:** IMP-TEST-01; Plan P8; PR gate G4/G10
**Type:** Static/build instrumentation isolation
**Owner:** Both

**Preconditions**
- Test instrumentation used by TCOV cases is identifiable.
- Production and test build configurations are available.

**Procedure**
1. Inventory test-only hooks, observability seams, mocks, and fault-injection helpers.
2. Verify each is:
   - guarded by a test-build mechanism or otherwise excluded from production,
   - not called by Application Integration as a production dependency,
   - absent from the frozen cross-team API.
3. Build/inspect the production target without test instrumentation.
4. Confirm production behavior does not require hooks to function.

**Expected result**
- Test instrumentation is available only where needed for verification.
- Production Application has no dependency on test-only seams.
- Removing/omitting test hooks does not alter required production behavior.
- Tests assert observed state against expected behavior rather than unconditional PASS flags.

**Required evidence**
- Instrumentation inventory and build guards/exclusion evidence.
- Production symbol/dependency evidence.
- Representative assertion evidence.

**Acceptance**
- `SW-PASS` when test observability remains strictly test-only and meaningful.
- `FAIL` if a test hook leaks into production behavior/API or tests rely on unconditional success markers instead of observed state.

---

## AITP-13-08 — Production Application has no raw hardware/non-boundary dependency

**Trace:** Contract Section 3.1; Application composition requirement
**Type:** Static architecture review
**Owner:** Tuấn

**Preconditions**
- Production Application source/dependencies can be searched.

**Procedure**
1. Search production Application code for forbidden/non-boundary dependencies including:
   - `GPIO_*`,
   - `I2C_*`,
   - `Timer_ISR_Handler()`,
   - `Timer_DelayUs()`,
   - raw MCU `Px`/SFR registers,
   - `WDTR`,
   - `PIN_HW_*`,
   - `Display_UpdateBlinkState()`,
   - `Display_ScanRoutine()`,
   - `Button_IsPressed()`.
2. Review any textual match to distinguish production dependency from comment/test-only text.

**Expected result**
- Application uses only frozen boundary APIs needed by the integration design.
- Raw hardware/register/timing-service ownership remains in Driver/Platform.
- No integration workaround bypasses the ownership boundary.

**Required evidence**
- Static search result with reviewed matches.
- Dependency/call graph evidence where useful.

**Acceptance**
- `SW-PASS` when production Application has no forbidden non-boundary dependency.
- `FAIL` if Application bypasses the frozen boundary to access hardware or Driver-internal service functions.

---

## AITP-13-09 — Shared `firmware/config/` and `firmware/project/` changes have shared-owner review evidence

**Trace:** Plan shared ownership / P10/P11 / safe-work rule
**Type:** Integration process/configuration gate
**Owner:** Both

**Preconditions**
- Integration branch diff/history and relevant PR/review metadata are available.
- Shared config/project changes, if any, can be identified.

**Procedure**
1. Identify changes under shared timing/config and production-project areas.
2. For each cross-impact change, verify shared-owner review/agreement evidence before merge.
3. Verify no Application-only convenience change silently modifies Trung-owned/shared project behavior.
4. If there are no such changes, record that the gate is not triggered rather than inventing review work.

**Expected result**
- Cross-impact `firmware/config/` and required `firmware/project/` production-target changes are reviewed as Shared ownership.
- No unilateral shared-project/config change silently changes frozen behavior.
- Absence of shared changes is acceptable with evidence.

**Required evidence**
- Relevant diff paths.
- PR/review/acknowledgement evidence when the gate is triggered.
- Explicit no-change result when not triggered.

**Acceptance**
- `SW-PASS` when shared-change governance is satisfied for the actual diff.
- `FAIL` if a cross-impact shared config/project change is merged without the required shared review.

---

## AITP-13-10 — Production Keil target is structurally clean: 0 errors, warnings reviewed, no unrelated/generated junk in integration diff

**Trace:** Plan P9; PR gate G7/G8/G9
**Type:** Build/repository quality gate
**Owner:** Shared

**Preconditions**
- Production target can be built from the reviewed integration baseline.
- Integration diff against its intended base is available.

**Procedure**
1. Perform a clean production Keil build.
2. Record error and warning counts.
3. Review every remaining warning for relevance/acceptability; do not equate “warning exists” with automatic failure unless it represents a real unresolved defect.
4. Inspect the integration diff for unrelated/generated/test-output junk accidentally included in production changes.

**Expected result**
- Production build has `0 errors`.
- Warnings are explicitly reviewed and any real blocker is resolved.
- Production target is structurally ready.
- Integration diff contains no unrelated/generated junk that should not be merged.

**Required evidence**
- Clean-build log/summary.
- Warning review notes.
- Scoped diff/file-list evidence.

**Acceptance**
- `SW-PASS` when build structure is clean, warnings are reviewed, and the diff is appropriately scoped.
- `FAIL` for build errors, unresolved blocker warnings, or unrelated/generated junk that invalidates the production merge gate.

---

## TCOV-13 exit gate

TCOV-13 may be marked software/structure-complete only when:

```text
[ ] AITP-13-01 SW-PASS
[ ] AITP-13-02 SW-PASS
[ ] AITP-13-03 SW-PASS
[ ] AITP-13-04 SW-PASS
[ ] AITP-13-05 SW-PASS
[ ] AITP-13-06 SW-PASS
[ ] AITP-13-07 SW-PASS
[ ] AITP-13-08 SW-PASS
[ ] AITP-13-09 SW-PASS
[ ] AITP-13-10 SW-PASS
```

TCOV-13 proves watchdog abstraction use, shared timing/config consistency, frozen API compatibility, production target structure, test-instrumentation isolation, ownership-boundary cleanliness, shared project/config governance, and build/diff readiness. Final end-to-end regression, 500 ms phase-stability closure, and physical EVK handoff remain TCOV-14.

---

# 16. TCOV-14 — Full software regression / 500 ms phase-stability regression / EVK HW-PENDING handoff

## TCOV-14 objective

Close the Integration Test Plan without converting unverified hardware assumptions into PASS claims. TCOV-14 re-runs the integrated software behavior after all preceding TCOV work, closes the remaining mandatory 5-second/500 ms Buzzer waveform obligation, verifies that Display/LED/Buzzer 500 ms periodic behavior does not accumulate scheduler-jitter phase drift, confirms that Driver simulator assertions observe real state rather than unconditional success flags, and creates an explicit EVK handoff matrix for all genuinely physical Button/Display/LED/Buzzer/EEPROM/Timer checks.

TCOV-14 is the final regression/handoff layer, not a place to invent new product semantics. A software-supported item may close as `SW-PASS`; a genuinely physical item remains `HW-PENDING` until board evidence exists; final `PASS` is permitted only after all applicable software and physical acceptance evidence is present.

---

## AITP-14-01 — Full integrated software regression re-runs all previously frozen TCOV behavior

**Trace:** T-REG-01; Plan TCOV-14
**Type:** Full software regression
**Owner:** Both

**Preconditions**
- TCOV-01 through TCOV-13 have reviewed/frozen test definitions.
- Production-equivalent software baseline and required test fixtures are available.
- Any known hardware-only checks remain explicitly separated.

**Procedure**
1. Re-run the software-applicable acceptance set from TCOV-01 through TCOV-13 against one reviewed integration baseline.
2. Record pass/fail status without silently carrying forward historical PASS results from an older baseline.
3. Re-run all previously corrected coverage cases, including:
   - EEPROM valid Save -> Read round-trip,
   - transactional alarm behavior,
   - timeout/button priority,
   - multi-second alarm catch-up,
   - production/build/config/static gates.
4. Record any skipped case with an explicit reason and classification.

**Expected result**
- No previously frozen software behavior regresses on the final integration baseline.
- No software-required case is silently omitted.
- Hardware-only evidence is not fabricated to turn `HW-PENDING` into `PASS`.
- A new software failure re-opens only the affected requirement/coverage area plus necessary regression dependencies.

**Required evidence**
- Regression matrix containing TCOV/test ID, baseline commit/build identity, result, and evidence reference.
- Explicit list of any non-executed cases and why.

**Acceptance**
- `SW-PASS` when all applicable software/architecture cases pass on the reviewed baseline.
- `FAIL` if any required software case fails or is silently skipped.

---

## AITP-14-02 — Base 5-second alarm waveform proves each 500 ms ON/OFF transition, not only total busy duration

**Trace:** REQ-BUZ-02; IMP-BUZ-02; T-BUZ-02
**Type:** Driver software timing + EVK waveform handoff
**Owner:** Trung

**Preconditions**
- No short-feedback conflict is active for the base waveform case.
- Buzzer logical output/state is observable in a simulator/test build.
- Alarm total = 5000 ms and half-period = 500 ms from the agreed timing configuration.

**Procedure**
1. Issue one legitimate `Buzzer_StartAlarm()` request at reference time `t0`.
2. Service the Driver using its production timing mechanism.
3. Observe logical Buzzer output immediately before/at/after the nominal phase boundaries:
   - `t0 + 500 ms`
   - `1000 ms`
   - `1500 ms`
   - `2000 ms`
   - `2500 ms`
   - `3000 ms`
   - `3500 ms`
   - `4000 ms`
   - `4500 ms`
   - completion at approximately `5000 ms`.
4. Record every ON/OFF phase transition, not merely `Buzzer_IsBusy()`.
5. Repeat the equivalent measurement on EVK when hardware is available.

**Expected result**
- The alarm sequence alternates ON/OFF at the required 500 ms phase boundaries for approximately 5 seconds total.
- Software evidence proves the actual logical output/phase sequence, not only total busy duration.
- No extra phase extends the sequence because of repeated processing.
- Simulator success does not by itself prove physical sound/electrical waveform timing.

**Required evidence**
- Time-indexed logical output/phase trace covering the complete 5-second sequence.
- EVK scope/logic/analyzed physical evidence when available.

**Acceptance**
- `SW-PASS` when the complete logical 500 ms transition sequence is proven in software.
- `HW-PENDING` until physical EVK timing/output is measured where required.
- `FAIL` if software only proves total duration while phase transitions are missing/incorrect.

---

## AITP-14-03 — Display/LED/Buzzer 500 ms periodic phases do not accumulate drift under foreground jitter

**Trace:** TIM-12; IMP-TIME-01; T-PHASE-01
**Type:** Driver timing regression / jitter injection
**Owner:** Trung

**Preconditions**
- Test-only timing observability is available.
- Display blink, LED alarm-setting blink, and Buzzer alarm phase logic can each be exercised.
- Foreground service jitter can be injected without changing the underlying tick source.

**Procedure**
1. Establish each 500 ms periodic behavior separately:
   - Display HH/MM blink,
   - LED D4 alarm-setting blink,
   - Buzzer alarm phase.
2. Inject repeated late/irregular `*_Process()` servicing around nominal phase boundaries.
3. Record the intended schedule and actual logical phase transitions over multiple periods.
4. Detect whether each small service delay shifts all later deadlines cumulatively.
5. Repeat long enough that cumulative drift would be visible if the implementation repeatedly used `last_tick = current_tick` incorrectly.

**Expected result**
- Late foreground servicing may delay an individual observable transition by finite service latency.
- Future 500 ms phase boundaries remain tied to a stable intended time base.
- Small delays do not accumulate into permanent phase drift across repeated periods.
- Equivalent tick/deadline schemes are accepted; the test does not require one specific implementation.

**Required evidence**
- Jitter injection schedule.
- Intended-vs-observed phase table for Display, LED, and Buzzer.
- Cumulative phase-error trend.

**Acceptance**
- `SW-PASS` when no tested 500 ms behavior accumulates scheduler-delay drift.
- `FAIL` if repeated foreground lateness permanently shifts later phase boundaries.

---

## AITP-14-04 — Display regression proves real simulated output state, blink boundaries, and concurrency safety

**Trace:** T-DSP-02; T-DSP-03; T-DSP-BLINK-01; IMP-CONC-01 / IMP-TEST-01
**Type:** Driver simulator/concurrency regression
**Owner:** Trung

**Preconditions**
- Display simulator/test build exposes actual segment/digit/blink state through test-only instrumentation.
- Any ISR/foreground shared Display state can be stressed in the software environment.

**Procedure**
1. Exercise HH blink and MM blink modes across multiple 500 ms half-period boundaries.
2. Assert actual simulated segment/digit state for visible and blanked fields.
3. Run repeated foreground Display updates while scan/blink service executes.
4. Check segment index/state/memory for corruption or invalid access.
5. Inspect test implementation for unconditional PASS variables or assertions disconnected from actual simulated output.

**Expected result**
- HH/MM requested blink modes visibly change the corresponding simulated field at the required logical boundaries.
- Assertions are derived from actual simulated output state.
- Concurrent foreground update + scan/blink servicing does not corrupt Display state or memory.
- Physical multiplex quality/brightness remains an EVK item.

**Required evidence**
- Simulated segment/digit traces around blink boundaries.
- Concurrency stress result.
- Representative real-state assertions.

**Acceptance**
- `SW-PASS` when Display simulator, blink, and concurrency obligations are genuinely proven.
- `HW-PENDING` for physical multiplex/visibility quality.
- `FAIL` for unconditional PASS instrumentation, corrupted shared state, or incorrect blink output.

---

## AITP-14-05 — Mandatory Driver/Platform software quality gates are explicitly closed before final HW handoff

**Trace:** Contract Section 11 mandatory quality gates; PR gate G3; T-REG-01
**Type:** Consolidated software/architecture closure audit
**Owner:** Both, with implementation evidence from the owning layer

**Preconditions**
- Final reviewed Driver/Platform/Application software baseline is fixed.
- Existing TCOV evidence may be reused where it already proves a listed gate.
- No genuinely physical requirement is forced into this software case.

**Procedure**
1. Build a closure matrix for each mandatory software/architecture gate below.
2. For every gate, either:
   - reference existing TCOV evidence that directly proves it, or
   - execute the smallest additional static/software test needed to prove it.
3. Do not mark a gate closed merely because related hardware evidence is still pending.
4. Record `SW-PASS` or `FAIL` for every software gate.

**Required gate matrix**

```text
IMP-TMR-01
    Atomic 32-bit tick read.
    Any temporary interrupt masking preserves/restores the previous interrupt state.

IMP-TMR-02
    Firmware clock-chain assumptions/configuration are internally consistent.
    Timer reload derives from the verified configured CPU/timer input clock.
    Documentation matches actual Timer Mode 1/software-reload behavior.

IMP-TMR-03
    Timer_DelayUs() is not claimed calibrated without evidence.

IMP-I2C-01
    Software I2C drive/release/ACK logic is correct for open-drain-style operation.
    No unsafe push-pull HIGH assumption is used.

IMP-BTN-01
    Button input mode/pull-up configuration is explicitly correct in software/configuration.

IMP-BTN-03
    Button FIFO behavior/overflow policy is documented and memory-safe.
    Application-facing event enum compatibility remains intact.

IMP-PIN-01
    pin_config.h and the actual C51 hardware binding cannot silently disagree.

IMP-EEP-02
    EEPROM device/part naming is consistent across production code and documentation.

IMP-DOC-01
    Driver specs/README scheduling statements match the final foreground-vs-ISR ownership.

IMP-DOC-02
    Board_Init() documentation does not claim system-clock initialization unless it actually configures the oscillator/CPU clock.

IMP-DOC-03
    Report/workflow/API examples match the frozen boundary and do not preserve outdated illustrative APIs or obsolete integration-order wording.
```

**Expected result**
- Every listed software/architecture/documentation gate has explicit evidence and a final `SW-PASS` or `FAIL`.
- Existing evidence may be reused rather than duplicated.
- EEPROM device naming is consistent across production code/docs.
- Driver scheduling/docs, `Board_Init()` documentation, and report/workflow/API examples agree with the frozen boundary and actual implementation ownership.
- No gate is considered closed by assumption, by comment wording alone, or by a future EVK plan.
- Hardware-only follow-up remains separate:
  - physical Timer accuracy/drift,
  - physical I2C pull-ups/bus behavior,
  - physical Button polarity,
  - physical Display/LED/Buzzer behavior,
  - physical EEPROM retention.
- A reproducible software/architecture failure remains `FAIL`, never `HW-PENDING`.

**Required evidence**
- Gate-by-gate closure matrix containing:
  - gate ID,
  - owning layer,
  - evidence source/test ID,
  - observed result,
  - final status.
- For reused evidence, exact cross-reference to the TCOV/AITP case or reviewed code/config evidence.
- For newly executed checks, static/test output sufficient to reproduce the conclusion.

**Acceptance**
- `SW-PASS` only when every listed mandatory software/architecture gate is explicitly closed with evidence.
- `FAIL` if any listed gate is unresolved, assumed, contradicted, or incorrectly deferred as hardware-only.

---

## AITP-14-06 — Button EVK handoff verifies electrical polarity and hold/no-retrigger behavior

**Trace:** IMP-BTN-HW-01; IMP-BTN-02; Button hardware handoff
**Type:** EVK hardware validation
**Owner:** Trung

**Preconditions**
- Physical EVK and SW3/SW6/SW10/SW16 are available.
- Software Button logic/configuration has already passed its software checks.

**Procedure**
1. Measure/observe each required button's idle and pressed electrical/logical state.
2. For each button, perform a normal press/release and confirm one accepted event.
3. Hold each button longer than debounce and normal human press duration.
4. Confirm:
   - first click/event occurs,
   - no repeated click/event is generated while held,
   - a later release + new press can generate the next event.
5. Preserve simultaneous-key behavior as out-of-scope except for safety/memory integrity already defined elsewhere.

**Expected result**
- EVK behavior matches the required idle/pressed polarity.
- Each held button produces one accepted click and no retrigger until release.
- No hardware evidence is inferred from simulator-only input injection.

**Required evidence**
- Per-button EVK observation/log.
- Event trace for press/hold/release/re-press.
- Optional scope/logic evidence where useful.

**Acceptance**
- `PASS` only when software acceptance plus EVK evidence are both present.
- `HW-PENDING` while physical button evidence is unavailable.
- `FAIL` if physical polarity or hold behavior violates the Driver contract.

---

## AITP-14-07 — Timer EVK handoff measures physical 1 ms interval/accuracy/drift

**Trace:** IMP-TMR-HW-01; T-TMR-02
**Type:** EVK hardware timing validation
**Owner:** Trung

**Preconditions**
- EVK timer output/derived observable can be measured against a suitable reference.
- Software timer configuration/clock-chain assumptions have already been reviewed.

**Procedure**
1. Run the production timer configuration on EVK.
2. Measure a suitable repeated timing observable derived from the intended 1 ms system tick.
3. Collect enough samples/duration to detect systematic interval error and meaningful drift.
4. Compare the measured behavior with the documented configured timer/CPU clock assumptions.
5. Record equipment/reference and measurement method.

**Expected result**
- Physical timer interval/drift is consistent with the verified production configuration and acceptable for the frozen observable timing behavior.
- Documentation and actual configured timing source agree.
- Software-only arithmetic does not substitute for this physical measurement.

**Required evidence**
- EVK measurement data.
- Reference/equipment/method.
- Configuration-to-measurement comparison.

**Acceptance**
- `PASS` only after acceptable EVK timing evidence exists.
- `HW-PENDING` before measurement.
- `FAIL` if measured timing contradicts the configured assumptions or breaks required observable behavior.

---

## AITP-14-08 — EEPROM/I2C EVK handoff verifies physical bus behavior, write-cycle use, retention, and reboot restore

**Trace:** IMP-I2C-HW-01; persistence hardware handoff; TCOV-10 dependency
**Type:** EVK hardware persistence validation
**Owner:** Trung + Tuấn at reboot integration boundary

**Preconditions**
- EVK EEPROM/I2C hardware is available.
- Corrected Contract v2.7 board-device identity/pin mapping is used.
- TCOV-10 software transaction/read/write semantics have passed.

**Procedure**
1. Verify physical SDA/SCL idle behavior and required pull-up/open-drain-style operation on EVK.
2. Save representative alarm values through the frozen public EEPROM boundary.
3. Confirm Driver handles the physical write-cycle/ACK behavior without publishing false success.
4. Read back the values on the running board.
5. Remove/reset power as appropriate for retention testing.
6. Reboot through the production startup path and verify the persisted valid alarm is restored.
7. Include the startup-equality non-trigger rule where a restored value equals the power-on Clock.

**Expected result**
- Physical bus operation is electrically safe and functional.
- Successful save/read corresponds to actual EEPROM behavior.
- Stored data survives the required retention/power-cycle test.
- Production reboot restores the valid persisted alarm through the frozen startup semantics.
- No mixed/corrupt record is accepted as valid.

**Required evidence**
- EVK bus/ACK/write-cycle observations.
- Saved/read values.
- Power-cycle/reboot restore trace.

**Acceptance**
- `PASS` only when software persistence tests and EVK retention/reboot evidence both pass.
- `HW-PENDING` until physical bus/retention evidence exists.
- `FAIL` for physical bus failure, false persistence success, lost required retention, or invalid restore behavior.

---

## AITP-14-09 — Display/LED/Buzzer EVK handoff verifies physical outputs without replacing software evidence

**Trace:** T-DSP-01; T-DSP-BLINK-01; T-BUZ-02; physical output handoff
**Type:** EVK output validation
**Owner:** Trung

**Preconditions**
- EVK and production build are available.
- Corresponding software tests have already produced SW evidence.

**Procedure**
1. Display:
   - verify correct visible HH.MM digits,
   - verify requested HH/MM blink behavior,
   - inspect multiplex stability/visibility.
2. LED D4:
   - verify OFF outside alarm setting,
   - verify visible alarm-setting blink and correct physical polarity.
3. Buzzer:
   - verify short feedback is physically produced,
   - verify the 5-second alarm sequence and approximately 500 ms ON/OFF phase behavior on real hardware.
4. Record physical evidence separately from simulator logs.

**Expected result**
- EVK outputs correspond to the already-proven logical software requests.
- Display multiplex is physically usable/stable.
- LED behavior/polarity matches the logical policy.
- Buzzer produces the required physical short-feedback/alarm behavior.
- Hardware observations do not excuse a failing software test, and software PASS does not replace physical evidence.

**Required evidence**
- EVK photos/video/logs and/or scope/logic measurements appropriate to each output.
- Mapping from physical evidence to the relevant test ID.

**Acceptance**
- `PASS` for each subsystem only when applicable software and EVK evidence both pass.
- `HW-PENDING` for any subsystem lacking physical evidence.
- `FAIL` for a physical output that contradicts the frozen logical behavior.

---

## AITP-14-10 — HW-PENDING ledger is honest, explicit, and limited to genuinely physical checks

**Trace:** Contract status semantics; PR gate G3/G5; Plan TCOV-14
**Type:** Acceptance-status audit
**Owner:** Both

**Preconditions**
- All TCOV software results and available EVK evidence are collected.

**Procedure**
1. Build a final ledger for Button, Display, LED, Buzzer, EEPROM/I2C, and Timer.
2. For every remaining `HW-PENDING` item, record:
   - exact requirement/test ID,
   - why software evidence cannot prove it,
   - required EVK evidence,
   - owner.
3. Check that no software/architecture defect is mislabeled `HW-PENDING`.
4. Check that no physical claim is marked `PASS` without physical evidence.

**Expected result**
- `HW-PENDING` contains only genuinely physical validation such as pin/electrical behavior, pull-ups/bus behavior, measured timing, physical multiplex/output quality, and real retention.
- All software/architecture quality gates are either `SW-PASS/PASS` or `FAIL`; they are not hidden behind hardware status.
- Physical PASS claims have traceable EVK evidence.

**Required evidence**
- Final HW-PENDING/PASS ledger with test IDs, owner, rationale, and evidence links.

**Acceptance**
- `SW-PASS` when status classification is evidence-based and honest.
- `FAIL` if a software blocker is hidden as HW-PENDING or a hardware PASS is unsupported.

---

## AITP-14-11 — Final release gate closes traceability with no unresolved blocker

**Trace:** T-REG-01; PR/release quality gates; File-1 R1..R10 review intent
**Type:** Final integration acceptance audit
**Owner:** Both

**Preconditions**
- AITP-14-01 through AITP-14-10 results are available.
- Final build/baseline identity is fixed.

**Procedure**
1. Confirm the final reviewed baseline/commit/build identity.
2. Verify traceability from Contract/Plan obligations to TCOV/AITP evidence.
3. Confirm:
   - no unresolved `CONFIRMED DEFECT`,
   - no failed mandatory software/architecture gate,
   - hardware-only pending items are explicitly listed,
   - final build/project/config/API evidence remains consistent,
   - Contract-deviation handling/change control matches File-1 R10 and the frozen change-control process; no unreviewed breaking boundary/semantic deviation is silently accepted.
4. Record final status without upgrading `HW-PENDING` merely to make the document look complete.

**Expected result**
- The Test Plan can state precisely what is software-complete and what still requires EVK evidence.
- No known blocker is omitted from the release record.
- Final `PASS` is issued only when every applicable required hardware item has also passed.
- If hardware evidence is not yet complete, the honest overall status remains software-complete with explicit `HW-PENDING`, not fabricated final PASS.

**Required evidence**
- Final traceability/status matrix.
- Baseline commit/build identity.
- Contract-deviation/change-control review result, including any approved deviation reference or explicit no-deviation result.
- Open-item ledger, if any.

**Acceptance**
- `SW-PASS / HW-PENDING` when all software obligations pass but genuine EVK checks remain.
- `PASS` only when all applicable software and hardware obligations pass.
- `FAIL` when any unresolved mandatory defect/gate remains.

---

## TCOV-14 final exit gate

TCOV-14 may close only when:

```text
[ ] AITP-14-01 all applicable software regression SW-PASS
[ ] AITP-14-02 base 5 s / 500 ms Buzzer waveform software proof SW-PASS
[ ] AITP-14-03 500 ms phase-stability jitter regression SW-PASS
[ ] AITP-14-04 Display simulator/blink/concurrency regression SW-PASS
[ ] AITP-14-05 mandatory Driver/Platform software quality-gate closure SW-PASS
[ ] AITP-14-06 Button EVK evidence PASS or explicit HW-PENDING
[ ] AITP-14-07 Timer EVK evidence PASS or explicit HW-PENDING
[ ] AITP-14-08 EEPROM/I2C EVK evidence PASS or explicit HW-PENDING
[ ] AITP-14-09 Display/LED/Buzzer EVK evidence PASS or explicit HW-PENDING
[ ] AITP-14-10 HW-PENDING ledger audit SW-PASS
[ ] AITP-14-11 final release-gate audit PASS / honest SW-PASS + HW-PENDING
```

Final status semantics:

```text
SW-PASS
    software/simulator/architecture requirement is verified

HW-PENDING
    genuinely physical EVK evidence is still required

PASS
    all applicable software + hardware acceptance evidence is complete

FAIL
    at least one mandatory requirement/gate is not met
```

TCOV-14 closes File 2 only by evidence. It must never convert missing EVK evidence into PASS, and it must never classify a reproducible software/architecture defect as HW-PENDING.

---
