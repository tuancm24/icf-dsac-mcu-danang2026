#include "led.h"
#include "gpio.h"
#include "timer.h"

/* =========================================================================
 * Status LED Driver Implementation (LED D4 on SONiX SN8F5708 EVK)
 * ========================================================================= */

static LED_Mode_t s_led_mode = LED_MODE_OFF;
static unsigned long s_last_blink_tick = 0;
static unsigned char s_blink_state = 0; /* 1 = ON, 0 = OFF */

void LED_Init(void)
{
    s_led_mode = LED_MODE_OFF;
    s_last_blink_tick = 0;
    s_blink_state = 0;
    GPIO_SetLED_D4(PIN_STATE_LOW);
}

void LED_SetMode(LED_Mode_t mode)
{
    s_led_mode = mode;
    s_last_blink_tick = Timer_GetTickMs();

    if (s_led_mode == LED_MODE_ON)
    {
        s_blink_state = 1;
        GPIO_SetLED_D4(PIN_STATE_HIGH);
    }
    else if (s_led_mode == LED_MODE_OFF)
    {
        s_blink_state = 0;
        GPIO_SetLED_D4(PIN_STATE_LOW);
    }
    else if (s_led_mode == LED_MODE_BLINK_ALARM_SETTING)
    {
        s_blink_state = 1;
        GPIO_SetLED_D4(PIN_STATE_HIGH);
    }
}

void LED_Process(void)
{
    unsigned long current_tick = Timer_GetTickMs();

    if (s_led_mode == LED_MODE_BLINK_ALARM_SETTING)
    {
        if (Timer_HasElapsed(s_last_blink_tick, BLINK_HALF_PERIOD_MS))
        {
            s_last_blink_tick = current_tick;
            s_blink_state = !s_blink_state;
            GPIO_SetLED_D4(s_blink_state ? PIN_STATE_HIGH : PIN_STATE_LOW);
        }
    }
}
