#ifndef ALARM_H
#define ALARM_H

#include "clock.h"

/* Initialize confirmed alarm time to 00:00:00 */
void Alarm_Init(void);

/* Set confirmed alarm time */
void Alarm_SetTime(unsigned char hour,
                   unsigned char minute);

/* Get confirmed alarm time */
Time_t Alarm_GetTime(void);

/* Check whether current time matches confirmed alarm time */
unsigned char Alarm_Check(Time_t current_time);

#endif