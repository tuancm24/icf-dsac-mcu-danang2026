# Final Release Record — DSAC MCU 2026

## Release Identity

| Field | Value |
|---|---|
| Candidate SHA | `1ecf0b4b1bfc60bebf6ea378d640378528d91d12` |
| Branch | `feature/application-integration` |
| Working tree at build | Clean |
| Production target | `dsac` |
| Build mode | Clean Rebuild |
| Build timestamp | 2026-08-21 16:55:14 |
| Flash / verify timestamp | 2026-08-21 16:55:20 |
| Toolchain | Keil PK51 Professional Developer's Kit 9.60.7.0 |
| Compiler | C51 V9.60.7.0 |
| Linker | BL51 V6.22.4.0 |

## Final Build Result

```text
Program Size:
data=81.0
xdata=62
code=4786

Errors:
0

Warnings:
4
```

The exact clean rebuild generated the production HEX from the `dsac` target.

## Binary / HEX Identity

| Field | Value |
|---|---|
| HEX | `firmware/project/Objects/dsac.hex` |
| HEX file size | 14,408 bytes (HEX text representation) |
| Flash programmed | 4,786 bytes |
| Flash verification | Verified by SN-LINK |

## Exact Production Source Identity

The final candidate was built directly from the exact Git commit listed above with a clean working tree.

The following production source files were checked by Git blob hash at the final candidate:

```text
firmware/app/main.c
firmware/config/board_config.h
firmware/drivers/button.c
firmware/drivers/display.c
firmware/drivers/buzzer.c
firmware/drivers/eeprom.c
firmware/platform/gpio.c
firmware/platform/timer.c
```

## Final Build Composition

The clean production build included:

```text
STARTUP.A51
OPTIONS_SN8F5708.A51

Application:
- main.c
- clock.c
- fsm.c
- alarm.c

Drivers:
- button.c
- buzzer.c
- display.c
- led.c
- eeprom.c

Platform:
- board.c
- gpio.c
- timer.c
- i2c.c
```

## Verification Status

The final candidate is associated with the project's staged integration flow:

```text
INT01 → INT02 → INT03 → INT04 → INT05 → INT06
                         ↓
                     BATON
                     05b05a5
                         ↓
INT07 → INT08 → INT09 → INT10 → INT11 → INT12
                         ↓
                     INT13
                         ↓
                     INT14
```

### Current release evidence

- Final source identity: CLOSED
- Final clean build provenance: CLOSED
- Final HEX generation: CLOSED
- Flash / verify: CLOSED
- INT11 / INT12: reported PASS
- INT13 / INT14: reported PASS

## Warning Classification

The final clean build reported four linker warnings:

```text
L16: ?PR?_BUTTON_ISPRESSED?BUTTON
L16: ?PR?BUZZER_ISBUSY?BUZZER
L16: ?PR?DISPLAY_CLEAR?DISPLAY
L16: ?PR?EEPROM_INIT?EEPROM
```

The four remaining L16 warnings were reviewed and classified as intentional/non-blocking for the final release.

## Release Freeze

From this point onward, the final candidate should be treated as a release baseline.

Recommended policy:

```text
NO feature additions
NO unrelated refactoring
NO driver branch replacement
NO source changes without explicit release justification
```

## Release Status

```text
FINAL CANDIDATE: 1ecf0b4
FINAL PROGRAM SIZE: 4786 bytes
BUILD: 0 errors / 4 warnings
FLASH: 4786 bytes programmed and verified

STATUS: FINAL RELEASE — SIGNED OFF
```

## Notes

`4642 bytes` belongs to an earlier INT07 intermediate build. The exact final clean rebuild associated with candidate `1ecf0b4` is `4786 bytes`.

This document records the final release identity and should be updated only when the release baseline itself changes.

---

### Final Release Promotion

- Verified candidate: `1ecf0b4b1bfc60bebf6ea378d640378528d91d12`
- Release tag: `dsac-mcu-final-2026`
- Verified source tree: `e1e272b1c322c6482530993b3637200be363de70`
- Promoted to `main` through PR #12
- Final main merge commit: `0f3fa72`
- Hardware smoke test: **PASS**

### Final Demonstration

- YouTube Demo: https://youtu.be/f7Rsa1xx3BQ
- Hardware Platform: SONiX SN8F5708 EVK
- Verified Candidate: `1ecf0b4b1bfc60bebf6ea378d640378528d91d12`
- Release Tag: `dsac-mcu-final-2026`
- Hardware Smoke Test: **PASS**
