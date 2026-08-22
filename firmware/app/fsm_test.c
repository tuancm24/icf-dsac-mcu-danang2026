#include "fsm.h"
#include "clock.h"

/* =========================================================
 * Test Observation Variables
 * ========================================================= */

volatile FSM_State test_state;
volatile Time_t test_time;


/* =========================================================
 * FSM Test Program
 * ========================================================= */

void main(void)
{
    /* =====================================================
     * TEST 1
     * Power-on / Init -> NORMAL
     * ===================================================== */

    FSM_Init();

    test_state = FSM_GetState();

    /*
     * Expected:
     * test_state = FSM_NORMAL
     */


    /* =====================================================
     * TEST 2
     * NORMAL + SW3 -> SET_TIME_HOUR
     * ===================================================== */

    FSM_HandleEvent(FSM_EVENT_SW3);

    test_state = FSM_GetState();

    /*
     * Expected:
     * test_state = FSM_SET_TIME_HOUR
     */


    /* =====================================================
     * TEST 3
     * SET_TIME_HOUR + SW6
     * Hour +1, state unchanged
     * ===================================================== */

    Clock_Init();

    FSM_HandleEvent(FSM_EVENT_SW6);

    test_time = Clock_GetTime();
    test_state = FSM_GetState();

    /*
     * Expected:
     * test_time.hour = 1
     * test_time.minute = 0
     * test_time.second = 0
     *
     * test_state = FSM_SET_TIME_HOUR
     */


    /* =====================================================
     * TEST 4
     * SET_TIME_HOUR + SW10
     * Hour -1, state unchanged
     * ===================================================== */

    FSM_HandleEvent(FSM_EVENT_SW10);

    test_time = Clock_GetTime();
    test_state = FSM_GetState();

    /*
     * Expected:
     * test_time.hour = 0
     * test_time.minute = 0
     * test_time.second = 0
     *
     * test_state = FSM_SET_TIME_HOUR
     */


    /* =====================================================
     * TEST 5
     * SET_TIME_HOUR + SW3 -> SET_TIME_MINUTE
     * ===================================================== */

    FSM_HandleEvent(FSM_EVENT_SW3);

    test_state = FSM_GetState();

    /*
     * Expected:
     * test_state = FSM_SET_TIME_MINUTE
     */


    /* =====================================================
     * TEST 6
     * SET_TIME_MINUTE + SW6
     * Minute +1, state unchanged
     * ===================================================== */

    FSM_HandleEvent(FSM_EVENT_SW6);

    test_time = Clock_GetTime();
    test_state = FSM_GetState();

    /*
     * Expected:
     * test_time.hour = 0
     * test_time.minute = 1
     * test_time.second = 0
     *
     * test_state = FSM_SET_TIME_MINUTE
     */


    /* =====================================================
     * TEST 7
     * SET_TIME_MINUTE + SW10
     * Minute -1, state unchanged
     * ===================================================== */

    FSM_HandleEvent(FSM_EVENT_SW10);

    test_time = Clock_GetTime();
    test_state = FSM_GetState();

    /*
     * Expected:
     * test_time.hour = 0
     * test_time.minute = 0
     * test_time.second = 0
     *
     * test_state = FSM_SET_TIME_MINUTE
     */


    /* =====================================================
     * TEST 8
     * SET_TIME_MINUTE + SW3 -> NORMAL
     * ===================================================== */

    FSM_HandleEvent(FSM_EVENT_SW3);

    test_state = FSM_GetState();

    /*
     * Expected:
     * test_state = FSM_NORMAL
     */


    /* =====================================================
     * TEST 9
     * NORMAL + SW16 -> SET_ALARM_HOUR
     * ===================================================== */

    FSM_HandleEvent(FSM_EVENT_SW16);

    test_state = FSM_GetState();

    /*
     * Expected:
     * test_state = FSM_SET_ALARM_HOUR
     */


    /* =====================================================
     * TEST 10
     * SET_ALARM_HOUR + SW16 -> SET_ALARM_MINUTE
     * ===================================================== */

    FSM_HandleEvent(FSM_EVENT_SW16);

    test_state = FSM_GetState();

    /*
     * Expected:
     * test_state = FSM_SET_ALARM_MINUTE
     */


    /* =====================================================
     * TEST 11
     * SET_ALARM_MINUTE + SW16 -> NORMAL
     *
     * EEPROM save is NOT verified here because
     * Alarm Core / EEPROM integration is not implemented yet.
     * ===================================================== */

    FSM_HandleEvent(FSM_EVENT_SW16);

    test_state = FSM_GetState();

    /*
     * Expected:
     * test_state = FSM_NORMAL
     */


    /* =====================================================
     * TEST 12
     * TIMEOUT from SET_TIME_HOUR -> NORMAL
     * ===================================================== */

    FSM_HandleEvent(FSM_EVENT_SW3);

    /*
     * Current state:
     * FSM_SET_TIME_HOUR
     */

    FSM_HandleEvent(FSM_EVENT_TIMEOUT_30S);

    test_state = FSM_GetState();

    /*
     * Expected:
     * test_state = FSM_NORMAL
     */


    /* =====================================================
     * TEST 13
     * TIMEOUT from SET_TIME_MINUTE -> NORMAL
     * ===================================================== */

    FSM_HandleEvent(FSM_EVENT_SW3);
    FSM_HandleEvent(FSM_EVENT_SW3);

    /*
     * Current state:
     * FSM_SET_TIME_MINUTE
     */

    FSM_HandleEvent(FSM_EVENT_TIMEOUT_30S);

    test_state = FSM_GetState();

    /*
     * Expected:
     * test_state = FSM_NORMAL
     */


    /* =====================================================
     * TEST 14
     * TIMEOUT from SET_ALARM_HOUR -> NORMAL
     * ===================================================== */

    FSM_HandleEvent(FSM_EVENT_SW16);

    /*
     * Current state:
     * FSM_SET_ALARM_HOUR
     */

    FSM_HandleEvent(FSM_EVENT_TIMEOUT_30S);

    test_state = FSM_GetState();

    /*
     * Expected:
     * test_state = FSM_NORMAL
     */


    /* =====================================================
     * TEST 15
     * TIMEOUT from SET_ALARM_MINUTE -> NORMAL
     * ===================================================== */

    FSM_HandleEvent(FSM_EVENT_SW16);
    FSM_HandleEvent(FSM_EVENT_SW16);

    /*
     * Current state:
     * FSM_SET_ALARM_MINUTE
     */

    FSM_HandleEvent(FSM_EVENT_TIMEOUT_30S);

    test_state = FSM_GetState();

    /*
     * Expected:
     * test_state = FSM_NORMAL
     */


    /* =====================================================
     * TEST 16
     * Undefined combination:
     * NORMAL + SW6
     *
     * Expected:
     * State remains NORMAL
     * ===================================================== */

    FSM_HandleEvent(FSM_EVENT_SW6);

    test_state = FSM_GetState();

    /*
     * Expected:
     * test_state = FSM_NORMAL
     */


    /* =====================================================
     * TEST 17
     * Undefined combination:
     * NORMAL + SW10
     *
     * Expected:
     * State remains NORMAL
     * ===================================================== */

    FSM_HandleEvent(FSM_EVENT_SW10);

    test_state = FSM_GetState();

    /*
     * Expected:
     * test_state = FSM_NORMAL
     */


    /* =====================================================
     * End of tests
     * Stop here for debugger inspection
     * ===================================================== */

    while (1)
    {
        /* Inspect test_state and test_time in Keil Watch */
    }
}