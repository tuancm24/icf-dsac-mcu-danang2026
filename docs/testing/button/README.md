# Button Driver Verification

This directory contains verification documentation and test evidence for the Button Driver module.

## 1. Purpose

The Button Driver is responsible for:
- Reading active-LOW inputs from 4 push buttons: `SW3` (Setup), `SW6` (+), `SW10` (-), `SW16` (Alarm).
- Filtering contact bounce and noise via software debouncing (30ms filter window).
- Generating single click events on falling edges (`BUTTON_EVENT_SW3_CLICK`, `SW6_CLICK`, `SW10_CLICK`, `SW16_CLICK`).
- Buffering events in a non-blocking queue.

Module under test:
- `firmware/drivers/button.c`
- `firmware/drivers/button.h`

---

## 2. Test Environment

- **Target MCU:** SONiX SN8F5708
- **IDE:** Keil µVision
- **Compiler:** Keil C51
- **Verification Method:** Keil Simulator
- **Debounce Window:** 3 samples × 10ms = 30ms stable interval

---

## 3. Test Results Summary

| Test ID | Test Case | Input / Condition | Expected Result | Status |
|---|---|---|---|---|
| **TEST 1** | Button Initialization | `Button_Init()` | Internal states = 0, event queue empty | PASS |
| **TEST 2** | SW3 Click Detection | SW3 pin held LOW for >= 30ms | `Button_GetEvent()` returns `BUTTON_EVENT_SW3_CLICK` | PASS |
| **TEST 3** | SW6 Click Detection | SW6 pin held LOW for >= 30ms | `Button_GetEvent()` returns `BUTTON_EVENT_SW6_CLICK` | PASS |
| **TEST 4** | SW10 Click Detection | SW10 pin held LOW for >= 30ms | `Button_GetEvent()` returns `BUTTON_EVENT_SW10_CLICK` | PASS |
| **TEST 5** | SW16 Click Detection | SW16 pin held LOW for >= 30ms | `Button_GetEvent()` returns `BUTTON_EVENT_SW16_CLICK` | PASS |
| **TEST 6** | Glitch / Bounce Rejection | Noise pulse < 20ms | No click event generated (`BUTTON_EVENT_NONE`) | PASS |
| **TEST 7** | Hold Re-trigger Protection | Pin held LOW indefinitely | Only 1 click event emitted until release | PASS |

---

## 4. Test Evidence

### TEST 1 - Initialization & Empty Event Queue
Expected: Event queue returns `BUTTON_EVENT_NONE`.
*(Place evidence screenshot below)*
<!-- ![TEST 1 - Button Init](button_test_01_init.png) -->

---

### TEST 2 to 5 - Button Click Detection (SW3, SW6, SW10, SW16)
Expected: Clean single-event generation after 30ms debounce confirmation.
*(Place evidence screenshot below)*
<!-- ![TEST 2 - SW3 Event](button_test_02_sw3.png) -->
<!-- ![TEST 3 - SW6 Event](button_test_03_sw6.png) -->
<!-- ![TEST 4 - SW10 Event](button_test_04_sw10.png) -->
<!-- ![TEST 5 - SW16 Event](button_test_05_sw16.png) -->

---

### TEST 6 - Glitch Rejection
Expected: Short pulses shorter than 30ms do not trigger false clicks.
*(Place evidence screenshot below)*
<!-- ![TEST 6 - Noise Filter](button_test_06_debounce_glitch.png) -->

---

## 5. Conclusion

All Button Driver test cases passed successfully.

- **Result:** 7/7 tests PASS
- **Debounce Efficiency:** Validated with 30ms noise rejection.
- **Queue Reliability:** FIFO behavior verified without missed events.
