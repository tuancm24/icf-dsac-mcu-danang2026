#include "clock.h"

static Time_t current_time;

void Clock_Init(void)
{
    current_time.hour = 0;
    current_time.minute = 0;
    current_time.second = 0;
}

void Clock_Tick1Second(void)
{
    current_time.second++;

    if (current_time.second >= 60)
    {
        current_time.second = 0;
        current_time.minute++;

        if (current_time.minute >= 60)
        {
            current_time.minute = 0;
            current_time.hour++;

            if (current_time.hour >= 24)
            {
                current_time.hour = 0;
            }
        }
    }
}

void Clock_IncrementHour(void)
{
    current_time.hour++;

    if (current_time.hour >= 24)
    {
        current_time.hour = 0;
    }
}

void Clock_DecrementHour(void)
{
    if (current_time.hour == 0)
    {
        current_time.hour = 23;
    }
    else
    {
        current_time.hour--;
    }
}

void Clock_IncrementMinute(void)
{
    current_time.minute++;

    if (current_time.minute >= 60)
    {
        current_time.minute = 0;
    }
}

void Clock_DecrementMinute(void)
{
    if (current_time.minute == 0)
    {
        current_time.minute = 59;
    }
    else
    {
        current_time.minute--;
    }
}

Time_t Clock_GetTime(void)
{
    return current_time;
}