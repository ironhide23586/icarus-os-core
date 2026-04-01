# Software Development Plan (SDP)

**Document ID:** ICARUS-SDP-001  
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

This Software Development Plan (SDP) defines the processes, methods, standards, and tools used to develop ICARUS OS. It establishes the framework for producing certifiable software that meets DO-178C DAL C objectives.

### 1.2 Scope

This plan covers development of:
- ICARUS OS Kernel (scheduler, task management, IPC)
- Board Support Package (BSP) for supported hardware
- AI Runtime Environment (deterministic inference subsystem)
- Verification and validation infrastructure

### 1.3 ICARUS Vision

ICARUS (Intelligent Certifiable Autonomous Real-time Unified System) is designed to be the first open-source RTOS with:
- **Native AI integration** with certifiable determinism
- **DO-178C alignment by design** (not retrofitted)
- **Hardware-agnostic portability** across safety-critical platforms
- **Formal verification readiness** for DAL A applications

---

## 2. Development Organization

### 2.1 Roles and Responsibilities

| Role | Responsibilities |
|------|------------------|
| Project Lead | Technical direction, architecture decisions, certification liaison |
| Kernel Developer | Scheduler, task management, memory management, IPC |
| BSP Developer | Hardware abstraction, drivers, platform ports |
| AI Runtime Developer | Inference engine, model loading, deterministic execution |
| Verification Engineer | Test development, coverage analysis, static analysis |
| Configuration Manager | Version control, release management, CI/CD |
| Quality Assurance | Process compliance, audits, documentation review |

### 2.2 Development Teams

```
┌─────────────────────────────────────────────────────────────┐
│                    ICARUS Development                        │
├─────────────────┬─────────────────┬─────────────────────────┤
│   Core Team     │   Platform Team │   Verification Team     │
│                 │                 │                         │
│ • Kernel        │ • STM32 BSP     │ • Unit Testing          │
│ • Scheduler     │ • RISC-V BSP    │ • Integration Testing   │
│ • IPC           │ • x86 Sim       │ • Coverage Analysis     │
│ • AI Runtime    │ • FPGA BSP      │ • Static Analysis       │
└─────────────────┴─────────────────┴─────────────────────────┘
```


---

## 3. Software Life Cycle

### 3.1 Development Model

ICARUS uses a **hybrid iterative-incremental model** optimized for certifiable development:

```
Phase 1: Foundation (Current)
├── Preemptive scheduler ✅
├── Task management ✅
├── Basic BSP ✅
└── Host-based testing ✅

Phase 2: Hardening (Q2 2026)
├── Memory protection (MPU)
├── Stack overflow detection
├── Watchdog integration
└── Fault recovery

Phase 3: Communication (Q3 2026)
├── Inter-process communication (IPC)
├── Message queues
├── Semaphores/Mutexes
├── Event flags

Phase 4: AI Runtime (Q4 2026)
├── Deterministic inference engine
├── Fixed-point neural network support
├── Model verification framework
├── WCET-bounded execution

Phase 5: Advanced Features (2027)
├── Multi-core support (AMP/SMP)
├── Hypervisor mode
├── Time/Space partitioning (ARINC 653)
├── Formal verification integration
```

### 3.2 Iteration Cycle

Each development iteration follows:

```
┌──────────┐    ┌──────────┐    ┌──────────┐    ┌──────────┐
│ Planning │───▶│  Design  │───▶│  Coding  │───▶│  Review  │
│ (1 day)  │    │ (2 days) │    │ (5 days) │    │ (2 days) │
└──────────┘    └──────────┘    └──────────┘    └──────────┘
                                                      │
     ┌────────────────────────────────────────────────┘
     ▼
┌──────────┐    ┌──────────┐    ┌──────────┐
│  Testing │───▶│ Coverage │───▶│ Release  │
│ (3 days) │    │ (1 day)  │    │ (1 day)  │
└──────────┘    └──────────┘    └──────────┘
```

**Iteration Duration:** 2 weeks  
**Release Cadence:** Monthly (feature releases), Weekly (patches)

---

## 4. Development Standards

### 4.1 Coding Standards

ICARUS follows a subset of MISRA C:2012 appropriate for RTOS development:

| Category | Rules | Compliance |
|----------|-------|------------|
| Mandatory | All 10 rules | Required |
| Required | 121 rules (subset) | Required with deviation |
| Advisory | Selected rules | Recommended |

**Key Standards:**
- No dynamic memory allocation after initialization
- No recursion in kernel code
- All functions shall have explicit return types
- All variables shall be initialized before use
- No pointer arithmetic except for buffer management
- Maximum function complexity: 15 (cyclomatic)
- Maximum nesting depth: 4 levels

### 4.2 Naming Conventions

```c
// Functions: module_action_object
void os_create_task(task_t *task, ...);
void ipc_send_message(queue_t *queue, ...);
void ai_run_inference(model_t *model, ...);

// Types: name_t for structs, NAME_E for enums
typedef struct task task_t;
typedef enum task_state TASK_STATE_E;

// Constants: UPPER_CASE
#define MAX_TASKS 16
#define STACK_SIZE_WORDS 512

// Global variables: module_name (minimize usage)
volatile uint32_t os_tick_count;

// Static variables: s_name or module-scoped
static uint8_t s_task_count;
```

### 4.3 File Organization

```
Core/
├── Inc/
│   ├── icarus/             # RTOS public and internal headers
│   │   ├── icarus.h        # Umbrella include
│   │   ├── kernel.h        # Kernel state, os_init/os_start
│   │   ├── task.h          # Task management API
│   │   ├── scheduler.h     # Scheduler internals
│   │   ├── semaphore.h
│   │   ├── pipe.h
│   │   ├── svc.h
│   │   ├── config.h
│   │   └── types.h
│   └── bsp/                # Board support headers
├── Src/
│   ├── icarus/
│   │   ├── kernel.c
│   │   ├── scheduler.c
│   │   ├── task.c
│   │   ├── semaphore.c
│   │   ├── pipe.c
│   │   ├── svc.c
│   │   └── context_switch.s
│   └── bsp/
└── Startup/
    └── startup_stm32h750vbtx.s   # (example: current target)
```


---

## 5. Target Hardware Platforms

### 5.1 Supported Platforms

| Platform | Status | DAL Target | Use Case |
|----------|--------|------------|----------|
| **STM32H7** (Cortex-M7) | ✅ Primary | DAL C | UAV flight controllers, sensors |
| **STM32F4** (Cortex-M4) | Planned | DAL D | Cost-sensitive applications |
| **RISC-V** (RV32IMAC) | Planned | DAL C | Open hardware initiatives |
| **Xilinx Zynq** (Cortex-A9 + FPGA) | Planned | DAL B | AI acceleration, radar |
| **x86-64** (QEMU) | ✅ Simulation | N/A | Development and CI testing |

### 5.2 Hardware Abstraction Strategy

```
┌─────────────────────────────────────────────────────────────┐
│                    ICARUS Kernel API                         │
├─────────────────────────────────────────────────────────────┤
│                 Platform Abstraction Layer                   │
│  ┌─────────────────────────────────────────────────────┐    │
│  │ icarus_hal.h - Unified HAL interface                │    │
│  │ • icarus_hal_init()                                 │    │
│  │ • icarus_hal_get_tick()                             │    │
│  │ • icarus_hal_enter_critical()                       │    │
│  │ • icarus_hal_context_switch()                       │    │
│  └─────────────────────────────────────────────────────┘    │
├──────────┬──────────┬──────────┬──────────┬─────────────────┤
│ STM32H7  │ STM32F4  │ RISC-V   │  Zynq    │  x86 (Sim)      │
│ BSP      │ BSP      │ BSP      │  BSP     │  BSP            │
└──────────┴──────────┴──────────┴──────────┴─────────────────┘
```

### 5.3 Memory Map (STM32H7 Reference)

| Region | Address | Size | Usage |
|--------|---------|------|-------|
| ITCM | 0x00000000 | 64 KB | Critical code (scheduler) |
| DTCM | 0x20000000 | 128 KB | Task stacks, kernel data |
| AXI SRAM | 0x24000000 | 512 KB | Application data, AI models |
| SRAM1-4 | 0x30000000 | 288 KB | DMA buffers, frame buffers |
| Flash | 0x08000000 | 128 KB | Code (+ external QSPI) |

---

## 6. Development Tools

### 6.1 Toolchain

| Tool | Version | Purpose | Qualification |
|------|---------|---------|---------------|
| **ARM GCC** | 12.x+ | Cross-compilation | Not required (development) |
| **LLVM/Clang** | 17.x+ | Static analysis, compilation | Not required |
| **GNU Make** | 4.x | Build automation | Not required |
| **CMake** | 3.20+ | Cross-platform builds | Not required |
| **Ninja** | 1.11+ | Fast builds | Not required |

### 6.2 Verification Tools

| Tool | Purpose | TQL | Qualification Status |
|------|---------|-----|---------------------|
| **Unity** | Unit testing | TQL-5 | Criteria-based |
| **lcov/gcov** | Coverage analysis | TQL-5 | Criteria-based |
| **cppcheck** | Static analysis | TQL-5 | Criteria-based |
| **PC-lint Plus** | MISRA checking | TQL-4 | Planned |
| **Polyspace** | Formal analysis | TQL-4 | Planned (DAL B) |

### 6.3 AI Development Tools

| Tool | Purpose | Notes |
|------|---------|-------|
| **TensorFlow Lite Micro** | Model conversion | Reference implementation |
| **ONNX Runtime** | Model interchange | Planned support |
| **ICARUS Model Compiler** | Deterministic code gen | Custom tool (planned) |
| **WCET Analyzer** | Timing verification | Integration planned |

### 6.4 CI/CD Pipeline

```yaml
# .github/workflows/ci.yml (conceptual)
Pipeline:
  Build:
    - ARM GCC compilation (all platforms)
    - Static analysis (cppcheck, MISRA subset)
    - Compiler warnings as errors
  
  Test:
    - Host-based unit tests (Unity)
    - Coverage collection (lcov)
    - Coverage threshold enforcement (>80%)
  
  Quality:
    - MISRA compliance report
    - Complexity metrics
    - Documentation generation
  
  Release:
    - Semantic versioning
    - Binary artifacts
    - Release notes generation
```


---

## 7. AI Runtime Architecture

### 7.1 Design Philosophy

ICARUS integrates AI as a **first-class citizen** while maintaining certifiable determinism:

```
┌─────────────────────────────────────────────────────────────┐
│                    ICARUS AI Architecture                    │
├─────────────────────────────────────────────────────────────┤
│  Design Principles:                                          │
│  • Deterministic execution (bounded WCET)                   │
│  • No dynamic memory allocation during inference            │
│  • Fixed-point arithmetic (no floating-point in kernel)     │
│  • Verifiable model structure (whitelist operators)         │
│  • Isolated execution (AI task sandboxing)                  │
└─────────────────────────────────────────────────────────────┘
```

### 7.2 AI Subsystem Components

```
┌─────────────────────────────────────────────────────────────┐
│                    Application Layer                         │
│         ai_predict(), ai_classify(), ai_detect()            │
├─────────────────────────────────────────────────────────────┤
│                    AI Runtime API                            │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────┐  │
│  │Model Loader │  │  Inference  │  │  Result Handler     │  │
│  │             │  │   Engine    │  │                     │  │
│  │• Validation │  │• Layer exec │  │• Output scaling     │  │
│  │• Allocation │  │• Activation │  │• Confidence calc    │  │
│  │• Integrity  │  │• Pooling    │  │• Decision logic     │  │
│  └─────────────┘  └─────────────┘  └─────────────────────┘  │
├─────────────────────────────────────────────────────────────┤
│                 Certified Operator Library                   │
│  ┌───────┐ ┌───────┐ ┌───────┐ ┌───────┐ ┌───────────────┐  │
│  │Conv2D │ │Dense  │ │ReLU   │ │Softmax│ │ MaxPool       │  │
│  │(int8) │ │(int8) │ │(int8) │ │(int16)│ │ (int8)        │  │
│  └───────┘ └───────┘ └───────┘ └───────┘ └───────────────┘  │
├─────────────────────────────────────────────────────────────┤
│                    Hardware Acceleration                     │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────┐  │
│  │  CMSIS-NN   │  │  FPGA Accel │  │  NPU (future)       │  │
│  │  (Cortex-M) │  │  (Zynq)     │  │  (dedicated)        │  │
│  └─────────────┘  └─────────────┘  └─────────────────────┘  │
└─────────────────────────────────────────────────────────────┘
```

### 7.3 Certifiable AI Constraints

| Constraint | Rationale | Implementation |
|------------|-----------|----------------|
| **Operator Whitelist** | Only verified operators allowed | Compile-time model validation |
| **Fixed Tensor Sizes** | No dynamic shapes | Static allocation at load |
| **Bounded Execution** | WCET must be calculable | Per-layer timing analysis |
| **No Recursion** | Stack depth predictable | Iterative implementations |
| **Integer Arithmetic** | Deterministic across platforms | int8/int16 quantization |
| **Model Integrity** | Prevent tampering | CRC/hash verification |

### 7.4 AI Use Cases

| Application | Model Type | Latency Target | DAL |
|-------------|------------|----------------|-----|
| Anomaly detection | Autoencoder | <10ms | D |
| Sensor fusion | MLP | <5ms | C |
| Object detection | TinyYOLO | <50ms | C |
| Flight envelope protection | Decision tree | <1ms | B |
| Predictive maintenance | LSTM (simplified) | <100ms | D |

### 7.5 AI Certification Strategy

Per DO-178C and emerging guidance (EASA AI Roadmap 2.0):

1. **Model as Software Item:** Trained model treated as derived requirement
2. **Training Data Traceability:** Dataset versioning and provenance
3. **Verification of Learning:** Test coverage of input space
4. **Runtime Monitoring:** Output bounds checking, confidence thresholds
5. **Fallback Behavior:** Graceful degradation when AI uncertain


---

## 8. Differentiation from Competitors

### 8.1 Competitive Landscape

| Feature | ICARUS | PX4/NuttX | LynxOS | VxWorks | FreeRTOS |
|---------|--------|-----------|--------|---------|----------|
| Open Source | ✅ | ✅ | ❌ | ❌ | ✅ |
| DO-178C Ready | ✅ | ❌ | ✅ | ✅ | ❌ |
| Native AI Runtime | ✅ | ❌ | ❌ | Partial | ❌ |
| Certifiable AI | ✅ | ❌ | ❌ | ❌ | ❌ |
| ARINC 653 | Planned | ❌ | ✅ | ✅ | ❌ |
| Formal Verification | Planned | ❌ | Partial | Partial | ❌ |
| Multi-core | Planned | ✅ | ✅ | ✅ | ✅ |
| RISC-V Support | Planned | ✅ | ❌ | Planned | ✅ |

### 8.2 ICARUS Unique Value

```
┌─────────────────────────────────────────────────────────────┐
│                 ICARUS Differentiators                       │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│  1. CERTIFICATION-FIRST DESIGN                              │
│     • DO-178C compliance built-in, not bolted-on            │
│     • Traceability from day one                             │
│     • Test infrastructure as core component                 │
│                                                              │
│  2. AI-NATIVE ARCHITECTURE                                  │
│     • First RTOS with certifiable AI runtime                │
│     • Deterministic inference with WCET guarantees          │
│     • Model verification framework                          │
│                                                              │
│  3. OPEN CORE MODEL                                         │
│     • Community-driven development                          │
│     • Transparent certification artifacts                   │
│     • Vendor-neutral hardware support                       │
│                                                              │
│  4. MODERN TOOLING                                          │
│     • CI/CD with coverage gates                             │
│     • Automated MISRA checking                              │
│     • Containerized build environment                       │
│                                                              │
└─────────────────────────────────────────────────────────────┘
```

### 8.3 Target Markets

| Market | Requirements | ICARUS Fit |
|--------|--------------|------------|
| **UAV/Drones** | Low cost, AI, DAL C/D | Primary target |
| **Urban Air Mobility** | DO-178C, AI, DAL B | Future target |
| **Space** | Radiation tolerance, formal | Future target |
| **Automotive** | ISO 26262, AI | Adaptation possible |
| **Industrial** | IEC 61508, determinism | Adaptation possible |

---

## 9. Review Process

### 9.1 Review Types

| Review Type | Participants | Timing | Output |
|-------------|--------------|--------|--------|
| **Requirements Review** | Lead, QA | After SRS update | Review checklist |
| **Design Review** | Lead, Developers | Before coding | Design approval |
| **Code Review** | 2+ developers | Before merge | PR approval |
| **Test Review** | Verification, QA | After test creation | Test approval |
| **Coverage Review** | Verification, Lead | After testing | Coverage report |

### 9.2 Code Review Checklist

- [ ] Follows coding standards (naming, formatting)
- [ ] No MISRA violations (or documented deviations)
- [ ] All functions have documentation
- [ ] No dynamic memory allocation (post-init)
- [ ] Error handling for all failure paths
- [ ] Unit tests provided with >80% coverage
- [ ] No compiler warnings
- [ ] Traceability to requirement documented

### 9.3 Independence Requirements

Per DO-178C for DAL C:
- Code author cannot be sole reviewer
- Test author cannot be sole test executor (for integration tests)
- QA independent from development

---

## 10. Problem Reporting

### 10.1 Problem Report Categories

| Category | Severity | Response Time |
|----------|----------|---------------|
| **Safety** | Critical | 24 hours |
| **Functional** | Major | 1 week |
| **Performance** | Minor | 2 weeks |
| **Documentation** | Low | Next release |

### 10.2 Problem Report Workflow

```
┌──────────┐    ┌──────────┐    ┌──────────┐    ┌──────────┐
│ Reported │───▶│ Analyzed │───▶│  Fixed   │───▶│ Verified │
└──────────┘    └──────────┘    └──────────┘    └──────────┘
                     │                               │
                     ▼                               ▼
              ┌──────────┐                    ┌──────────┐
              │ Deferred │                    │  Closed  │
              └──────────┘                    └──────────┘
```

---

## Appendix A: Coding Standard Deviations

| Rule | Deviation | Rationale | Approval |
|------|-----------|-----------|----------|
| MISRA 11.3 | Pointer cast in context switch | Required for stack manipulation | Pending |
| MISRA 17.7 | Unused return value (printf) | Debug output only | Pending |

---

*End of Document*
