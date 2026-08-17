#include "button.h"
#include "timer.h"
#include "gpio.h"
#include <SN8F5708.H>

/* =========================================================================
 * Button Driver Verification Test Runner (Full 7/7 Test Suite)
 * Target MCU: SONiX SN8F5708 EVK
 * Environment: Keil C51 Simulator / Hardware
 * ========================================================================= */

/* Global watch variables for verification inspection */
volatile unsigned char test1_init_pass = 0;
volatile unsigned char test2_sw3_click_pass = 0;
volatile unsigned char test3_sw6_click_pass = 0;
volatile unsigned char test4_sw10_click_pass = 0;
volatile unsigned char test5_sw16_click_pass = 0;
volatile unsigned char test6_glitch_rejected = 0;
volatile unsigned char test7_hold_no_retrigger = 0;

static void advance_10ms(void)
{
    unsigned char k;
    for (k = 0; k < 10; k++)
    {
        Timer_ISR_Handler();
    }
}

void main(void)
{
    unsigned char i;
    Button_Event_t event;

    WDTR = 0x5A;
    GPIO_Init();
    Timer_Init();

    /* =====================================================================
     * TEST 1: Button Initialization
     * ===================================================================== */
    Button_Init();
    test1_init_pass = (Button_GetEvent() == BUTTON_EVENT_NONE &&
                       Button_IsPressed(BUTTON_ID_SW3) == 0 &&
                       Button_IsPressed(BUTTON_ID_SW6) == 0 &&
                       Button_IsPressed(BUTTON_ID_SW10) == 0 &&
                       Button_IsPressed(BUTTON_ID_SW16) == 0) ? 1 : 0;

    /* =====================================================================
     * TEST 2: SW3 Click Detection (Debounce >= 30ms)
     * ===================================================================== */
    Button_Init();
    GPIO_SimulateButtonPress(BUTTON_ID_SW3, 1);
    for (i = 0; i < 4; i++)
    {
        advance_10ms();
        Button_Process();
    }
    GPIO_SimulateButtonPress(BUTTON_ID_SW3, 0);
    event = Button_GetEvent();
    test2_sw3_click_pass = (event == BUTTON_EVENT_SW3_CLICK) ? 1 : 0;

    /* =====================================================================
     * TEST 3: SW6 Click Detection
     * ===================================================================== */
    Button_Init();
    GPIO_SimulateButtonPress(BUTTON_ID_SW6, 1);
    for (i = 0; i < 4; i++)
    {
        advance_10ms();
        Button_Process();
    }
    GPIO_SimulateButtonPress(BUTTON_ID_SW6, 0);
    event = Button_GetEvent();
    test3_sw6_click_pass = (event == BUTTON_EVENT_SW6_CLICK) ? 1 : 0;

    /* =====================================================================
     * TEST 4: SW10 Click Detection
     * ===================================================================== */
    Button_Init();
    GPIO_SimulateButtonPress(BUTTON_ID_SW10, 1);
    for (i = 0; i < 4; i++)
    {
        advance_10ms();
        Button_Process();
    }
    GPIO_SimulateButtonPress(BUTTON_ID_SW10, 0);
    event = Button_GetEvent();
    test4_sw10_click_pass = (event == BUTTON_EVENT_SW10_CLICK) ? 1 : 0;

    /* =====================================================================
     * TEST 5: SW16 Click Detection
     * ===================================================================== */
    Button_Init();
    GPIO_SimulateButtonPress(BUTTON_ID_SW16, 1);
    for (i = 0; i < 4; i++)
    {
        advance_10ms();
        Button_Process();
    }
    GPIO_SimulateButtonPress(BUTTON_ID_SW16, 0);
    event = Button_GetEvent();
    test5_sw16_click_pass = (event == BUTTON_EVENT_SW16_CLICK) ? 1 : 0;

    /* =====================================================================
     * TEST 6: Glitch / Bounce Rejection (< 20ms pulse)
     * ===================================================================== */
    Button_Init();
    GPIO_SimulateButtonPress(BUTTON_ID_SW3, 1); /* 10ms pulse */
    advance_10ms();
    Button_Process();
    GPIO_SimulateButtonPress(BUTTON_ID_SW3, 0); /* Released before 30ms */
    advance_10ms();
    Button_Process();
    test6_glitch_rejected = (Button_GetEvent() == BUTTON_EVENT_NONE) ? 1 : 0;

    /* =====================================================================
     * TEST 7: Hold Re-trigger Protection (Continuous press -> only 1 event)
     * ===================================================================== */
    Button_Init();
    GPIO_SimulateButtonPress(BUTTON_ID_SW3, 1); /* Held for 100ms */
    for (i = 0; i < 10; i++)
    {
        advance_10ms();
        Button_Process();
    }
    event = Button_GetEvent(); /* First click consumed */
    event = Button_GetEvent(); /* Next check must be empty */
    test7_hold_no_retrigger = (event == BUTTON_EVENT_NONE) ? 1 : 0;

    /* End of Test Suite */
    while (1)
    {
        WDTR = 0x5A;
    }
}
