# ICF DSAC MCU Da Nang 2026
## Digital Clock & Alarm System on SONiX SN8F5708 EVK
## Demo Video

🎥 **Final Hardware Demo:** https://youtu.be/f7Rsa1xx3BQ

The video demonstrates the final system running on the **SONiX SN8F5708 EVK**, including:

- Clock operation and current-time setting
- Alarm configuration
- LED D4 state indication
- EEPROM power-cycle persistence
- Natural alarm triggering
- 5-second buzzer sequence
- One-shot alarm behavior
- 30-second inactivity timeout


Embedded firmware and hardware-integration project developed for the **DSAC FPGA & MCU Design Competition 2026**.

The system implements a real-time digital clock and alarm controller on the **SONiX SN8F5708 EVK**, including clock configuration, alarm configuration, EEPROM persistence, button feedback, display behavior, inactivity timeout handling, and natural alarm notification.

> **Portfolio focus:** Embedded Systems · MCU Firmware · Hardware Integration · Verification · Debugging · Test Planning

---

## Project Snapshot

| Category | Details |
|---|---|
| Platform | SONiX SN8F5708 EVK |
| MCU | SONiX SN8F5708 |
| Language | Embedded C |
| Toolchain | Keil C51 / µVision |
| Programmer | SN-Link |
| Display | 4-digit 7-segment |
| Storage | 24C05 EEPROM |
| Team | 2 engineers |
| Competition | DSAC FPGA & MCU Design Competition 2026 |
| Current integration candidate | `1ecf0b4` |

---

# My Role

## Cao Minh Tuấn — Application Firmware & Integration

I owned the **Application / Core / Integration** side of the project.

### Primary responsibilities

- Application state-machine integration
- Clock behavior and scheduler integration
- Alarm behavior and transaction semantics
- Button-event to FSM integration
- Current-time configuration lifecycle
- Alarm editing and persistence flow
- Application / Driver integration boundaries
- Integration testing
- Evidence and checkpoint tracking
- Technical documentation and engineering handoff

### Key implementation areas

#### Clock / Timer Integration

Worked through application-level timing and integration issues involving:

- 32 MHz IHRC system timing
- Timer tick behavior
- Timer ISR interaction
- Scheduler timing and anchoring
- Correct clock behavior during configuration states

#### Button → FSM Integration

Defined the application-side event flow:

```text
Button Driver
      ↓
Button Event / FIFO
      ↓
Application Event Mapping
      ↓
FSM
      ↓
state_before / state_after
      ↓
Application Feedback / Activity Handling
```

This boundary became a reusable integration point for later stages.

#### Current-Time Configuration

Implemented the application lifecycle for time configuration, including:

- hour/minute editing
- wrap behavior
- seconds preservation
- stopping normal clock progression during configuration
- fresh scheduler anchoring when returning to NORMAL

#### Alarm Transaction

Implemented a transactional alarm-edit flow separating temporary edit state from the confirmed alarm:

```text
Confirmed Alarm
      ↓
alarm_edit_time
      ↓
Temporary Editing
      ↓
Final Confirmation
      ↓
Runtime Commit
      ↓
EEPROM Persistence
```

The implementation also handles alarm validity and confirmation/latch semantics.

#### Production / Integration Hardening

Worked through embedded integration problems involving:

- MCU memory constraints
- timer / clock integration
- button matrix integration
- production/test-build isolation
- application/display timing ownership
- startup alarm validity semantics

---

# Project Overview

This project is a complete embedded digital clock and alarm system developed on a real SONiX SN8F5708 evaluation board.

The system combines:

- 24-hour digital clock in `HH.MM` format
- Current-time configuration
- Alarm-time configuration
- EEPROM persistence
- Button event handling
- 4-digit 7-segment display control
- Buzzer feedback
- 5-second natural alarm notification
- Alarm-setting status LED
- 30-second inactivity timeout

The project was developed as a **staged integration effort**, with explicit ownership between Application/Core and Drivers/Platform/Hardware.

---

# Team Structure

## Application / Core / Integration

**Tuấn**

- Application architecture
- Clock logic
- FSM integration
- Alarm logic
- Application-level integration
- INT01–INT06

## Drivers / Platform / Hardware

**Trung**

- GPIO
- Timer / interrupt platform
- Button driver
- 7-segment display driver
- Buzzer driver
- LED driver
- EEPROM driver
- EVK hardware validation
- Later integration and closure stages

The project intentionally separates Application/Core responsibilities from Drivers/Platform/Hardware responsibilities through explicit integration boundaries.

---

# Software Architecture

```text
                    Application Layer
                           │
              ┌────────────┼────────────┐
              │            │            │
            Clock         FSM         Alarm
              │            │            │
              └────────────┼────────────┘
                           │
                    API / Contract
                           │
              ┌────────────┼────────────┐
              │            │            │
           Button        Display      Buzzer
              │            │            │
              └────────────┼────────────┘
                           │
                    Platform Layer
                           │
                  GPIO / Timer / IRQ
                           │
                           ▼
                    SONiX SN8F5708
                           │
                           ▼
                          EVK
```

---

# Verification Methodology

A major focus of the project was separating implementation status from verification status.

```text
Requirement
    ↓
Integration Contract
    ↓
Test Plan
    ↓
Implementation
    ↓
Build
    ↓
Runtime / EVK Evidence
    ↓
PASS / OPEN / FAIL
    ↓
Regression
    ↓
Integration Closure
```

The project follows an **evidence-first** workflow.

In particular:

```text
CODE DONE
    ≠
TEST PASS
    ≠
GATE CLOSED
```

A commit message or source inspection alone is not treated as sufficient PASS evidence.

The verification process tracks:

- test cases
- expected behavior
- actual behavior
- runtime evidence
- hardware evidence
- regression status
- integration gate status
- deferred dependencies

---

# Integration Journey

## Phase 1 — Foundation: INT01 → INT06

The first integration stages established the Application/Core foundation.

### INT01–INT02 — Production and Timing Foundation

Focused on:

- production/test target isolation
- MCU memory constraints
- timer behavior
- 32 MHz system timing
- startup behavior
- application / platform ownership boundaries

### INT03 — Clock → Display

Integrated:

- clock output
- display state handling
- normal/configuration display behavior
- resulting-state oriented output

### INT04 — Button → FSM

Integrated:

- button-event mapping
- FIFO handling
- FSM transition flow
- state-before/state-after sequencing
- undefined input behavior

### INT05 — Current-Time Setting

Integrated:

- configuration-state clock handling
- pause/resume lifecycle
- scheduler anchoring
- protection against stale elapsed-time replay

### INT06 — Alarm Editing

Integrated:

- temporary alarm editing
- runtime commit behavior
- alarm validity
- persistence flow
- confirmation and latch semantics

---

# Application Checkpoint

## `05b05a5`

The Application branch reached a documented integration checkpoint at:

```text
05b05a5
feat(app): checkpoint integration through INT06
```

This checkpoint became the baton point for the next integration phase.

The later stages reused established Clock, FSM, Button and Alarm boundaries rather than rebuilding the Application/Core foundation.

---

# Phase 2 — Integration Completion: INT07 → INT14

Later stages focused on completing user-visible behavior, timeout handling, persistence verification, natural alarm behavior, regression and hardware closure.

```text
INT07
  ↓
Display / LED state policy
  ↓
INT08
  ↓
Accepted-key feedback
  ↓
INT09
  ↓
30-second inactivity timeout
  ↓
INT10
  ↓
EEPROM persistence verification
  ↓
INT11
  ↓
Natural alarm occurrence
  ↓
INT12
  ↓
Alarm / timing / feedback composition
  ↓
INT13
  ↓
Software regression
  ↓
INT14
  ↓
EVK hardware closure
```

The current Application integration candidate is:

```text
1ecf0b4
```

---

# Key Engineering Challenges

## Timing / Clock

- 32 MHz IHRC alignment
- Timer ISR interaction
- scheduler timing
- pause/resume behavior
- fresh timing anchors

## MCU Resource Constraints

- C51 memory model
- data / xdata placement
- production-target constraints

## Input / FSM

- Button matrix integration
- event FIFO handling
- state transition boundaries
- undefined-event handling

## Alarm System

- temporary vs confirmed alarm state
- alarm validity
- persistence ordering
- confirmation semantics
- latch behavior
- natural alarm occurrence

## Production Integration

- separation of test hooks from production targets
- application / driver ownership boundaries
- staged Git integration
- evidence reuse across integration stages
- regression control

---

# Hardware Platform

The system targets the real:

**SONiX SN8F5708 EVK**

### Main hardware interfaces

- 4-digit 7-segment display
- SW3 — time setup
- SW6 — increment
- SW10 — decrement
- SW16 — alarm setup
- Buzzer
- Status LED D4
- 24C05 EEPROM
- MCU GPIO / Timer / Interrupt infrastructure

Development and flashing use:

- **Keil µVision C51**
- **Keil C51 compiler**
- **SN-Link programmer**

---

# Repository Structure

```text
firmware/
├── app/          # Application / Core logic
├── drivers/      # Peripheral drivers
├── platform/     # GPIO / Timer / Interrupt
├── config/       # Configuration
└── project/      # Keil project configuration

docs/
├── design/       # Specifications and block diagrams
├── integration/  # Contract / Plan / Test Plan
└── testing/      # Test and hardware evidence

demo/              # Demo material
references/        # Reference material
tools/             # Development / verification helpers
release/           # Release-related material
```

---

# Technical Documentation

## Design

- [Clock Specification](docs/design/clock-specification.md)
- [FSM Specification](docs/design/fsm-specification.md)
- [Alarm Specification](docs/design/alarm-specification.md)
- [Button Specification](docs/design/button-specification.md)
- [Display Specification](docs/design/display-specification.md)
- [Buzzer Specification](docs/design/buzzer-specification.md)
- [EEPROM Specification](docs/design/eeprom-specification.md)
- [Timer Specification](docs/design/timer-specification.md)

## Integration

- [Integration Contract v2.7](docs/integration/MCU-APPLICATION-DRIVER-INTEGRATION-CONTRACT-v2.7.md)
- [Application Integration Plan v2.6](docs/integration/application-integration-plan-v2.6.md)
- [Application Integration Test Plan v2.1](docs/integration/application-integration-test-plan-v2.1.md)

## Test Evidence

Subsystem-specific evidence is available under:

```text
docs/testing/
```

including Clock, FSM, Alarm, Button, Display, Buzzer, EEPROM and Timer validation material.

---

# Development Workflow

The project uses checkpoint-based Git development:

```text
Feature Work
    ↓
Build / Test
    ↓
Evidence
    ↓
Checkpoint
    ↓
Integration Review
    ↓
Develop
    ↓
Release
```

The project also uses explicit ownership boundaries between Application/Core and Drivers/Platform/Hardware.

Cross-boundary changes are reviewed before integration, especially when Application and Driver/Platform branches touch the same source areas.

---

# Engineering Lessons

This project strengthened my practical experience in:

- Embedded C
- MCU architecture
- state-machine based application design
- timing and scheduler reasoning
- hardware / software integration
- debugging constrained microcontrollers
- requirement-driven testing
- evidence-based verification
- regression thinking
- Git checkpoint and handoff discipline

The project also strengthened my interest in **Design Verification**.

The next step is to apply the same verification mindset to RTL/SystemVerilog environments, including:

- assertions
- functional coverage
- constrained-random testing
- reference models
- scoreboards
- UVM
- automated regression

---

# Relevance to Design Verification

This project gave me hands-on experience with embedded integration,
hardware validation, debugging, staged verification, regression thinking,
and evidence-driven engineering.

I am building on this foundation toward **Design Verification** through
RTL/SystemVerilog-based projects, assertions, functional coverage,
scoreboards, constrained-random verification and UVM.

# Status

The repository contains the project's implementation history, design documentation, integration plans, test plans and hardware validation material.

Current integration candidate:

```text
1ecf0b4
```

For deeper technical details, start with:

- `docs/design/`
- `docs/integration/`
- `docs/testing/`

---

## Author

**Cao Minh Tuấn**

Software Engineering Student
Specialization: IC Design

Interested in:

- Design Verification
- Digital IC / SoC
- RTL / SystemVerilog
- Embedded Systems
- Hardware / Software Integration