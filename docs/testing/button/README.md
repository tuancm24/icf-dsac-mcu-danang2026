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

Test source:
- `firmware/drivers/button_test.c`

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
| **TEST 1** | Button Initialization | `Button_Init()` | Internal states = 0, event queue empty | PASS (Simulator) |
| **TEST 2** | SW3 Click Detection | SW3 pin held LOW for >= 30ms | `Button_GetEvent()` returns `BUTTON_EVENT_SW3_CLICK` | PASS (Simulator) |
| **TEST 3** | SW6 Click Detection | SW6 pin held LOW for >= 30ms | `Button_GetEvent()` returns `BUTTON_EVENT_SW6_CLICK` | PASS (Simulator) |
| **TEST 4** | SW10 Click Detection | SW10 pin held LOW for >= 30ms | `Button_GetEvent()` returns `BUTTON_EVENT_SW10_CLICK` | PASS (Simulator) |
| **TEST 5** | SW16 Click Detection | SW16 pin held LOW for >= 30ms | `Button_GetEvent()` returns `BUTTON_EVENT_SW16_CLICK` | PASS (Simulator) |
| **TEST 6** | Glitch / Bounce Rejection | Noise pulse < 20ms | No click event generated (`BUTTON_EVENT_NONE`) | PASS (Simulator) |
| **TEST 7** | Hold Re-trigger Protection | Pin held LOW indefinitely | Only 1 click event emitted until release | PASS (Simulator) |

---

## 4. Test Evidence

### Software Simulation Verification
- Test source: `firmware/drivers/button_test.c`
- Logic verified via Keil C51 unit execution.

### Hardware Verification Status
- **Status:** **Pending hardware validation**
- **Note:** Standalone hardware validation on physical matrix buttons will be conducted during system integration.

---

## 5. Conclusion

- **Software Simulator Verification:** PASS (7/7 tests logic verified)
- **Hardware Verification:** Pending hardware validation
