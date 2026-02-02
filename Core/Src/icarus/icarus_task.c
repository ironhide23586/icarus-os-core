/**
 * @file    icarus_task.c
 * @brief   ICARUS (Intelligent Cooperative Architecture for Real-time 
 *          Unified Systems) OS Task Management Implementation
 * @version 0.1.0
 *
 * @details Core kernel implementation including:
 *          - Task creation and lifecycle management
 *          - Round-robin preemptive scheduler
 *          - Counting semaphores
 *          - Message pipes (queues)
 *
 * @par Memory Layout (STM32H750):
 *      - DTCM (128KB): Task pools, semaphores, pipes - zero wait-state
 *      - ITCM (64KB):  Hot-path functions - zero wait-state
 *      - RAM_D1 (512KB): Stack pool, BSS
 *
 * @see docs/do178c/design/SDD.md for detailed design documentation
 * @see STM32H750 Reference Manual RM0433 for memory architecture
 *
 * @author  Souham Biswas
 * @date    2025
 *
 * @copyright Copyright 2025 Souham Biswas
 *            https://github.com/ironhide23586/icarus-os-core
 *            Licensed under the Apache License, Version 2.0
 */

#include "icarus/icarus_task.h"
#include "bsp/config.h"
#if ICARUS_ENABLE_HEARTBEAT_VIS
#include "bsp/display.h"
#endif
#include "bsp/led.h"
#include "bsp/error.h"

#ifdef HOST_TEST
#include "cmsis_gcc.h"
#endif

#include <string.h>
#include <stdio.h>
#include <stddef.h>

/* ============================================================================
 * SECTION PLACEMENT MACROS
 * ========================================================================= */

/**
 * @brief Place function in ITCM for zero wait-state execution
 * @note  ITCM is 64KB, use sparingly for hot-path code only
 * @see   STM32H750 Reference Manual RM0433, Section 2.3.1
 */
#ifndef HOST_TEST
#define ITCM_FUNC __attribute__((section(".itcm")))
#define DTCM_DATA __attribute__((section(".dtcm")))
#else
#define ITCM_FUNC
#define DTCM_DATA
#endif

/* ============================================================================
 * COMPILE-TIME STRUCTURE VALIDATION
 * ========================================================================= */

/**
 * @brief Verify TCB field offsets match assembly code expectations
 * @note  context_switch.s uses hardcoded offsets for performance
 * @see   Core/Src/icarus/context_switch.s
 */
#ifndef SKIP_STATIC_ASSERTS
_Static_assert(offsetof(icarus_task_t, stack_pointer) == 12,
               "TCB stack_pointer offset mismatch - update context_switch.s");
_Static_assert(offsetof(icarus_task_t, task_state) == 16,
               "TCB task_state offset mismatch - update context_switch.s");
_Static_assert(offsetof(icarus_task_t, global_tick_paused) == 20,
               "TCB global_tick_paused offset mismatch - update context_switch.s");
_Static_assert(offsetof(icarus_task_t, ticks_to_pause) == 24,
               "TCB ticks_to_pause offset mismatch - update context_switch.s");
#endif

/* ============================================================================
 * KERNEL DATA STRUCTURES (DTCM - Zero Wait State)
 * ========================================================================= */

/**
 * @defgroup KERNEL_DATA Kernel Data Structures
 * @brief    Core kernel state placed in DTCM for fast access
 * @{
 */

/** @brief Array of pointers to all task control blocks */
DTCM_DATA icarus_task_t* task_list[ICARUS_MAX_TASKS];

/** @brief Array of pointers to all semaphores */
DTCM_DATA icarus_semaphore_t* semaphore_list[ICARUS_MAX_SEMAPHORES];

/** @brief Array of pointers to all message pipes */
DTCM_DATA icarus_pipe_t* message_pipe_list[ICARUS_MAX_MESSAGE_QUEUES];

/** @brief Static pool of task control blocks */
DTCM_DATA static icarus_task_t task_pool[ICARUS_MAX_TASKS];

/** @brief Static pool of semaphores */
DTCM_DATA static icarus_semaphore_t semaphore_pool[ICARUS_MAX_SEMAPHORES];

/** @brief Static pool of message pipes */
DTCM_DATA static icarus_pipe_t message_pipe_pool[ICARUS_MAX_MESSAGE_QUEUES];

/** @brief Queue of task indices pending cleanup */
DTCM_DATA static int8_t cleanup_task_idx[ICARUS_MAX_TASKS];

/** @} */ /* End of KERNEL_DATA */

/* ============================================================================
 * SCHEDULER STATE (DTCM - Zero Wait State)
 * ========================================================================= */

/**
 * @defgroup SCHEDULER_STATE Scheduler State Variables
 * @brief    Variables accessed frequently by scheduler and ISRs
 * @{
 */

/** @brief Index of currently executing task */
DTCM_DATA uint8_t current_task_index;

/** @brief Count of tasks in active states (RUNNING/READY/BLOCKED) */
DTCM_DATA uint8_t running_task_count;

/** @brief Total number of created tasks */
DTCM_DATA uint8_t num_created_tasks;

/** @brief Ticks remaining in current task's time slice */
DTCM_DATA volatile uint32_t current_task_ticks_remaining;

/** @brief Configured ticks per time slice */
DTCM_DATA volatile uint32_t ticks_per_task;

/** @brief Virtual CPU registers for context save/restore */
DTCM_DATA uint32_t cpu_vregisters[16];

/** @brief System tick counter (incremented by SysTick_Handler) */
DTCM_DATA volatile uint32_t os_tick_count;

/** @brief Flag indicating OS is running (tasks are executing) */
DTCM_DATA volatile uint8_t os_running;

/** @brief Critical section nesting depth */
DTCM_DATA volatile uint8_t critical_stack_depth;

/** @brief Scheduler enable flag (false during critical sections) */
DTCM_DATA volatile bool scheduler_enabled;

/** @brief Index into cleanup_task_idx array */
DTCM_DATA int8_t current_cleanup_task_idx;

/** @} */ /* End of SCHEDULER_STATE */

/* ============================================================================
 * STACK POOL (RAM_D1 - Too large for DTCM)
 * ========================================================================= */

/**
 * @brief Static stack pool for all tasks
 * @note  Placed in RAM_D1 (512KB available)
 *        With ICARUS_MAX_TASKS=128 and ICARUS_STACK_WORDS=512:
 *        128 × 512 × 4 = 256KB
 */
static uint32_t stack_pool[ICARUS_MAX_TASKS][ICARUS_STACK_WORDS];

/* ============================================================================
 * EXTERNAL ASSEMBLY FUNCTIONS
 * ========================================================================= */

/**
 * @brief Start execution of a cold (never-run) task
 * @param task  Pointer to task control block
 * @see   Core/Src/icarus/context_switch.s
 */
extern void start_cold_task(icarus_task_t *task);

/**
 * @brief Context switch handler (called from PendSV)
 * @see   Core/Src/icarus/context_switch.s
 */
extern void os_yield_pendsv(void);

/* ============================================================================
 * CRITICAL SECTION MANAGEMENT
 * ========================================================================= */

/**
 * @brief   Enter critical section (disable scheduler)
 *
 * @details Disables the scheduler to prevent context switches during
 *          critical operations. Supports nesting - each enter_critical()
 *          must be paired with an exit_critical().
 *
 *          Critical sections protect shared data from race conditions
 *          when accessed by multiple tasks.
 *
 * @pre     Called from task context
 *
 * @post    scheduler_enabled = false
 * @post    critical_stack_depth incremented
 *
 * @note    Does NOT disable interrupts - ISRs still execute
 * @note    Must be paired with exit_critical()
 *
 * @warning Holding critical sections too long starves other tasks
 *
 * @see     exit_critical()
 */
static inline void enter_critical(void)
{
    scheduler_enabled = false;
    critical_stack_depth++;
}

/**
 * @brief   Exit critical section (re-enable scheduler if outermost)
 *
 * @details Decrements the critical section nesting depth. When depth
 *          reaches zero, re-enables the scheduler allowing context
 *          switches to occur.
 *
 * @pre     enter_critical() must have been called
 * @pre     critical_stack_depth > 0
 *
 * @post    critical_stack_depth decremented
 * @post    If depth == 0, scheduler_enabled = true
 *
 * @note    Must be paired with enter_critical()
 *
 * @see     enter_critical()
 */
static inline void exit_critical(void)
{
    if (--critical_stack_depth == 0) {
        scheduler_enabled = true;
    }
}

/* ============================================================================
 * INTERNAL HELPER FUNCTIONS
 * ========================================================================= */

/**
 * @brief   Initialize a semaphore structure (internal)
 *
 * @details Internal helper to configure semaphore fields. Called by
 *          os_init() for pool initialization and semaphore_init() for
 *          user semaphores.
 *
 * @param   semaphore_idx   Index in semaphore pool
 * @param   semaphore_count Initial and maximum count value
 * @param   should_engage   true to mark semaphore as active
 *
 * @note    Internal function - not part of public API
 * @note    Does not use critical section - caller must protect
 */
static inline void __init_sem(uint8_t semaphore_idx, uint32_t semaphore_count,
                              bool should_engage)
{
    semaphore_list[semaphore_idx]->count = semaphore_count;
    semaphore_list[semaphore_idx]->max_count = semaphore_count;
    semaphore_list[semaphore_idx]->tick_updated_at = os_tick_count;
    semaphore_list[semaphore_idx]->engaged = should_engage;
}

/**
 * @brief   Initialize a message pipe structure (internal)
 *
 * @details Internal helper to configure pipe fields and clear buffer.
 *          Called by os_init() for pool initialization and pipe_init()
 *          for user pipes.
 *
 * @param   message_pipe_idx  Index in pipe pool
 * @param   max_messages      Maximum buffer capacity in bytes
 * @param   should_engage     true to mark pipe as active
 *
 * @note    Internal function - not part of public API
 * @note    Does not use critical section - caller must protect
 * @note    Clears entire buffer to zero
 */
static inline void __init_pipe(uint8_t message_pipe_idx, uint8_t max_messages,
                               bool should_engage)
{
    message_pipe_list[message_pipe_idx]->count = 0;
    message_pipe_list[message_pipe_idx]->max_count = max_messages;
    message_pipe_list[message_pipe_idx]->enqueue_idx = 0;
    message_pipe_list[message_pipe_idx]->dequeue_idx = 0;
    message_pipe_list[message_pipe_idx]->tick_updated_at = os_tick_count;
    message_pipe_list[message_pipe_idx]->engaged = should_engage;

    /* Zero the buffer */
    for (uint16_t i = 0; i < ICARUS_MAX_MESSAGE_BYTES; i++) {
        message_pipe_list[message_pipe_idx]->buffer[i] = 0;
    }
}

/* ============================================================================
 * SYSTEM TASKS
 * ========================================================================= */

/**
 * @brief   Idle task - runs when no other tasks are ready
 *
 * @details The idle task is the lowest-priority system task that runs
 *          when all other tasks are blocked or sleeping. It performs
 *          one-time display initialization on first run, then yields
 *          continuously.
 *
 *          This task ensures the scheduler always has something to run
 *          and prevents the system from halting when all user tasks
 *          are blocked.
 *
 * @note    Registered automatically by os_init() as task index 0
 * @note    Cannot be killed - system stability depends on it
 * @note    Could use __WFI() for low power in production
 *
 * @see     os_init()
 */
static void os_idle_task(void)
{
    /* Initialize display subsystem (only first task does this) */
    display_init();

    while (1) {
        /* Could use __WFI() for low power, but yield for now */
        os_yield();
    }
}

/**
 * @brief   Heartbeat task - blinks LED to indicate OS is alive
 *
 * @details Visual indicator that the scheduler is running correctly.
 *          Blinks the onboard LED at a configurable rate and optionally
 *          updates the display with heartbeat status.
 *
 *          The heartbeat provides immediate visual feedback that:
 *          - The CPU is running
 *          - The scheduler is functioning
 *          - Tasks are being switched
 *
 * @par Timing (default):
 *      - ON period:  ICARUS_HEARTBEAT_ON_TICKS (100ms)
 *      - OFF period: ICARUS_HEARTBEAT_OFF_TICKS (900ms)
 *      - Total period: 1 second (1Hz blink rate)
 *
 * @note    Registered automatically by os_init() as task index 1
 * @note    Cannot be killed - useful for debugging
 * @note    Display updates controlled by ICARUS_ENABLE_HEARTBEAT_VIS
 *
 * @see     os_init()
 * @see     ICARUS_HEARTBEAT_ON_TICKS
 * @see     ICARUS_HEARTBEAT_OFF_TICKS
 * @see     ICARUS_ENABLE_HEARTBEAT_VIS
 */
static void os_heartbeat_task(void)
{
#if ICARUS_ENABLE_HEARTBEAT_VIS
    const char* task_name = os_get_current_task_name();
#endif

    while (1) {
        if (os_running) {
#if ICARUS_ENABLE_HEARTBEAT_VIS
            /* ON phase with display update */
            display_render_banner(ROW_HEARTBEAT, task_name, true);
            task_active_sleep(8);
            LED_On();
            task_active_sleep(ICARUS_HEARTBEAT_ON_TICKS - 1);

            /* OFF phase with display update */
            display_render_banner(ROW_HEARTBEAT, task_name, false);
            task_active_sleep(8);
            LED_Off();
            task_active_sleep(ICARUS_HEARTBEAT_OFF_TICKS - 1);
#else
            /* Simple LED blink without display */
            LED_Blink(ICARUS_HEARTBEAT_ON_TICKS, ICARUS_HEARTBEAT_OFF_TICKS);
#endif
        } else {
#if ICARUS_ENABLE_HEARTBEAT_VIS
            display_render_banner(ROW_HEARTBEAT, task_name, false);
#endif
            task_active_sleep(100);
        }
    }
}

/* ============================================================================
 * KERNEL INITIALIZATION
 * ========================================================================= */

/**
 * @brief   Initialize the ICARUS kernel and all subsystems
 *
 * @details This function performs complete kernel initialization including:
 *          - Resetting all scheduler state variables
 *          - Initializing the task pool with empty TCBs
 *          - Initializing the semaphore pool
 *          - Initializing the message pipe pool
 *          - Clearing virtual CPU registers
 *          - Registering system tasks (idle and heartbeat)
 *
 * @par Memory Initialization:
 *      | Pool            | Count                    | Location |
 *      |-----------------|--------------------------|----------|
 *      | Task TCBs       | ICARUS_MAX_TASKS         | DTCM     |
 *      | Semaphores      | ICARUS_MAX_SEMAPHORES    | DTCM     |
 *      | Message Pipes   | ICARUS_MAX_MESSAGE_QUEUES| DTCM     |
 *      | Task Stacks     | ICARUS_MAX_TASKS         | RAM_D1   |
 *
 * @pre     Hardware must be initialized (clocks, MPU, caches)
 * @pre     hal_init() must have been called
 *
 * @post    Kernel is ready for task registration via os_register_task()
 * @post    System tasks (idle, heartbeat) are registered
 * @post    Scheduler is enabled but not yet running
 *
 * @note    Call os_start() after registering application tasks to begin
 *          multitasking
 *
 * @warning Must be called exactly once before any other kernel functions
 *
 * @see     os_start()
 * @see     os_register_task()
 * @see     hal_init()
 *
 * @par Example:
 * @code
 *     int main(void) {
 *         hal_init();
 *         os_init();
 *         os_register_task(my_task, "MyTask");
 *         os_start();  // Never returns
 *     }
 * @endcode
 */
void os_init(void)
{
    uint8_t i;

    /* Initialize scheduler state */
    os_running = 0;
    ticks_per_task = ICARUS_TICKS_PER_TASK;
    running_task_count = 0;
    current_task_index = 0;
    current_cleanup_task_idx = -1;
    current_task_ticks_remaining = ticks_per_task;

    /* Initialize task pool */
    for (i = 0; i < ICARUS_MAX_TASKS; i++) {
        task_list[i] = &task_pool[i];
        cleanup_task_idx[i] = -1;
    }

    /* Initialize semaphore pool */
    for (i = 0; i < ICARUS_MAX_SEMAPHORES; i++) {
        semaphore_list[i] = &semaphore_pool[i];
        __init_sem(i, 0, false);
    }

    /* Initialize message pipe pool */
    for (i = 0; i < ICARUS_MAX_MESSAGE_QUEUES; i++) {
        message_pipe_list[i] = &message_pipe_pool[i];
        __init_pipe(i, 0, false);
    }

    /* Clear virtual registers */
    for (i = 0; i < 16; i++) {
        cpu_vregisters[i] = 0;
    }

    /* Register system tasks */
    os_register_task(os_idle_task, "ICARUS_KEEPALIVE_TASK");
    os_register_task(os_heartbeat_task, ">ICARUS_HEARTBEAT<");

    scheduler_enabled = true;
}

/* ============================================================================
 * TASK INFORMATION (ITCM - Hot Path)
 * ========================================================================= */

/**
 * @brief   Get the current system tick count
 *
 * @details Returns the number of SysTick interrupts since os_start() was
 *          called. The tick count is incremented by SysTick_Handler at
 *          the rate configured by ICARUS_TICK_RATE_HZ (default 1000 Hz).
 *
 *          This function is placed in ITCM for zero wait-state access
 *          as it is called frequently by timing-sensitive code.
 *
 * @return  uint32_t  Current tick count (wraps at 2^32)
 *
 * @note    Tick count wraps approximately every 49.7 days at 1kHz
 * @note    Thread-safe: reads volatile variable atomically
 *
 * @par Timing Calculations:
 *      - Ticks to milliseconds: ticks * 1000 / ICARUS_TICK_RATE_HZ
 *      - Milliseconds to ticks: ms * ICARUS_TICK_RATE_HZ / 1000
 *
 * @see     ICARUS_TICK_RATE_HZ
 * @see     SysTick_Handler()
 *
 * @par Example:
 * @code
 *     uint32_t start = os_get_tick_count();
 *     do_work();
 *     uint32_t elapsed = os_get_tick_count() - start;
 *     printf("Work took %lu ticks\n", elapsed);
 * @endcode
 */
ITCM_FUNC uint32_t os_get_tick_count(void)
{
    return os_tick_count;
}

/**
 * @brief   Get the count of tasks in active states
 *
 * @details Returns the number of tasks currently in RUNNING, READY, or
 *          BLOCKED states. Does not include KILLED or FINISHED tasks.
 *
 *          This count is used by the scheduler to determine if there
 *          are tasks available to run.
 *
 * @return  uint8_t  Number of active tasks (0 to ICARUS_MAX_TASKS)
 *
 * @note    Includes system tasks (idle, heartbeat) in the count
 *
 * @see     os_register_task()
 * @see     os_kill_process()
 */
uint8_t os_get_running_task_count(void)
{
    return running_task_count;
}

/**
 * @brief   Get the name of the currently executing task
 *
 * @details Returns a pointer to the null-terminated name string of the
 *          task currently running on the CPU. The name is set during
 *          task creation via os_register_task() or os_create_task().
 *
 * @return  const char*  Pointer to task name string, or "unknown" if
 *                       no valid task is running
 *
 * @note    The returned pointer is valid for the lifetime of the task
 * @note    Maximum name length is ICARUS_MAX_TASK_NAME_LEN (32 chars)
 *
 * @see     os_register_task()
 * @see     ICARUS_MAX_TASK_NAME_LEN
 *
 * @par Example:
 * @code
 *     printf("Running: %s\n", os_get_current_task_name());
 * @endcode
 */
const char* os_get_current_task_name(void)
{
    if (current_task_index < num_created_tasks &&
        task_list[current_task_index] != NULL) {
        return task_list[current_task_index]->name;
    }
    return "unknown";
}

/**
 * @brief   Get ticks remaining in current task's time slice
 *
 * @details Returns the number of ticks remaining before the scheduler
 *          will preempt the current task. This value is decremented
 *          by SysTick_Handler and reset to ticks_per_task on context
 *          switch or explicit yield.
 *
 * @return  uint32_t  Ticks remaining (0 to ICARUS_TICKS_PER_TASK)
 *
 * @note    A value of 0 indicates preemption is imminent
 * @note    Useful for tasks that need to complete atomic operations
 *          before being preempted
 *
 * @see     ICARUS_TICKS_PER_TASK
 * @see     os_yield()
 *
 * @par Example:
 * @code
 *     if (os_get_task_ticks_remaining() < 5) {
 *         os_yield();  // Voluntarily yield before preemption
 *     }
 * @endcode
 */
uint32_t os_get_task_ticks_remaining(void)
{
    return current_task_ticks_remaining;
}

/* ============================================================================
 * TASK SCHEDULING (ITCM - Hot Path)
 * ========================================================================= */

/**
 * @brief   Start a cold task (internal helper)
 *
 * @details Initiates execution of a task that has never run before.
 *          Called by os_start() to begin the first task.
 *
 * @param   task  Pointer to task control block to start
 *
 * @pre     Task must be in TASK_STATE_COLD state
 * @pre     Task stack must be properly initialized
 *
 * @note    Internal function - not part of public API
 *
 * @see     start_cold_task() (assembly implementation)
 */
static void task_start(icarus_task_t *task)
{
    start_cold_task(task);
}

/**
 * @brief   Voluntarily yield CPU to scheduler
 *
 * @details Causes the current task to immediately give up its remaining
 *          time slice and trigger a context switch. The scheduler will
 *          select the next ready task using round-robin scheduling.
 *
 *          This function triggers PendSV exception which performs the
 *          actual context switch in a safe manner (lowest priority).
 *
 *          Placed in ITCM for zero wait-state execution as this is
 *          called frequently by cooperative multitasking code.
 *
 * @pre     Scheduler must be running (os_start() called)
 * @pre     Called from task context (not ISR)
 *
 * @post    Current task is placed at end of ready queue
 * @post    Next ready task begins execution
 *
 * @note    Safe to call from any task at any time
 * @note    Does not block - returns immediately after context switch back
 *
 * @see     task_active_sleep() for timed yields
 * @see     os_yield_pendsv() (assembly handler)
 *
 * @par Example:
 * @code
 *     while (work_pending()) {
 *         do_some_work();
 *         os_yield();  // Let other tasks run
 *     }
 * @endcode
 */
ITCM_FUNC void os_yield(void)
{
    current_task_ticks_remaining = ticks_per_task;
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

/**
 * @brief   Sleep for specified ticks while allowing other tasks to run
 *
 * @details Puts the current task into BLOCKED state for the specified
 *          number of ticks. The scheduler will skip this task until
 *          the sleep period expires, allowing other tasks to execute.
 *
 *          This is the preferred sleep method for cooperative multitasking
 *          as it does not waste CPU cycles. The task is automatically
 *          returned to READY state by the scheduler when ticks expire.
 *
 *          Placed in ITCM for zero wait-state execution.
 *
 * @param   ticks  Number of ticks to sleep (1 tick = 1ms at 1kHz)
 *
 * @return  uint32_t  Actual ticks elapsed (may be >= requested due to
 *                    scheduling latency)
 *
 * @pre     Scheduler must be running
 * @pre     Called from task context (not ISR)
 *
 * @post    Task state temporarily set to TASK_STATE_BLOCKED
 * @post    Task resumes after at least 'ticks' have elapsed
 *
 * @note    Minimum sleep is 1 tick; 0 is treated as immediate yield
 * @note    Actual sleep may be longer due to higher-priority tasks
 *
 * @see     task_blocking_sleep() for non-preemptible sleep
 * @see     task_busy_wait() for spin-wait delay
 *
 * @par Example:
 * @code
 *     // Blink LED at 1Hz
 *     while (1) {
 *         LED_On();
 *         task_active_sleep(500);  // 500ms on
 *         LED_Off();
 *         task_active_sleep(500);  // 500ms off
 *     }
 * @endcode
 */
ITCM_FUNC uint32_t task_active_sleep(uint32_t ticks)
{
    task_list[current_task_index]->global_tick_paused = os_tick_count;
    task_list[current_task_index]->ticks_to_pause = ticks;
    task_list[current_task_index]->task_state = TASK_STATE_BLOCKED;
    os_yield();
    return os_tick_count - task_list[current_task_index]->global_tick_paused;
}

/**
 * @brief   Sleep for specified ticks with scheduler disabled
 *
 * @details Performs a blocking delay within a critical section, preventing
 *          any context switches during the sleep period. Uses busy-wait
 *          internally but protects against preemption.
 *
 *          Use this when you need guaranteed uninterrupted timing, such
 *          as hardware initialization sequences or protocol bit-banging.
 *
 * @param   ticks  Number of ticks to sleep (1 tick = 1ms at 1kHz)
 *
 * @return  uint32_t  Actual ticks elapsed
 *
 * @pre     Scheduler must be running
 *
 * @post    No context switches occur during sleep
 * @post    Critical section depth is preserved
 *
 * @warning Blocks ALL other tasks for the duration - use sparingly!
 * @warning Long sleeps will cause other tasks to miss deadlines
 *
 * @see     task_active_sleep() for cooperative sleep (preferred)
 * @see     task_busy_wait() for spin-wait without critical section
 *
 * @par Example:
 * @code
 *     // Hardware reset sequence requiring precise timing
 *     GPIO_WritePin(RESET_PIN, 0);
 *     task_blocking_sleep(10);  // Hold reset for exactly 10ms
 *     GPIO_WritePin(RESET_PIN, 1);
 * @endcode
 */
uint32_t task_blocking_sleep(uint32_t ticks)
{
    enter_critical();
    uint32_t delta = task_busy_wait(ticks);
    exit_critical();
    return delta;
}

/**
 * @brief   Spin-wait for specified number of ticks
 *
 * @details Performs a busy-wait loop, polling the tick counter until
 *          the specified number of ticks have elapsed. Does NOT yield
 *          to other tasks and does NOT disable the scheduler.
 *
 *          The task may still be preempted during this wait, so actual
 *          elapsed time may exceed the requested ticks.
 *
 * @param   ticks  Number of ticks to wait (1 tick = 1ms at 1kHz)
 *
 * @return  uint32_t  Actual ticks elapsed (>= requested)
 *
 * @note    Wastes CPU cycles - prefer task_active_sleep() when possible
 * @note    In HOST_TEST mode, auto-advances tick count to prevent hang
 *
 * @warning High CPU usage - only use for very short delays
 * @warning May be preempted, extending actual wait time
 *
 * @see     task_active_sleep() for cooperative sleep (preferred)
 * @see     task_blocking_sleep() for non-preemptible delay
 *
 * @par Example:
 * @code
 *     // Short delay for hardware settling
 *     task_busy_wait(2);  // Wait ~2ms for voltage to stabilize
 * @endcode
 */
uint32_t task_busy_wait(uint32_t ticks)
{
    uint32_t st = os_tick_count;
    uint32_t delta;

    while (1) {
#ifdef HOST_TEST
        /* In test mode, auto-advance ticks to prevent infinite loop */
        os_tick_count++;
#endif
        delta = os_tick_count - st;
        if (delta >= ticks) {
            break;
        }
    }
    return delta;
}

/* ============================================================================
 * TASK LIFECYCLE
 * ========================================================================= */

/**
 * @brief   Start the ICARUS scheduler and begin multitasking
 *
 * @details Begins execution of the first registered task (typically the
 *          idle task). This function does not return under normal operation
 *          as control is transferred to the task scheduler.
 *
 *          The scheduler uses round-robin preemptive scheduling with
 *          configurable time slices (ICARUS_TICKS_PER_TASK).
 *
 * @pre     os_init() must have been called
 * @pre     At least one task must be registered
 * @pre     SysTick must be configured for kernel tick rate
 *
 * @post    First task begins execution
 * @post    os_running flag is set by first task
 * @post    Function does not return
 *
 * @warning This function never returns!
 * @warning Must not be called from interrupt context
 *
 * @see     os_init()
 * @see     os_register_task()
 *
 * @par Example:
 * @code
 *     int main(void) {
 *         hal_init();
 *         os_init();
 *         os_register_task(app_task, "AppTask");
 *         os_start();  // Never returns
 *         // Unreachable code
 *     }
 * @endcode
 */
void os_start(void)
{
    if (num_created_tasks == 0 || num_created_tasks > ICARUS_MAX_TASKS) {
        return;
    }
    task_start(task_list[current_task_index]);
}

/**
 * @brief   Gracefully exit the current task
 *
 * @details Marks the current task as FINISHED and removes it from the
 *          active task pool. The task's resources are queued for cleanup
 *          and the scheduler immediately switches to the next ready task.
 *
 *          This function is automatically called if a task function returns
 *          (set as LR during task creation). Tasks can also call it
 *          explicitly to terminate early.
 *
 * @pre     Called from task context (not ISR)
 * @pre     Scheduler must be running
 *
 * @post    Task state set to TASK_STATE_FINISHED
 * @post    running_task_count decremented
 * @post    Task index added to cleanup queue
 * @post    Context switch to next task occurs
 *
 * @note    Task resources (stack, TCB) remain allocated until reused
 * @note    System tasks (idle, heartbeat) should never call this
 *
 * @warning This function does not return to the calling task!
 *
 * @see     os_kill_process() to terminate another task
 * @see     os_task_suicide() for explicit self-termination with logging
 *
 * @par Example:
 * @code
 *     void one_shot_task(void) {
 *         do_initialization();
 *         os_exit_task();  // Task complete, exit gracefully
 *         // Unreachable
 *     }
 * @endcode
 */
void os_exit_task(void)
{
    task_list[current_task_index]->task_state = TASK_STATE_FINISHED;

    if (running_task_count > 0) {
        running_task_count--;
    }

    if (current_cleanup_task_idx < (ICARUS_MAX_TASKS - 1)) {
        cleanup_task_idx[++current_cleanup_task_idx] = (int8_t)current_task_index;
    }

    os_yield();
}

/**
 * @brief   Terminate a task by index
 *
 * @details Forcibly terminates the specified task, marking it as KILLED
 *          and removing it from the scheduler's active pool. If the
 *          killed task is the current task, an immediate context switch
 *          occurs.
 *
 *          System tasks (index 0 = idle, index 1 = heartbeat) cannot be
 *          killed for system stability.
 *
 * @param   task_index  Index of task to kill (0 to num_created_tasks-1)
 *
 * @pre     task_index must be valid (< num_created_tasks)
 * @pre     Target task must be in active state (< TASK_STATE_KILLED)
 * @pre     Cannot kill system tasks (index 0 or 1)
 *
 * @post    Target task state set to TASK_STATE_KILLED
 * @post    running_task_count decremented
 * @post    Task index added to cleanup queue
 * @post    If killing self, context switch occurs
 *
 * @note    Logs kill action to console via printf
 * @note    Safe to call on already-killed tasks (no-op)
 *
 * @warning Killing a task holding resources may cause deadlocks
 * @warning No cleanup callbacks are invoked
 *
 * @see     os_task_suicide() for self-termination
 * @see     os_exit_task() for graceful exit
 *
 * @par Example:
 * @code
 *     // Watchdog task kills unresponsive tasks
 *     if (task_is_hung(task_idx)) {
 *         os_kill_process(task_idx);
 *     }
 * @endcode
 */
void os_kill_process(uint8_t task_index)
{
    if (task_index >= num_created_tasks) {
        return;
    }

    /* Cannot kill system tasks, and task must be in active state */
    if (task_index > 0 && task_index < num_created_tasks &&
        task_list[task_index]->task_state < TASK_STATE_KILLED) {

        printf("[INFO] %s killed %s\r\n",
               task_list[current_task_index]->name,
               task_list[task_index]->name);

        task_list[task_index]->task_state = TASK_STATE_KILLED;

        if (running_task_count > 0) {
            running_task_count--;
        }

        if (current_cleanup_task_idx < (ICARUS_MAX_TASKS - 1)) {
            cleanup_task_idx[++current_cleanup_task_idx] = (int8_t)task_index;
        }

        /* If killing self, yield immediately */
        if (task_index == current_task_index) {
            os_yield();
        }
    } else {
        printf("[ERROR] Cannot kill task %d (state=%d)\r\n",
               task_index, task_list[task_index]->task_state);
    }
}

/**
 * @brief   Terminate the current task (self-kill with logging)
 *
 * @details Convenience function for a task to terminate itself with
 *          diagnostic logging. Equivalent to os_kill_process(current_task_index)
 *          but includes a log message indicating voluntary termination.
 *
 * @pre     Called from task context (not ISR)
 * @pre     Scheduler must be running
 *
 * @post    Current task state set to TASK_STATE_KILLED
 * @post    Log message printed to console
 * @post    Context switch to next task occurs
 *
 * @note    Logs "committed suicide" message for debugging
 *
 * @warning This function does not return!
 * @warning System tasks should never call this
 *
 * @see     os_exit_task() for graceful exit without "suicide" log
 * @see     os_kill_process() to kill other tasks
 *
 * @par Example:
 * @code
 *     void self_destructing_task(void) {
 *         if (fatal_error_detected()) {
 *             os_task_suicide();  // Logs and terminates
 *         }
 *     }
 * @endcode
 */
void os_task_suicide(void)
{
    printf("[INFO] %s committed suicide\r\n",
           task_list[current_task_index]->name);
    os_kill_process(current_task_index);
}

/* ============================================================================
 * TASK CREATION
 * ========================================================================= */

/**
 * @brief   Create a new task with explicit stack allocation
 *
 * @details Initializes a task control block (TCB) with the provided
 *          parameters and prepares the stack for first execution. The
 *          task is created in COLD state and will begin execution when
 *          the scheduler runs.
 *
 *          The stack is initialized with an ARM Cortex-M exception frame
 *          so that the first context switch properly loads the task's
 *          entry point and initial register values.
 *
 * @par Stack Frame Layout (ARM Cortex-M):
 *      | Offset | Register | Value                    |
 *      |--------|----------|--------------------------|
 *      | SP+28  | xPSR     | 0x01000000 (Thumb bit)   |
 *      | SP+24  | PC       | function entry point     |
 *      | SP+20  | LR       | os_exit_task (return)    |
 *      | SP+16  | R12      | 0                        |
 *      | SP+12  | R3       | 0                        |
 *      | SP+8   | R2       | 0                        |
 *      | SP+4   | R1       | 0                        |
 *      | SP+0   | R0       | 0 <- Initial SP          |
 *
 * @param   task        Pointer to TCB to initialize
 * @param   function    Task entry point function
 * @param   stack       Pointer to stack memory (must be word-aligned)
 * @param   stack_size  Stack size in 32-bit words
 * @param   name        Task name (max ICARUS_MAX_TASK_NAME_LEN chars)
 *
 * @pre     task pointer must be valid
 * @pre     function must be a valid function pointer
 * @pre     stack must be word-aligned and have stack_size words
 * @pre     running_task_count < ICARUS_MAX_TASKS
 *
 * @post    TCB is fully initialized
 * @post    Stack frame is prepared for first context switch
 * @post    num_created_tasks is incremented
 * @post    Task is in TASK_STATE_COLD state
 *
 * @note    Use os_register_task() for simpler task creation with auto stack
 * @note    Task name is truncated if longer than ICARUS_MAX_TASK_NAME_LEN
 *
 * @see     os_register_task() for simplified task creation
 * @see     ICARUS_STACK_WORDS for default stack size
 *
 * @par Example:
 * @code
 *     static uint32_t my_stack[256];
 *     icarus_task_t my_tcb;
 *
 *     os_create_task(&my_tcb, my_task_func, my_stack, 256, "MyTask");
 * @endcode
 */
void os_create_task(icarus_task_t *task, void (*function)(void),
                    uint32_t *stack, uint32_t stack_size, const char *name)
{
    if (running_task_count >= ICARUS_MAX_TASKS) {
        return;
    }

    /* Initialize TCB */
    task->function = function;
    task->stack_base = stack;
    task->stack_size = stack_size;
    task->task_state = TASK_STATE_COLD;
    task->task_priority = TASK_PRIORITY_LOW;
    task->global_tick_paused = 0;
    task->ticks_to_pause = 0;

    /* Copy task name */
    strncpy(task->name, name, ICARUS_MAX_TASK_NAME_LEN);
    task->name[ICARUS_MAX_TASK_NAME_LEN - 1] = '\0';

    /*
     * Initialize stack frame for first context switch
     * ARM Cortex-M exception frame (pushed automatically on exception):
     *   [high addr] xPSR, PC, LR, R12, R3, R2, R1, R0 [low addr]
     *
     * We also reserve space for R4-R11 (saved by software in PendSV)
     */
    uint32_t *stack_top = stack + stack_size - 1;

    /* Exception frame (hardware saved) */
    *(stack_top--) = 0x01000000;                      /* xPSR (Thumb bit set) */
    *(stack_top--) = (uint32_t)(uintptr_t)function;   /* PC (entry point) */
    *(stack_top--) = (uint32_t)(uintptr_t)os_exit_task; /* LR (return addr) */
    *(stack_top--) = 0;                               /* R12 */
    *(stack_top--) = 0;                               /* R3 */
    *(stack_top--) = 0;                               /* R2 */
    *(stack_top--) = 0;                               /* R1 */
    *(stack_top)   = 0;                               /* R0 <- SP points here */

    task->stack_pointer = stack_top;
    num_created_tasks++;
}

/**
 * @brief   Register a task using kernel-managed stack pool
 *
 * @details Simplified task creation that allocates a TCB and stack from
 *          the kernel's static pools. This is the preferred method for
 *          creating application tasks.
 *
 *          The task is assigned:
 *          - TCB from task_pool[num_created_tasks]
 *          - Stack from stack_pool[num_created_tasks]
 *          - Stack size of ICARUS_STACK_WORDS (default 512 words = 2KB)
 *
 * @param   function  Task entry point function (void (*)(void))
 * @param   name      Task name string (max ICARUS_MAX_TASK_NAME_LEN chars)
 *
 * @pre     os_init() must have been called
 * @pre     num_created_tasks < ICARUS_MAX_TASKS
 *
 * @post    Task is registered and ready to run
 * @post    num_created_tasks is incremented
 *
 * @note    Task begins execution when os_start() is called
 * @note    Tasks are scheduled in registration order initially
 *
 * @warning Maximum ICARUS_MAX_TASKS tasks can be registered
 *
 * @see     os_create_task() for custom stack allocation
 * @see     ICARUS_MAX_TASKS
 * @see     ICARUS_STACK_WORDS
 *
 * @par Example:
 * @code
 *     void sensor_task(void) {
 *         while (1) {
 *             read_sensors();
 *             task_active_sleep(100);
 *         }
 *     }
 *
 *     int main(void) {
 *         hal_init();
 *         os_init();
 *         os_register_task(sensor_task, "SensorTask");
 *         os_start();
 *     }
 * @endcode
 */
void os_register_task(void (*function)(void), const char *name)
{
    os_create_task(task_list[num_created_tasks], function,
                   stack_pool[num_created_tasks], ICARUS_STACK_WORDS, name);
}

/* ============================================================================
 * SEMAPHORE IMPLEMENTATION
 * ========================================================================= */

/**
 * @brief   Initialize a counting semaphore
 *
 * @details Configures a semaphore from the kernel's static pool with the
 *          specified initial and maximum count. The semaphore is marked
 *          as engaged (active) upon successful initialization.
 *
 *          Counting semaphores are used for:
 *          - Resource counting (e.g., buffer slots available)
 *          - Producer-consumer synchronization
 *          - Rate limiting
 *
 * @param   semaphore_idx    Index in semaphore pool (0 to ICARUS_MAX_SEMAPHORES-1)
 * @param   semaphore_count  Initial and maximum count value
 *
 * @return  bool  true if initialization succeeded, false otherwise
 *
 * @retval  true   Semaphore initialized successfully
 * @retval  false  Invalid index, zero count, or already engaged
 *
 * @pre     os_init() must have been called
 * @pre     semaphore_idx < ICARUS_MAX_SEMAPHORES
 * @pre     semaphore_count > 0
 * @pre     Semaphore must not already be engaged
 *
 * @post    Semaphore count set to semaphore_count
 * @post    Semaphore max_count set to semaphore_count
 * @post    Semaphore marked as engaged
 *
 * @note    Thread-safe: uses critical section internally
 * @note    Cannot re-initialize an engaged semaphore
 *
 * @see     semaphore_feed()
 * @see     semaphore_consume()
 * @see     ICARUS_MAX_SEMAPHORES
 *
 * @par Example:
 * @code
 *     #define SEM_BUFFER 0
 *     #define BUFFER_SLOTS 10
 *
 *     if (!semaphore_init(SEM_BUFFER, BUFFER_SLOTS)) {
 *         Error_Handler();
 *     }
 * @endcode
 */
bool semaphore_init(uint8_t semaphore_idx, uint32_t semaphore_count)
{
    enter_critical();

    if (semaphore_idx < ICARUS_MAX_SEMAPHORES && semaphore_count > 0) {
        if (!semaphore_list[semaphore_idx]->engaged) {
            __init_sem(semaphore_idx, semaphore_count, true);
            exit_critical();
            return true;
        }
    }

    exit_critical();
    return false;
}

/**
 * @brief   Increment semaphore count (V operation / signal)
 *
 * @details Increments the semaphore count by one, signaling that a resource
 *          is available. If the semaphore is at maximum count, the calling
 *          task blocks until space is available.
 *
 *          This is the "producer" side of producer-consumer synchronization.
 *          Placed in ITCM for zero wait-state execution.
 *
 * @param   semaphore_idx  Index of semaphore to signal
 *
 * @return  bool  true if operation succeeded, false if invalid semaphore
 *
 * @retval  true   Count incremented successfully
 * @retval  false  Invalid index or semaphore not engaged
 *
 * @pre     Semaphore must be initialized via semaphore_init()
 * @pre     semaphore_idx < ICARUS_MAX_SEMAPHORES
 *
 * @post    Semaphore count incremented by 1
 * @post    tick_updated_at set to current tick
 *
 * @note    Blocks if count == max_count (bounded semaphore)
 * @note    Thread-safe: uses critical section for increment
 *
 * @see     semaphore_consume() for decrement operation
 * @see     semaphore_init()
 *
 * @par Example:
 * @code
 *     // Producer task
 *     void producer_task(void) {
 *         while (1) {
 *             data = produce_data();
 *             write_to_buffer(data);
 *             semaphore_feed(SEM_DATA_READY);  // Signal consumer
 *         }
 *     }
 * @endcode
 */
ITCM_FUNC bool semaphore_feed(uint8_t semaphore_idx)
{
    if (semaphore_idx >= ICARUS_MAX_SEMAPHORES ||
        !semaphore_list[semaphore_idx]->engaged) {
        return false;
    }

    /* Block while at max count */
    while (semaphore_list[semaphore_idx]->count >=
           semaphore_list[semaphore_idx]->max_count) {
        task_active_sleep(1);
    }

    enter_critical();
    ++semaphore_list[semaphore_idx]->count;
    semaphore_list[semaphore_idx]->tick_updated_at = os_tick_count;
    exit_critical();

    return true;
}

/**
 * @brief   Decrement semaphore count (P operation / wait)
 *
 * @details Decrements the semaphore count by one, consuming a resource.
 *          If the semaphore count is zero, the calling task blocks until
 *          another task signals the semaphore via semaphore_feed().
 *
 *          This is the "consumer" side of producer-consumer synchronization.
 *          Placed in ITCM for zero wait-state execution.
 *
 * @param   semaphore_idx  Index of semaphore to wait on
 *
 * @return  bool  true if operation succeeded, false if invalid semaphore
 *
 * @retval  true   Count decremented successfully
 * @retval  false  Invalid index or semaphore not engaged
 *
 * @pre     Semaphore must be initialized via semaphore_init()
 * @pre     semaphore_idx < ICARUS_MAX_SEMAPHORES
 *
 * @post    Semaphore count decremented by 1
 * @post    tick_updated_at set to current tick
 *
 * @note    Blocks if count == 0 until semaphore_feed() is called
 * @note    Thread-safe: uses critical section for decrement
 *
 * @see     semaphore_feed() for increment operation
 * @see     semaphore_init()
 *
 * @par Example:
 * @code
 *     // Consumer task
 *     void consumer_task(void) {
 *         while (1) {
 *             semaphore_consume(SEM_DATA_READY);  // Wait for data
 *             data = read_from_buffer();
 *             process_data(data);
 *         }
 *     }
 * @endcode
 */
ITCM_FUNC bool semaphore_consume(uint8_t semaphore_idx)
{
    if (semaphore_idx >= ICARUS_MAX_SEMAPHORES ||
        !semaphore_list[semaphore_idx]->engaged) {
        return false;
    }

    /* Block while count is zero */
    while (semaphore_list[semaphore_idx]->count == 0) {
        task_active_sleep(1);
    }

    enter_critical();
    --semaphore_list[semaphore_idx]->count;
    semaphore_list[semaphore_idx]->tick_updated_at = os_tick_count;
    exit_critical();

    return true;
}

/**
 * @brief   Get current semaphore count
 *
 * @details Returns the current count value of the specified semaphore.
 *          Useful for monitoring resource availability without blocking.
 *
 * @param   semaphore_idx  Index of semaphore to query
 *
 * @return  uint32_t  Current count, or 0 if invalid/not engaged
 *
 * @pre     Semaphore should be initialized for meaningful result
 *
 * @note    Non-blocking read operation
 * @note    Value may change immediately after return
 *
 * @see     semaphore_get_max_count()
 * @see     semaphore_init()
 *
 * @par Example:
 * @code
 *     uint32_t available = semaphore_get_count(SEM_BUFFER);
 *     printf("Buffer slots available: %lu\n", available);
 * @endcode
 */
uint32_t semaphore_get_count(uint8_t semaphore_idx)
{
    if (semaphore_idx >= ICARUS_MAX_SEMAPHORES ||
        !semaphore_list[semaphore_idx]->engaged) {
        return 0;
    }
    return semaphore_list[semaphore_idx]->count;
}

/**
 * @brief   Get maximum semaphore count
 *
 * @details Returns the maximum count value (capacity) of the specified
 *          semaphore. This is the value set during semaphore_init().
 *
 * @param   semaphore_idx  Index of semaphore to query
 *
 * @return  uint32_t  Maximum count, or 0 if invalid/not engaged
 *
 * @pre     Semaphore should be initialized for meaningful result
 *
 * @note    Non-blocking read operation
 * @note    Max count is immutable after initialization
 *
 * @see     semaphore_get_count()
 * @see     semaphore_init()
 *
 * @par Example:
 * @code
 *     uint32_t capacity = semaphore_get_max_count(SEM_BUFFER);
 *     uint32_t used = capacity - semaphore_get_count(SEM_BUFFER);
 *     printf("Buffer %lu%% full\n", (used * 100) / capacity);
 * @endcode
 */
uint32_t semaphore_get_max_count(uint8_t semaphore_idx)
{
    if (semaphore_idx >= ICARUS_MAX_SEMAPHORES ||
        !semaphore_list[semaphore_idx]->engaged) {
        return 0;
    }
    return semaphore_list[semaphore_idx]->max_count;
}

/* ============================================================================
 * MESSAGE PIPE IMPLEMENTATION
 * ========================================================================= */

/**
 * @brief   Initialize a message pipe (circular buffer queue)
 *
 * @details Configures a message pipe from the kernel's static pool with
 *          the specified byte capacity. Message pipes provide byte-stream
 *          inter-task communication using a circular buffer.
 *
 *          Message pipes are used for:
 *          - Variable-length message passing between tasks
 *          - Streaming data (sensor readings, log messages)
 *          - Command/response protocols
 *
 * @param   pipe_idx              Index in pipe pool (0 to ICARUS_MAX_MESSAGE_QUEUES-1)
 * @param   pipe_capacity_bytes   Maximum buffer size in bytes
 *
 * @return  bool  true if initialization succeeded, false otherwise
 *
 * @retval  true   Pipe initialized successfully
 * @retval  false  Invalid index, zero/excessive capacity, or already engaged
 *
 * @pre     os_init() must have been called
 * @pre     pipe_idx < ICARUS_MAX_MESSAGE_QUEUES
 * @pre     0 < pipe_capacity_bytes <= ICARUS_MAX_MESSAGE_BYTES
 * @pre     Pipe must not already be engaged
 *
 * @post    Pipe buffer cleared to zeros
 * @post    Pipe marked as engaged
 * @post    Read/write indices reset to 0
 *
 * @note    Thread-safe: uses critical section internally
 * @note    Cannot re-initialize an engaged pipe
 *
 * @see     pipe_enqueue()
 * @see     pipe_dequeue()
 * @see     ICARUS_MAX_MESSAGE_QUEUES
 * @see     ICARUS_MAX_MESSAGE_BYTES
 *
 * @par Example:
 * @code
 *     #define PIPE_SENSOR 0
 *     #define SENSOR_BUFFER_SIZE 64
 *
 *     if (!pipe_init(PIPE_SENSOR, SENSOR_BUFFER_SIZE)) {
 *         Error_Handler();
 *     }
 * @endcode
 */
bool pipe_init(uint8_t pipe_idx, uint8_t pipe_capacity_bytes)
{
    enter_critical();

    if (pipe_idx < ICARUS_MAX_MESSAGE_QUEUES &&
        pipe_capacity_bytes > 0 &&
        pipe_capacity_bytes <= ICARUS_MAX_MESSAGE_BYTES) {

        if (!message_pipe_list[pipe_idx]->engaged) {
            __init_pipe(pipe_idx, pipe_capacity_bytes, true);
            exit_critical();
            return true;
        }
    }

    exit_critical();
    return false;
}

/**
 * @brief   Write bytes to a message pipe
 *
 * @details Copies the specified bytes into the pipe's circular buffer.
 *          If insufficient space is available, the calling task blocks
 *          until space becomes available (via pipe_dequeue()).
 *
 *          Bytes are written atomically - either all bytes are written
 *          or the operation blocks. Partial writes do not occur.
 *
 *          Placed in ITCM for zero wait-state execution.
 *
 * @param   pipe_idx       Index of pipe to write to
 * @param   message        Pointer to bytes to write
 * @param   message_bytes  Number of bytes to write
 *
 * @return  bool  true if write succeeded, false if invalid parameters
 *
 * @retval  true   All bytes written successfully
 * @retval  false  Invalid pipe, not engaged, or message too large
 *
 * @pre     Pipe must be initialized via pipe_init()
 * @pre     pipe_idx < ICARUS_MAX_MESSAGE_QUEUES
 * @pre     message_bytes <= pipe capacity
 *
 * @post    Bytes copied to circular buffer
 * @post    Pipe count increased by message_bytes
 * @post    tick_updated_at set to current tick
 *
 * @note    Blocks if insufficient space available
 * @note    Thread-safe: uses critical section for buffer update
 *
 * @see     pipe_dequeue() for reading bytes
 * @see     pipe_init()
 *
 * @par Example:
 * @code
 *     // Producer task
 *     void sensor_task(void) {
 *         uint8_t reading[4];
 *         while (1) {
 *             read_sensor(reading, sizeof(reading));
 *             pipe_enqueue(PIPE_SENSOR, reading, sizeof(reading));
 *             task_active_sleep(100);
 *         }
 *     }
 * @endcode
 */
ITCM_FUNC bool pipe_enqueue(uint8_t pipe_idx, uint8_t* message,
                            uint8_t message_bytes)
{
    if (pipe_idx >= ICARUS_MAX_MESSAGE_QUEUES ||
        !message_pipe_list[pipe_idx]->engaged ||
        message_bytes > message_pipe_list[pipe_idx]->max_count) {
        return false;
    }

    /* Block while insufficient space */
    while ((message_pipe_list[pipe_idx]->max_count -
            message_pipe_list[pipe_idx]->count) < message_bytes) {
        task_active_sleep(1);
        if ((message_pipe_list[pipe_idx]->max_count -
             message_pipe_list[pipe_idx]->count) >= message_bytes) {
            scheduler_enabled = false;
        }
    }

    enter_critical();

    /* Copy message bytes to circular buffer */
    for (uint8_t i = 0; i < message_bytes; i++) {
        message_pipe_list[pipe_idx]->buffer[
            message_pipe_list[pipe_idx]->enqueue_idx] = message[i];
        message_pipe_list[pipe_idx]->enqueue_idx =
            (uint8_t)(message_pipe_list[pipe_idx]->enqueue_idx + 1) %
            message_pipe_list[pipe_idx]->max_count;
        message_pipe_list[pipe_idx]->count++;
    }

    message_pipe_list[pipe_idx]->tick_updated_at = os_tick_count;
    exit_critical();

    return true;
}

/**
 * @brief   Read bytes from a message pipe
 *
 * @details Copies the specified number of bytes from the pipe's circular
 *          buffer into the provided buffer. If insufficient data is
 *          available, the calling task blocks until data arrives.
 *
 *          Bytes are read atomically - either all requested bytes are
 *          read or the operation blocks. Partial reads do not occur.
 *
 *          Placed in ITCM for zero wait-state execution.
 *
 * @param   pipe_idx       Index of pipe to read from
 * @param   message        Buffer to receive bytes (must be >= message_bytes)
 * @param   message_bytes  Number of bytes to read
 *
 * @return  bool  true if read succeeded, false if invalid parameters
 *
 * @retval  true   All bytes read successfully
 * @retval  false  Invalid pipe, not engaged, or request too large
 *
 * @pre     Pipe must be initialized via pipe_init()
 * @pre     pipe_idx < ICARUS_MAX_MESSAGE_QUEUES
 * @pre     message_bytes <= pipe capacity
 * @pre     message buffer must have space for message_bytes
 *
 * @post    Bytes copied from circular buffer to message
 * @post    Pipe count decreased by message_bytes
 * @post    tick_updated_at set to current tick
 *
 * @note    Blocks if insufficient data available
 * @note    Thread-safe: uses critical section for buffer update
 *
 * @see     pipe_enqueue() for writing bytes
 * @see     pipe_init()
 *
 * @par Example:
 * @code
 *     // Consumer task
 *     void display_task(void) {
 *         uint8_t reading[4];
 *         while (1) {
 *             pipe_dequeue(PIPE_SENSOR, reading, sizeof(reading));
 *             display_value(reading);
 *         }
 *     }
 * @endcode
 */
ITCM_FUNC bool pipe_dequeue(uint8_t pipe_idx, uint8_t* message,
                            uint8_t message_bytes)
{
    if (pipe_idx >= ICARUS_MAX_MESSAGE_QUEUES ||
        !message_pipe_list[pipe_idx]->engaged ||
        message_bytes > message_pipe_list[pipe_idx]->max_count) {
        return false;
    }

    /* Block while insufficient data */
    while (message_pipe_list[pipe_idx]->count < message_bytes) {
        task_active_sleep(1);
        if (message_pipe_list[pipe_idx]->count >= message_bytes) {
            scheduler_enabled = false;
        }
    }

    enter_critical();

    /* Copy message bytes from circular buffer */
    for (uint8_t i = 0; i < message_bytes; i++) {
        message[i] = message_pipe_list[pipe_idx]->buffer[
            message_pipe_list[pipe_idx]->dequeue_idx];
        message_pipe_list[pipe_idx]->dequeue_idx =
            (uint8_t)(message_pipe_list[pipe_idx]->dequeue_idx + 1) %
            message_pipe_list[pipe_idx]->max_count;
        message_pipe_list[pipe_idx]->count--;
    }

    message_pipe_list[pipe_idx]->tick_updated_at = os_tick_count;
    exit_critical();

    return true;
}

/**
 * @brief   Get current byte count in pipe
 *
 * @details Returns the number of bytes currently stored in the pipe's
 *          circular buffer. Useful for checking data availability
 *          without blocking.
 *
 * @param   pipe_idx  Index of pipe to query
 *
 * @return  uint8_t  Current byte count, or 0 if invalid/not engaged
 *
 * @pre     Pipe should be initialized for meaningful result
 *
 * @note    Non-blocking read operation
 * @note    Value may change immediately after return
 *
 * @see     pipe_get_max_count()
 * @see     pipe_init()
 *
 * @par Example:
 * @code
 *     if (pipe_get_count(PIPE_SENSOR) >= sizeof(reading)) {
 *         pipe_dequeue(PIPE_SENSOR, reading, sizeof(reading));
 *     }
 * @endcode
 */
uint8_t pipe_get_count(uint8_t pipe_idx)
{
    if (pipe_idx >= ICARUS_MAX_MESSAGE_QUEUES ||
        !message_pipe_list[pipe_idx]->engaged) {
        return 0;
    }
    return message_pipe_list[pipe_idx]->count;
}

/**
 * @brief   Get maximum byte capacity of pipe
 *
 * @details Returns the maximum byte capacity of the specified pipe.
 *          This is the value set during pipe_init().
 *
 * @param   pipe_idx  Index of pipe to query
 *
 * @return  uint8_t  Maximum capacity in bytes, or 0 if invalid/not engaged
 *
 * @pre     Pipe should be initialized for meaningful result
 *
 * @note    Non-blocking read operation
 * @note    Capacity is immutable after initialization
 *
 * @see     pipe_get_count()
 * @see     pipe_init()
 *
 * @par Example:
 * @code
 *     uint8_t capacity = pipe_get_max_count(PIPE_SENSOR);
 *     uint8_t used = pipe_get_count(PIPE_SENSOR);
 *     printf("Pipe %d%% full\n", (used * 100) / capacity);
 * @endcode
 */
uint8_t pipe_get_max_count(uint8_t pipe_idx)
{
    if (pipe_idx >= ICARUS_MAX_MESSAGE_QUEUES ||
        !message_pipe_list[pipe_idx]->engaged) {
        return 0;
    }
    return message_pipe_list[pipe_idx]->max_count;
}
