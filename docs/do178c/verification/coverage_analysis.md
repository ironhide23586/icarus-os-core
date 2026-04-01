# Structural Coverage Analysis Report

**Document ID:** ICARUS-VER-001  
**Version:** 0.2  
**Date:** 2026-04-01  
**Status:** Draft  
**Classification:** Public (Open Source)  

## 1. Purpose

This document provides structural coverage analysis for the ICARUS OS kernel and BSP software in accordance with DO-178C objectives A-7 (Structural Coverage Analysis).

## 2. Scope

This analysis covers host-based unit testing of kernel and BSP sources under `Core/Src/`, driven by the `tests/` Unity suite.

| Component | Source path | Description |
|-----------|-------------|-------------|
| Kernel core | `Core/Src/icarus/kernel.c` | Initialization, critical sections, lifecycle |
| Scheduler | `Core/Src/icarus/scheduler.c` | Preemption and scheduling |
| Tasks | `Core/Src/icarus/task.c` | Task control blocks, yields, sleep |
| SVC | `Core/Src/icarus/svc.c` | Supervisor call dispatch |
| Semaphores | `Core/Src/icarus/semaphore.c` | Counting semaphores |
| Pipes | `Core/Src/icarus/pipe.c` | Byte-stream IPC |
| Display BSP | `Core/Src/bsp/display.c` | Terminal / GUI helpers |
| I/O BSP | `Core/Src/bsp/retarget_stdio.c` | Stdio retarget |
| HAL BSP | `Core/Src/bsp/retarget_hal.c` | HAL glue for tests |
| Interrupts | `Core/Src/bsp/stm32h7xx_it.c` | Vector stubs / fault handlers |
| Error | `Core/Src/bsp/error.c` | `Error_Handler` |

**Out of scope (host-unobservable or third-party):**
- `Core/Src/icarus/context_switch.s` — ARM assembly (target integration)
- STM32 HAL, USB stack, CMSIS — vendor or board-integration code

## 3. Coverage Metrics

### 3.1 Summary

Figures below were produced with `cd tests && make coverage-summary` on 2026-04-01 (macOS, Homebrew `lcov`, after excluding mocks, `unity.c`, and `test_task.c` per the project Makefile filters).

| Metric | Achieved | Required (DAL C) | Required (DAL B) | Required (DAL A) |
|--------|----------|------------------|------------------|------------------|
| Statement Coverage | 91.1% | 100%* | 100% | 100% |
| Decision Coverage | TBD | - | 100% | 100% |
| MC/DC Coverage | TBD | - | - | 100% |

*With justified deactivated/dead code exclusions

### 3.2 Per-file coverage (instrumented lines)

| File | Lines hit / total | Line % | Function % |
|------|-------------------|--------|------------|
| `icarus/task.c` | 59/59 | 100% | 100% |
| `icarus/pipe.c` | 85/91 | 93.4% | 100% |
| `icarus/svc.c` | 83/89 | 93.3% | 92.5% |
| `icarus/semaphore.c` | 64/68 | 94.1% | 100% |
| `icarus/kernel.c` | 75/88 | 85.2% | 84.6% |
| `icarus/scheduler.c` | 34/42 | 81.0% | 100% |
| `bsp/display.c` | 220/233 | 94.4% | 100% |
| `bsp/retarget_hal.c` | 70/74 | 94.6% | 100% |
| `bsp/retarget_stdio.c` | 9/12 | 75.0% | 100% |
| `bsp/stm32h7xx_it.c` | 15/24 | 62.5% | 40.0% |
| `bsp/error.c` | 0/4 | 0.0% | 0.0% |
| **Total (filtered)** | **714/784** | **91.1%** | **89.5%** |

### 3.3 Partially covered or deactivated areas

See `deactivated_code.md` for fault handlers, infinite-loop tasks, and similar items. Host coverage gaps remain in `error.c` and fault-vector paths in `stm32h7xx_it.c` by design.

## 4. Test Environment

### 4.1 Host test configuration

- **Host OS:** macOS (darwin)
- **Compiler:** GCC (host native)
- **Test framework:** Unity
- **Coverage:** lcov / gcov
- **Build system:** GNU Make (`tests/Makefile`)

### 4.2 Commands

```bash
cd tests
make clean test          # Run all tests
make coverage-summary    # Regenerate coverage.info and print summary
make coverage-html       # HTML under tests/build/coverage/html/
```

### 4.3 Test results

Re-run after changes. Example command:

```bash
cd tests && make test
```

The suite defines **140** tests; record pass/fail counts from your baseline run in verification records.

## 5. Coverage gap analysis

### 5.1 Justified exclusions

As documented in `deactivated_code.md`:

1. **Fault handlers** — Run only on hardware faults; infinite loops by design.  
2. **Certain interrupt paths** — Require target or HW stubs beyond current mocks.  
3. **Assembly** — Context switch not executed on the host.  

### 5.2 Certification follow-up

- Maintain this report by re-running `make coverage-summary` after substantive kernel or BSP changes.  
- For DAL B/A, add decision and MC/DC tooling on top of statement coverage.

## 6. Recommendations

### 6.1 For DAL C certification
- Keep deactivated-code analysis aligned with coverage exclusions.  
- Archive `coverage.info` / HTML for each baseline.

### 6.2 For DAL B certification
- Add decision-coverage tooling and target-based tests for loop tasks.

### 6.3 For DAL A certification
- Plan MC/DC evidence where required by the certification authority.

## 7. Approval

| Role | Name | Signature | Date |
|------|------|-----------|------|
| Author | | | |
| Reviewer | | | |
| QA | | | |

## 8. Revision history

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 0.1 | 2025-01-26 | Souham Biswas | Initial draft |
| 0.2 | 2026-04-01 | Souham Biswas | Paths `Core/Src/icarus/`; refreshed metrics from host `lcov` |
