#ifndef CLOCK_H
#define CLOCK_H

typedef struct
{
    unsigned char hour;
    unsigned char minute;
    unsigned char second;
} Time_t;

/* Initialize clock to 00:00:00 */
void Clock_Init(void);

/* Called once every second */
void Clock_Tick1Second(void);

/* Functions used while setting the clock */
void Clock_IncrementHour(void);
void Clock_DecrementHour(void);

void Clock_IncrementMinute(void);
void Clock_DecrementMinute(void);

/* Get current clock value */
Time_t Clock_GetTime(void);

#endif