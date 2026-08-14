# 4-Digit 7-Segment Display Driver Specification

## 1. Purpose

This document defines the requirements, architecture, behavior, and public interface of the 4-Digit 7-Segment Display Driver for the SONiX SN8F5708 EVK digital clock application.

The driver provides time formatting, 7-segment segment decoding, time-multiplexed digit scanning, colon / decimal point control, and independent blinking modes for hour and minute digits.

---

## 2. Scope

This specification covers:
- Hardware target: 4-digit multiplexed 7-segment display on SONiX SN8F5708 EVK.
- Source files:
  - `firmware/drivers/display.c`
  - `firmware/drivers/display.h`
- Verification files:
  - `docs/testing/display/README.md`

---

## 3. Responsibilities

### Display Driver Owns:
- BCD digit segment decoding (0-9).
- Multiplexing 4 digits in periodic Timer ISR routines.
- Ghosting prevention via inter-digit blanking.
- Handling `HH` and `MM` blinking phases (500ms ON / 500ms OFF).
- Decimal point / colon control.

### Display Driver Does NOT Own:
- Clock time accumulation (owned by Clock Core).
- Deciding which setup mode is active (owned by Application FSM).
- Reading push buttons or generating sound.

---

## 4. Architecture

```text
+-----------------------------------------------------------+
|                     Application Layer                     |
|         (Display_SetTime, Display_SetBlinkMode)           |
+-----------------------------------------------------------+
                              |
                              v
+-----------------------------------------------------------+
|                   Display Driver Layer                    |
|  - Font Lookup Table (0-9 -> 7-Segments a..g, dp)         |
|  - Blinking Phase Manager (500ms ON / 500ms OFF)          |
|  - Time-Multiplexing Scanner (Digit 0 -> 1 -> 2 -> 3)     |
+-----------------------------------------------------------+
                              |
                              v
+-----------------------------------------------------------+
|                      Platform Layer                       |
|   (GPIO_SetDisplaySegments, GPIO_SelectDisplayDigit)      |
+-----------------------------------------------------------+
                              |
                              v
+-----------------------------------------------------------+
|               4-Digit 7-Segment Display Hardware          |
|                 (DIG1, DIG2, DIG3, DIG4, Segs)            |
+-----------------------------------------------------------+
```

---

## 5. Public Interface

```c
#ifndef DISPLAY_H
#define DISPLAY_H

#include "app_config.h"

typedef enum
{
    DISPLAY_BLINK_NONE = 0,
    DISPLAY_BLINK_HOURS,
    DISPLAY_BLINK_MINUTES,
    DISPLAY_BLINK_ALL
} Display_BlinkMode_t;

void Display_Init(void);
void Display_SetTime(unsigned char hour, unsigned char minute);
void Display_SetColon(unsigned char enable);
void Display_SetBlinkMode(Display_BlinkMode_t mode);
void Display_UpdateBlinkState(void);
void Display_ScanRoutine(void);
void Display_Clear(void);

#endif /* DISPLAY_H */
```

---

## 6. Functional Behavior

1. **Multiplexing Routine (`Display_ScanRoutine`):**
   - Invoked every 1ms - 2ms by hardware timer ISR.
   - Blanks all digits $\rightarrow$ Updates segment bus $\rightarrow$ Enables active digit $\rightarrow$ Advances digit index (`0 -> 1 -> 2 -> 3 -> 0`).
2. **Blinking Behavior:**
   - `DISPLAY_BLINK_HOURS`: Digit 0 & 1 blanked during OFF phase (500ms), Digit 2 & 3 visible.
   - `DISPLAY_BLINK_MINUTES`: Digit 2 & 3 blanked during OFF phase (500ms), Digit 0 & 1 visible.
   - `DISPLAY_BLINK_NONE`: All digits steadily visible.

---

## 7. Initialization

Immediately after `Display_Init()`:
- Display buffer is cleared to `00:00`.
- All digits are turned OFF (`0xFF`).
- Blink mode is `DISPLAY_BLINK_NONE`.

---

## 8. Input / Output Behavior

| Display Mode | Digits 0, 1 (HH) | Digits 2, 3 (MM) | Colon / DP |
|---|---|---|---|
| `DISPLAY_BLINK_NONE` | Steady ON | Steady ON | Steady ON |
| `DISPLAY_BLINK_HOURS` | 500ms ON / 500ms OFF | Steady ON | Steady ON |
| `DISPLAY_BLINK_MINUTES` | Steady ON | 500ms ON / 500ms OFF | Steady ON |

---

## 9. Boundary / Invalid Conditions

- `hour > 23` is clamped to 23.
- `minute > 59` is clamped to 59.
- Invalid digit index wrapped safely via modulo 4.

---

## 10. Software Verification

Verified via Keil C51 Simulator:
- `TEST 1`: Initialization and blank display.
- `TEST 2`: Normal time decoding (`12:34`).
- `TEST 3`: Boundary values (`00:00` and `23:59`).
- `TEST 4`: Hours blinking mode (suppression during OFF phase).
- `TEST 5`: Minutes blinking mode (suppression during OFF phase).

---

## 11. Hardware Verification Status

> Hardware verification has not yet been performed because the SN8F5708 EVK is currently unavailable for individual testing. Current verification is limited to software/simulator-level behavior.

---

## 12. Integration Notes

- `Display_ScanRoutine()` must be called in a high-priority Timer ISR (1ms or 2ms) to ensure flicker-free rendering.
- `Display_UpdateBlinkState()` should be called periodically in the 1ms/10ms system tick.

---

## 13. Known Limitations

- Common Anode configuration is standard; Common Cathode can be selected via compile macro `DISPLAY_COMMON_ANODE`.
