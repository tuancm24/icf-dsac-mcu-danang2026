# Button Driver Specification

## 1. Purpose

This document defines the requirements, architecture, behavior, and public interface of the Button Driver module for the SONiX SN8F5708 EVK digital clock application.

The Button Driver is responsible for sampling raw push-button inputs, performing software debouncing, detecting falling-edge press events, and exposing clean button events to the Application layer.

---

## 2. Scope

This specification applies to the button input subsystem:
- Hardware targets: `SW3` (Setup), `SW6` (+), `SW10` (-), `SW16` (Alarm).
- Source files:
  - `firmware/drivers/button.c`
  - `firmware/drivers/button.h`
- Verification files:
  - `firmware/drivers/button_test.c`
  - `docs/testing/button/README.md`

---

## 3. Responsibilities

### Button Driver Owns:
- Sampling button GPIO pins periodically.
- Software debouncing (30ms noise rejection window).
- Detecting valid button click events (falling edges).
- Event buffering in a FIFO event queue.
- Exposing hardware-independent button events.

### Button Driver Does NOT Own:
- Application FSM state transitions.
- Deciding functional actions (e.g. whether `SW6` increments current hour or alarm hour).
- Generating the 0.3s button buzzer feedback (requested by higher-level integration).
- 30-second inactivity timeout management.

---

## 4. Architecture

```text
+-----------------------------------------------------------+
|                     Application Layer                     |
|            (FSM, Clock Setting, Alarm Setting)            |
+-----------------------------------------------------------+
                              ^
                              | Button_GetEvent()
+-----------------------------------------------------------+
|                    Button Driver Layer                    |
|  - Debounce Filtering (3 samples x 10ms = 30ms stable)    |
|  - Edge Detection (Active LOW Press Event)                |
|  - FIFO Event Queue                                       |
+-----------------------------------------------------------+
                              ^
                              | GPIO_ReadButton_*()
+-----------------------------------------------------------+
|                      Platform Layer                       |
|               (GPIO Pin Input Configuration)              |
+-----------------------------------------------------------+
                              ^
                              |
+-----------------------------------------------------------+
|               SONiX SN8F5708 Hardware Switches            |
|                   (SW3, SW6, SW10, SW16)                  |
+-----------------------------------------------------------+
```

---

## 5. Public Interface

```c
#ifndef BUTTON_H
#define BUTTON_H

#include "app_config.h"

typedef enum
{
    BUTTON_ID_SW3 = 0,
    BUTTON_ID_SW6,
    BUTTON_ID_SW10,
    BUTTON_ID_SW16,
    BUTTON_ID_COUNT
} Button_Id_t;

typedef enum
{
    BUTTON_EVENT_NONE = 0,
    BUTTON_EVENT_SW3_CLICK,
    BUTTON_EVENT_SW6_CLICK,
    BUTTON_EVENT_SW10_CLICK,
    BUTTON_EVENT_SW16_CLICK
} Button_Event_t;

void Button_Init(void);
void Button_Process(void);
Button_Event_t Button_GetEvent(void);
unsigned char Button_IsPressed(Button_Id_t button_id);

#endif /* BUTTON_H */
```

---

## 6. Functional Behavior

1. **Sampling & Debounce:**
   - `Button_Process()` is executed every 10ms by the system tick scheduler.
   - A button state change is validated only after remaining stable for 3 consecutive samples (30ms).
2. **Click Event Generation:**
   - When a transition from Released (`HIGH`) to Pressed (`LOW`) is confirmed, a single click event is enqueued.
   - Holding a button down does not emit repeated events.
3. **Queue Consumption:**
   - `Button_GetEvent()` retrieves and removes the oldest event from the queue. If empty, it returns `BUTTON_EVENT_NONE`.

---

## 7. Initialization

Immediately after `Button_Init()`:
- All button debounce counters reset to 0.
- All internal states set to Released.
- Event queue is cleared (`s_queue_count = 0`).

---

## 8. Input / Output Behavior

| Physical Button | Raw Pin State | Process Output |
|---|---|---|
| None | All pins `HIGH` | `BUTTON_EVENT_NONE` |
| `SW3` Pressed (>= 30ms) | Pin `LOW` | `BUTTON_EVENT_SW3_CLICK` |
| `SW6` Pressed (>= 30ms) | Pin `LOW` | `BUTTON_EVENT_SW6_CLICK` |
| `SW10` Pressed (>= 30ms) | Pin `LOW` | `BUTTON_EVENT_SW10_CLICK` |
| `SW16` Pressed (>= 30ms) | Pin `LOW` | `BUTTON_EVENT_SW16_CLICK` |

---

## 9. Boundary / Invalid Conditions

- **Glitches (< 20ms):** Filtered out by the debounce counter; no event is generated.
- **Multiple Simultaneous Presses:** Handled independently in order of scan (`SW3 -> SW6 -> SW10 -> SW16`).
- **Queue Overflow:** If the queue is full (4 unhandled events), new events are discarded safely without memory corruption.

---

## 10. Software Verification

Software verification is executed in Keil C51 Simulator using `firmware/drivers/button_test.c`:
- `TEST 1`: Initialization and empty queue verification.
- `TEST 2`: `SW3` single-click generation.
- `TEST 3`: `SW6` single-click generation.
- `TEST 4`: `SW10` single-click generation.
- `TEST 5`: `SW16` single-click generation.
- `TEST 6`: Glitch noise rejection (< 20ms).
- `TEST 7`: Continuous hold re-trigger protection.

---

## 11. Hardware Verification Status

- **Status:** **Pending hardware validation**
- **Note:** Standalone hardware validation on physical matrix buttons will be conducted during system integration.

---

## 12. Integration Notes

- `Button_Process()` must be called periodically inside the main loop or timer tick every 10ms.
- Application layer / FSM should poll `Button_GetEvent()` in the main execution cycle.

---

## 13. Known Limitations

- Long-press / double-click detection is intentionally omitted as the competition requirements only require single-click events.
