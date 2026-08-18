#include "led.h"
#include "gpio.h"
#include "timer.h"

/* =========================================================================
 * Status LED Driver Implementation (LED D4 on SONiX SN8F5708 EVK)
 * Idempotent Mode Control & Drift-Free Phase Timing (Contract v2.7)
 * ========================================================================= */

static LED_Mode_t s_led_mode = LED_MODE_OFF;
static unsigned long s_mode_start_tick = 0;
static unsigned char s_blink_state = 0; /* 1 = ON, 0 = OFF */

void LED_Init(void)
{
    s_led_mode = LED_MODE_OFF;
    s_mode_start_tick = 0;
    s_blink_state = 0;
    GPIO_SetLED_D4(PIN_STATE_LOW);
}

void LED_SetMode(LED_Mode_t mode)
{
    /* IMP-LED-01: Idempotence - identical requests must not restart blink timing */
    if (s_led_mode == mode)
    {
        return;
    }

    s_led_mode = mode;
    s_mode_start_tick = Timer_GetTickMs();

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
    unsigned long elapsed;

    if (s_led_mode == LED_MODE_BLINK_ALARM_SETTING)
    {
        elapsed = current_tick - s_mode_start_tick;

        /* Drift-free 500ms ON / 500ms OFF phase calculation (TIM-12) */
        {
            unsigned char expected_state = ((elapsed / BLINK_HALF_PERIOD_MS) % 2 == 0) ? 1 : 0;
            if (expected_state != s_blink_state)
            {
                s_blink_state = expected_state;
                GPIO_SetLED_D4(s_blink_state ? PIN_STATE_HIGH : PIN_STATE_LOW);
            }
        }
    }
}
