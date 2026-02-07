# ICARUS OS Architecture

**Version:** 1.0  
**Date:** 2025-02-07  
**Status:** Current  

---

## Table of Contents

1. [Overview](#overview)
2. [System Architecture](#system-architecture)
3. [Memory Architecture](#memory-architecture)
4. [Privilege Model](#privilege-model)
5. [Naming Conventions](#naming-conventions)
6. [Module Organization](#module-organization)
7. [Execution Flow](#execution-flow)
8. [Performance Optimizations](#performance-optimizations)

---

## Overview

ICARUS OS is a preemptive real-time kernel for ARM Cortex-M7 (STM32H750) designed with safety-critical certification in mind. The architecture emphasizes:

- **Determinism**: Predictable timing and bounded execution
- **Protection**: MPU-based memory protection with privilege separation
- **Performance**: Zero wait-state execution via ITCM/DTCM
- **Verifiability**: Clean layered architecture for formal analysis

### Key Characteristics

| Characteristic | Value |
|----------------|-------|
| **Scheduling** | Preemptive round-robin, 50ms time quantum |
| **Context Switch** | ~2μs (ITCM-optimized) |
| **Max Tasks** | 32 (configurable) |
| **Task Stack** | 2KB per task (DTCM) |
| **IPC** | Semaphores (32), Pipes (32) |
| **Memory Model** | Static allocation only |
| **Privilege Model** | SVC-based kernel protection |

---

## System Architecture

### Layered Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                     Application Tasks                        │
│  (User code running in unprivileged mode)                   │
├─────────────────────────────────────────────────────────────┤
│                    Public API Layer                          │
│  os_init(), task_create(), semaphore_feed(), etc.           │
│  (SVC wrappers - transition to privileged mode)             │
├─────────────────────────────────────────────────────────────┤
│                   SVC Handler Layer                          │
│  SVC_Handler() - dispatches to privileged functions          │
│  (Privilege boundary enforcement)                            │
├─────────────────────────────────────────────────────────────┤
│                 Privileged Kernel Layer                      │
│  __os_init(), __task_create(), __semaphore_feed(), etc.     │
│  (MPU-protected kernel implementations)                      │
├─────────────────────────────────────────────────────────────┤
│                  Hardware Abstraction                        │
│  BSP, HAL, CMSIS (STM32H7 specific)                         │
└─────────────────────────────────────────────────────────────┘
```

### Three-Layer Protection Model

1. **Public API Layer** (Unprivileged)
   - Clean interface for application tasks
   - No direct access to kernel data structures
   - All calls go through SVC mechanism

2. **SVC Layer** (Privilege Transition)
   - `svc.c` contains all SVC wrapper implementations
   - Invokes SVC instruction to enter privileged mode
   - Minimal overhead (~10 cycles)

3. **Privileged Kernel Layer** (Protected)
   - Functions prefixed with `__` (double underscore)
   - Direct access to kernel data structures in DTCM
   - MPU-protected from unprivileged access

---

## Memory Architecture

### STM32H750 Memory Map

```
┌─────────────────────────────────────────────────────────────┐
│ ITCM (0x00000000) - 64 KB                                   │
│   Instruction Tightly-Coupled Memory (zero wait-state)      │
│   ┌─────────────────────────────────────────────────────┐   │
│   │ Critical kernel code:                                │   │
│   │ - __os_yield()                                       │   │
│   │ - __task_active_sleep()                              │   │
│   │ - __os_get_tick_count()                              │   │
│   │ - __semaphore_feed/consume()                         │   │
│   │ - __pipe_enqueue/dequeue()                           │   │
│   │ - SVC_Handler(), PendSV_Handler(), SysTick_Handler() │   │
│   │ - Context switch assembly                            │   │
│   └─────────────────────────────────────────────────────┘   │
│   Used: ~3 KB (5%) | Free: ~61 KB                           │
├─────────────────────────────────────────────────────────────┤
│ DTCM (0x20000000) - 128 KB                                  │
│   Data Tightly-Coupled Memory (zero wait-state)             │
│   ┌─────────────────────────────────────────────────────┐   │
│   │ Kernel data structures:                              │   │
│   │ - __task_list[32]                                    │   │
│   │ - __semaphore_list[32]                               │   │
│   │ - __message_pipe_list[32]                            │   │
│   │ - __current_task_index                               │   │
│   │ - __os_tick_count                                    │   │
│   │ - __scheduler_enabled                                │   │
│   │ - Task stacks (64 KB total, 2KB each)               │   │
│   │ - Task data pools (64 KB total, 2KB each)           │   │
│   └─────────────────────────────────────────────────────┘   │
│   Used: ~70 KB (55%) | Free: ~58 KB                         │
├─────────────────────────────────────────────────────────────┤
│ Flash (0x08000000) - 128 KB                                 │
│   Program code and constants                                │
│   - Application code                                         │
│   - Non-critical kernel code                                │
│   - Constant data                                            │
│   Used: ~50 KB (39%) | Free: ~78 KB                         │
├─────────────────────────────────────────────────────────────┤
│ AXI SRAM (0x24000000) - 512 KB                              │
│   General purpose RAM                                        │
│   - Heap (if needed)                                         │
│   - BSS segment                                              │
│   - Other data                                               │
└─────────────────────────────────────────────────────────────┘
```

### Section Placement Strategy

**ITCM Placement** (`ITCM_FUNC` attribute):
- Functions in critical execution paths
- Interrupt handlers (SVC, PendSV, SysTick)
- Scheduler hot paths
- IPC operations (semaphores, pipes)
- Goal: Zero wait-state execution for determinism

**DTCM Placement** (`DTCM_DATA` attribute):
- All kernel state variables (prefixed with `__`)
- Task control blocks and stacks
- Semaphore and pipe data structures
- Scheduler state
- Goal: Zero wait-state data access

**Flash Placement** (default):
- Application code
- Non-critical kernel functions
- Initialization code
- Constant data

---

## Privilege Model

### SVC-Based Protection

ICARUS OS uses ARM Cortex-M Supervisor Call (SVC) mechanism for privilege separation:

```
Application Task (Unprivileged)
        ↓
    os_yield()  ← Public API (no __ prefix)
        ↓
    SVC #9      ← Trigger supervisor call
        ↓
  SVC_Handler() ← Dispatch based on SVC number
        ↓
  __os_yield()  ← Privileged implementation (__ prefix)
        ↓
   Kernel Data  ← Access DTCM structures
```

### Privilege Levels

1. **Unprivileged (Thread Mode)**
   - Application tasks execute here
   - Cannot access kernel data structures
   - Cannot execute privileged instructions
   - Must use SVC to call kernel services

2. **Privileged (Handler Mode)**
   - Kernel code executes here
   - Full access to all memory regions
   - Can execute privileged instructions
   - Can access MPU-protected DTCM data

### SVC Number Assignments

| SVC # | Function | Module |
|-------|----------|--------|
| 0 | task_active_sleep | scheduler |
| 1 | task_blocking_sleep | scheduler |
| 2 | enter_critical | kernel |
| 3 | exit_critical | kernel |
| 4 | pipe_init | pipe |
| 5 | pipe_enqueue | pipe |
| 6 | pipe_dequeue | pipe |
| 7 | os_init | kernel |
| 8 | os_start | kernel |
| 9 | os_yield | scheduler |
| 10 | os_register_task | task |
| 11 | os_get_current_task_name | scheduler |
| 12 | os_get_tick_count | scheduler |
| 13 | os_exit_task | task |
| 14 | os_kill_process | task |
| 15 | os_task_suicide | task |
| 16 | semaphore_init | semaphore |
| 17 | semaphore_consume | semaphore |
| 18 | semaphore_feed | semaphore |
| 19 | semaphore_get_count | semaphore |
| 20 | semaphore_get_max_count | semaphore |
| 21 | pipe_get_count | pipe |
| 22 | pipe_get_max_count | pipe |
| 23 | task_busy_wait | scheduler |
| 24 | os_get_running_task_count | scheduler |
| 25 | os_get_task_ticks_remaining | scheduler |
| 26 | kernel_get_stack | kernel |
| 27 | kernel_get_data | kernel |
| 28 | kernel_protected_data | kernel |

---

## Naming Conventions

### Global Variables

All kernel state variables use `__` prefix (double underscore):

```c
// Kernel state (DTCM)
extern DTCM_DATA icarus_task_t* __task_list[ICARUS_MAX_TASKS];
extern DTCM_DATA uint8_t __current_task_index;
extern DTCM_DATA volatile uint32_t __os_tick_count;
extern DTCM_DATA volatile bool __scheduler_enabled;
```

**Rationale**: The `__` prefix indicates these variables are:
- MPU-protected in DTCM
- Only accessible from privileged mode
- Not directly accessible to application tasks

### Functions

**Public API** (no prefix):
```c
void os_init(void);
void os_yield(void);
uint32_t task_active_sleep(uint32_t ticks);
bool semaphore_feed(uint8_t semaphore_idx);
```

**Privileged Implementations** (`__` prefix):
```c
void __os_init(void);
ITCM_FUNC void __os_yield(void);
ITCM_FUNC uint32_t __task_active_sleep(uint32_t ticks);
ITCM_FUNC bool __semaphore_feed(uint8_t semaphore_idx);
```

**Rationale**: 
- Public API provides clean interface for applications
- Privileged functions are internal implementation details
- `__` prefix indicates MPU-protected kernel code

### Local Variables

All local variables in kernel functions use `_` prefix (single underscore):

```c
void __os_init(void) {
    uint8_t _i;  // Local variable
    for (_i = 0; _i < ICARUS_MAX_TASKS; _i++) {
        __task_list[_i] = &__task_pool[_i];  // Global kernel variable
    }
}
```

**Rationale**: Distinguishes local variables from:
- Global kernel variables (`__` prefix)
- Function parameters (no prefix)
- Improves code readability and maintainability

### Function Parameters

Function parameters have no prefix:

```c
uint32_t __task_active_sleep(uint32_t ticks) {
    uint32_t _start_tick = __os_tick_count;  // Local variable
    // ... use 'ticks' parameter directly
}
```

---

## Module Organization

### Core Kernel Modules

```
Core/
├── Inc/icarus/
│   ├── kernel.h          # Kernel initialization, critical sections
│   ├── scheduler.h       # Task scheduling, timing services
│   ├── task.h            # Task management, lifecycle
│   ├── semaphore.h       # Counting semaphores
│   ├── pipe.h            # Message pipes (queues)
│   ├── svc.h             # SVC number definitions
│   ├── types.h           # Kernel data structures
│   └── config.h          # Configuration constants
│
└── Src/icarus/
    ├── kernel.c          # Kernel core implementation
    ├── scheduler.c       # Scheduler implementation
    ├── task.c            # Task management implementation
    ├── semaphore.c       # Semaphore implementation
    ├── pipe.c            # Pipe implementation
    ├── svc.c             # SVC wrapper implementations
    └── context_switch.s  # Assembly context switching
```

### Module Responsibilities

**kernel.c/h**:
- Kernel initialization (`os_init`, `os_start`)
- Critical section management (`enter_critical`, `exit_critical`)
- Kernel state variables (task lists, scheduler state)
- Protected data allocation (`kernel_protected_data`)
- System tasks (idle, heartbeat)

**scheduler.c/h**:
- Task scheduling (`os_yield`)
- Timing services (`task_active_sleep`, `task_blocking_sleep`)
- Tick management (`os_get_tick_count`)
- Task information queries

**task.c/h**:
- Task creation and registration (`os_register_task`)
- Task lifecycle (`os_exit_task`, `os_kill_process`, `os_task_suicide`)
- Task state management
- Stack initialization

**semaphore.c/h**:
- Semaphore initialization (`semaphore_init`)
- Semaphore operations (`semaphore_feed`, `semaphore_consume`)
- Semaphore queries (`semaphore_get_count`, `semaphore_get_max_count`)

**pipe.c/h**:
- Pipe initialization (`pipe_init`)
- Pipe operations (`pipe_enqueue`, `pipe_dequeue`)
- Pipe queries (`pipe_get_count`, `pipe_get_max_count`)

**svc.c/h**:
- SVC number definitions
- SVC wrapper implementations (all public API functions)
- SVC handler dispatch logic

**context_switch.s**:
- PendSV handler (context switching)
- Cold task start
- Register save/restore

---

## Execution Flow

### System Initialization

```
1. main()
   ↓
2. os_init()  [SVC #7]
   ↓
3. __os_init()  [Privileged]
   - Initialize HAL
   - Initialize task pools
   - Initialize semaphore pools
   - Initialize pipe pools
   - Register system tasks (idle, heartbeat)
   - Enable scheduler
   ↓
4. Application registers tasks
   - os_register_task(task_func, "name")
   ↓
5. os_start()  [SVC #8]
   ↓
6. __os_start()  [Privileged]
   - Start first task
   - Never returns
```

### Task Scheduling

```
SysTick Interrupt (1ms)
   ↓
SysTick_Handler()  [ITCM, Privileged]
   - Increment __os_tick_count
   - Decrement __current_task_ticks_remaining
   - If time quantum expired:
     ↓
   Trigger PendSV
     ↓
PendSV_Handler()  [ITCM, Privileged]
   - Save current task context
   - Select next task (round-robin)
   - Restore next task context
   - Return to next task
```

### SVC Call Flow

```
Application Task
   ↓
os_yield()  [Public API]
   - Inline assembly: SVC #9
   ↓
SVC_Handler()  [ITCM, Privileged]
   - Extract SVC number from instruction
   - Switch on SVC number
   - Call __os_yield()
   ↓
__os_yield()  [ITCM, Privileged]
   - Reset time quantum
   - Trigger PendSV for context switch
   ↓
Return to application task
```

### IPC Operation Flow

**Semaphore Feed (Signal)**:
```
Application Task
   ↓
semaphore_feed(idx)  [Public API]
   - SVC #18
   ↓
__semaphore_feed(idx)  [ITCM, Privileged]
   - Check if semaphore engaged
   - If count < max_count:
     - Increment count
     - Return true
   - Else:
     - Block task (set BLOCKED state)
     - Yield CPU
     - Retry when unblocked
```

**Pipe Enqueue (Send)**:
```
Application Task
   ↓
pipe_enqueue(idx, msg, len)  [Public API]
   - SVC #5
   ↓
__pipe_enqueue(idx, msg, len)  [ITCM, Privileged]
   - Check if pipe engaged
   - Check if space available
   - If space available:
     - Copy message to circular buffer
     - Update enqueue index
     - Increment count
     - Return true
   - Else:
     - Block task
     - Yield CPU
     - Retry when space available
```

---

## Performance Optimizations

### Zero Wait-State Execution

**ITCM Placement**:
- Critical kernel functions marked with `ITCM_FUNC`
- Interrupt handlers in ITCM
- Context switch code in ITCM
- Result: Deterministic execution, no cache misses

**DTCM Placement**:
- All kernel data structures marked with `DTCM_DATA`
- Task stacks in DTCM
- Scheduler state in DTCM
- Result: Deterministic data access, no cache misses

### Context Switch Optimization

**Lazy FPU Context Save**:
- FPU context only saved if task uses FPU
- Reduces context switch overhead
- Typical context switch: ~2μs

**Assembly Implementation**:
- Hand-optimized assembly for PendSV handler
- Minimal instruction count
- No function call overhead

### Scheduler Optimization

**Round-Robin with Time Quantum**:
- Simple, deterministic algorithm
- O(1) task selection
- No priority inversion issues

**Tick-Based Timing**:
- 1ms tick resolution
- Efficient sleep/delay implementation
- Bounded wakeup latency

### IPC Optimization

**Blocking with Yield**:
- Tasks block when resource unavailable
- Automatic retry when resource available
- No busy-waiting, efficient CPU usage

**Circular Buffers**:
- Efficient FIFO implementation
- No memory allocation
- Bounded execution time

---

## Testing and Verification

### Unit Testing

- 142 unit tests covering all kernel functions
- Unity test framework
- 88.5% line coverage, 85.9% function coverage
- Host-based testing with mocks

### Static Analysis

- cppcheck clean
- MISRA C:2012 subset compliance
- Enhanced compiler warnings enabled

### Integration Testing

- Stress test suite with 19 concurrent tasks
- Semaphore and pipe stress testing
- Data integrity verification
- Real-time visualization

---

## Future Enhancements

### Planned Features

1. **Priority-Based Scheduling**
   - Multiple priority levels
   - Priority inheritance for mutexes
   - Preemption by higher-priority tasks

2. **MPU Configuration**
   - Memory protection for task stacks
   - Kernel/user space separation
   - Fault isolation

3. **Stack Overflow Detection**
   - Stack canaries
   - MPU-based detection
   - Automatic task termination

4. **Formal Verification**
   - Model checking for deadlock detection
   - Proof of scheduling properties
   - WCET analysis

---

## References

- ARM Cortex-M7 Technical Reference Manual
- STM32H750 Reference Manual
- DO-178C Software Considerations in Airborne Systems
- MISRA C:2012 Guidelines for the Use of C in Critical Systems

---

**Document Control**

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 1.0 | 2025-02-07 | Souham Biswas | Initial comprehensive architecture document |
