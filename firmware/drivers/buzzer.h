#ifndef BUZZER_H
#define BUZZER_H

#include "app_config.h"

/* =========================================================================
 * Buzzer Driver Interface (SONiX SN8F5708 EVK)
 * ========================================================================= */

/* Initialize Buzzer GPIO pin */
void Buzzer_Init(void);

/* Trigger a non-blocking short beep (0.3s) for button press or timeout */
void Buzzer_BeepShort(void);

/* Trigger the non-blocking 5-second alarm sequence (0.5s ON - 0.5s OFF) */
void Buzzer_StartAlarm(void);

/* Stop any active buzzer output immediately */
void Buzzer_Stop(void);

/* Update buzzer timing state machine (called in 1ms periodic system tick) */
void Buzzer_Process(void);

/* Check if buzzer is currently sounding: 1 = Active, 0 = Idle */
unsigned char Buzzer_IsBusy(void);

#endif /* BUZZER_H */
