# ICARUS OS Core

A minimal, deterministic real-time kernel for Cortex-M. No flight logic. No AI. No magic. Just rock-solid foundations.

> **⚠️ WARNING: This is work in progress and NOT production ready.**
> 
> This kernel is under active development. It may contain bugs, incomplete features, and is not suitable for use in production systems. Use at your own risk.

## Overview

ICARUS OS is a lightweight, preemptive real-time operating system kernel designed for ARM Cortex-M7 microcontrollers (specifically STM32H750). The kernel provides deterministic task scheduling, context switching, and a clean API for embedded real-time applications.

### Key Features

- **Preemptive Round-Robin Scheduling**: Time-sliced task execution with configurable time quantum
- **Deterministic Context Switching**: Assembly-optimized context save/restore using PendSV
- **Task State Management**: Full lifecycle support (COLD, READY, RUNNING, BLOCKED, KILLED, FINISHED)
- **Active Sleep**: Cooperative sleep that allows other tasks to run
- **Blocking Sleep**: Busy-wait sleep for critical timing
- **Visual Debugging**: Terminal-based display system with progress bars and task visualization
- **USB CDC Support**: Serial communication via USB Virtual COM Port
- **MPU Configuration**: Memory Protection Unit setup for QSPI flash access

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

- **TASK_COLD (0)**: Newly created, never executed
- **TASK_RUNNING (1)**: Currently executing
- **TASK_READY (2)**: Ready to run, waiting for scheduler
- **TASK_BLOCKED (3)**: Sleeping or waiting for event
- **TASK_KILLED (4)**: Terminated by another task
- **TASK_FINISHED (5)**: Completed execution normally

### Scheduler

The kernel implements a **preemptive round-robin scheduler** with the following characteristics:

- **Time Quantum**: Configurable via `TICKS_PER_TASK` (default: 50 ticks)
- **Scheduling Trigger**: SysTick interrupt decrements `current_task_ticks_remaining`
- **Context Switch**: PendSV exception performs actual task switch
- **Task Selection**: Circular search for next READY or COLD task
- **Blocked Task Wake**: Automatically wakes tasks when sleep period expires

#### Scheduling Algorithm

1. SysTick fires every 1ms
2. Decrement `current_task_ticks_remaining`
3. When counter reaches 0:
   - Set PendSV pending bit
   - Reset counter to `TICKS_PER_TASK`
4. PendSV handler:
   - Save current task context (R4-R11, PSP)
   - Find next runnable task (circular search)
   - Check blocked tasks for wake-up
   - Restore next task context
   - Switch to next task

### Context Switching

Context switching is implemented in ARM assembly (`context_switch.s`) for optimal performance:

#### `os_yield_pendsv` - Main Context Switch Handler

- **Saves**: R4-R11 (callee-saved registers) to current task's stack
- **Stores**: Updated PSP to TCB
- **Finds**: Next runnable task (READY or COLD state)
- **Checks**: Blocked tasks for wake-up conditions
- **Restores**: R4-R11 from next task's stack
- **Switches**: PSP to next task's stack pointer

#### `start_cold_task` - Cold Task Startup

- Initializes PSP with pre-stacked frame
- Sets CONTROL register to use PSP (not MSP)
- Enables interrupts
- Branches to task function

#### Stack Frame Layout

When a task is created, the stack is initialized with:

```
High Address
    ┌─────────────┐
    │   PSR       │  ← Status register
    │   PC        │  ← Task function address
    │   LR        │  ← os_exit_task (return address)
    │   R12       │
    │   R3        │
    │   R2        │
    │   R1        │
    │   R0        │  ← SP points here initially
Low Address
```

## Board Support Package (BSP)

### Display System (`bsp/display.c`)

Terminal-based visualization system using ANSI escape codes:

- **Progress Bars**: Visual representation of task execution periods
- **Banner Display**: Heartbeat indicator with on/off states
- **Fixed-Position Rendering**: Tasks render to dedicated terminal rows
- **Non-Intrusive**: Cursor hidden, screen cleared on init

#### Display Layout

```
Row 1-8:   ICARUS OS ASCII logo and header
Row 9:     Separator line
Row 10:    Heartbeat banner (if enabled)
Row 11-13: Task progress bars (task_a, task_b, task_c)
```

#### Functions

- `display_init()`: Clears screen, prints header, initializes task rows
- `display_render_bar()`: Updates progress bar for a task
- `display_render_banner()`: Updates heartbeat indicator

### Hardware Abstraction (`bsp/retarget_hal.c`)

Initializes all hardware peripherals:

- **MPU Configuration**: Memory protection for QSPI flash
- **Cache Enable**: Instruction and data cache
- **Clock Configuration**: System clock setup (480 MHz)
- **Peripheral Init**: GPIO, RTC, SPI4, TIM1, I2C2, USB Device

### System Tick (`stm32h7xx_it.c`)

SysTick interrupt handler (1ms period):

```c
void SysTick_Handler(void) {
    os_tick_count++;  // Global tick counter
    
    // Preemptive scheduling
    if (os_running && --current_task_ticks_remaining == 0 && scheduler_enabled) {
        current_task_ticks_remaining = TICKS_PER_TASK;
        SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;  // Trigger context switch
    }
    
    HAL_IncTick();  // HAL tick counter
}
```

## API Reference

### Kernel Initialization

```c
void os_init(void);
```
Initializes the kernel, creates system tasks (idle, heartbeat), and sets up task pools.

```c
void os_start(void);
```
Starts the kernel scheduler. This function never returns - it begins executing the first task.

### Task Management

```c
void os_register_task(void (*function)(void), const char *name);
```
Registers a new task with the kernel. The task will be created in COLD state.

**Parameters:**
- `function`: Task entry point (must be `void function(void)`)
- `name`: Task name for debugging (max 32 characters)

**Example:**
```c
os_register_task(my_task, "my_task");
```

### Task Control

```c
void os_yield(void);
```
Voluntarily yields the CPU to the next task. Resets the time quantum.

```c
uint32_t task_active_sleep(uint32_t ticks);
```
Puts the current task to sleep for the specified number of ticks. Other tasks can run during this time. Returns actual sleep duration.

```c
uint32_t task_blocking_sleep(uint32_t ticks);
```
Busy-waits for the specified number of ticks. CPU is not yielded. Use sparingly.

```c
void os_exit_task(void);
```
Terminates the current task. Task state changes to FINISHED.

```c
void os_kill_process(uint8_t task_index);
```
Kills another task by index. Task state changes to KILLED.

```c
void suicide(void);
```
Kills the current task (convenience wrapper).

### System Information

```c
uint32_t os_get_tick_count(void);
```
Returns the global system tick count (milliseconds since boot).

```c
const char* os_get_current_task_name(void);
```
Returns the name of the currently running task.

```c
uint32_t os_get_task_ticks_remaining(void);
```
Returns remaining ticks in the current time quantum.

```c
uint8_t os_get_running_task_count(void);
```
Returns the number of tasks in RUNNING or READY state.

## Configuration

### Kernel Configuration (`Core/Inc/main.h`)

```c
#define TICKS_PER_TASK 50  // Time quantum per task (in SysTick periods = 50ms)
```

### Task Limits (`Core/Inc/kernel/task.h`)

```c
#define MAX_TASKS 10              // Maximum number of tasks
#define STACK_WORDS 512           // Stack size per task (512 * 4 = 2048 bytes)
#define MAX_TASK_NAME_LENGTH 32   // Maximum task name length
```

### Display Configuration (`Core/Inc/bsp/display.h`)

```c
#define ENABLE_HEARTBEAT_VISUALIZATION 1  // Enable/disable heartbeat display

// Task period durations (in ticks)
#define TASK_A_PERIOD_TICKS  2000  // 2 seconds
#define TASK_B_PERIOD_TICKS  4000  // 4 seconds
#define TASK_C_PERIOD_TICKS  3000  // 3 seconds

#define RENDER_INTERVAL_TICKS  20  // Display update frequency
#define CYCLE_PAUSE_TICKS      100 // Pause between task cycles
```

## Code Structure

```
icarus-os-core/
├── Core/
│   ├── Inc/
│   │   ├── main.h                    # Main header, kernel config
│   │   ├── kernel/
│   │   │   └── task.h                # Task API and TCB definition
│   │   └── bsp/
│   │       ├── display.h             # Display system API
│   │       ├── retarget_hal.h        # HAL initialization
│   │       ├── gpio.h                # GPIO configuration
│   │       ├── i2c.h                 # I2C configuration
│   │       ├── spi.h                 # SPI configuration
│   │       ├── rtc.h                 # RTC configuration
│   │       ├── tim.h                 # Timer configuration
│   │       └── stm32h7xx_it.h        # Interrupt handlers
│   └── Src/
│       ├── main.c                    # Application entry, user tasks
│       ├── kernel/
│       │   ├── task.c                # Task management, scheduler logic
│       │   └── context_switch.s      # Assembly context switching
│       └── bsp/
│           ├── display.c              # Terminal display system
│           ├── retarget_hal.c         # Hardware initialization
│           ├── gpio.c                 # GPIO setup
│           ├── i2c.c                  # I2C setup
│           ├── spi.c                  # SPI setup
│           ├── rtc.c                  # RTC setup
│           ├── tim.c                  # Timer setup
│           └── stm32h7xx_it.c         # Interrupt service routines
├── Drivers/                           # STM32 HAL drivers
├── Middlewares/                       # USB Device Library
├── USB_DEVICE/                        # USB CDC configuration
├── STM32H750VBTX_FLASH.ld             # Flash linker script
└── STM32H750VBTX_RAM.ld               # RAM linker script
```

## System Tasks

The kernel automatically creates two system tasks:

1. **ICARUS_KEEPALIVE_TASK** (`os_idle_task`): Idle task that initializes display and yields CPU
2. **ICARUS_HEARTBEART_TASK** (`os_heartbeart_task`): Heartbeat LED and display indicator

These tasks are registered first (indices 0 and 1), so user tasks start at index 2.

## Example Application

The main application demonstrates three concurrent tasks with different execution periods:

```c
void test_task_a(void) {
    const char* task_name = os_get_current_task_name();
    
    while (1) {
        uint32_t period_start = os_get_tick_count();
        
        // Render progress bar continuously
        do {
            uint32_t elapsed = os_get_tick_count() - period_start;
            display_render_bar(ROW_TASK_A, task_name, elapsed, TASK_A_PERIOD_TICKS);
            
            uint32_t remaining = (elapsed < TASK_A_PERIOD_TICKS) ? 
                                 (TASK_A_PERIOD_TICKS - elapsed) : 0;
            
            if (remaining > 0) {
                uint32_t sleep = (remaining < RENDER_INTERVAL_TICKS) ? 
                                remaining : RENDER_INTERVAL_TICKS;
                task_active_sleep(sleep);
            }
        } while (elapsed < TASK_A_PERIOD_TICKS);
        
        task_active_sleep(CYCLE_PAUSE_TICKS);
    }
}
```

## Hardware Requirements

- **MCU**: STM32H750VBTx (Cortex-M7, 480 MHz)
- **Flash**: Internal or external QSPI flash
- **RAM**: Internal SRAM
- **Display**: Terminal via USB CDC (Virtual COM Port)
- **LED**: GPIO pin E3 for heartbeat indicator
- **Peripherals**: 
  - SPI4: Display controller (ST7735)
  - I2C2: IMU sensor (LSM9DS1)
  - TIM1: Timer
  - RTC: Real-time clock
  - USB OTG FS: USB communication

## Build System

The project includes both STM32CubeIDE and standalone Makefile build systems. The standalone Makefile provides enhanced features for DO-178C compliance.

### Building with Makefile (Recommended for DO-178C)

See `build/README.md` for detailed build instructions.

**Quick start:**
```bash
cd build
make                    # Build project
make check-build        # Run static analysis + build
make COVERAGE=yes all   # Build with code coverage
make coverage-html      # Generate HTML coverage report
```

**Features:**
- Enhanced compiler warnings (DO-178C preparation)
- Static analysis with cppcheck
- Code coverage with gcov/lcov
- Clean build system without IDE dependencies

### Building with STM32CubeIDE

1. Open project in STM32CubeIDE
2. Build project (Project → Build All)
3. Flash to target (Run → Debug)

### Debugging

- USB CDC provides serial terminal output
- Connect via serial terminal (115200 baud, 8N1)
- Terminal should support ANSI escape codes for proper display

## Memory Layout

- **Stack per Task**: 512 words (2048 bytes)
- **Task Pool**: Static allocation for MAX_TASKS tasks
- **Stack Pool**: Static allocation for MAX_TASKS stacks
- **Print Buffer**: 64 bytes circular buffer for printf

## Interrupt Priority

- **SysTick**: Highest priority (tick counter and scheduler)
- **PendSV**: Lowest priority (context switching)
- **USB OTG FS**: Medium priority (USB communication)

## Critical Sections

The kernel uses a critical section mechanism to protect shared resources:

```c
static inline void enter_critical() {
    scheduler_enabled = false;
    critical_stack_depth++;
}

static inline void exit_critical() {
    if (--critical_stack_depth == 0)
        scheduler_enabled = true;
}
```

Critical sections disable preemption but do not disable interrupts. Use for short operations only.

## DO-178C Compliance Status

This project is being developed with DO-178C standards in mind. Current compliance features:

✅ **Completed:**
- Static analysis with cppcheck (0 errors, 0 warnings)
- Enhanced compiler warnings (-Wall -Wextra -Wpedantic, etc.)
- Code coverage support (gcov/lcov)
- Unit testing framework (Unity)
- Host-based unit tests with mocks
- Test build system integrated with coverage

🔄 **In Progress:**
- Expanding test coverage (currently 5 tests, targeting 100% statement coverage)
- Code coverage collection and reporting
- Requirements traceability

📋 **Planned:**
- MISRA-C compliance checking
- MC/DC coverage analysis
- Integration tests
- Formal verification documentation

### Running Tests

```bash
cd tests
make test              # Run tests
make COVERAGE=yes test # Run tests with coverage
make coverage-html     # Generate HTML coverage report
```

See `tests/README.md` for detailed testing documentation.

## Limitations

**Note**: This kernel is work in progress and not production ready. The following limitations are current as of this version:

- **No Priority Preemption**: All tasks have equal priority (round-robin only)
- **No Mutex/Semaphore**: No synchronization primitives (yet)
- **No Dynamic Allocation**: All memory is statically allocated
- **Fixed Stack Size**: All tasks use the same stack size
- **No Interrupt Nesting Control**: Critical sections don't disable interrupts

## License

Apache License 2.0 - See LICENSE file for details.

## Author

Created by Souham Biswas for deterministic real-time embedded systems.