#include "button.h"
#include "timer.h"
#include "gpio.h"
#include <SN8F5708.H>

/* =========================================================================
 * Button Driver Module Verification Test Runner
 * Target MCU: SONiX SN8F5708 EVK
 * Verification Environment: Keil C51 Simulator
 * ========================================================================= */

/* Global watch variables for Keil Watch Window inspection */
volatile Button_Event_t test1_init_event = BUTTON_EVENT_SW3_CLICK;
volatile unsigned char  test2_idle_no_event = 0xFF;
volatile unsigned char  test3_debounce_in_progress = 0xFF;

void main(void)
{
    /* 1. Initialize hardware */
    WDTR = 0x5A;
    GPIO_Init();
    Timer_Init();
    Button_Init();

    /* =====================================================================
     * TEST 1: Button Initialization
     * Expected: Event queue is empty -> BUTTON_EVENT_NONE (0)
     * ===================================================================== */
    test1_init_event = Button_GetEvent(); /* Expected: 0 */

    /* =====================================================================
     * TEST 2: Idle State (No key pressed)
     * Expected: Button_Process() keeps queue empty
     * ===================================================================== */
    Button_Process();
    test2_idle_no_event = (Button_GetEvent() == BUTTON_EVENT_NONE) ? 1 : 0; /* Expected: 1 */

    /* =====================================================================
     * TEST 3: Debounce State Machine Initial Check
     * Expected: Raw unconfirmed state returns not pressed (0)
     * ===================================================================== */
    test3_debounce_in_progress = Button_IsPressed(BUTTON_ID_SW3); /* Expected: 0 */

    /* End of Test - Infinite loop for debugger inspection */
    while (1)
    {
        WDTR = 0x5A;
        /* Place breakpoint here (F9) */
    }
}
