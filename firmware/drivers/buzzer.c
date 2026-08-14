#include "buzzer.h"
#include "gpio.h"
#include "timer.h"

/* =========================================================================
 * Buzzer Driver Implementation (SONiX SN8F5708 EVK)
 * Non-blocking Timing State Machine
 * ========================================================================= */

typedef enum
{
    BUZZER_STATE_IDLE = 0,
    BUZZER_STATE_BEEP_SHORT,
    BUZZER_STATE_ALARM
} Buzzer_State_t;

static Buzzer_State_t s_buzzer_state = BUZZER_STATE_IDLE;
static unsigned long s_state_start_tick = 0;
static unsigned long s_cycle_start_tick = 0;
static unsigned char s_alarm_pin_phase = 0; /* 1 = ON phase, 0 = OFF phase */

void Buzzer_Init(void)
{
    s_buzzer_state = BUZZER_STATE_IDLE;
    s_state_start_tick = 0;
    s_cycle_start_tick = 0;
    s_alarm_pin_phase = 0;
    GPIO_SetBuzzer(PIN_STATE_LOW);
}

void Buzzer_BeepShort(void)
{
    s_buzzer_state = BUZZER_STATE_BEEP_SHORT;
    s_state_start_tick = Timer_GetTickMs();
    GPIO_SetBuzzer(PIN_STATE_HIGH);
}

void Buzzer_StartAlarm(void)
{
    s_buzzer_state = BUZZER_STATE_ALARM;
    s_state_start_tick = Timer_GetTickMs();
    s_cycle_start_tick = s_state_start_tick;
    s_alarm_pin_phase = 1;
    GPIO_SetBuzzer(PIN_STATE_HIGH);
}

void Buzzer_Stop(void)
{
    s_buzzer_state = BUZZER_STATE_IDLE;
    GPIO_SetBuzzer(PIN_STATE_LOW);
}

unsigned char Buzzer_IsBusy(void)
{
    return (s_buzzer_state != BUZZER_STATE_IDLE) ? 1 : 0;
}

void Buzzer_Process(void)
{
    unsigned long current_tick = Timer_GetTickMs();

    switch (s_buzzer_state)
    {
        case BUZZER_STATE_IDLE:
            break;

        case BUZZER_STATE_BEEP_SHORT:
            /* Check if 0.3s (300ms) has elapsed */
            if ((current_tick - s_state_start_tick) >= BUZZER_KEYPRESS_BEEP_MS)
            {
                Buzzer_Stop();
            }
            break;

        case BUZZER_STATE_ALARM:
            /* 1. Check if total 5-second duration has elapsed */
            if ((current_tick - s_state_start_tick) >= BUZZER_ALARM_TOTAL_DURATION_MS)
            {
                Buzzer_Stop();
                break;
            }

            /* 2. Toggle 0.5s ON / 0.5s OFF pulse */
            if (s_alarm_pin_phase == 1)
            {
                if ((current_tick - s_cycle_start_tick) >= BUZZER_ALARM_CYCLE_ON_MS)
                {
                    s_alarm_pin_phase = 0;
                    s_cycle_start_tick = current_tick;
                    GPIO_SetBuzzer(PIN_STATE_LOW);
                }
            }
            else
            {
                if ((current_tick - s_cycle_start_tick) >= BUZZER_ALARM_CYCLE_OFF_MS)
                {
                    s_alarm_pin_phase = 1;
                    s_cycle_start_tick = current_tick;
                    GPIO_SetBuzzer(PIN_STATE_HIGH);
                }
            }
            break;

        default:
            Buzzer_Stop();
            break;
    }
}
