# Platform Timer & System Tick Specification

## 1. Purpose

This document defines the requirements, architecture, behavior, and public interface of the Platform Timer and System Tick module for the SONiX SN8F5708 EVK digital clock application.

The module provides approximately 1 ms periodic interrupt generation, global tick counting, non-blocking time measuring utilities, and microsecond delay functions.

---

## 2. Scope

This specification covers:
- Hardware target: Hardware Timer 0 on SONiX SN8F5708 using 32 MHz IHRC with Timer0 clocked from Fcpu/12.
- Source files:
  - `firmware/platform/timer.c`
  - `firmware/platform/timer.h`
- Verification files:
  - `firmware/platform/timer_test.c`
  - `docs/testing/timer/README.md`
  - `docs/testing/timer/timer_test_watch.png`

---

## 3. Responsibilities

### Module Owns:
- Hardware Timer 0 register initialization for 1ms periodic interrupt.
- Global millisecond system tick counter (`s_system_tick_ms`).
- Elapsed time calculation (`Timer_HasElapsed`).
- Microsecond timing delay (`Timer_DelayUs`) for I2C communication.

### Module Does NOT Own:
- 1-second clock increment logic (owned by Clock Core).
- Button debouncing logic or application-level display content/state policy.

---

## 4. Architecture

```text
+-----------------------------------------------------------+
|                     Application Layer                     |
|           (Reads Timer_GetTickMs for timeouts)            |
+-----------------------------------------------------------+
                              ^
                              |
+-----------------------------------------------------------+
|                    Drivers / Middleware                   |
|        (Button, Buzzer, LED, Display, Clock Tick)         |
+-----------------------------------------------------------+
                              ^
                              | Timer_ISR_Handler()
+-----------------------------------------------------------+
|                    Platform Timer Layer                   |
|  - 1ms System Tick Accumulator (s_system_tick_ms)         |
|  - Elapsed Time Evaluation (Timer_HasElapsed)             |
|  - Microsecond Delay Utility                              |
+-----------------------------------------------------------+
                              ^
                              | 1ms Hardware Interrupt (Vector 1)
+-----------------------------------------------------------+
|             SONiX SN8F5708 Timer 0 Peripheral             |
+-----------------------------------------------------------+
```

---

## 5. Public Interface

```c
#ifndef TIMER_H
#define TIMER_H

#include "board_config.h"

void Timer_Init(void);
unsigned long Timer_GetTickMs(void);
unsigned char Timer_HasElapsed(unsigned long start_tick, unsigned long duration_ms);
void Timer_ISR_Handler(void);
void Timer_DelayUs(unsigned int us);

#endif /* TIMER_H */
```

---

## 6. Functional Behavior

1. **1ms Periodic Interrupt:**
   - Timer 0 is configured with 16-bit auto-reload to fire every 1.000 ms.
   - `Timer0_ISR` executes `Timer_ISR_Handler()`, incrementing `s_system_tick_ms` by 1 on each interrupt.
2. **Elapsed Time Check:**
   - `Timer_HasElapsed(start_tick, duration_ms)` evaluates if `(current_tick - start_tick) >= duration_ms` safely handling unsigned 32-bit arithmetic.

---

## 7. Initialization

Immediately after `Timer_Init()`:
- `s_system_tick_ms = 0`.
- Timer 0 interrupt enabled (`IE |= 0x82`) and counter started (`TCON |= 0x10`).

---

## 8. Input / Output Behavior

| Function | Input | Expected Output |
|---|---|---|
| `Timer_GetTickMs()` | None | Current tick in milliseconds |
| `Timer_HasElapsed(0, 500)` at 1000ms | `start=0, duration=500` | Returns `1` (True) |
| `Timer_HasElapsed(0, 1500)` at 1000ms | `start=0, duration=1500` | Returns `0` (False) |

---

## 9. Boundary / Invalid Conditions

- **Tick Counter Rollover:** At 1ms/tick, a 32-bit `unsigned long` overflows after ~49.7 days. Elapsed subtraction `(current - start)` remains valid across rollover.

---

## 10. Software Verification

Verified via `firmware/platform/timer_test.c` on Keil C51 Simulator:
- `TEST 1`: `Timer_Init()` resets tick to 0.
- `TEST 2`: 1000 ISR executions accumulate to exactly 1000ms.
- `TEST 3A`: `Timer_HasElapsed(0, 500)` returns `1` at 1000ms.
- `TEST 3B`: `Timer_HasElapsed(0, 1500)` returns `0` at 1000ms.
- `TEST 4`: `Timer_DelayUs(10)` completes successfully.

Status: **PASS (100% - 4/4 tests)**

---

## 11. Hardware Verification Status

- **Status:** **PASS (100%)**
- **Target:** SONiX 5708_EVK-V1.0 Board
- **Method:** 1ms Hardware Timer 0 Interrupt driving 1Hz LED D4 (P0.3) blinking and periodic buzzer ticking.
- **Evidence:** Verified visually on physical EVK board.

---

## 12. Integration Notes

- System Timer is the master heartbeat of the entire firmware. All non-blocking drivers rely on `Timer_GetTickMs()` or `Timer_ISR_Handler()`.

---

## 13. Known Limitations

- Microsecond delay `Timer_DelayUs()` is implemented via calibrated software loop for bit-banging I2C.
