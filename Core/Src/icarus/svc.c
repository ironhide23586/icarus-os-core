/**
 * @file    svc.c
 * @brief   ICARUS Supervisor Call (SVC) Implementation
 * @version 0.1.0
 *
 * @details SVC handler and public wrapper functions for MPU-protected kernel
 *          calls. Non-spinning wrappers issue SVC instructions to execute in
 *          privileged handler mode. Spinning wrappers (semaphore_feed/consume,
 *          pipe_enqueue/dequeue, task_blocking_sleep/busy_wait) call __func
 *          directly in thread mode — they cannot run inside the SVC handler
 *          because PendSV is lower priority and cannot preempt it.
 *
 * @author  Souham Biswas
 * @date    2025
 *
 * @copyright Copyright 2025-2026 Souham Biswas
 *            https://github.com/ironhide23586/icarus-os-core
 *            Licensed under the Apache License, Version 2.0
 */

#include "icarus/svc.h"
#include "icarus/kernel.h"
#include "icarus/task.h"
#include "icarus/scheduler.h"
#include "icarus/pipe.h"
#include "icarus/semaphore.h"

/* ============================================================================
 * SVC HANDLER (target only)
 * ========================================================================= */

#ifndef HOST_TEST

/**
 * @brief C-level SVC dispatcher called from SVC_Handler assembly trampoline
 * @param stack_frame Pointer to exception stack frame (R0-R3, R12, LR, PC, xPSR)
 *
 * @note SVC number is extracted from the SVC instruction encoding:
 *       The SVC instruction is at [PC - 2], and the immediate is the low byte.
 */
void SVC_Handler_C(uint32_t *stack_frame) {
    uint8_t svc_number = ((uint8_t *)(uintptr_t)stack_frame[6])[-2];
    uint32_t arg0 = stack_frame[0];
    uint32_t arg1 = stack_frame[1];

    switch (svc_number) {

        /* Critical section */
        case SVC_ENTER_CRITICAL:
            __enter_critical();
            break;
        case SVC_EXIT_CRITICAL:
            __exit_critical();
            break;

        /* Scheduler */
        case SVC_OS_YIELD:
            __os_yield();
            break;
        case SVC_TASK_ACTIVE_SLEEP:
            stack_frame[0] = __task_active_sleep(arg0);
            break;
        case SVC_OS_GET_TICK_COUNT:
            stack_frame[0] = __os_get_tick_count();
            break;
        case SVC_OS_GET_CURRENT_TASK_NAME: {
            const char *name = __os_get_current_task_name();
            stack_frame[0] = (uint32_t)(uintptr_t)name;
            break;
        }
        case SVC_OS_GET_RUNNING_TASK_COUNT:
            stack_frame[0] = __os_get_running_task_count();
            break;
        case SVC_OS_GET_TASK_TICKS_REMAINING:
            stack_frame[0] = __os_get_task_ticks_remaining();
            break;

        /* Task lifecycle */
        case SVC_OS_REGISTER_TASK:
            __os_register_task((void (*)(void))(uintptr_t)arg0,
                               (const char *)(uintptr_t)arg1);
            break;
        case SVC_OS_EXIT_TASK:
            __os_exit_task();
            break;
        case SVC_OS_KILL_PROCESS:
            __os_kill_process((uint8_t)arg0);
            break;
        case SVC_OS_TASK_SUICIDE:
            __os_task_suicide();
            break;

        /* Kernel data */
        case SVC_KERNEL_GET_STACK: {
            uint32_t *ptr = __kernel_get_stack((uint8_t)arg0);
            stack_frame[0] = (uint32_t)(uintptr_t)ptr;
            break;
        }
        case SVC_KERNEL_GET_DATA: {
            uint32_t *ptr = __kernel_get_data((uint8_t)arg0);
            stack_frame[0] = (uint32_t)(uintptr_t)ptr;
            break;
        }
        case SVC_KERNEL_PROTECTED_DATA: {
            void *ptr = __kernel_protected_data((uint16_t)arg0);
            stack_frame[0] = (uint32_t)(uintptr_t)ptr;
            break;
        }

        /* Semaphore (non-spinning) */
        case SVC_SEMAPHORE_INIT: {
            bool ret = __semaphore_init((uint8_t)arg0, arg1);
            stack_frame[0] = (uint32_t)ret;
            break;
        }
        case SVC_SEMAPHORE_GET_COUNT:
            stack_frame[0] = __semaphore_get_count((uint8_t)arg0);
            break;
        case SVC_SEMAPHORE_GET_MAX_COUNT:
            stack_frame[0] = __semaphore_get_max_count((uint8_t)arg0);
            break;

        /* Pipe (non-spinning) */
        case SVC_PIPE_INIT: {
            bool ret = __pipe_init((uint8_t)arg0, (uint8_t)arg1);
            stack_frame[0] = (uint32_t)ret;
            break;
        }
        case SVC_PIPE_GET_COUNT:
            stack_frame[0] = (uint32_t)__pipe_get_count((uint8_t)arg0);
            break;
        case SVC_PIPE_GET_MAX_COUNT:
            stack_frame[0] = (uint32_t)__pipe_get_max_count((uint8_t)arg0);
            break;

        /* Spinning functions — not dispatched via SVC, run in thread mode */
        case SVC_SEMAPHORE_FEED:
        case SVC_SEMAPHORE_CONSUME:
        case SVC_PIPE_ENQUEUE:
        case SVC_PIPE_DEQUEUE:
        case SVC_TASK_BUSY_WAIT:
        case SVC_TASK_BLOCKING_SLEEP:
            break;

        /* Call gates for spinning functions — read kernel state atomically */
        case SVC_SEM_CAN_FEED: {
            bool r = __sem_can_feed((uint8_t)arg0);
            stack_frame[0] = r ? 1u : 0u;
            break;
        }
        case SVC_SEM_CAN_CONSUME: {
            bool r = __sem_can_consume((uint8_t)arg0);
            stack_frame[0] = r ? 1u : 0u;
            break;
        }
        case SVC_PIPE_CAN_ENQUEUE: {
            bool r = __pipe_can_enqueue((uint8_t)arg0, (uint8_t)arg1);
            stack_frame[0] = r ? 1u : 0u;
            break;
        }
        case SVC_PIPE_CAN_DEQUEUE: {
            bool r = __pipe_can_dequeue((uint8_t)arg0, (uint8_t)arg1);
            stack_frame[0] = r ? 1u : 0u;
            break;
        }

        /* Write gates for spinning functions — modify kernel state atomically */
        case SVC_SEM_INCREMENT:
            __sem_increment((uint8_t)arg0);
            break;
        case SVC_SEM_DECREMENT:
            __sem_decrement((uint8_t)arg0);
            break;
        case SVC_PIPE_WRITE_BYTES:
            __pipe_write_bytes((uint8_t)arg0, (uint8_t *)(uintptr_t)arg1,
                               (uint8_t)stack_frame[2]);
            break;
        case SVC_PIPE_READ_BYTES:
            __pipe_read_bytes((uint8_t)arg0, (uint8_t *)(uintptr_t)arg1,
                              (uint8_t)stack_frame[2]);
            break;

        /* Task metadata read gates — for display/diagnostics from unprivileged mode */
        case SVC_GET_TASK_NAME: {
            const char *name = __os_get_task_name((uint8_t)arg0);
            stack_frame[0] = (uint32_t)(uintptr_t)name;
            break;
        }
        case SVC_GET_NUM_TASKS:
            stack_frame[0] = (uint32_t)__os_get_num_created_tasks();
            break;
        case SVC_OS_IS_RUNNING:
            stack_frame[0] = (uint32_t)__os_is_running();
            break;

        default:
            break;
    }
}

#endif /* HOST_TEST */

/* ============================================================================
 * CRITICAL SECTION WRAPPERS
 * ========================================================================= */

/**
 * @brief Enter critical section (disable scheduler)
 */
void enter_critical(void) {
#ifndef HOST_TEST
    __asm__ volatile ("svc %0\n" : : "I" (SVC_ENTER_CRITICAL));
#else
    __enter_critical();
#endif
}

/**
 * @brief Exit critical section (re-enable scheduler if outermost)
 */
void exit_critical(void) {
#ifndef HOST_TEST
    __asm__ volatile ("svc %0\n" : : "I" (SVC_EXIT_CRITICAL));
#else
    __exit_critical();
#endif
}

/* ============================================================================
 * KERNEL INIT / START WRAPPERS
 * ========================================================================= */

void os_init(void)  { __os_init(); }
void os_start(void) { __os_start(); }

/* ============================================================================
 * SCHEDULER WRAPPERS
 * ========================================================================= */

/**
 * @brief Voluntarily yield CPU to scheduler
 */
void os_yield(void) {
#ifndef HOST_TEST
    __asm__ volatile ("svc %0\n" : : "I" (SVC_OS_YIELD));
#else
    __os_yield();
#endif
}

/**
 * @brief Sleep for specified ticks (cooperative, via SVC)
 */
uint32_t task_active_sleep(uint32_t ticks) {
#ifndef HOST_TEST
    uint32_t result;
    __asm__ volatile (
        "mov r0, %1\n"
        "svc %2\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "r" (ticks), "I" (SVC_TASK_ACTIVE_SLEEP)
        : "r0"
    );
    return result;
#else
    return __task_active_sleep(ticks);
#endif
}

/**
 * @brief Blocking sleep — spin-wait in thread mode (cannot run in SVC handler)
 */
uint32_t task_blocking_sleep(uint32_t ticks) {
    return __task_blocking_sleep(ticks);
}

/**
 * @brief Busy-wait for specified ticks — spin in thread mode
 */
uint32_t task_busy_wait(uint32_t ticks) {
    return __task_busy_wait(ticks);
}

/**
 * @brief Get current system tick count
 */
uint32_t os_get_tick_count(void) {
#ifndef HOST_TEST
    uint32_t result;
    __asm__ volatile (
        "svc %1\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "I" (SVC_OS_GET_TICK_COUNT)
        : "r0"
    );
    return result;
#else
    return __os_get_tick_count();
#endif
}

/**
 * @brief Get name of currently executing task
 */
const char *os_get_current_task_name(void) {
#ifndef HOST_TEST
    const char *result;
    __asm__ volatile (
        "svc %1\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "I" (SVC_OS_GET_CURRENT_TASK_NAME)
        : "r0"
    );
    return result;
#else
    return __os_get_current_task_name();
#endif
}

/**
 * @brief Get number of active tasks
 */
uint8_t os_get_running_task_count(void) {
#ifndef HOST_TEST
    uint8_t result;
    __asm__ volatile (
        "svc %1\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "I" (SVC_OS_GET_RUNNING_TASK_COUNT)
        : "r0"
    );
    return result;
#else
    return __os_get_running_task_count();
#endif
}

/**
 * @brief Get remaining ticks in current time slice
 */
uint32_t os_get_task_ticks_remaining(void) {
#ifndef HOST_TEST
    uint32_t result;
    __asm__ volatile (
        "svc %1\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "I" (SVC_OS_GET_TASK_TICKS_REMAINING)
        : "r0"
    );
    return result;
#else
    return __os_get_task_ticks_remaining();
#endif
}

/* ============================================================================
 * TASK LIFECYCLE WRAPPERS
 * ========================================================================= */

/**
 * @brief Exit current task
 */
void os_exit_task(void) {
#ifndef HOST_TEST
    __asm__ volatile ("svc %0\n" : : "I" (SVC_OS_EXIT_TASK));
#else
    __os_exit_task();
#endif
}

/**
 * @brief Terminate current task (suicide)
 */
void os_task_suicide(void) {
#ifndef HOST_TEST
    __asm__ volatile ("svc %0\n" : : "I" (SVC_OS_TASK_SUICIDE));
#else
    __os_task_suicide();
#endif
}

/**
 * @brief Register a task with the scheduler
 */
void os_register_task(void (*function)(void), const char *name) {
#ifndef HOST_TEST
    __asm__ volatile (
        "mov r0, %0\n"
        "mov r1, %1\n"
        "svc %2\n"
        :
        : "r" (function), "r" (name), "I" (SVC_OS_REGISTER_TASK)
        : "r0", "r1"
    );
#else
    __os_register_task(function, name);
#endif
}

/**
 * @brief Kill a task by index
 */
void os_kill_process(uint8_t task_index) {
#ifndef HOST_TEST
    __asm__ volatile (
        "mov r0, %0\n"
        "svc %1\n"
        :
        : "r" ((uint32_t)task_index), "I" (SVC_OS_KILL_PROCESS)
        : "r0"
    );
#else
    __os_kill_process(task_index);
#endif
}

/* ============================================================================
 * KERNEL DATA WRAPPERS
 * ========================================================================= */

/**
 * @brief Get stack pointer for task index
 */
uint32_t *kernel_get_stack(uint8_t task_idx) {
#ifndef HOST_TEST
    uint32_t *result;
    __asm__ volatile (
        "mov r0, %1\n"
        "svc %2\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "r" ((uint32_t)task_idx), "I" (SVC_KERNEL_GET_STACK)
        : "r0"
    );
    return result;
#else
    return __kernel_get_stack(task_idx);
#endif
}

/**
 * @brief Get data pointer for task index
 */
uint32_t *kernel_get_data(uint8_t task_idx) {
#ifndef HOST_TEST
    uint32_t *result;
    __asm__ volatile (
        "mov r0, %1\n"
        "svc %2\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "r" ((uint32_t)task_idx), "I" (SVC_KERNEL_GET_DATA)
        : "r0"
    );
    return result;
#else
    return __kernel_get_data(task_idx);
#endif
}

/**
 * @brief Allocate protected data words for current task
 */
void *kernel_protected_data(uint16_t num_words) {
#ifndef HOST_TEST
    void *result;
    __asm__ volatile (
        "mov r0, %1\n"
        "svc %2\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "r" ((uint32_t)num_words), "I" (SVC_KERNEL_PROTECTED_DATA)
        : "r0"
    );
    return result;
#else
    return __kernel_protected_data(num_words);
#endif
}

/* ============================================================================
 * SEMAPHORE WRAPPERS
 * ========================================================================= */

/**
 * @brief Initialize a counting semaphore
 */
bool semaphore_init(uint8_t semaphore_idx, uint32_t semaphore_count) {
#ifndef HOST_TEST
    uint32_t result;
    __asm__ volatile (
        "mov r0, %1\n"
        "mov r1, %2\n"
        "svc %3\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "r" ((uint32_t)semaphore_idx), "r" (semaphore_count),
          "I" (SVC_SEMAPHORE_INIT)
        : "r0", "r1"
    );
    return (bool)result;
#else
    return __semaphore_init(semaphore_idx, semaphore_count);
#endif
}

/**
 * @brief Increment semaphore count — spin-wait in thread mode
 */
bool semaphore_feed(uint8_t semaphore_idx) {
    return __semaphore_feed(semaphore_idx);
}

/**
 * @brief Decrement semaphore count — spin-wait in thread mode
 */
bool semaphore_consume(uint8_t semaphore_idx) {
    return __semaphore_consume(semaphore_idx);
}

/**
 * @brief Get current semaphore count
 */
uint32_t semaphore_get_count(uint8_t semaphore_idx) {
#ifndef HOST_TEST
    uint32_t result;
    __asm__ volatile (
        "mov r0, %1\n"
        "svc %2\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "r" ((uint32_t)semaphore_idx), "I" (SVC_SEMAPHORE_GET_COUNT)
        : "r0"
    );
    return result;
#else
    return __semaphore_get_count(semaphore_idx);
#endif
}

/**
 * @brief Get semaphore maximum count
 */
uint32_t semaphore_get_max_count(uint8_t semaphore_idx) {
#ifndef HOST_TEST
    uint32_t result;
    __asm__ volatile (
        "mov r0, %1\n"
        "svc %2\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "r" ((uint32_t)semaphore_idx), "I" (SVC_SEMAPHORE_GET_MAX_COUNT)
        : "r0"
    );
    return result;
#else
    return __semaphore_get_max_count(semaphore_idx);
#endif
}

/* ============================================================================
 * PIPE WRAPPERS
 * ========================================================================= */

/**
 * @brief Initialize a message pipe
 */
bool pipe_init(uint8_t pipe_idx, uint8_t pipe_capacity_bytes) {
#ifndef HOST_TEST
    uint32_t result;
    __asm__ volatile (
        "mov r0, %1\n"
        "mov r1, %2\n"
        "svc %3\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "r" ((uint32_t)pipe_idx), "r" ((uint32_t)pipe_capacity_bytes),
          "I" (SVC_PIPE_INIT)
        : "r0", "r1"
    );
    return (bool)result;
#else
    return __pipe_init(pipe_idx, pipe_capacity_bytes);
#endif
}

/**
 * @brief Enqueue bytes to pipe — spin-wait in thread mode
 */
bool pipe_enqueue(uint8_t pipe_idx, uint8_t *message, uint8_t message_bytes) {
    return __pipe_enqueue(pipe_idx, message, message_bytes);
}

/**
 * @brief Dequeue bytes from pipe — spin-wait in thread mode
 */
bool pipe_dequeue(uint8_t pipe_idx, uint8_t *message, uint8_t message_bytes) {
    return __pipe_dequeue(pipe_idx, message, message_bytes);
}

/**
 * @brief Get current byte count in pipe
 */
uint8_t pipe_get_count(uint8_t pipe_idx) {
#ifndef HOST_TEST
    uint8_t result;
    __asm__ volatile (
        "mov r0, %1\n"
        "svc %2\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "r" ((uint32_t)pipe_idx), "I" (SVC_PIPE_GET_COUNT)
        : "r0"
    );
    return result;
#else
    return __pipe_get_count(pipe_idx);
#endif
}

/**
 * @brief Get pipe capacity
 */
uint8_t pipe_get_max_count(uint8_t pipe_idx) {
#ifndef HOST_TEST
    uint8_t result;
    __asm__ volatile (
        "mov r0, %1\n"
        "svc %2\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "r" ((uint32_t)pipe_idx), "I" (SVC_PIPE_GET_MAX_COUNT)
        : "r0"
    );
    return result;
#else
    return __pipe_get_max_count(pipe_idx);
#endif
}

/* ============================================================================
 * CALL GATE WRAPPERS (for spinning functions — read kernel state via SVC)
 * ========================================================================= */

/**
 * @brief Check if semaphore can accept a feed (count < max && engaged)
 * @note  Runs in privileged mode — safe once DTCM is priv-only
 */
bool sem_can_feed(uint8_t semaphore_idx) {
#ifndef HOST_TEST
    uint32_t result;
    __asm__ volatile (
        "mov r0, %1\n"
        "svc %2\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "r" ((uint32_t)semaphore_idx), "I" (SVC_SEM_CAN_FEED)
        : "r0"
    );
    return (bool)result;
#else
    return __sem_can_feed(semaphore_idx);
#endif
}

/**
 * @brief Check if semaphore can be consumed (count > 0 && engaged)
 * @note  Runs in privileged mode — safe once DTCM is priv-only
 */
bool sem_can_consume(uint8_t semaphore_idx) {
#ifndef HOST_TEST
    uint32_t result;
    __asm__ volatile (
        "mov r0, %1\n"
        "svc %2\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "r" ((uint32_t)semaphore_idx), "I" (SVC_SEM_CAN_CONSUME)
        : "r0"
    );
    return (bool)result;
#else
    return __sem_can_consume(semaphore_idx);
#endif
}

/**
 * @brief Check if pipe has room for message_bytes (free >= bytes && engaged)
 * @note  Runs in privileged mode — safe once DTCM is priv-only
 */
bool pipe_can_enqueue(uint8_t pipe_idx, uint8_t message_bytes) {
#ifndef HOST_TEST
    uint32_t result;
    __asm__ volatile (
        "mov r0, %1\n"
        "mov r1, %2\n"
        "svc %3\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "r" ((uint32_t)pipe_idx), "r" ((uint32_t)message_bytes),
          "I" (SVC_PIPE_CAN_ENQUEUE)
        : "r0", "r1"
    );
    return (bool)result;
#else
    return __pipe_can_enqueue(pipe_idx, message_bytes);
#endif
}

/**
 * @brief Check if pipe has message_bytes available (count >= bytes && engaged)
 * @note  Runs in privileged mode — safe once DTCM is priv-only
 */
bool pipe_can_dequeue(uint8_t pipe_idx, uint8_t message_bytes) {
#ifndef HOST_TEST
    uint32_t result;
    __asm__ volatile (
        "mov r0, %1\n"
        "mov r1, %2\n"
        "svc %3\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "r" ((uint32_t)pipe_idx), "r" ((uint32_t)message_bytes),
          "I" (SVC_PIPE_CAN_DEQUEUE)
        : "r0", "r1"
    );
    return (bool)result;
#else
    return __pipe_can_dequeue(pipe_idx, message_bytes);
#endif
}

/* ============================================================================
 * WRITE GATE WRAPPERS (for spinning functions — modify kernel state via SVC)
 * ========================================================================= */

/**
 * @brief Increment semaphore count atomically in privileged mode
 * @note  Called after sem_can_feed() spin loop exits
 */
void sem_increment(uint8_t semaphore_idx) {
#ifndef HOST_TEST
    __asm__ volatile (
        "mov r0, %0\n"
        "svc %1\n"
        :
        : "r" ((uint32_t)semaphore_idx), "I" (SVC_SEM_INCREMENT)
        : "r0"
    );
#else
    __sem_increment(semaphore_idx);
#endif
}

/**
 * @brief Decrement semaphore count atomically in privileged mode
 * @note  Called after sem_can_consume() spin loop exits
 */
void sem_decrement(uint8_t semaphore_idx) {
#ifndef HOST_TEST
    __asm__ volatile (
        "mov r0, %0\n"
        "svc %1\n"
        :
        : "r" ((uint32_t)semaphore_idx), "I" (SVC_SEM_DECREMENT)
        : "r0"
    );
#else
    __sem_decrement(semaphore_idx);
#endif
}

/**
 * @brief Write bytes to pipe buffer atomically in privileged mode
 * @note  Called after pipe_can_enqueue() spin loop exits
 */
void pipe_write_bytes(uint8_t pipe_idx, uint8_t *message, uint8_t message_bytes) {
#ifndef HOST_TEST
    __asm__ volatile (
        "mov r0, %0\n"
        "mov r1, %1\n"
        "mov r2, %2\n"
        "svc %3\n"
        :
        : "r" ((uint32_t)pipe_idx), "r" (message), "r" ((uint32_t)message_bytes),
          "I" (SVC_PIPE_WRITE_BYTES)
        : "r0", "r1", "r2"
    );
#else
    __pipe_write_bytes(pipe_idx, message, message_bytes);
#endif
}

/**
 * @brief Read bytes from pipe buffer atomically in privileged mode
 * @note  Called after pipe_can_dequeue() spin loop exits
 */
void pipe_read_bytes(uint8_t pipe_idx, uint8_t *message, uint8_t message_bytes) {
#ifndef HOST_TEST
    __asm__ volatile (
        "mov r0, %0\n"
        "mov r1, %1\n"
        "mov r2, %2\n"
        "svc %3\n"
        :
        : "r" ((uint32_t)pipe_idx), "r" (message), "r" ((uint32_t)message_bytes),
          "I" (SVC_PIPE_READ_BYTES)
        : "r0", "r1", "r2"
    );
#else
    __pipe_read_bytes(pipe_idx, message, message_bytes);
#endif
}


/* ============================================================================
 * TASK METADATA READ GATES (for display/diagnostics from unprivileged mode)
 * ========================================================================= */

/**
 * @brief Get task name by index (read from task_list in privileged mode)
 * @param task_idx Task index (0 to num_created_tasks-1)
 * @return Pointer to task name string, or NULL if invalid index
 * @note  Safe to call from unprivileged mode once DTCM is priv-only
 */
const char *os_get_task_name(uint8_t task_idx) {
#ifndef HOST_TEST
    const char *result;
    __asm__ volatile (
        "mov r0, %1\n"
        "svc %2\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "r" ((uint32_t)task_idx), "I" (SVC_GET_TASK_NAME)
        : "r0"
    );
    return result;
#else
    return __os_get_task_name(task_idx);
#endif
}

/**
 * @brief Get number of created tasks (read num_created_tasks in privileged mode)
 * @return Number of tasks registered with os_register_task()
 * @note  Safe to call from unprivileged mode once DTCM is priv-only
 */
uint8_t os_get_num_created_tasks(void) {
#ifndef HOST_TEST
    uint8_t result;
    __asm__ volatile (
        "svc %1\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "I" (SVC_GET_NUM_TASKS)
        : "r0"
    );
    return result;
#else
    return __os_get_num_created_tasks();
#endif
}

/**
 * @brief Check if OS is running (safe from unprivileged mode)
 * @return 1 if OS is running, 0 otherwise
 * @note  Safe to call from unprivileged mode once DTCM is priv-only
 */
uint8_t os_is_running(void) {
#ifndef HOST_TEST
    uint8_t result;
    __asm__ volatile (
        "svc %1\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "I" (SVC_OS_IS_RUNNING)
        : "r0"
    );
    return result;
#else
    return __os_is_running();
#endif
}
