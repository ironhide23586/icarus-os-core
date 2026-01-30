/**
 * @file    icarus_task.c
 * @brief   ICARUS OS Task Management Implementation
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
 * @date    2026
 *
 * @copyright Copyright (c) 2026 ICARUS Project
 *            https://github.com/ironhide23586/icarus-os-core
 *            Licensed under MIT License
 */

#include "icarus/icarus_task.h"
#if ICARUS_ENABLE_HEARTBEAT_VIS
#include "bsp/bsp_display.h"
#endif
#include "bsp/bsp_led.h"
#include "main.h"

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
 * @brief Enter critical section (disable scheduler)
 * @note  Supports nesting - must pair with exit_critical()
 */
static inline void enter_critical(void)
{
    scheduler_enabled = false;
    critical_stack_depth++;
}

/**
 * @brief Exit critical section (re-enable scheduler if outermost)
 * @note  Supports nesting - must pair with enter_critical()
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
 * @brief Initialize a semaphore structure
 * @param semaphore_idx   Index in semaphore pool
 * @param semaphore_count Initial/max count
 * @param should_engage   Whether to mark as active
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
 * @brief Initialize a message pipe structure
 * @param message_pipe_idx  Index in pipe pool
 * @param max_messages      Maximum buffer capacity
 * @param should_engage     Whether to mark as active
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
 * @brief Idle task - runs when no other tasks are ready
 * @note  Initializes display on first run, then yields continuously
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
 * @brief Heartbeat task - blinks LED to indicate OS is alive
 * @note  Visual confirmation that scheduler is running
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

ITCM_FUNC uint32_t os_get_tick_count(void)
{
    return os_tick_count;
}

uint8_t os_get_running_task_count(void)
{
    return running_task_count;
}

const char* os_get_current_task_name(void)
{
    if (current_task_index < num_created_tasks &&
        task_list[current_task_index] != NULL) {
        return task_list[current_task_index]->name;
    }
    return "unknown";
}

uint32_t os_get_task_ticks_remaining(void)
{
    return current_task_ticks_remaining;
}

/* ============================================================================
 * TASK SCHEDULING (ITCM - Hot Path)
 * ========================================================================= */

/**
 * @brief Start a cold task (internal)
 * @param task  Task to start
 */
static void task_start(icarus_task_t *task)
{
    start_cold_task(task);
}

ITCM_FUNC void os_yield(void)
{
    current_task_ticks_remaining = ticks_per_task;
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

ITCM_FUNC uint32_t task_active_sleep(uint32_t ticks)
{
    task_list[current_task_index]->global_tick_paused = os_tick_count;
    task_list[current_task_index]->ticks_to_pause = ticks;
    task_list[current_task_index]->task_state = TASK_STATE_BLOCKED;
    os_yield();
    return os_tick_count - task_list[current_task_index]->global_tick_paused;
}

uint32_t task_blocking_sleep(uint32_t ticks)
{
    enter_critical();
    uint32_t delta = task_busy_wait(ticks);
    exit_critical();
    return delta;
}

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

void os_start(void)
{
    if (num_created_tasks == 0 || num_created_tasks > ICARUS_MAX_TASKS) {
        return;
    }
    task_start(task_list[current_task_index]);
}

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

void os_task_suicide(void)
{
    printf("[INFO] %s committed suicide\r\n",
           task_list[current_task_index]->name);
    os_kill_process(current_task_index);
}

/* ============================================================================
 * TASK CREATION
 * ========================================================================= */

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

void os_register_task(void (*function)(void), const char *name)
{
    os_create_task(task_list[num_created_tasks], function,
                   stack_pool[num_created_tasks], ICARUS_STACK_WORDS, name);
}

/* ============================================================================
 * SEMAPHORE IMPLEMENTATION
 * ========================================================================= */

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

uint32_t semaphore_get_count(uint8_t semaphore_idx)
{
    if (semaphore_idx >= ICARUS_MAX_SEMAPHORES ||
        !semaphore_list[semaphore_idx]->engaged) {
        return 0;
    }
    return semaphore_list[semaphore_idx]->count;
}

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

uint8_t pipe_get_count(uint8_t pipe_idx)
{
    if (pipe_idx >= ICARUS_MAX_MESSAGE_QUEUES ||
        !message_pipe_list[pipe_idx]->engaged) {
        return 0;
    }
    return message_pipe_list[pipe_idx]->count;
}

uint8_t pipe_get_max_count(uint8_t pipe_idx)
{
    if (pipe_idx >= ICARUS_MAX_MESSAGE_QUEUES ||
        !message_pipe_list[pipe_idx]->engaged) {
        return 0;
    }
    return message_pipe_list[pipe_idx]->max_count;
}
