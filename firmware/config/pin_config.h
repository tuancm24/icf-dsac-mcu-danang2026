#ifndef PIN_CONFIG_H
#define PIN_CONFIG_H

#include "board_config.h"

/* =========================================================================
 * SONiX SN8F5708 EVK - Hardware Pin Mapping
 * Verified from Board Silkscreen Layout (5708_EVK-V1.0 2024.02.26)
 * ========================================================================= */

/* -------------------------------------------------------------------------
 * 1. Status LED Outputs
 * ------------------------------------------------------------------------- */
#define PIN_LED_D4_PORT                 (0)
#define PIN_LED_D4_BIT                  (3)     /* P0.3 (Silkscreen: P03) */
#define LED_D4_ACTIVE_LEVEL             (0)     /* Active LOW: 0 = ON, 1 = OFF */

#define PIN_LED_D5_PORT                 (0)
#define PIN_LED_D5_BIT                  (4)     /* P0.4 (Silkscreen: P04) */

/* -------------------------------------------------------------------------
 * 2. Buzzer Output (PZ1)
 * ------------------------------------------------------------------------- */
#define PIN_BUZZER_PORT                 (1)
#define PIN_BUZZER_BIT                  (0)     /* P1.0 (Silkscreen: P10) */
#define BUZZER_ACTIVE_LEVEL             (1)     /* Active HIGH: 1 = ON, 0 = OFF */

/* -------------------------------------------------------------------------
 * 3. 4-Digit 7-Segment Display (SMG1 - 3461AS Common Cathode)
 *    - Segments (a..dp): Port 3 (P3.0 -> P3.7, Active HIGH)
 *    - Digits (DIG1..DIG4): Q1..Q4 NPN BJTs (P5.0 -> P5.3, Active HIGH)
 * ------------------------------------------------------------------------- */
#define DISPLAY_COMMON_ANODE            (0)     /* 3461AS is Common Cathode */

#define PIN_SEG_PORT                    (3)     /* P3.0=a, P3.1=b, P3.2=c, P3.3=d, P3.4=e, P3.5=f, P3.6=g, P3.7=dp */
#define PIN_DIG1_BIT                    (0)     /* P5.0 (Q1: Hour Tens) */
#define PIN_DIG2_BIT                    (1)     /* P5.1 (Q2: Hour Units + Colon) */
#define PIN_DIG3_BIT                    (2)     /* P5.2 (Q3: Minute Tens) */
#define PIN_DIG4_BIT                    (3)     /* P5.3 (Q4: Minute Units) */

/* -------------------------------------------------------------------------
 * 4. Button Matrix 4x4 (SW3, SW6, SW10, SW16)
 *    - Rows: P2.4 (Row 1), P2.5 (Row 2), P2.6 (Row 3), P2.7 (Row 4)
 *    - Cols: P4.4 (Col 1), P4.5 (Col 2), P4.6 (Col 3), P4.7 (Col 4)
 * ------------------------------------------------------------------------- */
#define BUTTON_ROW_SW3_SW6              (4)     /* P2.4 (Row 1: SW3, SW4, SW5, SW6) */
#define BUTTON_ROW_SW10                 (5)     /* P2.5 (Row 2: SW7, SW8, SW9, SW10) */
#define BUTTON_ROW_SW16                 (7)     /* P2.7 (Row 4: SW16, SW15, SW17, SW18) */

#define BUTTON_COL_SW3_SW16             (4)     /* P4.4 (Col 1: SW3, SW7, SW11, SW16) */
#define BUTTON_COL_SW6_SW10             (7)     /* P4.7 (Col 4: SW6, SW10, SW14, SW18) */

/* -------------------------------------------------------------------------
 * 5. I2C EEPROM Interface (24C08 / Header J6)
 * ------------------------------------------------------------------------- */
#define PIN_I2C_SCL_BIT                 (4)     /* P1.4 (Silkscreen: SCL_P14) */
#define PIN_I2C_SDA_BIT                 (5)     /* P1.5 (Silkscreen: SDA_P15) */
#define EEPROM_I2C_DEV_ADDR             (0xA0)

#endif /* PIN_CONFIG_H */
