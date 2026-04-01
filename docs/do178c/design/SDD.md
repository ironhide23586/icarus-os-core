# Software Design Description (SDD)

**Document ID:** ICARUS-SDD-001  
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

This Software Design Description (SDD) defines the architecture, components, interfaces, and data structures of ICARUS OS. It provides the bridge between requirements (SRS) and implementation, enabling traceability and design verification.

### 1.2 Scope

This document covers:
- System architecture and decomposition
- Component design and interfaces
- Data structures and algorithms
- Memory architecture
- Timing design

### 1.3 Design Principles

| Principle | Description |
|-----------|-------------|
| **Determinism** | All operations have bounded, predictable timing |
| **Static Allocation** | No dynamic memory after initialization |
| **Minimal Footprint** | Kernel <32KB code, <8KB RAM |
| **Portability** | Hardware abstraction enables multi-platform |
| **Certifiability** | Design supports DO-178C verification |

---

## 2. System Architecture

### 2.1 Architectural Overview

```
┌─────────────────────────────────────────────────────────────────────────┐
│                         APPLICATION LAYER                                │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐  ┌─────────────────┐ │
│  │ User Task 1 │  │ User Task 2 │  │ User Task N │  │   AI Tasks      │ │
│  └──────┬──────┘  └──────┬──────┘  └──────┬──────┘  └────────┬────────┘ │
├─────────┴────────────────┴────────────────┴──────────────────┴──────────┤
│                          ICARUS KERNEL                                   │
│  ┌───────────────────────────────────────────────────────────────────┐  │
│  │                      KERNEL API                                    │  │
│  │  os_init() os_start() os_yield() task_sleep() ipc_send() ai_run() │  │
│  └───────────────────────────────────────────────────────────────────┘  │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐  ┌─────────────────┐ │
│  │  Scheduler  │  │Task Manager │  │     IPC     │  │   AI Runtime    │ │
│  │             │  │             │  │  (Planned)  │  │   (Planned)     │ │
│  │• Round-robin│  │• Create     │  │• Queues     │  │• Model loader   │ │
│  │• Preemptive │  │• Kill       │  │• Semaphores │  │• Inference      │ │
│  │• Priority   │  │• Sleep      │  │• Mutexes    │  │• Operators      │ │
│  └──────┬──────┘  └──────┬──────┘  └──────┬──────┘  └────────┬────────┘ │
├─────────┴────────────────┴────────────────┴──────────────────┴──────────┤
│                    PLATFORM ABSTRACTION LAYER                            │
│  ┌───────────────────────────────────────────────────────────────────┐  │
│  │  icarus_hal_init()  icarus_hal_context_switch()  icarus_hal_tick()│  │
│  └───────────────────────────────────────────────────────────────────┘  │
├─────────────────────────────────────────────────────────────────────────┤
│                     BOARD SUPPORT PACKAGE (BSP)                          │
│  ┌───────────┐ ┌───────────┐ ┌───────────┐ ┌───────────┐ ┌───────────┐ │
│  │   GPIO    │ │    I2C    │ │    SPI    │ │    USB    │ │  Display  │ │
│  └─────┬─────┘ └─────┬─────┘ └─────┬─────┘ └─────┬─────┘ └─────┬─────┘ │
├────────┴─────────────┴─────────────┴─────────────┴─────────────┴────────┤
│                        HARDWARE LAYER                                    │
│  ┌───────────────────────────────────────────────────────────────────┐  │
│  │  STM32H750 (Cortex-M7) │ STM32F4 │ RISC-V │ Zynq │ x86 (Sim)     │  │
│  └───────────────────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────────────────┘
```

### 2.2 Component Summary

| Component | Status | Source Location | Description |
|-----------|--------|-----------------|-------------|
| Scheduler | ✅ Implemented | `Core/Src/icarus/scheduler.c` | Preemptive round-robin |
| Task Manager | ✅ Implemented | `Core/Src/icarus/task.c` | Task lifecycle management |
| Context Switch | ✅ Implemented | `Core/Src/icarus/context_switch.s` | ARM assembly context save/restore |
| Semaphores | ✅ Implemented | `Core/Src/icarus/semaphore.c` | Bounded counting semaphores |
| Message pipes (IPC) | ✅ Implemented | `Core/Src/icarus/pipe.c` | FIFO byte-stream queues (see `pipe.h`) |
| AI Runtime | 🔲 Planned | TBD | Deterministic inference |
| BSP - GPIO | ✅ Implemented | `bsp/gpio.c` | Digital I/O |
| BSP - I2C | ✅ Implemented | `bsp/i2c.c` | I2C communication |
| BSP - SPI | ✅ Implemented | `bsp/spi.c` | SPI communication |
| BSP - Display | ✅ Implemented | `bsp/display.c` | Terminal display with vertical bar |
| BSP - USB | ✅ Implemented | `USB_DEVICE/` | USB CDC serial |


---

## 3. Component Design

### 3.1 Scheduler Component

#### 3.1.1 Design Overview

The scheduler implements preemptive round-robin scheduling with configurable time quantum.

```
┌─────────────────────────────────────────────────────────────┐
│                    SCHEDULER STATE MACHINE                   │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│                        ┌──────────┐                         │
│            os_start()  │   COLD   │  Initial state          │
│                ┌───────┤          ├───────┐                 │
│                │       └──────────┘       │                 │
│                ▼                          │                 │
│         ┌──────────┐              ┌───────┴────┐            │
│         │ RUNNING  │◄────────────│   READY    │            │
│         │          │  schedule() │            │            │
│         └────┬─────┘             └──────▲─────┘            │
│              │                          │                   │
│    yield()   │    ┌──────────┐         │ wake()            │
│    preempt() │    │ BLOCKED  │─────────┘                   │
│              └───►│ (sleep)  │                             │
│                   └──────────┘                             │
│                        │                                    │
│         kill()         │         exit()                     │
│              ┌─────────┴─────────┐                         │
│              ▼                   ▼                          │
│         ┌──────────┐       ┌──────────┐                    │
│         │  KILLED  │       │ FINISHED │                    │
│         └──────────┘       └──────────┘                    │
│                                                              │
└─────────────────────────────────────────────────────────────┘
```

#### 3.1.2 Scheduling Algorithm

```
ALGORITHM: Round-Robin Preemptive Scheduler

INPUT: task_list[], current_task_index, os_tick_count
OUTPUT: next task to run

ON SysTick_Handler:
    os_tick_count++
    IF os_running AND scheduler_enabled:
        current_task_ticks_remaining--
        IF current_task_ticks_remaining == 0:
            current_task_ticks_remaining = TICKS_PER_TASK
            trigger_context_switch()  // Set PendSV pending

ON PendSV_Handler (lowest priority):
    save_context(current_task)
    
    // Find next runnable task (round-robin)
    FOR i = 1 TO num_created_tasks:
        next_idx = (current_task_index + i) % num_created_tasks
        task = task_list[next_idx]
        
        IF task.state == BLOCKED:
            IF os_tick_count - task.global_tick_paused >= task.ticks_to_pause:
                task.state = READY  // Wake up
        
        IF task.state == READY OR task.state == RUNNING:
            current_task_index = next_idx
            BREAK
    
    task_list[current_task_index].state = RUNNING
    restore_context(task_list[current_task_index])
```

#### 3.1.3 Timing Characteristics

| Parameter | Value | Configurable |
|-----------|-------|--------------|
| Tick period | 1 ms | Yes (SysTick) |
| Time quantum | 50 ticks (50 ms) | Yes (TICKS_PER_TASK) |
| Context switch time | <10 μs | No |
| Interrupt latency | <5 μs | No |

#### 3.1.4 Requirements Traceability

| Design Element | Implements Requirement |
|----------------|----------------------|
| Round-robin selection | HLR-KRN-011 |
| Preemptive switching | HLR-KRN-010 |
| Configurable quantum | HLR-KRN-012 |
| Voluntary yield | HLR-KRN-013 |
| Tick counter | HLR-KRN-020 |

---

### 3.2 Task Manager Component

#### 3.2.1 Design Overview

The Task Manager handles task lifecycle: creation, execution, sleep, and termination.

#### 3.2.2 Task Control Block (TCB)

```c
typedef struct task {
    void (*function)(void);      // Task entry point
    uint32_t *stack_base;        // Stack base address
    uint32_t stack_size;         // Stack size in words
    uint32_t *stack_pointer;     // Current stack pointer (offset: 12)
    uint8_t task_state;          // Current state (offset: 16)
    uint32_t global_tick_paused; // Tick when sleep started (offset: 20)
    uint32_t ticks_to_pause;     // Sleep duration (offset: 24)
    uint8_t task_priority;       // Priority level
    char name[MAX_TASK_NAME_LENGTH]; // Task name for debug
} task_t;

// Static assertions ensure offsets match assembly expectations
_Static_assert(offsetof(task_t, stack_pointer) == 12);
_Static_assert(offsetof(task_t, task_state) == 16);
```

#### 3.2.3 Task States

| State | Value | Description |
|-------|-------|-------------|
| TASK_COLD | 0 | Created but never run |
| TASK_RUNNING | 1 | Currently executing |
| TASK_READY | 2 | Ready to run |
| TASK_BLOCKED | 3 | Waiting (sleep) |
| TASK_KILLED | 4 | Terminated by another task |
| TASK_FINISHED | 5 | Exited normally |

#### 3.2.4 Stack Frame Layout

```
High Address (stack_base + stack_size)
┌─────────────────────────────────────┐
│           xPSR (0x01000000)         │ ← Initial: Thumb bit set
├─────────────────────────────────────┤
│           PC (function)             │ ← Task entry point
├─────────────────────────────────────┤
│           LR (os_exit_task)         │ ← Return address
├─────────────────────────────────────┤
│           R12                       │
├─────────────────────────────────────┤
│           R3                        │
├─────────────────────────────────────┤
│           R2                        │
├─────────────────────────────────────┤
│           R1                        │
├─────────────────────────────────────┤
│           R0                        │ ← stack_pointer points here
├─────────────────────────────────────┤
│      (Software saved: R4-R11)       │ ← Saved on context switch
├─────────────────────────────────────┤
│           ...                       │
│       (Task local variables)        │
│           ...                       │
├─────────────────────────────────────┤
Low Address (stack_base)
```

#### 3.2.5 Requirements Traceability

| Design Element | Implements Requirement |
|----------------|----------------------|
| task_t structure | HLR-KRN-001, HLR-KRN-002 |
| Stack allocation | HLR-KRN-003 |
| os_exit_task() | HLR-KRN-004 |
| os_kill_process() | HLR-KRN-005 |
| Task 0 protection | HLR-KRN-006 |
| task_state field | HLR-KRN-007 |


---

### 3.3 Context Switch Component

#### 3.3.1 Design Overview

Context switching is implemented in ARM assembly for Cortex-M7. It uses the PendSV exception (lowest priority) to ensure all other interrupts complete before switching.

#### 3.3.2 Context Switch Flow

```
┌─────────────────────────────────────────────────────────────┐
│                  CONTEXT SWITCH SEQUENCE                     │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│  1. TRIGGER (SysTick or os_yield)                           │
│     └── SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk                 │
│                                                              │
│  2. PENDSV ENTRY (hardware automatic)                       │
│     └── Push xPSR, PC, LR, R12, R3-R0 to PSP               │
│                                                              │
│  3. SAVE CONTEXT (software - context_switch.s)              │
│     ├── MRS R0, PSP           // Get process stack pointer  │
│     ├── STMDB R0!, {R4-R11}   // Push R4-R11               │
│     └── Store R0 to current_task->stack_pointer            │
│                                                              │
│  4. SELECT NEXT TASK (C code)                               │
│     └── Round-robin selection, wake blocked tasks          │
│                                                              │
│  5. RESTORE CONTEXT (software - context_switch.s)           │
│     ├── Load R0 from next_task->stack_pointer              │
│     ├── LDMIA R0!, {R4-R11}   // Pop R4-R11                │
│     └── MSR PSP, R0           // Set process stack pointer  │
│                                                              │
│  6. PENDSV EXIT (hardware automatic)                        │
│     └── Pop R0-R3, R12, LR, PC, xPSR from PSP              │
│                                                              │
└─────────────────────────────────────────────────────────────┘
```

#### 3.3.3 Assembly Implementation

```asm
// context_switch.s (simplified)
.syntax unified
.thumb

.global context_switch
.type context_switch, %function

context_switch:
    // Save current context
    MRS     R0, PSP                 // Get current PSP
    STMDB   R0!, {R4-R11}          // Save R4-R11
    
    // Store stack pointer to current task
    LDR     R1, =current_task_index
    LDRB    R1, [R1]
    LDR     R2, =task_list
    LDR     R2, [R2, R1, LSL #2]   // task_list[current_task_index]
    STR     R0, [R2, #12]          // task->stack_pointer (offset 12)
    
    // Call scheduler to select next task
    PUSH    {LR}
    BL      schedule_next_task
    POP     {LR}
    
    // Load next task context
    LDR     R1, =current_task_index
    LDRB    R1, [R1]
    LDR     R2, =task_list
    LDR     R2, [R2, R1, LSL #2]
    LDR     R0, [R2, #12]          // next_task->stack_pointer
    
    LDMIA   R0!, {R4-R11}          // Restore R4-R11
    MSR     PSP, R0                 // Set PSP
    
    BX      LR                      // Return (hardware restores rest)
```

#### 3.3.4 Requirements Traceability

| Design Element | Implements Requirement |
|----------------|----------------------|
| PendSV mechanism | HLR-BSP-004 |
| PSP usage | HLR-KRN-003 (stack isolation) |
| Register save/restore | HLR-KRN-010 (preemption) |

---

### 3.4 Critical Section Component

#### 3.4.1 Design Overview

Critical sections protect shared data from concurrent access by disabling the scheduler (not interrupts).

#### 3.4.2 Implementation

```c
static volatile uint8_t critical_stack_depth = 0;
volatile bool scheduler_enabled = true;

static inline void enter_critical(void) {
    scheduler_enabled = false;
    critical_stack_depth++;
}

static inline void exit_critical(void) {
    if (--critical_stack_depth == 0)
        scheduler_enabled = true;
}
```

#### 3.4.3 Design Rationale

| Approach | Pros | Cons | Decision |
|----------|------|------|----------|
| Disable interrupts | Simple, absolute protection | Increases interrupt latency | Not used |
| Disable scheduler | Interrupts still serviced | Only protects from task preemption | **Selected** |
| Spinlock | Fine-grained | Complex, potential deadlock | Future (multi-core) |

#### 3.4.4 Requirements Traceability

| Design Element | Implements Requirement |
|----------------|----------------------|
| scheduler_enabled flag | HLR-KRN-030 |
| critical_stack_depth | HLR-KRN-031 |
| Inline implementation | HLR-KRN-032 (minimal duration) |

---

### 3.5 Semaphore Component

#### 3.5.1 Design Overview

The semaphore component implements bounded counting semaphores for producer-consumer synchronization. Semaphores use non-blocking waits (task_active_sleep) to allow other tasks to run while waiting.

#### 3.5.2 Data Structure

```c
#define MAX_SEMAPHORES 32

typedef struct {
    uint32_t count;                          // Current count
    uint32_t init_count;                     // Maximum capacity (bounded)
    uint8_t consumer_task_idx_list[MAX_TASKS]; // Tasks waiting to consume
    uint8_t feeder_task_idx_list[MAX_TASKS];   // Tasks waiting to feed
    uint8_t num_consumers_queued;            // Number of waiting consumers
    uint8_t num_feeders_queued;              // Number of waiting feeders
    uint32_t tick_updated_at;                // Last update timestamp
    bool engaged;                            // Semaphore is initialized
} semaphore_t;

semaphore_t* semaphore_list[MAX_SEMAPHORES];
```

#### 3.5.3 Semaphore Operations

```
ALGORITHM: Bounded Semaphore Feed (Producer)

INPUT: semaphore_idx
OUTPUT: true on success, false on invalid index

semaphore_feed(semaphore_idx):
    IF semaphore_idx >= MAX_SEMAPHORES OR NOT engaged:
        RETURN false
    
    WHILE count >= init_count:      // Buffer full
        task_active_sleep(1)        // Yield, let consumer run
    
    ENTER_CRITICAL
    count++
    EXIT_CRITICAL
    RETURN true


ALGORITHM: Bounded Semaphore Consume (Consumer)

INPUT: semaphore_idx
OUTPUT: true on success, false on invalid index

semaphore_consume(semaphore_idx):
    IF semaphore_idx >= MAX_SEMAPHORES OR NOT engaged:
        RETURN false
    
    WHILE count == 0:               // Buffer empty
        task_active_sleep(1)        // Yield, let producer run
    
    ENTER_CRITICAL
    count--
    EXIT_CRITICAL
    RETURN true
```

#### 3.5.4 Blocking Behavior

| Condition | Behavior |
|-----------|----------|
| Feed when count < init_count | Immediate increment |
| Feed when count >= init_count | Block until consumer decrements |
| Consume when count > 0 | Immediate decrement |
| Consume when count == 0 | Block until producer increments |

The blocking uses `task_active_sleep(1)` which yields to the scheduler, allowing other tasks to run. This is cooperative blocking, not busy-waiting.

#### 3.5.5 Requirements Traceability

| Design Element | Implements Requirement |
|----------------|----------------------|
| semaphore_t structure | HLR-KRN-041, HLR-KRN-042, HLR-KRN-043 |
| count field | HLR-KRN-042 (counting) |
| init_count (bounded) | HLR-KRN-043 |
| task_active_sleep blocking | HLR-KRN-049 (bounded WCET) |
| engaged flag | Initialization safety |
| MAX_SEMAPHORES=32 | HLR-KRN-051 |

---

### 3.6 Message Pipe Component

#### 3.6.1 Design Overview

The message pipe component implements FIFO byte-stream communication channels between tasks. Pipes support variable-length messages with bounded capacity and blocking semantics.

#### 3.6.2 Data Structure

```c
#define MAX_MESSAGE_QUEUES 32
#define MAX_PIPE_CAPACITY 128

typedef struct {
    uint8_t buffer[MAX_PIPE_CAPACITY];  // Circular buffer storage
    uint8_t head;                       // Read index
    uint8_t tail;                       // Write index
    uint8_t count;                      // Current bytes in pipe
    uint8_t capacity;                   // Maximum bytes (≤128)
    bool engaged;                       // Pipe is initialized
} message_pipe_t;

message_pipe_t* message_pipe_list[MAX_MESSAGE_QUEUES];
```

#### 3.6.3 Pipe Operations

```
ALGORITHM: Pipe Enqueue (Producer)

INPUT: pipe_idx, data (byte)
OUTPUT: true on success, false on full/invalid

pipe_enqueue(pipe_idx, data):
    IF pipe_idx >= MAX_MESSAGE_QUEUES OR NOT engaged:
        RETURN false
    
    WHILE count >= capacity:        // Pipe full
        task_active_sleep(1)        // Yield, let consumer run
    
    ENTER_CRITICAL
    buffer[tail] = data
    tail = (tail + 1) % capacity
    count++
    EXIT_CRITICAL
    RETURN true


ALGORITHM: Pipe Dequeue (Consumer)

INPUT: pipe_idx
OUTPUT: byte value, or -1 on empty/invalid

pipe_dequeue(pipe_idx):
    IF pipe_idx >= MAX_MESSAGE_QUEUES OR NOT engaged:
        RETURN -1
    
    WHILE count == 0:               // Pipe empty
        task_active_sleep(1)        // Yield, let producer run
    
    ENTER_CRITICAL
    data = buffer[head]
    head = (head + 1) % capacity
    count--
    EXIT_CRITICAL
    RETURN data
```

#### 3.6.4 Circular Buffer Layout

```
┌─────────────────────────────────────────────────────────────┐
│                    CIRCULAR PIPE BUFFER                      │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│  Index:  0   1   2   3   4   5   6   7   ...  126  127      │
│        ┌───┬───┬───┬───┬───┬───┬───┬───┬─────┬───┬───┐     │
│  Data: │ A │ B │ C │ D │   │   │   │   │ ... │   │   │     │
│        └───┴───┴───┴───┴───┴───┴───┴───┴─────┴───┴───┘     │
│          ▲               ▲                                   │
│          │               │                                   │
│       head=0          tail=4                                │
│                                                              │
│  Empty: count == 0                                          │
│  Full:  count == capacity                                   │
│  FIFO:  head follows tail, wraps at capacity                │
│                                                              │
└─────────────────────────────────────────────────────────────┘
```

#### 3.6.5 Pipe Characteristics

| Property | Value | Rationale |
|----------|-------|-----------|
| Ordering | FIFO | HLR-KRN-047 |
| Capacity | 1-128 bytes | HLR-KRN-053 |
| Message size | Variable (1-N bytes) | HLR-KRN-048 |
| Blocking | Cooperative (task_active_sleep) | HLR-KRN-050 |
| Atomicity | Critical sections | Data integrity |
| Max pipes | 32 | HLR-KRN-052 |

#### 3.6.6 Requirements Traceability

| Design Element | Implements Requirement |
|----------------|----------------------|
| message_pipe_t structure | HLR-KRN-040 |
| Circular buffer | HLR-KRN-047 (FIFO) |
| Variable count | HLR-KRN-048 (variable-length) |
| task_active_sleep blocking | HLR-KRN-050 (blocking) |
| MAX_MESSAGE_QUEUES=32 | HLR-KRN-052 |
| MAX_PIPE_CAPACITY=128 | HLR-KRN-053 |
| Bounded operations | HLR-KRN-046 (bounded WCET) |

---

### 3.7 Print Buffer Component

#### 3.7.1 Design Overview

Circular buffer for asynchronous printf output via USB CDC.

#### 3.7.2 Data Structure

```c
#define PRINT_BUFFER_BYTES 256

static uint8_t print_buffer[PRINT_BUFFER_BYTES];
volatile uint8_t print_buffer_dequeue_idx = 0;
volatile uint8_t print_buffer_enqueue_idx = 0;
```

#### 3.7.3 Buffer Operations

```
┌─────────────────────────────────────────────────────────────┐
│                    CIRCULAR BUFFER                           │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│  Index:  0   1   2   3   4   5   6   7   ...  254  255      │
│        ┌───┬───┬───┬───┬───┬───┬───┬───┬─────┬───┬───┐     │
│  Data: │ H │ e │ l │ l │ o │   │   │   │ ... │   │   │     │
│        └───┴───┴───┴───┴───┴───┴───┴───┴─────┴───┴───┘     │
│          ▲                   ▲                               │
│          │                   │                               │
│     dequeue_idx=0      enqueue_idx=5                        │
│                                                              │
│  Empty: dequeue_idx == enqueue_idx                          │
│  Full:  (enqueue_idx + 1) % SIZE == dequeue_idx             │
│                                                              │
└─────────────────────────────────────────────────────────────┘
```


---

### 3.8 Mutex Component (Planned)

#### 3.8.1 Design Overview

Inter-Process Communication provides synchronization and data exchange between tasks.

#### 3.8.2 Message Queue Design

```c
typedef struct {
    uint8_t *buffer;           // Queue storage
    size_t item_size;          // Size of each item
    size_t capacity;           // Maximum items
    volatile size_t head;      // Dequeue index
    volatile size_t tail;      // Enqueue index
    volatile size_t count;     // Current item count
    task_t *waiting_send;      // Tasks blocked on send
    task_t *waiting_recv;      // Tasks blocked on receive
} ipc_queue_t;
```

#### 3.8.3 Queue Operations

| Operation | Blocking | Timeout | WCET |
|-----------|----------|---------|------|
| `ipc_queue_send()` | Optional | Yes | O(1) + wake |
| `ipc_queue_receive()` | Optional | Yes | O(1) + wake |
| `ipc_queue_peek()` | No | N/A | O(1) |
| `ipc_queue_count()` | No | N/A | O(1) |

#### 3.8.4 Mutex Design

```c
typedef struct {
    volatile int32_t count;    // Current count (signed for mutex)
    int32_t max_count;         // Maximum count (1 for binary)
    task_t *waiting_list;      // Tasks blocked on acquire
    task_t *owner;             // For mutex: current owner
    uint8_t original_priority; // For priority inheritance
} ipc_sem_t;
```

#### 3.8.5 Requirements Traceability

| Design Element | Implements Requirement |
|----------------|----------------------|
| ipc_queue_t | HLR-KRN-044 (planned) |
| ipc_sem_t (count=1) | HLR-KRN-044 (planned) |
| ipc_sem_t (count>1) | HLR-KRN-044 (planned) |
| owner + priority fields | HLR-KRN-044 (planned) |
| Bounded operations | HLR-KRN-046 (planned) |

---

### 3.9 AI Runtime Component (Planned)

#### 3.9.1 Design Overview

The AI Runtime provides deterministic neural network inference with certifiable properties.

#### 3.9.2 Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    AI RUNTIME ARCHITECTURE                   │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│  ┌─────────────────────────────────────────────────────┐    │
│  │                    AI API Layer                      │    │
│  │  ai_model_load()  ai_inference_run()  ai_validate() │    │
│  └─────────────────────────┬───────────────────────────┘    │
│                            │                                 │
│  ┌─────────────────────────▼───────────────────────────┐    │
│  │                  Model Manager                       │    │
│  │  • Model parsing (flatbuffer/custom)                │    │
│  │  • Memory allocation (static)                       │    │
│  │  • Integrity verification (CRC32)                   │    │
│  │  • Operator validation (whitelist)                  │    │
│  └─────────────────────────┬───────────────────────────┘    │
│                            │                                 │
│  ┌─────────────────────────▼───────────────────────────┐    │
│  │                 Inference Engine                     │    │
│  │  • Layer-by-layer execution                         │    │
│  │  • Tensor memory management                         │    │
│  │  • Intermediate buffer reuse                        │    │
│  │  • WCET tracking per layer                          │    │
│  └─────────────────────────┬───────────────────────────┘    │
│                            │                                 │
│  ┌─────────────────────────▼───────────────────────────┐    │
│  │              Certified Operator Library              │    │
│  │  ┌───────┐ ┌───────┐ ┌───────┐ ┌───────┐ ┌───────┐ │    │
│  │  │Conv2D │ │ Dense │ │ ReLU  │ │Softmax│ │MaxPool│ │    │
│  │  │ int8  │ │ int8  │ │ int8  │ │ int16 │ │ int8  │ │    │
│  │  └───────┘ └───────┘ └───────┘ └───────┘ └───────┘ │    │
│  └─────────────────────────────────────────────────────┘    │
│                                                              │
└─────────────────────────────────────────────────────────────┘
```

#### 3.9.3 Model Structure

```c
typedef struct {
    uint32_t magic;            // "ICAR" (0x52414349)
    uint32_t version;          // Model format version
    uint32_t crc32;            // Integrity check
    uint32_t num_layers;       // Layer count
    uint32_t input_size;       // Input tensor size (bytes)
    uint32_t output_size;      // Output tensor size (bytes)
    uint32_t scratch_size;     // Intermediate buffer size
    uint32_t wcet_us;          // Worst-case execution time
    ai_layer_t layers[];       // Layer definitions
} ai_model_header_t;

typedef struct {
    uint8_t op_type;           // Operator type (enum)
    uint8_t activation;        // Activation function
    uint16_t flags;            // Layer flags
    uint32_t input_offset;     // Input tensor offset
    uint32_t output_offset;    // Output tensor offset
    uint32_t weights_offset;   // Weights offset
    uint32_t bias_offset;      // Bias offset
    uint16_t params[8];        // Operator-specific params
} ai_layer_t;
```

#### 3.9.4 Operator Specifications

| Operator | Input Type | Output Type | WCET Formula |
|----------|------------|-------------|--------------|
| Conv2D | int8 | int8 | O(H×W×C_in×C_out×K²) |
| Dense | int8 | int8 | O(N_in×N_out) |
| ReLU | int8 | int8 | O(N) |
| Softmax | int8 | int16 | O(N) |
| MaxPool2D | int8 | int8 | O(H×W×C×K²) |
| Add | int8 | int8 | O(N) |
| Concat | int8 | int8 | O(N) |

#### 3.9.5 Determinism Guarantees

| Property | Mechanism |
|----------|-----------|
| No dynamic allocation | Static tensor buffers |
| Bounded execution | WCET per layer, total budget |
| Reproducible results | Fixed-point only, no FP |
| Verifiable structure | Operator whitelist |
| Integrity | CRC32 on model load |

#### 3.9.6 Requirements Traceability

| Design Element | Implements Requirement |
|----------------|----------------------|
| ai_model_load() | HLR-AI-001 |
| CRC32 verification | HLR-AI-002 |
| Operator whitelist | HLR-AI-003, HLR-AI-004 |
| Layer-by-layer execution | HLR-AI-010 |
| int8/int16 types | HLR-AI-011, HLR-AI-014, HLR-AI-015 |
| wcet_us field | HLR-AI-012 |
| Static buffers | HLR-AI-013 |


---

## 4. Memory Architecture

### 4.1 Memory Map (STM32H750)

```
┌─────────────────────────────────────────────────────────────┐
│                    MEMORY MAP (STM32H750)                    │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│  0x00000000 ┌─────────────────────────────────────┐         │
│             │           ITCM (64 KB)              │         │
│             │  • Interrupt vectors                │         │
│             │  • Critical kernel code             │         │
│             │  • Context switch routine           │         │
│  0x00010000 └─────────────────────────────────────┘         │
│                                                              │
│  0x08000000 ┌─────────────────────────────────────┐         │
│             │         FLASH (128 KB)              │         │
│             │  • Application code                 │         │
│             │  • Constant data                    │         │
│             │  • AI model storage (if fits)       │         │
│  0x08020000 └─────────────────────────────────────┘         │
│                                                              │
│  0x20000000 ┌─────────────────────────────────────┐         │
│             │          DTCM (128 KB)              │         │
│             │  • Task stacks (fast access)        │         │
│             │  • Kernel data structures           │         │
│             │  • Critical variables               │         │
│  0x20020000 └─────────────────────────────────────┘         │
│                                                              │
│  0x24000000 ┌─────────────────────────────────────┐         │
│             │        AXI SRAM (512 KB)            │         │
│             │  • AI model data                    │         │
│             │  • AI tensor buffers                │         │
│             │  • Application heap (if needed)     │         │
│  0x24080000 └─────────────────────────────────────┘         │
│                                                              │
│  0x30000000 ┌─────────────────────────────────────┐         │
│             │       SRAM1-4 (288 KB)              │         │
│             │  • DMA buffers                      │         │
│             │  • Display frame buffer             │         │
│             │  • USB buffers                      │         │
│  0x30048000 └─────────────────────────────────────┘         │
│                                                              │
└─────────────────────────────────────────────────────────────┘
```

### 4.2 MPU Protection Architecture

#### 4.2.1 Overview

ICARUS OS implements hardware-enforced memory protection using the ARM Cortex-M7 Memory Protection Unit (MPU). The protection model provides:
- **Privilege separation**: Kernel runs privileged, tasks run unprivileged
- **Code protection**: ITCM is read-only to prevent code modification
- **Data isolation**: DTCM is privileged-only to protect kernel data
- **Task isolation**: Each task's data region is isolated from other tasks
- **SVC call gates**: Controlled kernel service access via Supervisor Calls

#### 4.2.2 MPU Region Configuration

| Region | Base Address | Size | Access | Purpose | Implements |
|--------|--------------|------|--------|---------|------------|
| 0 | 0x00000000 | 64 KB | PRIV_RO_URO | ITCM code (read-only for all) | HLR-KRN-063, HLR-KRN-066 |
| 1 | 0x90000000 | 8 MB | PRIV_RO_URO | QSPI Flash (read-only) | HLR-KRN-066 |
| 2 | DISABLED | - | - | Reserved (was ITCM_PRIV) | - |
| 3 | DISABLED | - | - | Reserved | - |
| 4 | Dynamic | 2 KB | PRIV_RW_URW | Task data (reconfigured on switch) | HLR-KRN-065, HLR-KRN-071 |
| 5 | 0x20000000 | 128 KB | PRIV_RO | DTCM kernel data (privileged-only) | HLR-KRN-064, HLR-KRN-070 |
| 6 | 0x24000000 | 512 KB | FULL_ACCESS | RAM_D1 (shared buffers) | - |
| 7 | 0x30000000 | 256 KB | FULL_ACCESS | RAM_D2 (DMA, display) | - |

#### 4.2.3 Memory Region Details

**Region 0: ITCM Code Protection**
```c
r.BaseAddress      = 0x00000000;
r.Size             = MPU_REGION_SIZE_64KB;
r.AccessPermission = MPU_REGION_PRIV_RO_URO;  // Read-only for all
r.DisableExec      = MPU_INSTRUCTION_ACCESS_ENABLE;
```
- Contains kernel functions marked with `ITCM_FUNC` macro
- Read-only prevents code modification attacks
- Executable by all (ARM MPU limitation: cannot restrict execution by privilege level)
- Protection enforced by DTCM isolation (kernel functions fault when accessing kernel data from unprivileged mode)

**Region 4: Task Data Isolation**
```c
// Reconfigured on every context switch
r.BaseAddress      = task->data_base;  // 2KB-aligned
r.Size             = MPU_REGION_SIZE_2KB;
r.AccessPermission = MPU_REGION_PRIV_RW_URW;  // Full access for current task
```
- Each task has 2KB data region in `data_pool` (RAM_D2)
- MPU reconfigured on context switch to grant access only to current task
- Prevents cross-task memory corruption
- Verified by MPU_REDTEAM stress test

**Region 5: DTCM Kernel Data Protection**
```c
r.BaseAddress      = 0x20000000;
r.Size             = MPU_REGION_SIZE_128KB;
r.AccessPermission = MPU_REGION_PRIV_RO;  // Privileged-only
```
- Contains kernel data structures: `task_list`, `semaphore_list`, `pipe_list`, `os_tick_count`
- Unprivileged tasks cannot read or write
- Forces use of SVC call gates for kernel services
- Prevents direct kernel function calls from working

#### 4.2.4 Privilege Separation Model

```
┌─────────────────────────────────────────────────────────────┐
│                    PRIVILEGE ARCHITECTURE                    │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│  PRIVILEGED MODE (Handler/SVC)                              │
│  ┌────────────────────────────────────────────────────────┐ │
│  │  • Kernel functions (__os_* prefix)                    │ │
│  │  • Exception handlers (SysTick, PendSV, MemManage)    │ │
│  │  • SVC dispatcher                                      │ │
│  │  • MPU configuration                                   │ │
│  │  • Access to DTCM (kernel data)                       │ │
│  │  • Access to ITCM (kernel code)                       │ │
│  └────────────────────────────────────────────────────────┘ │
│                           ▲                                  │
│                           │ SVC instruction                  │
│                           │ (controlled transition)          │
│  ┌────────────────────────┴───────────────────────────────┐ │
│  │  SVC CALL GATES (os_* wrappers)                       │ │
│  │  • os_semaphore_wait() → SVC 29                       │ │
│  │  • os_semaphore_signal() → SVC 30                     │ │
│  │  • os_pipe_send() → SVC 31                            │ │
│  │  • os_pipe_receive() → SVC 32                         │ │
│  │  • os_get_task_name() → SVC 37                        │ │
│  └────────────────────────────────────────────────────────┘ │
│                           ▲                                  │
│                           │ Function call                    │
│                           │                                  │
│  UNPRIVILEGED MODE (Thread)                                 │
│  ┌────────────────────────┴───────────────────────────────┐ │
│  │  • User task code                                      │ │
│  │  • Task local variables (stack)                       │ │
│  │  • Task data region (2KB, MPU Region 4)               │ │
│  │  • Shared buffers (RAM_D1, RAM_D2)                    │ │
│  │  • NO access to DTCM (kernel data)                    │ │
│  │  • Read-only access to ITCM (kernel code)             │ │
│  └────────────────────────────────────────────────────────┘ │
│                                                              │
└─────────────────────────────────────────────────────────────┘
```

#### 4.2.5 SVC Call Gates

All kernel services are accessed via SVC (Supervisor Call) instructions:

| SVC # | Function | Purpose | Implements |
|-------|----------|---------|------------|
| 29 | `sem_can_feed()` | Check if semaphore can accept feed | HLR-KRN-067 |
| 30 | `sem_can_consume()` | Check if semaphore can be consumed | HLR-KRN-067 |
| 31 | `sem_increment()` | Increment semaphore count | HLR-KRN-067 |
| 32 | `sem_decrement()` | Decrement semaphore count | HLR-KRN-067 |
| 33 | `pipe_can_send()` | Check if pipe can accept data | HLR-KRN-067 |
| 34 | `pipe_can_receive()` | Check if pipe has data | HLR-KRN-067 |
| 35 | `pipe_write()` | Write data to pipe | HLR-KRN-067 |
| 36 | `pipe_read()` | Read data from pipe | HLR-KRN-067 |
| 37 | `os_get_task_name()` | Get current task name | HLR-KRN-067 |
| 38 | `os_get_num_created_tasks()` | Get task count | HLR-KRN-067 |
| 39 | `os_is_running()` | Check if OS is running | HLR-KRN-067 |

#### 4.2.6 Fault Handling

The MemManage fault handler provides graceful recovery from memory protection violations:

```c
void MemManage_Handler(void) {
    // Check fault type
    bool is_daccviol = (CFSR & SCB_CFSR_DACCVIOL_Msk) != 0;  // Data access
    bool is_iaccviol = (CFSR & SCB_CFSR_IACCVIOL_Msk) != 0;  // Instruction fetch
    
    // Check privilege level
    bool from_psp = (LR & 0x4) != 0;  // From unprivileged task
    
    if (is_daccviol && !is_iaccviol && from_psp) {
        // Recoverable: skip faulting instruction
        advance_pc_by_instruction_length();
        clear_fault_status();
        return;  // Task continues
    }
    
    // Non-recoverable: halt with LED blink code
    halt_with_4_blinks();
}
```

**Fault Recovery Policy:**
- **Data access violations (DACCVIOL)** from unprivileged tasks: Recoverable, skip instruction
- **Instruction fetch violations (IACCVIOL)**: Non-recoverable, system halt
- **Faults from privileged code**: Non-recoverable, system halt

Implements: HLR-KRN-081, HLR-KRN-082, HLR-KRN-083

#### 4.2.7 Protection Verification

MPU protection is verified by stress tests:

| Test | Purpose | Expected Result | Implements |
|------|---------|-----------------|------------|
| `mpu_redteam_task` | Cross-task memory attack | Faults, shows [PROTECTED] | HLR-KRN-071 |
| `mpu_itcm_write_test` | ITCM write attempt | Faults, shows [WRITE PROTECTED] | HLR-KRN-066 |
| `mpu_dtcm_attack_task` | DTCM read attempt | Faults, shows [DTCM PROTECTED] | HLR-KRN-070 |
| `mpu_kernel_bypass_test` | Direct kernel function call | Faults, shows [DTCM PROTECTED] | HLR-KRN-070 |

All tests run continuously and display real-time status via USB CDC terminal.

### 4.3 Stack Allocation

```c
#define MAX_TASKS 16
#define STACK_WORDS 512  // 2KB per task

// Static allocation - no malloc
static uint32_t stack_pool[MAX_TASKS][STACK_WORDS];
static task_t task_pool[MAX_TASKS];
task_t* task_list[MAX_TASKS];
```

| Task Type | Stack Size | Location |
|-----------|------------|----------|
| System tasks | 2 KB | DTCM |
| User tasks | 2 KB (configurable) | DTCM |
| AI inference | 4 KB (planned) | DTCM |

### 4.4 AI Memory Budget

| Component | Size | Location |
|-----------|------|----------|
| Model header | 64 B | AXI SRAM |
| Model weights | Up to 256 KB | AXI SRAM |
| Input tensor | Model-dependent | AXI SRAM |
| Output tensor | Model-dependent | AXI SRAM |
| Scratch buffer | Model-dependent | AXI SRAM |

---

## 5. Interface Design

### 5.1 Kernel API Summary

```c
// ═══════════════════════════════════════════════════════════
// INITIALIZATION
// ═══════════════════════════════════════════════════════════
void os_init(void);
    // Initialize kernel state, register system tasks
    // Must be called before os_start()
    // Implements: HLR-KRN-001

void os_start(void);
    // Start scheduler, never returns
    // Implements: HLR-KRN-010

// ═══════════════════════════════════════════════════════════
// TASK MANAGEMENT
// ═══════════════════════════════════════════════════════════
void os_register_task(void (*function)(void), const char *name);
    // Register task using internal pool
    // Implements: HLR-KRN-001, HLR-KRN-002, HLR-KRN-003

void os_create_task(task_t *task, void (*function)(void),
                    uint32_t *stack, uint32_t stack_size,
                    const char *name);
    // Create task with external storage
    // Implements: HLR-KRN-001, HLR-KRN-002, HLR-KRN-003

void os_exit_task(void);
    // Exit current task (called automatically on return)
    // Implements: HLR-KRN-004

void os_kill_process(uint8_t task_index);
    // Terminate another task
    // Implements: HLR-KRN-005, HLR-KRN-006

// ═══════════════════════════════════════════════════════════
// SCHEDULING
// ═══════════════════════════════════════════════════════════
void os_yield(void);
    // Voluntarily give up CPU
    // Implements: HLR-KRN-013

uint32_t task_active_sleep(uint32_t ticks);
    // Non-blocking sleep (task yields)
    // Implements: HLR-KRN-021

uint32_t task_blocking_sleep(uint32_t ticks);
    // Blocking sleep (busy wait)
    // Implements: HLR-KRN-022

// ═══════════════════════════════════════════════════════════
// INFORMATION
// ═══════════════════════════════════════════════════════════
uint32_t os_get_tick_count(void);
    // Get current tick count
    // Implements: HLR-KRN-023

uint8_t os_get_running_task_count(void);
const char* os_get_current_task_name(void);
uint32_t os_get_task_ticks_remaining(void);
```

### 5.2 BSP Interface Summary

```c
// ═══════════════════════════════════════════════════════════
// PLATFORM ABSTRACTION (Current)
// ═══════════════════════════════════════════════════════════
void hal_init(void);
    // Initialize all hardware
    // Implements: HLR-BSP-001, HLR-BSP-002, HLR-BSP-003

// ═══════════════════════════════════════════════════════════
// GPIO
// ═══════════════════════════════════════════════════════════
void LED_On(void);
void LED_Off(void);
void LED_Blink(uint32_t on_ticks, uint32_t off_ticks);
    // Implements: HLR-BSP-010

// ═══════════════════════════════════════════════════════════
// COMMUNICATION
// ═══════════════════════════════════════════════════════════
int32_t platform_write(void *handle, uint8_t reg,
                       uint8_t *data, uint16_t len);
int32_t platform_read(void *handle, uint8_t reg,
                      uint8_t *data, uint16_t len);
    // Implements: HLR-BSP-011

// ═══════════════════════════════════════════════════════════
// DISPLAY
// ═══════════════════════════════════════════════════════════
void display_init(void);
void display_render_bar(uint8_t row, const char *task_name,
                        uint32_t elapsed_ticks, uint32_t period_ticks);
void display_render_banner(uint8_t row, const char *task_name,
                           bool is_on);
    // Implements: HLR-BSP-014
```


---

## 6. Design Traceability Matrix

### 6.1 Requirements → Design Traceability

This matrix traces each requirement to its design element(s).

#### 6.1.1 Kernel Requirements

| Requirement | Design Element | Section | Status |
|-------------|----------------|---------|--------|
| HLR-KRN-001 | task_pool[], MAX_TASKS | 3.2.2 | ✅ |
| HLR-KRN-002 | task_t.name, current_task_index | 3.2.2 | ✅ |
| HLR-KRN-003 | stack_pool[], task_t.stack_base | 3.2.4 | ✅ |
| HLR-KRN-004 | os_exit_task() | 5.1 | ✅ |
| HLR-KRN-005 | os_kill_process() | 5.1 | ✅ |
| HLR-KRN-006 | task_index > 0 check | 3.2 | ✅ |
| HLR-KRN-007 | task_t.task_state, TASK_* enums | 3.2.3 | ✅ |
| HLR-KRN-010 | SysTick + PendSV mechanism | 3.1, 3.3 | ✅ |
| HLR-KRN-011 | Round-robin in schedule_next_task | 3.1.2 | ✅ |
| HLR-KRN-012 | TICKS_PER_TASK constant | 3.1.3 | ✅ |
| HLR-KRN-013 | os_yield() | 5.1 | ✅ |
| HLR-KRN-014 | task_t.task_priority | 3.2.2 | 🔲 Planned |
| HLR-KRN-015 | ipc_sem_t.original_priority | 3.6.4 | 🔲 Planned |
| HLR-KRN-020 | os_tick_count | 3.1 | ✅ |
| HLR-KRN-021 | task_active_sleep() | 5.1 | ✅ |
| HLR-KRN-022 | task_blocking_sleep() | 5.1 | ✅ |
| HLR-KRN-023 | os_get_tick_count() | 5.1 | ✅ |
| HLR-KRN-024 | Tick-based wake check | 3.1.2 | ✅ |
| HLR-KRN-030 | scheduler_enabled flag | 3.4.2 | ✅ |
| HLR-KRN-031 | critical_stack_depth | 3.4.2 | ✅ |
| HLR-KRN-032 | Inline enter/exit_critical | 3.4.2 | ✅ |
| HLR-KRN-040 | ipc_queue_t | 3.8.2 | 🔲 Planned |
| HLR-KRN-041 | semaphore_t (binary) | 3.5.2 | ✅ |
| HLR-KRN-042 | semaphore_t (counting) | 3.5.2 | ✅ |
| HLR-KRN-043 | semaphore_t (bounded) | 3.5.2 | ✅ |
| HLR-KRN-044 | owner, original_priority | 3.8.4 | 🔲 Planned |
| HLR-KRN-045 | Event flags | - | 🔲 Planned |
| HLR-KRN-046 | Bounded operations | 3.5.3, 3.6.3 | ✅ |
| HLR-KRN-047 | FIFO ordering | 3.6.4 | ✅ |
| HLR-KRN-048 | Variable-length messages | 3.6.5 | ✅ |
| HLR-KRN-049 | Semaphore blocking | 3.5.4 | ✅ |
| HLR-KRN-050 | Pipe blocking | 3.6.3 | ✅ |
| HLR-KRN-051 | MAX_SEMAPHORES=32 | 3.5.2 | ✅ |
| HLR-KRN-052 | MAX_MESSAGE_QUEUES=32 | 3.6.2 | ✅ |
| HLR-KRN-053 | Pipe capacity 128 bytes | 3.6.2 | ✅ |
| HLR-KRN-060 | Static allocation only | 4.2 | ✅ |
| HLR-KRN-051 | Stack canary (planned) | - | 🔲 Planned |
| HLR-KRN-052 | MPU configuration | - | 🔲 Planned |
| HLR-KRN-060 | Fault handlers | 3.3 | ✅ |
| HLR-KRN-061 | Watchdog integration | - | 🔲 Planned |

#### 6.1.2 BSP Requirements

| Requirement | Design Element | Section | Status |
|-------------|----------------|---------|--------|
| HLR-BSP-001 | hal_init() | 5.2 | ✅ |
| HLR-BSP-002 | SystemClock_Config() | 5.2 | ✅ |
| HLR-BSP-003 | NVIC configuration | 5.2 | ✅ |
| HLR-BSP-004 | context_switch.s | 3.3 | ✅ |
| HLR-BSP-010 | LED_On/Off/Blink | 5.2 | ✅ |
| HLR-BSP-011 | platform_read/write | 5.2 | ✅ |
| HLR-BSP-012 | SPI HAL wrapper | 5.2 | ✅ |
| HLR-BSP-013 | USB CDC | 5.2 | ✅ |
| HLR-BSP-014 | display_* functions | 5.2 | ✅ |
| HLR-BSP-020 | SysTick_Config() | 3.1 | ✅ |

#### 6.1.3 AI Runtime Requirements

| Requirement | Design Element | Section | Status |
|-------------|----------------|---------|--------|
| HLR-AI-001 | ai_model_load() | 3.7.2 | 🔲 Planned |
| HLR-AI-002 | CRC32 in header | 3.7.3 | 🔲 Planned |
| HLR-AI-003 | Model validation | 3.7.2 | 🔲 Planned |
| HLR-AI-004 | Operator whitelist | 3.7.4 | 🔲 Planned |
| HLR-AI-010 | Layer-by-layer exec | 3.7.2 | 🔲 Planned |
| HLR-AI-011 | int8/int16 types | 3.7.4 | 🔲 Planned |
| HLR-AI-012 | wcet_us field | 3.7.3 | 🔲 Planned |
| HLR-AI-013 | Static buffers | 3.7.5 | 🔲 Planned |
| HLR-AI-014 | int8 operators | 3.7.4 | 🔲 Planned |
| HLR-AI-015 | int16 operators | 3.7.4 | 🔲 Planned |
| HLR-AI-020-027 | Operator library | 3.7.4 | 🔲 Planned |
| HLR-AI-030 | Output bounds check | 3.7.5 | 🔲 Planned |
| HLR-AI-031 | Confidence scores | 3.7.2 | 🔲 Planned |

### 6.2 Design → Code Traceability

| Design Element | Source File | Function/Symbol |
|----------------|-------------|-----------------|
| Scheduler | `icarus/scheduler.c` | SysTick-driven scheduling, `schedule_next_task` |
| Task Manager | `icarus/task.c` | os_create_task, os_kill_process |
| Context Switch | `icarus/context_switch.s` | context_switch, start_cold_task |
| Critical Section | `icarus/kernel.c` | `__enter_critical` / `__exit_critical` (via SVC) |
| Semaphores | `icarus/semaphore.c` | semaphore_init, semaphore_feed, semaphore_consume |
| Message Pipes | `icarus/pipe.c` | pipe_init, pipe_enqueue, pipe_dequeue |
| Print Buffer | `icarus/task.c` | enqueue_print_buffer, dequeue_print_buffer |
| BSP Init | `bsp/retarget_hal.c` | hal_init |
| GPIO | `bsp/gpio.c` | LED_On, LED_Off |
| Display | `bsp/display.c` | display_init, display_render_*, display_render_vbar |
| Interrupts | `bsp/stm32h7xx_it.c` | SysTick_Handler, PendSV_Handler |

### 6.3 Traceability Summary

This matrix covers High-Level Requirements (HLR) only. Performance (PRF) and Safety (SAF) requirements are traced separately in the SRS.

| Category | Total HLR | Designed | Implemented | Coverage |
|----------|-----------|----------|-------------|----------|
| Kernel | 35 | 35 | 30 | 86% |
| BSP | 15 | 15 | 11 | 73% |
| AI Runtime | 24 | 24 | 0 | 0% |
| **Total** | **74** | **74** | **41** | **55%** |

---

## 7. Design Verification

### 7.1 Design Review Checklist

- [ ] All requirements have corresponding design elements
- [ ] Design elements are traceable to requirements
- [ ] Data structures are adequately defined
- [ ] Algorithms are specified with complexity bounds
- [ ] Interfaces are fully documented
- [ ] Memory usage is bounded and documented
- [ ] Timing characteristics are specified
- [ ] Error handling is defined

### 7.2 Design Metrics

| Metric | Target | Current |
|--------|--------|---------|
| Requirements coverage | 100% | 100% (designed) |
| Implementation coverage | 100% | 45% |
| Interface documentation | 100% | 100% |
| Algorithm specification | 100% | 85% |

---

*End of Document*
