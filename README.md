# ICF DSAC MCU Danang 2026

Firmware implementation of a Digital Clock and Alarm System on the SONiX SN8F5708 EVK for the DSAC FPGA & MCU Design Competition 2026.

## Project Overview

The system is developed on the SONiX SN8F5708 EVK and implements:

- 24-hour digital clock in `HH.MM` format
- Current time configuration
- Alarm time configuration
- Alarm storage in EEPROM
- Button feedback using a buzzer
- Five-second alarm notification
- Blinking display during configuration
- Alarm-setting status LED
- Thirty-second inactivity timeout

## Hardware Platform

- Board: SONiX SN8F5708 EVK
- MCU: SN8F5708
- Display: Four-digit 7-segment display
- Input buttons:
  - SW3: Time setup
  - SW6: Increment
  - SW10: Decrement
  - SW16: Alarm setup
- Outputs:
  - Buzzer
  - Status LED D4
- Storage:
  - EEPROM

## Development Environment

- IDE: Keil µVision C51 v9.61
- Compiler: Keil C51
- Programmer: SN-Link
- Target MCU: SONiX SN8F5708

## Team

| Member | Role | Responsibilities |
|---|---|---|
| Cao Minh Tuấn | Application Firmware & Integration | Architecture, FSM, clock logic, alarm logic, integration, testing and documentation |
| Nguyễn Đình Phan Trung | Board Support & Driver Firmware | GPIO, timer, interrupt, buttons, 7-segment display, buzzer, LED and EEPROM |

## Software Architecture

```text
Application Layer
├── Application State Machine
├── Clock Logic
└── Alarm Logic

Driver Layer
├── Button Driver
├── 7-Segment Display Driver
├── Buzzer Driver
├── LED Driver
└── EEPROM Driver

Platform Layer
├── GPIO
├── Timer
└── Interrupt

