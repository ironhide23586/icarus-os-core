# Software Verification Plan (SVP)

**Document ID:** ICARUS-SVP-001  
**Version:** 0.1  
**Date:** 2025-01-26  
**Status:** Draft  
**Classification:** Public (Open Source)  

---

## Document Control

| Role | Name | Date | Signature |
|------|------|------|-----------|
| Author | | | |
| Reviewer | | | |
| Approver | | | |

---

## Revision History

| Version | Date | Author | Description |
|---------|------|--------|-------------|
| 0.1 | 2025-01-26 | Souham Biswas | Initial draft |

---

## 1. Introduction

### 1.1 Purpose

This Software Verification Plan (SVP) defines the verification strategy, methods, and procedures for ICARUS OS. It establishes how software requirements are verified and how structural coverage objectives are achieved per DO-178C DAL C.

### 1.2 Scope

This plan covers verification of:
- ICARUS OS Kernel
- Board Support Package (BSP)
- AI Runtime Environment
- Integration with target hardware

### 1.3 Verification Objectives

| DO-178C Table | Objective | Applicable |
|---------------|-----------|------------|
| A-3 | Verify HLR compliance with system requirements | Yes |
| A-4 | Verify LLR compliance with HLR | Yes |
| A-5 | Verify source code compliance with LLR | Yes |
| A-6 | Verify executable object code | Yes |
| A-7 | Verify test coverage | Yes |

---

## 2. Verification Strategy

### 2.1 Multi-Level Verification

```
┌─────────────────────────────────────────────────────────────┐
│                 ICARUS Verification Pyramid                  │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│                    ┌─────────────┐                          │
│                    │   System    │  Target hardware         │
│                    │   Tests     │  End-to-end scenarios    │
│                    └──────┬──────┘                          │
│                   ┌───────┴───────┐                         │
│                   │  Integration  │  Component interaction  │
│                   │    Tests      │  Hardware-in-loop       │
│                   └───────┬───────┘                         │
│              ┌────────────┴────────────┐                    │
│              │      Unit Tests         │  Function-level    │
│              │   (Host + Target)       │  Branch coverage   │
│              └────────────┬────────────┘                    │
│         ┌─────────────────┴─────────────────┐               │
│         │        Static Analysis            │  MISRA, lint  │
│         │     (Continuous Integration)      │  Complexity   │
│         └─────────────────┬─────────────────┘               │
│    ┌──────────────────────┴──────────────────────┐          │
│    │              Code Reviews                    │  Manual  │
│    │         (Every Pull Request)                │  Inspect │
│    └─────────────────────────────────────────────┘          │
│                                                              │
└─────────────────────────────────────────────────────────────┘
```

### 2.2 Verification Methods

| Method | Description | When Used |
|--------|-------------|-----------|
| **Review** | Manual inspection of artifacts | Requirements, design, code |
| **Analysis** | Automated examination | Static analysis, coverage |
| **Testing** | Dynamic execution | Unit, integration, system |
| **Formal Methods** | Mathematical proof | Critical algorithms (planned) |


---

## 3. Test Environment

### 3.1 Host-Based Testing

**Purpose:** Rapid iteration, high coverage, CI integration

| Component | Tool | Version |
|-----------|------|---------|
| Compiler | GCC (native) | System |
| Test Framework | Unity | 2.5.x |
| Coverage | lcov/gcov | 2.x |
| Mocking | Custom mocks | - |
| CI Runner | GitHub Actions | - |

**Capabilities:**
- ✅ Kernel logic testing
- ✅ Algorithm verification
- ✅ Branch coverage measurement
- ❌ Hardware interaction
- ❌ Timing verification
- ❌ Interrupt behavior

### 3.2 Target-Based Testing

**Purpose:** Hardware verification, timing analysis, integration

| Component | Tool | Version |
|-----------|------|---------|
| Compiler | ARM GCC | 12.x |
| Debug Probe | ST-Link / J-Link | - |
| Trace | ITM/SWO | - |
| Logic Analyzer | Saleae / similar | - |
| Serial Monitor | USB CDC | - |

**Capabilities:**
- ✅ Real hardware execution
- ✅ Interrupt handling
- ✅ Timing measurement
- ✅ Peripheral interaction
- ✅ Context switch verification

### 3.3 Simulation Testing

**Purpose:** CI integration for target code, fault injection

| Component | Tool | Purpose |
|-----------|------|---------|
| QEMU | ARM Cortex-M | Instruction-level simulation |
| Renode | Multi-platform | Peripheral simulation |
| Custom Sim | x86 BSP | Fast iteration |

### 3.4 AI Model Testing

**Purpose:** Verify AI inference correctness and timing

| Test Type | Method | Criteria |
|-----------|--------|----------|
| Accuracy | Golden reference comparison | <0.1% deviation |
| Timing | WCET measurement | Within budget |
| Robustness | Input perturbation | Bounded output |
| Integrity | Model hash verification | Match expected |

---

## 4. Test Categories

### 4.1 Unit Tests

**Scope:** Individual functions and modules

**Location:** `tests/src/test_*.c`

**Naming Convention:**
```c
void test_<module>_<function>_<scenario>(void);

// Examples:
void test_task_create_normal(void);
void test_task_create_max_tasks(void);
void test_scheduler_yield_blocked_task(void);
void test_ai_inference_simple_model(void);
```

**Coverage Target:** 100% statement coverage (testable code)

### 4.2 Integration Tests

**Scope:** Component interactions

**Categories:**

| Category | Description | Environment |
|----------|-------------|-------------|
| Kernel Integration | Scheduler + Task + IPC | Host + Target |
| BSP Integration | Kernel + Hardware drivers | Target only |
| AI Integration | AI Runtime + Kernel | Target only |
| Full Stack | Application + Kernel + BSP | Target only |

### 4.3 System Tests

**Scope:** End-to-end scenarios

**Test Scenarios:**

| Scenario | Description | Pass Criteria |
|----------|-------------|---------------|
| Boot sequence | Cold start to running | <100ms to first task |
| Task stress | MAX_TASKS concurrent | No crashes, fair scheduling |
| Sleep accuracy | Various sleep durations | ±1 tick accuracy |
| Watchdog recovery | Induced hang | Reset and recovery |
| AI inference | Model execution | Correct output, within WCET |

### 4.4 Robustness Tests

**Scope:** Error handling and edge cases

| Test | Input | Expected Behavior |
|------|-------|-------------------|
| Invalid task pointer | NULL | Return error, no crash |
| Stack overflow | Deep recursion | Detection, controlled halt |
| Invalid priority | Out of range | Clamp or reject |
| Corrupted model | Bad CRC | Reject, fallback |

### 4.5 Memory Protection Tests

**Scope:** MPU configuration and fault handling

**Purpose:** Verify memory protection mechanisms prevent unauthorized access

| Test | Attack Vector | Expected Behavior | Verifies |
|------|---------------|-------------------|----------|
| `mpu_redteam_task` | Write to another task's data region | MemManage fault, task continues | HLR-KRN-071 |
| `mpu_itcm_write_test` | Write to ITCM (code region) | MemManage fault, task continues | HLR-KRN-066 |
| `mpu_dtcm_attack_task` | Read from DTCM (kernel data) | MemManage fault, task continues | HLR-KRN-070 |
| `mpu_kernel_bypass_test` | Direct call to `__os_*` function | MemManage fault when accessing DTCM | HLR-KRN-070 |

**Test Execution:**
- Tests run continuously as stress test tasks
- Real-time status displayed via USB CDC terminal
- Fault count monitored to verify protection active
- Pass criteria: Fault count increases on each attack attempt

**Verification Evidence:**
```
MPU_VICTIM: allocs=3 verifies=7 corruptions=0 [PASS]
MPU_REDTEAM: attacks=26 own_data=OK [PROTECTED]
MPU_ITCM_WR: attempts=4 faults=27 [WRITE PROTECTED]
MPU_DTCM: attempts=4 faults=28 [DTCM PROTECTED]
MPU_BYPASS: attempts=4 faults=30 [DTCM PROTECTED]
```

**Fault Recovery Verification:**
- MemManage handler advances PC past faulting instruction
- Task continues execution after fault
- Fault address and PC captured for analysis
- System remains stable after multiple faults


---

## 5. Structural Coverage

### 5.1 Coverage Requirements (DAL C)

| Coverage Type | Required | Tool |
|---------------|----------|------|
| Statement Coverage | 100% | lcov/gcov |
| Decision Coverage | Not required | - |
| MC/DC | Not required | - |

### 5.2 Coverage Analysis Process

```
┌──────────────┐    ┌──────────────┐    ┌──────────────┐
│  Run Tests   │───▶│   Collect    │───▶│   Generate   │
│  (Unity)     │    │   .gcda      │    │   Report     │
└──────────────┘    └──────────────┘    └──────────────┘
                                               │
                                               ▼
                                        ┌──────────────┐
                                        │   Analyze    │
                                        │    Gaps      │
                                        └──────────────┘
                                               │
                    ┌──────────────────────────┼──────────────────────────┐
                    ▼                          ▼                          ▼
             ┌──────────────┐          ┌──────────────┐          ┌──────────────┐
             │  Add Tests   │          │   Justify    │          │   Mark as    │
             │  (if needed) │          │  (dead code) │          │  Deactivated │
             └──────────────┘          └──────────────┘          └──────────────┘
```

### 5.3 Coverage Exclusions

The following code is excluded from coverage requirements:

| Category | Examples | Justification |
|----------|----------|---------------|
| Fault handlers | HardFault_Handler | Dead code - only on faults |
| Infinite loop tasks | os_idle_task | Requires target testing |
| Assembly code | context_switch.s | Not measurable on host |
| Vendor code | STM32 HAL | Third-party, not in scope |

See `ICARUS-VER-002 Deactivated Code Analysis` for complete list.

### 5.4 Current Coverage Status

| Component | Statement | Functions | Status |
|-----------|-----------|-----------|--------|
| kernel/task.c | 73.8% | 83.3% | Host tests complete |
| bsp/display.c | 83.5% | 100% | Host tests complete |
| bsp/retarget_hal.c | 96.2% | 100% | Host tests complete |
| bsp/stm32h7xx_it.c | 53.6% | 40% | Target tests needed |
| **Overall** | **79.3%** | **77.6%** | In progress |

**Adjusted (excluding deactivated):** ~100%

---

## 6. Static Analysis

### 6.1 Tools and Configuration

| Tool | Purpose | Configuration |
|------|---------|---------------|
| **cppcheck** | General static analysis | `--enable=all --std=c11` |
| **PC-lint Plus** | MISRA C:2012 | DAL C rule subset |
| **Clang-Tidy** | Modern C analysis | cert-*, bugprone-* |
| **Coverity** | Deep analysis | Planned (CI integration) |

### 6.2 MISRA Compliance

**Target:** MISRA C:2012 (DAL C subset)

| Category | Rules | Compliance |
|----------|-------|------------|
| Mandatory | 10 | 100% required |
| Required | 121 | 95%+ target |
| Advisory | 39 | Best effort |

**Deviation Process:**
1. Document deviation in code comment
2. Add to deviation log (Appendix A of SDP)
3. Review and approve deviation
4. Track in configuration management

### 6.3 Complexity Metrics

| Metric | Threshold | Action if Exceeded |
|--------|-----------|-------------------|
| Cyclomatic complexity | ≤15 | Refactor required |
| Function length | ≤100 lines | Refactor recommended |
| Nesting depth | ≤4 levels | Refactor required |
| Parameters | ≤6 | Consider struct |


---

## 7. AI Verification

### 7.1 AI-Specific Verification Challenges

| Challenge | ICARUS Approach |
|-----------|-----------------|
| Non-determinism | Fixed-point only, no floating-point |
| Opacity (black box) | Operator whitelist, layer-by-layer verification |
| Training data | Out of scope (model provided as input) |
| Edge cases | Input space coverage analysis |
| Timing variability | WCET analysis per layer |

### 7.2 AI Verification Methods

```
┌─────────────────────────────────────────────────────────────┐
│                 AI Verification Framework                    │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│  1. MODEL VALIDATION (Pre-deployment)                       │
│     ├── Structure verification (whitelist operators)        │
│     ├── Size verification (fits in memory)                  │
│     ├── Integrity verification (CRC/hash)                   │
│     └── Quantization verification (int8/int16 only)         │
│                                                              │
│  2. INFERENCE VERIFICATION (Runtime)                        │
│     ├── Golden reference comparison                         │
│     ├── Output bounds checking                              │
│     ├── Timing measurement vs WCET budget                   │
│     └── Confidence threshold enforcement                    │
│                                                              │
│  3. OPERATOR VERIFICATION (Library)                         │
│     ├── Unit tests for each operator                        │
│     ├── Numerical accuracy tests                            │
│     ├── Boundary condition tests                            │
│     └── Overflow/underflow tests                            │
│                                                              │
└─────────────────────────────────────────────────────────────┘
```

### 7.3 AI Test Cases

| Test Category | Test Cases | Coverage |
|---------------|------------|----------|
| Operator correctness | Conv2D, Dense, ReLU, etc. | Per operator |
| Model loading | Valid, invalid, corrupted | All paths |
| Inference accuracy | Reference comparison | Representative inputs |
| Timing bounds | WCET measurement | All models |
| Memory bounds | Allocation verification | All models |

### 7.4 AI Certification Artifacts

| Artifact | Description | Status |
|----------|-------------|--------|
| Operator Library Test Report | Unit test results for all operators | Planned |
| Model Verification Report | Per-model validation results | Planned |
| WCET Analysis Report | Timing bounds for inference | Planned |
| AI Runtime Coverage Report | Code coverage for AI subsystem | Planned |

---

## 8. Test Procedures

### 8.1 Unit Test Execution

```bash
# Host-based unit tests
cd tests
make clean test              # Run all tests
make coverage-summary        # Show coverage summary
make coverage-html           # Generate HTML report

# Expected output:
# 76 Tests 0 Failures 0 Ignored
# Coverage: 79.3% lines, 77.6% functions
```

### 8.2 Integration Test Execution

```bash
# Target-based tests (requires hardware)
cd tests/integration
make flash                   # Flash test firmware
make run                     # Execute tests via serial
make collect                 # Collect results

# Or via debug probe:
arm-none-eabi-gdb -x test_script.gdb
```

### 8.3 Coverage Collection

```bash
# Collect and report coverage
cd tests
make coverage-html
open build/coverage/html/index.html

# CI enforcement
if [ $(lcov --summary coverage.info | grep lines | awk '{print $2}') -lt 80 ]; then
    echo "Coverage below threshold"
    exit 1
fi
```

---

## 9. Verification Records

### 9.1 Test Results Format

```
Test Run: ICARUS-TR-2025-01-26-001
Date: 2025-01-26
Environment: Host (macOS, GCC 14.x)
Configuration: Debug, HOST_TEST defined

Results:
  Total:    76
  Passed:   76
  Failed:   0
  Ignored:  0

Coverage:
  Lines:     79.3% (341/430)
  Functions: 77.6% (38/49)
  
Deactivated Code: 11 functions (see VER-002)
Adjusted Coverage: ~100%
```

### 9.2 Verification Matrix

| Requirement | Test Case | Result | Coverage |
|-------------|-----------|--------|----------|
| KRN-001 | test_os_init | PASS | 100% |
| KRN-002 | test_os_register_task | PASS | 100% |
| ... | ... | ... | ... |

See `ICARUS-VER-003 Test Traceability Matrix` for complete matrix.

---

## 10. Verification Schedule

| Phase | Activity | Target Date | Status |
|-------|----------|-------------|--------|
| 1 | Host unit tests | 2025-01-26 | ✅ Complete |
| 2 | Static analysis (cppcheck) | 2025-01-26 | ✅ Complete |
| 3 | MISRA analysis | TBD | Planned |
| 4 | Target unit tests | TBD | Planned |
| 5 | Integration tests | TBD | Planned |
| 6 | System tests | TBD | Planned |
| 7 | AI verification | TBD | Planned |
| 8 | Final coverage analysis | TBD | Planned |

---

*End of Document*
