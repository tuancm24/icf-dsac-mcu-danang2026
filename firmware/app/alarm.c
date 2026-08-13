#include "alarm.h"

static Time_t alarm_time;

void Alarm_Init(void)
{
    alarm_time.hour = 0;
    alarm_time.minute = 0;
    alarm_time.second = 0;
}

void Alarm_SetTime(unsigned char hour,
                   unsigned char minute)
{
    alarm_time.hour = hour;
    alarm_time.minute = minute;
    alarm_time.second = 0;
}

Time_t Alarm_GetTime(void)
{
    return alarm_time;
}

unsigned char Alarm_Check(Time_t current_time)
{
    if ((current_time.hour == alarm_time.hour) &&
        (current_time.minute == alarm_time.minute) &&
        (current_time.second == alarm_time.second))
    {
        return 1;
    }

    return 0;
}