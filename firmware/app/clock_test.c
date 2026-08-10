#include "clock.h"

volatile Time_t test_time;

void main(void)
{
    /* TEST 1: Init -> 00:00:00 */
    Clock_Init();
    test_time = Clock_GetTime();

    /* TEST 2: 60 seconds -> 00:01:00 */
    {
        unsigned char i;

        for (i = 0; i < 60; i++)
        {
            Clock_Tick1Second();
        }
    }

    test_time = Clock_GetTime();

    /* TEST 3: Hour 00 - 1 -> 23 */
    Clock_Init();
    Clock_DecrementHour();
    test_time = Clock_GetTime();

    /* TEST 4: Hour 23 + 1 -> 00 */
    Clock_IncrementHour();
    test_time = Clock_GetTime();

    /* TEST 5: Minute 00 - 1 -> 59 */
    Clock_Init();
    Clock_DecrementMinute();
    test_time = Clock_GetTime();

    /* TEST 6: Minute 59 + 1 -> 00 */
    Clock_IncrementMinute();
    test_time = Clock_GetTime();

    /* TEST 7: 23:59:59 + 1 second -> 00:00:00 */
Clock_Init();

{
    unsigned char i;

    /* Set hour to 23 */
    for (i = 0; i < 23; i++)
    {
        Clock_IncrementHour();
    }

    /* Set minute to 59 */
    for (i = 0; i < 59; i++)
    {
        Clock_IncrementMinute();
    }

    /* Set second to 59 */
    for (i = 0; i < 59; i++)
    {
        Clock_Tick1Second();
    }
}

test_time = Clock_GetTime();

/* 23:59:59 + 1 second */
Clock_Tick1Second();

test_time = Clock_GetTime();

    while (1)
    {
        /* Stop here for debugger inspection */
    }
}