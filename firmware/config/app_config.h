#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include "board_config.h"

/* =========================================================================
 * Application Timing & Behavior Constants
 * Derived from the Official Competition Requirements & Design Specifications
 * ========================================================================= */

/* 1. Timeout Requirements */
#define TIMEOUT_INACTIVITY_SEC          (30)
#define TIMEOUT_INACTIVITY_MS           (30000UL)

/* 2. Buzzer Timing Requirements */
#define BUZZER_KEYPRESS_BEEP_MS         (300)   /* 0.3s beep on button press / timeout */
#define BUZZER_ALARM_TOTAL_DURATION_MS  (5000)  /* 5s continuous alarm ringing */
#define BUZZER_ALARM_CYCLE_ON_MS        (500)   /* 0.5s ON in alarm pattern */
#define BUZZER_ALARM_CYCLE_OFF_MS       (500)   /* 0.5s OFF in alarm pattern */

/* 3. Blinking Rate Requirements (1s period: 0.5s ON - 0.5s OFF) */
#define BLINK_HALF_PERIOD_MS            (500)
#define BLINK_FULL_PERIOD_MS            (1000)

/* 4. Button Scan & Debounce Parameters */
#define BUTTON_SCAN_INTERVAL_MS         (10)    /* Scan buttons every 10ms */
#define BUTTON_DEBOUNCE_COUNT           (3)     /* 3 consecutive samples = 30ms stable */

/* 5. 7-Segment Display Refresh Parameters */
#define DISPLAY_SCAN_DIGIT_INTERVAL_MS  (2)     /* Multiplex next digit every 2ms */

/* 6. EEPROM Address Allocation (24C05) */
#define EEPROM_ADDR_ALARM_HOUR          (0x00)
#define EEPROM_ADDR_ALARM_MINUTE        (0x01)
#define EEPROM_ADDR_MAGIC_BYTE          (0x02)  /* Used to verify valid saved data (0xA5) */
#define EEPROM_MAGIC_VALUE              (0xA5)

#endif /* APP_CONFIG_H */
