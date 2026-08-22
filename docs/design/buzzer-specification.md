# Buzzer and LED Driver Specification

## 1. Purpose

This document defines the requirements, architecture, behavior, and public interface of the Buzzer Driver and Status LED Driver for the SONiX SN8F5708 EVK digital clock application.

The modules provide acoustic feedback (short keypress beep and 5s alarm pattern) and visual feedback (LED D4 blinking in alarm configuration mode).

---

## 2. Scope

This specification covers:
- Hardware targets on SONiX 5708_EVK-V1.0:
  - **Buzzer (`PZ1`):** Pin `P1.0` (Active HIGH, Silkscreen: `P10`)
  - **Status LED D4 (`D4`):** Pin `P0.3` (Active LOW, Silkscreen: `P03`)
- Source files:
  - `firmware/drivers/buzzer.c` / `buzzer.h`
  - `firmware/drivers/led.c` / `led.h`
  - `firmware/platform/gpio.c` / `gpio.h`
- Verification files:
  - `firmware/drivers/buzzer_test.c`
  - `docs/testing/buzzer/README.md`

---

## 3. Responsibilities

### Modules Own:
- Non-blocking timing management for 0.3s (300ms) short beep on keypress / timeout.
- Non-blocking timing management for 5s alarm pattern (0.5s ON - 0.5s OFF).
- Immediate stop control for acoustic output via `Buzzer_Stop()`.
- LED D4 operating modes (OFF, ON, 0.5s ON / 0.5s OFF blinking).
- Hardware pin driving on `P1.0` (Buzzer) and `P0.3` (LED D4).

### Modules Do NOT Own:
- Deciding when an alarm should ring (owned by Application Integration & Alarm Core).
- Button debouncing or event generation.
- 30-second timeout decision logic.

---

## 4. Architecture

```text
+-----------------------------------------------------------+
|                     Application Layer                     |
|           (Requests beep on button click / alarm)         |
+-----------------------------------------------------------+
               |                               |
               | Buzzer_BeepShort()            | LED_SetMode()
               | Buzzer_StartAlarm()           |
               v                               v
+-----------------------------+ +---------------------------+
|        Buzzer Driver        | |         LED Driver        |
|  - 300ms Keypress Timer     | |  - 500ms ON / OFF Blink   |
|  - 5s Alarm Pattern FSM     | |  - Mode Management        |
+-----------------------------+ +---------------------------+
               |                               |
               +---------------+---------------+
                               |
                               v
+-----------------------------------------------------------+
|                      Platform Layer                       |
|               (GPIO_SetBuzzer, GPIO_SetLED_D4)            |
+-----------------------------------------------------------+
                               |
                               v
+-----------------------------------------------------------+
|               SONiX 5708_EVK-V1.0 Hardware                |
|           Buzzer (P1.0)  |  Status LED D4 (P0.3)          |
+-----------------------------------------------------------+
```

---

## 5. Public Interface

### Buzzer Interface (`buzzer.h`)
```c
void Buzzer_Init(void);
void Buzzer_BeepShort(void);
void Buzzer_StartAlarm(void);
void Buzzer_Stop(void);
void Buzzer_Process(void);
unsigned char Buzzer_IsBusy(void);
```

### LED Interface (`led.h`)
```c
typedef enum {
    LED_MODE_OFF = 0,
    LED_MODE_ON,
    LED_MODE_BLINK_ALARM_SETTING
} LED_Mode_t;

void LED_Init(void);
void LED_SetMode(LED_Mode_t mode);
void LED_Process(void);
```

---

## 6. Functional Behavior

1. **Short Beep (`Buzzer_BeepShort`):**
   - Drives `P1.0` HIGH for exactly 300ms (`BUZZER_KEYPRESS_BEEP_MS`), then automatically drives LOW and returns to `IDLE`.
2. **Alarm Sequence (`Buzzer_StartAlarm`):**
   - Executes a 5000ms (`BUZZER_ALARM_TOTAL_DURATION_MS`) sequence with alternating 500ms ON (`P1.0 = 1`) and 500ms OFF (`P1.0 = 0`) phases.
   - Automatically shuts off after 5 seconds.
3. **LED D4 Blinking:**
   - In `LED_MODE_BLINK_ALARM_SETTING`, toggles `P0.3` every 500ms (`BLINK_HALF_PERIOD_MS`).

---

## 7. Initialization

Immediately after `Buzzer_Init()` and `LED_Init()`:
- `P1.0` set to `0` (Buzzer OFF).
- `P0.3` set to `1` (LED D4 OFF, active LOW).
- Internal states reset to `IDLE`.

---

## 8. Input / Output Behavior

| Request API | Physical Pin State | Hardware Action |
|---|---|---|
| `Buzzer_BeepShort()` | `P1.0 = 1` for 300ms | Buzzer beeps for 0.3s |
| `Buzzer_StartAlarm()` | `P1.0` toggles 0.5s ON / 0.5s OFF for 5s | Continuous alarm pattern |
| `Buzzer_Stop()` | `P1.0 = 0` immediately | Buzzer silenced instantly |
| `LED_SetMode(BLINK)` | `P0.3` toggles 0.5s LOW / 0.5s HIGH | LED D4 blinks 1s cycle |
| `LED_SetMode(OFF)` | `P0.3 = 1` | LED D4 is OFF |

---

## 9. Boundary / Invalid Conditions

- Calling `Buzzer_BeepShort()` while another beep is active refreshes the 300ms timer safely.
- Calling `Buzzer_Stop()` when idle is safe (no-op).

---

## 10. Software Verification

Verified via `firmware/drivers/buzzer_test.c` on Keil C51 Simulator:
- `TEST 1`: Initialization and idle state (`test1_init_busy == 0x00`).
- `TEST 2`: 300ms short beep active (`0x01`) and auto-stop (`0x00`).
- `TEST 3`: 5-second alarm sequence active (`0x01`) and auto-stop (`0x00`).
- `TEST 4`: Immediate `Buzzer_Stop()` termination (`0x00`).

Status: **PASS (100%)**

---

## 11. Hardware Verification Status

- Hardware Target: SONiX 5708_EVK-V1.0 Board
- Pin Binding: Buzzer on `P1.0`, LED D4 on `P0.3`.
- Hardware Bring-Up Mode: Verified via continuous 1-second periodic beep & blink loop.

---

## 12. Integration Notes

- `Buzzer_Process()` and `LED_Process()` must be invoked periodically (every 1ms) from the system tick handler.
- Non-blocking architecture guarantees zero CPU stalls.

---

## 13. Known Limitations

- Active buzzer digital gating only (passive tone pitch modulation not used).
