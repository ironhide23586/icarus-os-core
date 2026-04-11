# ICARUS OS Architecture

**Version:** 0.3.0
**Target:** STM32H750VBT6 (ARM Cortex-M7 @ 480MHz)
**Author:** Souham Biswas
**Date:** 2026

## Table of Contents

1. [System Overview](#system-overview)
2. [Kernel Architecture](#kernel-architecture)
3. [Memory Architecture](#memory-architecture)
4. [Task Management](#task-management)
5. [Scheduling](#scheduling)
6. [Inter-Process Communication](#inter-process-communication)
7. [Memory Protection](#memory-protection)
8. [Board Support Package](#board-support-package)
9. [Application Layer](#application-layer)
10. [Build System](#build-system)

## System Overview

ICARUS OS is a preemptive real-time operating system kernel designed for safety-critical embedded applications. The architecture follows a layered approach with clear separation between hardware abstraction, kernel services, and application code.

### Design Principles

1. **Determinism**: All operations have bounded execution time
2. **Safety**: Hardware-enforced memory protection prevents corruption
3. **Simplicity**: Minimal, auditable codebase for certification
4. **Portability**: Clean HAL abstraction for multiple platforms
5. **Verifiability**: Designed for formal verification and DO-178C compliance

### System Layers

```
┌─────────────────────────────────────────────────────────────┐
│                    APPLICATION LAYER                         │
│  User tasks, game logic, demo applications, showcase tasks   │
├─────────────────────────────────────────────────────────────┤
│                      RTOS API LAYER                          │
│  Semaphores, Pipes, Tasks                                    │
│  Software Bus (pub/sub) · Background Checksum (CRC16 scan)  │
│  CDC RX ring · Event ring + squelch · CRC16 (HW-accelerated) │
│  Internal flat-file FS · Ground-loadable table engine        │
├─────────────────────────────────────────────────────────────┤
│                      KERNEL LAYER                            │
│  Scheduler, Context Switch, SVC Handler (63 gates),          │
│  MPU Manager                                                 │
├─────────────────────────────────────────────────────────────┤
│                   BOARD SUPPORT PACKAGE                      │
│  GPIO, SPI, I2C, USB, Display, RTC, Timer                    │
│  IWDG watchdog · K1 button · CDC raw write                   │
├─────────────────────────────────────────────────────────────┤
│                    HARDWARE LAYER                            │
│  STM32H750VBT6 (Cortex-M7, MPU, NVIC, SysTick, CRC unit)     │
└─────────────────────────────────────────────────────────────┘
```


## Kernel Architecture

### Core Components

The ICARUS kernel consists of six primary components plus a set of
shared service modules:

1. **Kernel Core** (`Core/Src/icarus/kernel.c`, `Core/Inc/icarus/kernel.h`) - System initialization and state management
2. **Scheduler** (`scheduler.c/h`) - Task selection and time-slicing
3. **Task Manager** (`task.c/h`) - Task lifecycle and registration
4. **Context Switch** (`context_switch.s`) - Low-level task switching
5. **SVC Handler** (`svc.c/h`) - Privilege separation and call gates (63 SVC numbers)
6. **IPC Manager** (`semaphore.c/h`, `pipe.c/h`) - Inter-process communication

**Shared service modules** (added in v0.3.0, all reachable through
`#include "icarus/icarus.h"`):

7. **CDC RX ring buffer** (`cdc_rx.c/h`) - 512 B SPSC USB CDC receive ring;
   producer is the privileged USB ISR, consumer is any RTOS task. Backing
   data in `DTCM_DATA_PRIV`, hot path in `ITCM_FUNC`, thread-mode reads
   through SVC gates 40–42.
8. **Event ring + squelch** (`event.c/h`) - 32-slot ring of compact 16-byte
   event entries with a 16-entry per-module severity squelch. Transport-
   agnostic (drains into a caller-provided buffer). SVC gates 43–48.
9. **CRC16-CCITT helper** (`crc.c/h`) - `crc16_ccitt(data, len)` with poly
   0x1021. **Hardware-accelerated** on STM32H7 via the on-chip CRC
   peripheral on the AHB4 bus, lazy-initialised on first call;
   approximately 4× faster than the bytewise software loop. Portable
   bytewise fallback under `HOST_TEST`.
10. **Internal flat-file filesystem** (`fs.c/h`) - 16 files × 2 KB = 32 KB
    RAM-backed store. Functions placed in ITCM. The store itself stays in
    regular SRAM (32 KB won't fit DTCM); the deliberate trade-off is
    documented in `fs.c`.
11. **Ground-loadable table engine** (`tables.c/h`) - Registry of up to 8
    tables, each with a staging buffer and an active buffer. CRC-validated
    load / activate / dump with a registered activate callback. The
    `tbl_activate` operation is split across two SVCs (`prepare` +
    `commit`) so the user callback runs in unprivileged thread mode against
    a stack scratch copy without ever touching `DTCM_PRIV` directly.
    SVC gates 49–56.
12. **Software Bus** (`sb.c/h`) - Lightweight pub/sub message router built
    on top of kernel pipes. 32 routes × 4 subscribers per message ID.
    Best-effort delivery: if a subscriber's pipe is full the message is
    silently dropped for that subscriber. Route table in `DTCM_DATA_PRIV`,
    hot-path functions in `ITCM_FUNC`. Uses existing pipe SVC gates for
    the underlying IPC.
13. **Background Checksum integrity monitor** (`cs.c/h`) - Periodic
    CRC16-CCITT scanner over up to 8 registered memory regions. Baselines
    captured at registration time; mismatches reported through a
    user-supplied callback. Hardware CRC self-test on init (expected
    0x29B1). Region table in `DTCM_DATA_PRIV`, functions in `ITCM_FUNC`.

Headers and sources live under `Core/Inc/icarus/` and `Core/Src/icarus/`
(not `kernel/`).

### Kernel State

All kernel state is stored in DTCM (privileged-only) for protection:

```c
// Task management
icarus_task_t* task_list[ICARUS_MAX_TASKS];           // 128 task pointers
uint8_t current_task_index;                            // Currently running task
uint8_t running_task_count;                            // Active task count
uint8_t num_created_tasks;                             // Total created tasks

// Scheduler state
volatile uint32_t current_task_ticks_remaining;        // Time slice remaining
volatile uint32_t ticks_per_task;                      // Time quantum (50ms)
volatile uint32_t os_tick_count;                       // System tick counter
volatile bool scheduler_enabled;                       // Scheduler on/off

// IPC state
icarus_semaphore_t* semaphore_list[ICARUS_MAX_SEMAPHORES];  // 64 semaphores
icarus_pipe_t* message_pipe_list[ICARUS_MAX_MESSAGE_QUEUES]; // 64 pipes

// Cleanup queue
int8_t cleanup_task_idx[ICARUS_MAX_TASKS];            // Finished task queue
int8_t current_cleanup_task_idx;                      // Queue write index
```

### Initialization Sequence

```
1. Reset_Handler (startup_stm32h750vbtx.s)
   ├─> Copy .data from Flash to RAM
   ├─> Zero .bss section
   ├─> Copy .itcm from Flash to ITCM
   ├─> Copy .dtcm_priv from Flash to DTCM
   └─> Call SystemInit()

2. SystemInit() (system_stm32h7xx.c)
   ├─> Configure MPU (8 regions)
   ├─> Enable I-Cache and D-Cache
   ├─> Configure FPU
   └─> Set vector table offset

3. main() (main.c)
   ├─> hal_init() - Initialize BSP
   ├─> os_init() - Initialize kernel
   ├─> game_init() / demo_tasks_init() / stress init — per `main.c` compile-time flags
   └─> os_start() - Start scheduler (never returns)

4. os_init() (kernel.c)
   ├─> Initialize task pool
   ├─> Initialize semaphore pool
   ├─> Initialize pipe pool
   ├─> Configure SysTick (1ms)
   └─> Set scheduler_enabled = false

5. os_start() (kernel.c)
   ├─> Find first ready task
   ├─> Set current_task_index
   ├─> Enable scheduler
   ├─> Drop to unprivileged mode
   └─> Jump to first task (never returns)
```


## Memory Architecture

### Memory Regions (STM32H750VBT6)

| Region | Base Address | Size | Purpose | Protection |
|--------|--------------|------|---------|------------|
| ITCM | 0x00000000 | 64KB | Kernel code (zero wait-state) | RO+Exec (MPU Region 0) |
| DTCM | 0x20000000 | 128KB | Kernel data (zero wait-state) | Priv RW only (MPU Region 5) |
| RAM_D1 | 0x24000000 | 512KB | Task stacks, display buffers | Full access (MPU Region 6) |
| RAM_D2 | 0x30000000 | 288KB | Task data regions (2KB each) | Dynamic (MPU Region 4) |
| RAM_D3 | 0x38000000 | 64KB | Reserved for future use | Full access |
| Flash | 0x08000000 | 128KB | Program code and constants | RO+Exec (MPU Region 2) |
| QSPI | 0x90000000 | 8MB | External flash (future) | RO+Exec (MPU Region 1) |
| Peripherals | 0x40000000 | 512MB | Memory-mapped I/O | Full access (MPU Region 7) |

### Memory Layout

```
ITCM (0x00000000 - 0x0000FFFF) [64KB]
├─ .itcm section (7.5KB used, 12%)
│  ├─ os_yield_pendsv (context switch)
│  ├─ scheduler functions
│  ├─ SVC dispatcher
│  ├─ semaphore/pipe operations
│  └─ critical kernel functions
└─ Free: 56.5KB

DTCM (0x20000000 - 0x2001FFFF) [128KB]
├─ .dtcm_priv section (8KB used, 6%)
│  ├─ task_list[128]
│  ├─ semaphore_list[64]
│  ├─ message_pipe_list[64]
│  ├─ scheduler state
│  └─ kernel counters
└─ Free: 120KB

RAM_D1 (0x24000000 - 0x2407FFFF) [512KB]
├─ Task stacks (256KB, 50%)
│  ├─ 128 stacks × 2KB each
│  └─ 8-byte aligned
├─ Display buffers (16KB, 3%)
│  ├─ Terminal line buffers
│  └─ Message history
└─ Free: 240KB (47%)

RAM_D2 (0x30000000 - 0x30047FFF) [288KB]
├─ Task data regions (256KB, 89%)
│  ├─ 128 regions × 2KB each
│  ├─ 2KB-aligned for MPU
│  └─ Dynamically protected
└─ Free: 32KB (11%)

Flash (0x08000000 - 0x0801FFFF) [128KB]
├─ .isr_vector (1KB)
├─ .text (40KB)
├─ .rodata (8KB)
└─ Free: ~79KB (62%)
```

### MPU Configuration

ICARUS OS uses 8 MPU regions for memory protection:

```c
// Region 0: ITCM (kernel code protection)
Base: 0x00000000, Size: 64KB
Access: Priv+User RO+Exec
Purpose: Prevent code modification attacks

// Region 1: QSPI Flash (external code)
Base: 0x90000000, Size: 8MB
Access: Priv+User RO+Exec
Purpose: Execute-in-place from external flash

// Region 2: Internal Flash (program code)
Base: 0x08000000, Size: 128KB
Access: Priv+User RO+Exec
Purpose: Protect program code from modification

// Region 3: DISABLED
Purpose: Reserved for future use

// Region 4: Task Data (dynamic)
Base: Varies per task, Size: 2KB
Access: Priv+User RW
Purpose: Isolate task data regions
Note: Reconfigured on every context switch

// Region 5: DTCM (kernel data protection)
Base: 0x20000000, Size: 128KB
Access: Priv RW only
Purpose: Isolate kernel state from unprivileged tasks

// Region 6: RAM_D1 (task stacks)
Base: 0x24000000, Size: 512KB
Access: Full Access
Purpose: Task stacks and shared buffers

// Region 7: Peripherals
Base: 0x40000000, Size: 512MB
Access: Full Access
Purpose: Memory-mapped I/O access
```

