#include "buzzer.h"
#include "gpio.h"
#include "timer.h"

/* =========================================================================
 * Buzzer Driver Implementation (SONiX SN8F5708 EVK)
 * Acoustic Priority Rules & Non-blocking State Machine (Contract v2.7)
 * ========================================================================= */

typedef enum
{
    BUZZER_STATE_IDLE = 0,
    BUZZER_STATE_BEEP_SHORT,
    BUZZER_STATE_ALARM
} Buzzer_State_t;

static Buzzer_State_t xdata s_buzzer_state = BUZZER_STATE_IDLE;
static unsigned long xdata s_state_start_tick = 0;
static unsigned char xdata s_alarm_pin_phase = 0; /* 1 = ON, 0 = OFF */

void Buzzer_Init(void)
{
    s_buzzer_state = BUZZER_STATE_IDLE;
    s_state_start_tick = 0;
    s_alarm_pin_phase = 0;
    GPIO_SetBuzzer(PIN_STATE_LOW);
}

void Buzzer_BeepShort(void)
{
    /* Rule: Any short-beep request must not cancel/restart/shorten an active alarm sequence */
    if (s_buzzer_state == BUZZER_STATE_ALARM)
    {
        return; /* Absorbed/suppressed during active alarm */
    }

    /* DEC-14: Refresh 300ms timer from newest request */
    s_buzzer_state = BUZZER_STATE_BEEP_SHORT;
    s_state_start_tick = Timer_GetTickMs();
    GPIO_SetBuzzer(PIN_STATE_HIGH);
}

void Buzzer_StartAlarm(void)
{
    /* DEC-15: Repeated alarm-start request while ALARM is already active is ignored */
    if (s_buzzer_state == BUZZER_STATE_ALARM)
    {
        return;
    }

    /* Alarm immediately preempts any active short beep */
    s_buzzer_state = BUZZER_STATE_ALARM;
    s_state_start_tick = Timer_GetTickMs();
    s_alarm_pin_phase = 1;
    GPIO_SetBuzzer(PIN_STATE_HIGH);
}

void Buzzer_Stop(void)
{
    s_buzzer_state = BUZZER_STATE_IDLE;
    s_alarm_pin_phase = 0;
    GPIO_SetBuzzer(PIN_STATE_LOW);
}

unsigned char Buzzer_IsBusy(void)
{
    return (s_buzzer_state != BUZZER_STATE_IDLE) ? 1 : 0;
}

void Buzzer_Process(void)
{
    unsigned long current_tick = Timer_GetTickMs();
    unsigned long elapsed;

    switch (s_buzzer_state)
    {
        case BUZZER_STATE_IDLE:
            break;

        case BUZZER_STATE_BEEP_SHORT:
            /* 0.3s (300ms) short key/timeout beep */
            if ((current_tick - s_state_start_tick) >= BUZZER_KEYPRESS_BEEP_MS)
            {
                Buzzer_Stop();
            }
            break;

        case BUZZER_STATE_ALARM:
            elapsed = current_tick - s_state_start_tick;

            /* 1. Total 5-second duration check */
            if (elapsed >= BUZZER_ALARM_TOTAL_DURATION_MS)
            {
                Buzzer_Stop();
                break;
            }

            /* 2. Drift-free 500ms ON / 500ms OFF phase calculation (TIM-12) */
            {
                unsigned char expected_phase = ((elapsed / BUZZER_ALARM_CYCLE_ON_MS) % 2 == 0) ? 1 : 0;
                if (expected_phase != s_alarm_pin_phase)
                {
                    s_alarm_pin_phase = expected_phase;
                    GPIO_SetBuzzer(s_alarm_pin_phase ? PIN_STATE_HIGH : PIN_STATE_LOW);
                }
            }
            break;

        default:
            Buzzer_Stop();
            break;
    }
}
