#include "fsm.h"
#include "clock.h"

/* =========================================================
 * Private Data
 * ========================================================= */

static FSM_State current_state;


/* =========================================================
 * Public Functions
 * ========================================================= */

void FSM_Init(void)
{
    current_state = FSM_NORMAL;
}


FSM_State FSM_GetState(void)
{
    return current_state;
}


void FSM_HandleEvent(FSM_Event event)
{
    switch (current_state)
    {
        /* =================================================
         * NORMAL
         * ================================================= */
        case FSM_NORMAL:

            if (event == FSM_EVENT_SW3)
            {
                current_state = FSM_SET_TIME_HOUR;
            }
            else if (event == FSM_EVENT_SW16)
            {
                current_state = FSM_SET_ALARM_HOUR;
            }

            /*
             * SW6 / SW10:
             * No state transition and no value modification
             * according to the current team design rule.
             */

            break;


        /* =================================================
         * SET CURRENT TIME - HOUR
         * ================================================= */
        case FSM_SET_TIME_HOUR:

            if (event == FSM_EVENT_SW3)
            {
                current_state = FSM_SET_TIME_MINUTE;
            }
            else if (event == FSM_EVENT_SW6)
            {
                Clock_IncrementHour();
            }
            else if (event == FSM_EVENT_SW10)
            {
                Clock_DecrementHour();
            }
            else if (event == FSM_EVENT_TIMEOUT_30S)
            {
                current_state = FSM_NORMAL;
            }

            /*
             * SW16:
             * No state transition and no value modification.
             */

            break;


        /* =================================================
         * SET CURRENT TIME - MINUTE
         * ================================================= */
        case FSM_SET_TIME_MINUTE:

            if (event == FSM_EVENT_SW3)
            {
                current_state = FSM_NORMAL;
            }
            else if (event == FSM_EVENT_SW6)
            {
                Clock_IncrementMinute();
            }
            else if (event == FSM_EVENT_SW10)
            {
                Clock_DecrementMinute();
            }
            else if (event == FSM_EVENT_TIMEOUT_30S)
            {
                current_state = FSM_NORMAL;
            }

            /*
             * SW16:
             * No state transition and no value modification.
             */

            break;


        /* =================================================
         * SET ALARM - HOUR
         * ================================================= */
        case FSM_SET_ALARM_HOUR:

            if (event == FSM_EVENT_SW16)
            {
                current_state = FSM_SET_ALARM_MINUTE;
            }
            else if (event == FSM_EVENT_SW6)
            {
                /*
                 * TODO:
                 * Alarm hour +1.
                 * Alarm Core will be integrated later.
                 */
            }
            else if (event == FSM_EVENT_SW10)
            {
                /*
                 * TODO:
                 * Alarm hour -1.
                 * Alarm Core will be integrated later.
                 */
            }
            else if (event == FSM_EVENT_TIMEOUT_30S)
            {
                current_state = FSM_NORMAL;
            }

            /*
             * SW3:
             * No state transition and no value modification.
             */

            break;


        /* =================================================
         * SET ALARM - MINUTE
         * ================================================= */
        case FSM_SET_ALARM_MINUTE:

            if (event == FSM_EVENT_SW16)
            {
                /*
                 * TODO:
                 * Save alarm hour/minute to EEPROM.
                 * Alarm second = 0.
                 */

                current_state = FSM_NORMAL;
            }
            else if (event == FSM_EVENT_SW6)
            {
                /*
                 * TODO:
                 * Alarm minute +1.
                 * Alarm Core will be integrated later.
                 */
            }
            else if (event == FSM_EVENT_SW10)
            {
                /*
                 * TODO:
                 * Alarm minute -1.
                 * Alarm Core will be integrated later.
                 */
            }
            else if (event == FSM_EVENT_TIMEOUT_30S)
            {
                current_state = FSM_NORMAL;
            }

            /*
             * SW3:
             * No state transition and no value modification.
             */

            break;


        /* =================================================
         * SAFETY FALLBACK
         * ================================================= */
        default:

            current_state = FSM_NORMAL;

            break;
    }
}