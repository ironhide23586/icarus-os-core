---
inclusion: always
---

# ICARUS OS — Project Steering

## Overview
ICARUS is a preemptive RTOS for STM32H750VBT6 (Cortex-M7, 480MHz, 64KB ITCM, 128KB DTCM, 512KB RAM_D1, 128KB internal flash, 8MB QSPI flash).

## Branch & Workflow
- Working branch: `feature/usb-enumeration-fix`
- Build: `make rebuild` from `build/` directory
- Tests: `make -C tests` (Unity, 129 tests, host-compiled)
- Flash + terminal: `./build/rebuild_and_flash.sh` → physical reset → `./icarus_terminal.sh`
- `hal_init()` must stay in `main.c`

## Fault Blink Codes (LED E3)
| Blinks | Fault |
|--------|-------|
| 3 | HardFault |
| 4 | MemManage |
| 5 | BusFault |
| 6 | UsageFault |

## Key Constraints
- Do NOT make large changes then debug — add features iteratively, test after each step
- `MPU_ENABLE_ATTACK_TEST` must stay `0` until MPU is fully working
- Spinning functions (`semaphore_feed/consume`, `pipe_enqueue/dequeue`, `task_busy_wait`, `task_blocking_sleep`) CANNOT run inside the SVC handler — PendSV is lower priority and cannot preempt SVC, so the scheduler would deadlock
- `__func` implementations CAN call public SVC wrappers — they only run from thread mode

## Section Placement Macros (config.h)
| Macro | Section | MPU |
|-------|---------|-----|
| `ITCM_FUNC_PRIV` | `.itcm_priv` | Priv-only RO+exec |
| `ITCM_FUNC` | `.itcm` (→ `.itcm_user`) | Priv+User RO+exec |
| `DTCM_DATA_PRIV` | `.dtcm_priv` | Priv+User RW (for now) |
| `DTCM_DATA` | `.dtcm` | Priv+User RW |

## ARM ABI Notes
- `arm-none-eabi-gcc` uses `Tag_ABI_enum_size: small` by default — enums are 1 byte if values fit
- This means `icarus_task_state_t` and `icarus_task_priority_t` are 1 byte each
- `icarus_task_t.name` is at offset **33** (not 36) because `task_priority` is 1 byte at offset 32
- Always verify struct offsets with `arm-none-eabi-readelf -A` and cross-check against `context_switch.s` hardcoded offsets
