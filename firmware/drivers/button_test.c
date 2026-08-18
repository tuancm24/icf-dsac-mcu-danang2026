#include "button.h"
#include "buzzer.h"
#include "timer.h"
#include "gpio.h"
#include <SN8F5708.H>

/* =========================================================================
 * Button Driver Verification & Production Board Test Runner
 * Target MCU: SONiX SN8F5708 EVK
 * ========================================================================= */

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
    Button_Event_t ev1, ev2, ev3, ev4;

    /* 1. Initialize Hardware Platform */
    WDTR = 0x5A;
    GPIO_Init();
    Timer_Init();
    Buzzer_Init();
    Button_Init();

    /* =====================================================================
     * SECTION A: Software Simulation Unit Verification
     * ===================================================================== */
    test1_init_pass = (Button_GetEvent() == BUTTON_EVENT_NONE &&
                       Button_IsPressed(BUTTON_ID_SW3) == 0 &&
                       Button_IsPressed(BUTTON_ID_SW6) == 0 &&
                       Button_IsPressed(BUTTON_ID_SW10) == 0 &&
                       Button_IsPressed(BUTTON_ID_SW16) == 0) ? 1 : 0;

    /* TEST 2: SW3 Click */
    Button_Init();
    GPIO_SimulateButtonPress(BUTTON_ID_SW3, 1);
    for (i = 0; i < 4; i++) { advance_10ms(); Button_Process(); }
    GPIO_SimulateButtonPress(BUTTON_ID_SW3, 0);
    for (i = 0; i < 4; i++) { advance_10ms(); Button_Process(); }
    event = Button_GetEvent();
    test2_sw3_click_pass = (event == BUTTON_EVENT_SW3_CLICK) ? 1 : 0;

    /* TEST 3: SW6 Click */
    Button_Init();
    GPIO_SimulateButtonPress(BUTTON_ID_SW6, 1);
    for (i = 0; i < 4; i++) { advance_10ms(); Button_Process(); }
    GPIO_SimulateButtonPress(BUTTON_ID_SW6, 0);
    for (i = 0; i < 4; i++) { advance_10ms(); Button_Process(); }
    event = Button_GetEvent();
    test3_sw6_click_pass = (event == BUTTON_EVENT_SW6_CLICK) ? 1 : 0;

    /* TEST 4: SW10 Click */
    Button_Init();
    GPIO_SimulateButtonPress(BUTTON_ID_SW10, 1);
    for (i = 0; i < 4; i++) { advance_10ms(); Button_Process(); }
    GPIO_SimulateButtonPress(BUTTON_ID_SW10, 0);
    for (i = 0; i < 4; i++) { advance_10ms(); Button_Process(); }
    event = Button_GetEvent();
    test4_sw10_click_pass = (event == BUTTON_EVENT_SW10_CLICK) ? 1 : 0;

    /* TEST 5: SW16 Click */
    Button_Init();
    GPIO_SimulateButtonPress(BUTTON_ID_SW16, 1);
    for (i = 0; i < 4; i++) { advance_10ms(); Button_Process(); }
    GPIO_SimulateButtonPress(BUTTON_ID_SW16, 0);
    for (i = 0; i < 4; i++) { advance_10ms(); Button_Process(); }
    event = Button_GetEvent();
    test5_sw16_click_pass = (event == BUTTON_EVENT_SW16_CLICK) ? 1 : 0;

    /* TEST 6: Glitch Rejection (< 30ms glitch) */
    Button_Init();
    GPIO_SimulateButtonPress(BUTTON_ID_SW3, 1);
    advance_10ms();
    Button_Process();
    GPIO_SimulateButtonPress(BUTTON_ID_SW3, 0);
    advance_10ms();
    Button_Process();
    test6_glitch_rejected = (Button_GetEvent() == BUTTON_EVENT_NONE) ? 1 : 0;

    /* TEST 7: Hold Proves First Click AND No Retrigger Until Release & Re-press */
    Button_Init();
    GPIO_SimulateButtonPress(BUTTON_ID_SW3, 1);
    for (i = 0; i < 4; i++) { advance_10ms(); Button_Process(); }
    ev1 = Button_GetEvent(); /* Must be SW3_CLICK */
    ev2 = Button_GetEvent(); /* Must be NONE */

    /* Continue holding for 100ms */
    for (i = 0; i < 10; i++) { advance_10ms(); Button_Process(); }
    ev3 = Button_GetEvent(); /* Must still be NONE */

    /* Release button and wait 40ms */
    GPIO_SimulateButtonPress(BUTTON_ID_SW3, 0);
    for (i = 0; i < 4; i++) { advance_10ms(); Button_Process(); }

    /* Re-press button */
    GPIO_SimulateButtonPress(BUTTON_ID_SW3, 1);
    for (i = 0; i < 4; i++) { advance_10ms(); Button_Process(); }
    ev4 = Button_GetEvent(); /* Must be new SW3_CLICK */

    test7_hold_no_retrigger = (ev1 == BUTTON_EVENT_SW3_CLICK &&
                               ev2 == BUTTON_EVENT_NONE &&
                               ev3 == BUTTON_EVENT_NONE &&
                               ev4 == BUTTON_EVENT_SW3_CLICK) ? 1 : 0;

    /* =====================================================================
     * SECTION B: Interactive Board Execution Loop
     * ===================================================================== */
    Button_Init();

    while (1)
    {
        WDTR = 0x5A;

        /* 1. Process non-blocking drivers in foreground loop */
        Button_Process();
        Buzzer_Process();

        /* 2. Check for button click event from FIFO queue */
        event = Button_GetEvent();
        if (event != BUTTON_EVENT_NONE)
        {
            Buzzer_BeepShort();
            GPIO_SetLED_D4(PIN_STATE_HIGH);
        }

        /* Turn off LED D4 when beep completes */
        if (!Buzzer_IsBusy())
        {
            GPIO_SetLED_D4(PIN_STATE_LOW);
        }
    }
}