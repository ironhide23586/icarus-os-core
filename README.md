# ICARUS OS Core

**Intelligent Certifiable Autonomous Real-time Unified System**

A minimal, deterministic real-time kernel for Cortex-M designed for DO-178C certification. Built from the ground up with safety-critical aerospace and defense applications in mind.

```
┌──────────────────────────────────────────────────────────────┐
│   ██╗ ██████╗  █████╗ ██████╗ ██╗   ██╗ ██████╗              │
│   ██║██╔════╝ ██╔══██╗██╔══██╗██║   ██║██╔════╝              │
│   ██║██║      ███████║██████╔╝██║   ██║╚█████╗               │
│   ██║██║      ██╔══██║██╔══██╗██║   ██║ ╚═══██╗              │
│   ██║╚██████╗ ██║  ██║██║  ██║╚██████╔╝██████╔╝              │
│   ╚═╝ ╚═════╝ ╚═╝  ╚═╝╚═╝  ╚═╝ ╚═════╝ ╚═════╝               │
│   Preemptive Kernel • ARMv7E-M • STM32H750                   │
└──────────────────────────────────────────────────────────────┘
                                                           SEM
[>ICARUS_HEARTBEAT<] ★★★★★★★★★★★★★★★★★★★★ [>ICARUS_HEARTBEAT<]                 ┌───┐
[producer]  ████████████████────────────────  160/200     │   │
[consumer]  ██████████──────────────────────  100/300     │   │
[reference] ████████████████████────────────  1200/3000   │   │
                                                          │   │
                                                          │   │
                                                          │   │
                                                          │   │
                                                          │ █ │
                                                          │ █ │
                                                          │ █ │
                                                          └───┘
                                                           3/10
```

> **⚠️ WARNING: This is work in progress and NOT production ready.**
> 
> This kernel is under active development. It may contain bugs, incomplete features, and is not suitable for use in production systems. Use at your own risk.

---

## Highlights

| Metric | Status |
|--------|--------|
| **Test Coverage** | 79.3% line, 77.6% function |
| **Unit Tests** | 83 tests, 0 failures |
| **Static Analysis** | cppcheck clean |
| **Certification Target** | DO-178C DAL C |
| **Coding Standard** | MISRA C:2012 subset |

---

## Overview

ICARUS OS is a lightweight, preemptive real-time operating system kernel designed for ARM Cortex-M7 microcontrollers (specifically STM32H750). The kernel provides deterministic task scheduling, context switching, and a clean API for embedded real-time applications.

### Vision

ICARUS is designed to be the first open-source RTOS with:
- **Native AI integration** with certifiable determinism (planned)
- **DO-178C compliance by design** (not retrofitted)
- **Hardware-agnostic portability** across safety-critical platforms
- **Formal verification readiness** for DAL A applications (future)

### Key Features

- **Preemptive Round-Robin Scheduling**: Time-sliced task execution with configurable time quantum (50ms default)
- **Deterministic Context Switching**: Assembly-optimized context save/restore using PendSV
- **Task State Management**: Full lifecycle support (COLD, READY, RUNNING, BLOCKED, KILLED, FINISHED)
- **Bounded Semaphores**: Counting semaphores with blocking feed/consume for producer-consumer patterns
- **Active Sleep**: Cooperative sleep that allows other tasks to run
- **Blocking Sleep**: Busy-wait sleep for critical timing
- **Visual Debugging**: Terminal-based display system with progress bars, task visualization, and semaphore state
- **USB CDC Support**: Serial communication via USB Virtual COM Port
- **MPU Configuration**: Memory Protection Unit setup for QSPI flash access
- **MISRA C Compliant**: Follows MISRA C:2012 coding standards

---

## DO-178C Certification

ICARUS OS is being developed with DO-178C DAL C certification as a primary goal. Complete certification documentation is available in `docs/do178c/`.

### Documentation Suite

| Category | Documents |
|----------|-----------|
| **Plans** | PSAC, SDP, SVP, SCMP, SQAP |
| **Requirements** | SRS (101 requirements: 41 implemented, 60 planned) |
| **Design** | SDD with full traceability matrix |
| **Verification** | Coverage analysis, deactivated code analysis, test traceability |

### Compliance Status

| Objective | Status |
|-----------|--------|
| Static analysis (cppcheck) | ✅ Complete |
| MISRA C:2012 subset | ✅ Complete |
| Unit testing (Unity) | ✅ 83 tests |
| Line coverage | ✅ 79.3% |
| Function coverage | ✅ 77.6% |
| Requirements traceability | ✅ SRS complete |
| Design traceability | ✅ SDD complete |
| MC/DC coverage | 🔄 In progress |
| PC-lint Plus (full MISRA) | 📋 Planned |
| Formal verification | 📋 Planned (DAL B) |

### Running Verification

```bash
# Run all tests
cd tests && make clean test

# Generate coverage report
cd tests && make coverage

# Run static analysis
cd build && make cppcheck
```

---

## Development Roadmap

```
Phase 1: Foundation (Current) ✅
├── Preemptive scheduler
├── Task management
├── Basic BSP
├── Host-based testing (79.3% coverage)
└── DO-178C documentation suite

Phase 2: Hardening (Q2 2026)
├── Memory protection (MPU)
├── Stack overflow detection
├── Watchdog integration
└── Fault recovery

Phase 3: Communication (Q3 2026) 🔄 In Progress
├── Inter-process communication (IPC)
├── Message queues
├── Semaphores/Mutexes ✅ Bounded semaphores implemented
└── Event flags

Phase 4: AI Runtime (Q4 2026)
├── Deterministic inference engine
├── Fixed-point neural network support (int8/int16)
├── Model verification framework
└── WCET-bounded execution

Phase 5: Advanced Features (2027)
├── Multi-core support (AMP/SMP)
├── Hypervisor mode
├── Time/Space partitioning (ARINC 653)
└── Formal verification integration
```

---

## Architecture

### System Components

```
┌─────────────────────────────────────────────────────────┐
│                    Application Layer                     │
│  (User Tasks: test_task_a, test_task_b, test_task_c)     │
└─────────────────────────────────────────────────────────┘
                            │
┌─────────────────────────────────────────────────────────┐
│                      Kernel Layer                        │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐  │
│  │ Task Manager │  │  Scheduler   │  │ Context Sw.  │  │
│  └──────────────┘  └──────────────┘  └──────────────┘  │
└─────────────────────────────────────────────────────────┘
                            │
┌─────────────────────────────────────────────────────────┐
│                    BSP Layer (HAL)                        │
│  Display │ GPIO │ I2C │ SPI │ RTC │ TIM │ USB │ SysTick│
└─────────────────────────────────────────────────────────┘
                            │
┌─────────────────────────────────────────────────────────┐
│                    Hardware (STM32H750)                 │
└─────────────────────────────────────────────────────────┘
```

### Target Platforms

| Platform | Status | DAL Target | Use Case |
|----------|--------|------------|----------|
| **STM32H7** (Cortex-M7) | ✅ Primary | DAL C | UAV flight controllers, sensors |
| **STM32F4** (Cortex-M4) | Planned | DAL D | Cost-sensitive applications |
| **RISC-V** (RV32IMAC) | Planned | DAL C | Open hardware initiatives |
| **Xilinx Zynq** (Cortex-A9 + FPGA) | Planned | DAL B | AI acceleration, radar |
| **x86-64** (Host) | ✅ Testing | N/A | Development and CI testing |

---

## Kernel Components

### Task Control Block (TCB)

Each task is represented by a `task_t` structure:

```c
typedef struct {
    void (*function)(void);           // Task entry point
    uint32_t *stack_base;             // Stack base address
    uint32_t stack_size;              // Stack size in words
    uint32_t *stack_pointer;          // Current stack pointer
    task_state_t task_state;          // Current state
    uint32_t global_tick_paused;      // Tick when task was paused
    uint32_t ticks_to_pause;          // Remaining pause duration
    task_pri_t task_priority;         // Task priority (LOW/MED/HIGH)
    char name[MAX_TASK_NAME_LENGTH];  // Task name for debugging
} task_t;
```

### Task States

| State | Value | Description |
|-------|-------|-------------|
| TASK_COLD | 0 | Newly created, never executed |
| TASK_RUNNING | 1 | Currently executing |
| TASK_READY | 2 | Ready to run, waiting for scheduler |
| TASK_BLOCKED | 3 | Sleeping or waiting for event |
| TASK_KILLED | 4 | Terminated by another task |
| TASK_FINISHED | 5 | Completed execution normally |

### Scheduler

The kernel implements a **preemptive round-robin scheduler**:

- **Time Quantum**: Configurable via `TICKS_PER_TASK` (default: 50 ticks = 50ms)
- **Scheduling Trigger**: SysTick interrupt decrements `current_task_ticks_remaining`
- **Context Switch**: PendSV exception performs actual task switch
- **Task Selection**: Circular search for next READY or COLD task
- **Blocked Task Wake**: Automatically wakes tasks when sleep period expires

### Context Switching

Context switching is implemented in ARM assembly (`context_switch.s`) for optimal performance and determinism:

- **Saves**: R4-R11 (callee-saved registers) to current task's stack
- **Stores**: Updated PSP to TCB
- **Finds**: Next runnable task (READY or COLD state)
- **Checks**: Blocked tasks for wake-up conditions
- **Restores**: R4-R11 from next task's stack
- **Switches**: PSP to next task's stack pointer

---

## API Reference

### Kernel Initialization

```c
void os_init(void);      // Initialize kernel, create system tasks
void os_start(void);     // Start scheduler (never returns)
```

### Task Management

```c
void os_register_task(void (*function)(void), const char *name);
void os_yield(void);                        // Yield CPU to next task
uint32_t task_active_sleep(uint32_t ticks); // Sleep, allow other tasks
uint32_t task_blocking_sleep(uint32_t ticks); // Busy-wait sleep
void os_exit_task(void);                    // Terminate current task
void os_kill_process(uint8_t task_index);   // Kill task by index
void os_task_suicide(void);                 // Kill current task
```

### System Information

```c
uint32_t os_get_tick_count(void);           // System tick count (ms)
const char* os_get_current_task_name(void); // Current task name
uint32_t os_get_task_ticks_remaining(void); // Remaining time quantum
uint8_t os_get_running_task_count(void);    // Active task count
```

### Semaphores

```c
bool semaphore_init(uint8_t idx, uint32_t count);  // Initialize bounded semaphore
bool semaphore_feed(uint8_t idx);                   // Increment (blocks if full)
bool semaphore_consume(uint8_t idx);                // Decrement (blocks if empty)
uint32_t semaphore_get_count(uint8_t idx);          // Get current count
uint32_t semaphore_get_init_count(uint8_t idx);     // Get max capacity
```

---

## Quick Start

### Building

```bash
# Build with Makefile (recommended)
cd build
make clean all

# Or with STM32CubeIDE
# Open project and Build All
```

### Running Tests

```bash
cd tests
make clean test          # Run 76 unit tests
make coverage            # Generate coverage report (79.3%)
```

### Flashing

1. Connect STM32H750 via ST-Link
2. Flash using STM32CubeIDE or `st-flash`
3. Connect USB for serial terminal output

---

## Code Structure

```
icarus-os-core/
├── Core/
│   ├── Inc/
│   │   ├── kernel/task.h          # Task API and TCB
│   │   └── bsp/                   # BSP headers
│   └── Src/
│       ├── main.c                 # Application entry
│       ├── kernel/
│       │   ├── task.c             # Task management
│       │   └── context_switch.s   # Assembly context switch
│       └── bsp/                   # Board support
├── docs/
│   └── do178c/                    # DO-178C documentation
│       ├── plans/                 # PSAC, SDP, SVP, SCMP, SQAP
│       ├── requirements/          # SRS
│       ├── design/                # SDD
│       └── verification/          # Coverage, traceability
├── tests/
│   ├── src/test_task.c            # 76 unit tests
│   ├── mocks/                     # Hardware mocks
│   └── unity/                     # Unity test framework
├── build/
│   └── reports/                   # Static analysis, coverage
├── Drivers/                       # STM32 HAL
└── Middlewares/                   # USB Device Library
```

---

## Configuration

### Kernel (`Core/Inc/main.h`)

```c
#define TICKS_PER_TASK 50  // Time quantum (50ms)
```

### Task Limits (`Core/Inc/kernel/task.h`)

```c
#define MAX_TASKS 10              // Maximum tasks
#define STACK_WORDS 512           // Stack per task (2KB)
#define MAX_TASK_NAME_LENGTH 32   // Task name length
```

---

## Hardware Requirements

- **MCU**: STM32H750VBTx (Cortex-M7, 480 MHz)
- **Flash**: Internal or external QSPI
- **RAM**: Internal SRAM (DTCM, AXI SRAM)
- **Display**: Terminal via USB CDC
- **LED**: GPIO E3 for heartbeat

---

## Limitations

Current limitations (work in progress):

- **No Priority Preemption**: Round-robin only (priority support planned)
- **No Mutex**: Mutex with priority inheritance planned
- **No Message Queues**: Message queue IPC planned
- **No Dynamic Allocation**: All memory statically allocated (by design for certification)
- **Fixed Stack Size**: All tasks use same stack size
- **Single Core**: Multi-core support planned for Phase 5

---

## License

Apache License 2.0 - See LICENSE file for details.

---

## Author

Created by Souham Biswas for deterministic real-time embedded systems.

GitHub: https://github.com/ironhide23586/icarus-os-core
