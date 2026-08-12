#ifndef FSM_H
#define FSM_H

/* =========================================================
 * Application FSM States
 * ========================================================= */
typedef enum
{
    FSM_NORMAL = 0,
    FSM_SET_TIME_HOUR,
    FSM_SET_TIME_MINUTE,
    FSM_SET_ALARM_HOUR,
    FSM_SET_ALARM_MINUTE
} FSM_State;


/* =========================================================
 * Application FSM Events
 * ========================================================= */
typedef enum
{
    FSM_EVENT_NONE = 0,
    FSM_EVENT_SW3,
    FSM_EVENT_SW6,
    FSM_EVENT_SW10,
    FSM_EVENT_SW16,
    FSM_EVENT_TIMEOUT_30S
} FSM_Event;


/* =========================================================
 * FSM Public Interface
 * ========================================================= */

/* Initialize FSM to NORMAL state */
void FSM_Init(void);

/* Process one FSM event */
void FSM_HandleEvent(FSM_Event event);

/* Return current FSM state */
FSM_State FSM_GetState(void);


#endif /* FSM_H */