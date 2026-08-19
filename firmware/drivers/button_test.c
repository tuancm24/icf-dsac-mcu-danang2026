#include "button.h"
#include "buzzer.h"
#include "timer.h"
#include "gpio.h"
#include <SN8F5708.H>

/* =========================================================================
 * Button Matrix Complete Driver Verification & Live Board Test Runner
 * Target MCU: SONiX SN8F5708 EVK
 * ========================================================================= */

sfr SCON_REG = 0x98; /* SFR SCON */
sfr SBUF_REG = 0x99; /* SFR SBUF */
sbit TI_BIT  = 0x98^1; /* Transmit Interrupt Flag (SCON.1) */

/* Unit Test Assertion Results (Observable in Watch Window) */
volatile unsigned char test1_init_pass = 0;
volatile unsigned char test2_sw3_click_pass = 0;
volatile unsigned char test3_sw6_click_pass = 0;
volatile unsigned char test4_sw10_click_pass = 0;
volatile unsigned char test5_sw16_click_pass = 0;
volatile unsigned char test6_glitch_rejected = 0;
volatile unsigned char test7_hold_no_retrigger = 0;

static unsigned char prev_sw3  = 0;
static unsigned char prev_sw6  = 0;
static unsigned char prev_sw10 = 0;
static unsigned char prev_sw16 = 0;

static unsigned char count_sw3  = 0;
static unsigned char count_sw6  = 0;
static unsigned char count_sw10 = 0;
static unsigned char count_sw16 = 0;

static void UART_SendChar(char c)
{
    while (!TI_BIT);
    TI_BIT = 0;
    SBUF_REG = c;
}

static void UART_SendString(const char code *str)
{
    while (*str)
    {
        UART_SendChar(*str++);
    }
}

static void UART_SendNum(unsigned char n)
{
    unsigned char tens = 0;
    while (n >= 10)
    {
        tens++;
        n -= 10;
    }
    if (tens > 0)
    {
        UART_SendChar((char)('0' + tens));
    }
    UART_SendChar((char)('0' + n));
}

static void UART_Console_Init(void)
{
    SCON_REG = 0x52; /* Mode 1: 8-bit UART, REN=1, TI=1 */
    TMOD &= 0x0F;
    TMOD |= 0x20;    /* Timer 1 Mode 2: 8-bit auto-reload */
    TH1  = 0xFD;    /* 9600 baud @ 12MHz */
    TR1  = 1;       /* Start Timer 1 */
    TI_BIT = 1;     /* Ready */
}

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
    unsigned char curr_sw3, curr_sw6, curr_sw10, curr_sw16;
    Button_Event_t event;
    Button_Event_t ev1, ev2, ev3, ev4;

    /* 1. Initialize Hardware Platform */
    WDTR = 0x5A;
    UART_Console_Init();
    GPIO_Init();
    Timer_Init();
    Buzzer_Init();
    Button_Init();

    /* =====================================================================
     * SECTION A: Software Simulation Unit Verification (T-BTN-01..07)
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
     * SECTION B: Interactive Live Board Test Execution Loop
     * ===================================================================== */
    Button_Init();
    UART_SendString("\r\n=== BUTTON LIVE MONITOR READY ===\r\n");

    while (1)
    {
        WDTR = 0x5A;

        /* 1. Read real-time pin levels */
        curr_sw3  = (GPIO_ReadButton_SW3()  == PIN_STATE_LOW) ? 1 : 0;
        curr_sw6  = (GPIO_ReadButton_SW6()  == PIN_STATE_LOW) ? 1 : 0;
        curr_sw10 = (GPIO_ReadButton_SW10() == PIN_STATE_LOW) ? 1 : 0;
        curr_sw16 = (GPIO_ReadButton_SW16() == PIN_STATE_LOW) ? 1 : 0;

        if (curr_sw3 != prev_sw3)
        {
            prev_sw3 = curr_sw3;
            UART_SendString(curr_sw3 ? "[PIN] SW3 DOWN (0V)\r\n" : "[PIN] SW3 UP (5V)\r\n");
        }

        if (curr_sw6 != prev_sw6)
        {
            prev_sw6 = curr_sw6;
            UART_SendString(curr_sw6 ? "[PIN] SW6 DOWN (0V)\r\n" : "[PIN] SW6 UP (5V)\r\n");
        }

        if (curr_sw10 != prev_sw10)
        {
            prev_sw10 = curr_sw10;
            UART_SendString(curr_sw10 ? "[PIN] SW10 DOWN (0V)\r\n" : "[PIN] SW10 UP (5V)\r\n");
        }

        if (curr_sw16 != prev_sw16)
        {
            prev_sw16 = curr_sw16;
            UART_SendString(curr_sw16 ? "[PIN] SW16 DOWN (0V)\r\n" : "[PIN] SW16 UP (5V)\r\n");
        }

        /* 2. Process scanning */
        Button_Process();
        Buzzer_Process();

        /* 3. Check click events */
        event = Button_GetEvent();
        if (event != BUTTON_EVENT_NONE)
        {
            if (event == BUTTON_EVENT_SW3_CLICK)
            {
                UART_SendString("==> [CLICK] SW3 (Count: ");
                UART_SendNum(++count_sw3);
                UART_SendString(")\r\n");
            }
            else if (event == BUTTON_EVENT_SW6_CLICK)
            {
                UART_SendString("==> [CLICK] SW6 (Count: ");
                UART_SendNum(++count_sw6);
                UART_SendString(")\r\n");
            }
            else if (event == BUTTON_EVENT_SW10_CLICK)
            {
                UART_SendString("==> [CLICK] SW10 (Count: ");
                UART_SendNum(++count_sw10);
                UART_SendString(")\r\n");
            }
            else if (event == BUTTON_EVENT_SW16_CLICK)
            {
                UART_SendString("==> [CLICK] SW16 (Count: ");
                UART_SendNum(++count_sw16);
                UART_SendString(")\r\n");
            }

            Buzzer_BeepShort();
            GPIO_SetLED_D4(PIN_STATE_HIGH);
        }

        if (!Buzzer_IsBusy())
        {
            GPIO_SetLED_D4(PIN_STATE_LOW);
        }
    }
}