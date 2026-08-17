#include "button.h"
#include "buzzer.h"
#include "timer.h"
#include "gpio.h"
#include <SN8F5708.H>

/* =========================================================================
 * Button Matrix Driver Verification & Production Board Test Runner
 * Target MCU: SONiX SN8F5708 EVK
 *
 * Behaviors:
 * - Debounced Single Click per press
 * - Anti-hold: Holding button produces only ONE single 0.3s beep
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

    /* SW3 */
    Button_Init();
    GPIO_SimulateButtonPress(BUTTON_ID_SW3, 1);
    for (i = 0; i < 4; i++) { advance_10ms(); Button_Process(); }
    GPIO_SimulateButtonPress(BUTTON_ID_SW3, 0);
    event = Button_GetEvent();
    test2_sw3_click_pass = (event == BUTTON_EVENT_SW3_CLICK) ? 1 : 0;

    /* SW6 */
    Button_Init();
    GPIO_SimulateButtonPress(BUTTON_ID_SW6, 1);
    for (i = 0; i < 4; i++) { advance_10ms(); Button_Process(); }
    GPIO_SimulateButtonPress(BUTTON_ID_SW6, 0);
    event = Button_GetEvent();
    test3_sw6_click_pass = (event == BUTTON_EVENT_SW6_CLICK) ? 1 : 0;

    /* SW10 */
    Button_Init();
    GPIO_SimulateButtonPress(BUTTON_ID_SW10, 1);
    for (i = 0; i < 4; i++) { advance_10ms(); Button_Process(); }
    GPIO_SimulateButtonPress(BUTTON_ID_SW10, 0);
    event = Button_GetEvent();
    test4_sw10_click_pass = (event == BUTTON_EVENT_SW10_CLICK) ? 1 : 0;

    /* SW16 */
    Button_Init();
    GPIO_SimulateButtonPress(BUTTON_ID_SW16, 1);
    for (i = 0; i < 4; i++) { advance_10ms(); Button_Process(); }
    GPIO_SimulateButtonPress(BUTTON_ID_SW16, 0);
    event = Button_GetEvent();
    test5_sw16_click_pass = (event == BUTTON_EVENT_SW16_CLICK) ? 1 : 0;

    /* Glitch Rejection */
    Button_Init();
    GPIO_SimulateButtonPress(BUTTON_ID_SW3, 1);
    advance_10ms();
    Button_Process();
    GPIO_SimulateButtonPress(BUTTON_ID_SW3, 0);
    advance_10ms();
    Button_Process();
    test6_glitch_rejected = (Button_GetEvent() == BUTTON_EVENT_NONE) ? 1 : 0;

    /* Hold Re-trigger Protection */
    Button_Init();
    GPIO_SimulateButtonPress(BUTTON_ID_SW3, 1);
    for (i = 0; i < 10; i++) { advance_10ms(); Button_Process(); }
    event = Button_GetEvent();
    event = Button_GetEvent();
    test7_hold_no_retrigger = (event == BUTTON_EVENT_NONE) ? 1 : 0;

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

        /* 2. Check for button click event */
        event = Button_GetEvent();
        if (event != BUTTON_EVENT_NONE)
        {
            /* Produce crisp 0.3s key-press beep & blink LED D4 */
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