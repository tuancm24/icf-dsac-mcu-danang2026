#include "alarm.h"

volatile Time_t test_alarm_time;
volatile unsigned char test_alarm_match;

void main(void)
{
    Time_t current_time;

    /* TEST 1: Init -> 00:00:00 */
    Alarm_Init();
    test_alarm_time = Alarm_GetTime();

    /* TEST 2: Set alarm time -> 07:30:00 */
    Alarm_SetTime(7, 30);
    test_alarm_time = Alarm_GetTime();

    /* TEST 3: Alarm_SetTime forces second -> 00 */
    Alarm_SetTime(12, 45);
    test_alarm_time = Alarm_GetTime();

    /* TEST 4: Matching alarm time -> 1 */
    Alarm_SetTime(7, 30);

    current_time.hour = 7;
    current_time.minute = 30;
    current_time.second = 0;

    test_alarm_match = Alarm_Check(current_time);

    /* TEST 5: Hour mismatch -> 0 */
    current_time.hour = 8;
    current_time.minute = 30;
    current_time.second = 0;

    test_alarm_match = Alarm_Check(current_time);

    /* TEST 6: Minute mismatch -> 0 */
    current_time.hour = 7;
    current_time.minute = 31;
    current_time.second = 0;

    test_alarm_match = Alarm_Check(current_time);

    /* TEST 7: Second mismatch -> 0 */
    current_time.hour = 7;
    current_time.minute = 30;
    current_time.second = 1;

    test_alarm_match = Alarm_Check(current_time);

    while (1)
    {
        /* Stop here for debugger inspection */
    }
}