$NOMOD51
;------------------------------------------------------------------------------
;
;  Copyright (c) 2015 SONiX Technology Co., Ltd.
;  Version 2.1 - SN8F5708, SN8F5707, SN8F5705, SN8F57081, SN8F57082, SN8F57084, SN8F57085, SN8F57086, SN8F57087
;  *** <<< Use Configuration Wizard in Context Menu >>> ***
;------------------------------------------------------------------------------
ROM_SIZE		EQU		0x4000
;
;   <o> Program Memory Security <0x01=> Disable <0x00=> Enable
    SECURITY_SET    EQU     0x01	;	{0x3FFF}
;
;   <o.1..3> CPU Clock Source <0x07=> IHRC 32 MHz <0x03=> IHRC 32 MHz with RTC <0x01=> X'tal 12 MHz <0x00=> X'tal 4 MHz <0x02=> External Clock
    CLOCKSRC_SET    EQU     0x0E	;	{0x3FFF}
;
;   <o> Noise Filter <0x01=> Disable <0x00=> Enable
    NOISEFILT_SET   EQU     0x01	;	{0x3FFC}
;
;   <h> Reset Sources
;       <o.1..2> VDD Voltage (Low Voltage Detection) <0x00=> LVD_Max <0x01=> LVD_H <0x02=> LVD_M <0x03=> LVD_L
        LVDMODE_SET     EQU     0x06	;	{0x3FFC}
;
;       <o.4..5> External Reset / GPIO Shared Pin <0x00=> Reset with De-bounce <0x02=> Reset without De-bounce <0x03=> GPIO
        RESETPIN_SET    EQU     0x00	;	{0x3FFC}
;
;       <o.4..7> Watchdog Reset <0x00=> Always <0x05=> Enable <0x0A=> Disable
        WATCHDOG_SET    EQU     0xA0	;	{0x3FFF}
;
;       <o.6..7> Watchdog Overflow Period <0x00=> 64 ms <0x01=> 128 ms <0x02=> 256 ms <0x03=> 512 ms
        WATCHCLK_SET    EQU     0xC0	;	{0x3FFC}
;   </h>

    CSEG    AT      0x3FF6
    DB      0xFF
    DB      0xFF
    DB      0xFF
    DB      0xFF
    DB      0xFF
    DB      0xFF
    DB      WATCHCLK_SET + RESETPIN_SET + LVDMODE_SET + NOISEFILT_SET
    DB      0x5A
    DB      0xA5
    DB      WATCHDOG_SET + CLOCKSRC_SET + SECURITY_SET
    END
