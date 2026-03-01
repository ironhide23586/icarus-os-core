/**
 * @file    svc.c
 * @brief   ICARUS Supervisor Call (SVC) Implementation
 * @version 0.1.0
 *
 * @details SVC handler and wrapper functions for MPU-protected kernel calls.
 *
 *          TEMPORARY: All wrappers are direct calls to __func for debugging.
 *          SVC inline assembly and SVC_Handler_C dispatcher are preserved
 *          but disabled behind ENABLE_SVC_DISPATCH (not yet defined).
 *
 * @author  Souham Biswas
 * @date    2025
 *
 * @copyright Copyright 2025 Souham Biswas
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
void SVC_Handler_C(uint32_t *stack_frame)
{
    uint8_t svc_number = ((uint8_t *)(uintptr_t)stack_frame[6])[-2];
    uint32_t arg0 = stack_frame[0];
    uint32_t arg1 = stack_frame[1];
    uint32_t arg2 = stack_frame[2];
    (void)arg2;

    switch (svc_number) {
        case SVC_ENTER_CRITICAL:
            __enter_critical();
            break;
        case SVC_EXIT_CRITICAL:
            __exit_critical();
            break;

        /* Scheduler group */
        case SVC_OS_YIELD:
            __os_yield();
            break;
        case SVC_TASK_ACTIVE_SLEEP:
            stack_frame[0] = __task_active_sleep(arg0);
            break;
        case SVC_TASK_BUSY_WAIT:
            /* Not dispatched via SVC — spin-loop runs in thread mode */
            break;
        case SVC_TASK_BLOCKING_SLEEP:
            /* Not dispatched via SVC — uses enter_critical + busy_wait */
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

        /* Task lifecycle group */
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

        /* Kernel data group */
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

        /* Semaphore group (non-spinning) */
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

        /* Pipe group (non-spinning) */
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

        /* Spinning functions — dispatched from thread mode wrappers */
        case SVC_SEMAPHORE_FEED:
        case SVC_SEMAPHORE_CONSUME:
        case SVC_PIPE_ENQUEUE:
        case SVC_PIPE_DEQUEUE:
            break;

        default:
            break;
    }
}

#endif /* HOST_TEST */

/* ============================================================================
 * ALL WRAPPERS - Direct calls (SVC dispatch disabled for debugging)
 * ========================================================================= */

void enter_critical(void)
{
#ifndef HOST_TEST
    __asm__ volatile (
        "svc %0\n"
        :
        : "I" (SVC_ENTER_CRITICAL)
    );
#else
    __enter_critical();
#endif
}

void exit_critical(void)
{
#ifndef HOST_TEST
    __asm__ volatile (
        "svc %0\n"
        :
        : "I" (SVC_EXIT_CRITICAL)
    );
#else
    __exit_critical();
#endif
}
void os_init(void)                     { __os_init(); }
void os_start(void)                    { __os_start(); }
void os_yield(void)
{
#ifndef HOST_TEST
    __asm__ volatile ("svc %0\n" : : "I" (SVC_OS_YIELD));
#else
    __os_yield();
#endif
}

void os_exit_task(void)
{
#ifndef HOST_TEST
    __asm__ volatile ("svc %0\n" : : "I" (SVC_OS_EXIT_TASK));
#else
    __os_exit_task();
#endif
}
void os_task_suicide(void)
{
#ifndef HOST_TEST
    __asm__ volatile ("svc %0\n" : : "I" (SVC_OS_TASK_SUICIDE));
#else
    __os_task_suicide();
#endif
}

void* kernel_protected_data(uint16_t num_words) {
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

uint32_t* kernel_get_stack(uint8_t task_idx) {
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
uint32_t* kernel_get_data(uint8_t task_idx) {
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
const char* os_get_current_task_name(void) {
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
uint32_t task_blocking_sleep(uint32_t ticks) {
#ifndef HOST_TEST
    enter_critical();
    uint32_t delta = task_busy_wait(ticks);
    exit_critical();
    return delta;
#else
    return __task_blocking_sleep(ticks);
#endif
}
uint32_t task_busy_wait(uint32_t ticks) {
#ifndef HOST_TEST
    /* Spin-loop must run in thread mode so SysTick can fire.
       Reads os_tick_count directly — will be in user-readable
       memory once MPU regions are configured. */
    extern volatile uint32_t os_tick_count;
    uint32_t st = os_tick_count;
    uint32_t delta;
    while (1) {
        delta = os_tick_count - st;
        if (delta >= ticks) {
            break;
        }
    }
    return delta;
#else
    return __task_busy_wait(ticks);
#endif
}
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
bool pipe_enqueue(uint8_t pipe_idx, uint8_t* message, uint8_t message_bytes) {
#ifndef HOST_TEST
    if (pipe_idx >= ICARUS_MAX_MESSAGE_QUEUES ||
        !message_pipe_list[pipe_idx]->engaged ||
        message_bytes > message_pipe_list[pipe_idx]->max_count) {
        return false;
    }

    while ((message_pipe_list[pipe_idx]->max_count -
            message_pipe_list[pipe_idx]->count) < message_bytes) {
        task_active_sleep(1);
    }

    enter_critical();

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
#else
    return __pipe_enqueue(pipe_idx, message, message_bytes);
#endif
}
bool pipe_dequeue(uint8_t pipe_idx, uint8_t* message, uint8_t message_bytes) {
#ifndef HOST_TEST
    if (pipe_idx >= ICARUS_MAX_MESSAGE_QUEUES ||
        !message_pipe_list[pipe_idx]->engaged ||
        message_bytes > message_pipe_list[pipe_idx]->max_count) {
        return false;
    }

    while (message_pipe_list[pipe_idx]->count < message_bytes) {
        task_active_sleep(1);
    }

    enter_critical();

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
#else
    return __pipe_dequeue(pipe_idx, message, message_bytes);
#endif
}
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
bool semaphore_feed(uint8_t semaphore_idx) {
#ifndef HOST_TEST
    if (semaphore_idx >= ICARUS_MAX_SEMAPHORES ||
        !semaphore_list[semaphore_idx]->engaged) {
        return false;
    }

    while (semaphore_list[semaphore_idx]->count >=
           semaphore_list[semaphore_idx]->max_count) {
        task_active_sleep(1);
    }

    enter_critical();
    ++semaphore_list[semaphore_idx]->count;
    semaphore_list[semaphore_idx]->tick_updated_at = os_tick_count;
    exit_critical();

    return true;
#else
    return __semaphore_feed(semaphore_idx);
#endif
}
bool semaphore_consume(uint8_t semaphore_idx) {
#ifndef HOST_TEST
    if (semaphore_idx >= ICARUS_MAX_SEMAPHORES ||
        !semaphore_list[semaphore_idx]->engaged) {
        return false;
    }

    while (semaphore_list[semaphore_idx]->count == 0) {
        task_active_sleep(1);
    }

    enter_critical();
    --semaphore_list[semaphore_idx]->count;
    semaphore_list[semaphore_idx]->tick_updated_at = os_tick_count;
    exit_critical();

    return true;
#else
    return __semaphore_consume(semaphore_idx);
#endif
}
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
