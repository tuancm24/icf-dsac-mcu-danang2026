#include "button.h"
#include "gpio.h"
#include "timer.h"
#include "app_config.h"

/* =========================================================================
 * Button Driver Implementation (SONiX SN8F5708 EVK)
 * Debouncing & Click Event Generation (Optimized with IDATA buffers)
 * ========================================================================= */

#define BUTTON_EVENT_QUEUE_SIZE     (4)

#ifndef BUTTON_DEBOUNCE_THRESHOLD
#ifdef BUTTON_DEBOUNCE_COUNT
#define BUTTON_DEBOUNCE_THRESHOLD   (BUTTON_DEBOUNCE_COUNT)
#else
#define BUTTON_DEBOUNCE_THRESHOLD   (3)
#endif
#endif

typedef struct
{
    unsigned char debounce_counter;
    unsigned char stable_state;     /* 1 = Pressed, 0 = Released */
    unsigned char last_raw_state;
} Button_State_t;

static Button_State_t idata s_buttons[BUTTON_ID_COUNT];
static Button_Event_t idata s_event_queue[BUTTON_EVENT_QUEUE_SIZE];
static unsigned char s_queue_head = 0;
static unsigned char s_queue_tail = 0;
static unsigned char s_queue_count = 0;
static unsigned long s_last_scan_tick = 0;

static void Button_EnqueueEvent(Button_Event_t event)
{
    if (s_queue_count < BUTTON_EVENT_QUEUE_SIZE)
    {
        s_event_queue[s_queue_head] = event;
        s_queue_head = (s_queue_head + 1) % BUTTON_EVENT_QUEUE_SIZE;
        s_queue_count++;
    }
}

void Button_Init(void)
{
    unsigned char i;
    for (i = 0; i < BUTTON_ID_COUNT; i++)
    {
        s_buttons[i].debounce_counter = 0;
        s_buttons[i].stable_state = 0;
        s_buttons[i].last_raw_state = 0;
    }

    s_queue_head = 0;
    s_queue_tail = 0;
    s_queue_count = 0;
    s_last_scan_tick = 0;
}

unsigned char Button_IsPressed(Button_Id_t button_id)
{
    if (button_id < BUTTON_ID_COUNT)
    {
        return s_buttons[button_id].stable_state;
    }
    return 0;
}

Button_Event_t Button_GetEvent(void)
{
    Button_Event_t event = BUTTON_EVENT_NONE;

    if (s_queue_count > 0)
    {
        event = s_event_queue[s_queue_tail];
        s_queue_tail = (s_queue_tail + 1) % BUTTON_EVENT_QUEUE_SIZE;
        s_queue_count--;
    }

    return event;
}

static unsigned char Button_ReadPhysicalPin(Button_Id_t id)
{
    switch (id)
    {
        case BUTTON_ID_SW3:
            return (GPIO_ReadButton_SW3() == PIN_STATE_LOW) ? 1 : 0;
        case BUTTON_ID_SW6:
            return (GPIO_ReadButton_SW6() == PIN_STATE_LOW) ? 1 : 0;
        case BUTTON_ID_SW10:
            return (GPIO_ReadButton_SW10() == PIN_STATE_LOW) ? 1 : 0;
        case BUTTON_ID_SW16:
            return (GPIO_ReadButton_SW16() == PIN_STATE_LOW) ? 1 : 0;
        default:
            return 0;
    }
}

void Button_Process(void)
{
    unsigned char i;
    unsigned long current_tick = Timer_GetTickMs();

    if (!Timer_HasElapsed(s_last_scan_tick, BUTTON_SCAN_INTERVAL_MS))
    {
        return;
    }
    s_last_scan_tick = current_tick;

    for (i = 0; i < BUTTON_ID_COUNT; i++)
    {
        unsigned char raw_state = Button_ReadPhysicalPin((Button_Id_t)i);

        if (raw_state == s_buttons[i].last_raw_state)
        {
            if (s_buttons[i].debounce_counter < BUTTON_DEBOUNCE_THRESHOLD)
            {
                s_buttons[i].debounce_counter++;
                if (s_buttons[i].debounce_counter >= BUTTON_DEBOUNCE_THRESHOLD)
                {
                    if (s_buttons[i].stable_state != raw_state)
                    {
                        s_buttons[i].stable_state = raw_state;

                        /* Emit CLICK event only on Press transition (0 -> 1) */
                        if (raw_state == 1)
                        {
                            switch ((Button_Id_t)i)
                            {
                                case BUTTON_ID_SW3:
                                    Button_EnqueueEvent(BUTTON_EVENT_SW3_CLICK);
                                    break;
                                case BUTTON_ID_SW6:
                                    Button_EnqueueEvent(BUTTON_EVENT_SW6_CLICK);
                                    break;
                                case BUTTON_ID_SW10:
                                    Button_EnqueueEvent(BUTTON_EVENT_SW10_CLICK);
                                    break;
                                case BUTTON_ID_SW16:
                                    Button_EnqueueEvent(BUTTON_EVENT_SW16_CLICK);
                                    break;
                                default:
                                    break;
                            }
                        }
                    }
                }
            }
        }
        else
        {
            s_buttons[i].last_raw_state = raw_state;
            s_buttons[i].debounce_counter = 0;
        }
    }
}
