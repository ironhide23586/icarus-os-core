/**
 * @file    svc.c
 * @brief   ICARUS Supervisor Call (SVC) Implementation
 * @version 0.1.0
 *
 * @details SVC handler and wrapper functions for MPU-protected kernel calls.
 *
 *          ARCHITECTURE:
 *          - Public API functions (no prefix) are defined here as SVC wrappers
 *          - Each wrapper invokes an SVC instruction with a unique SVC number
 *          - The SVC handler (in assembly) switches to privileged mode
 *          - Privileged implementations (__prefix) execute with full access
 *          - This provides memory protection via MPU while maintaining clean API
 *
 *          NAMING CONVENTION:
 *          - Public API: function_name() - SVC wrapper (this file)
 *          - Privileged: __function_name() - actual implementation (other files)
 *
 *          HOST_TEST MODE:
 *          - When HOST_TEST is defined, wrappers call privileged functions directly
 *          - This allows unit testing on host without ARM SVC instructions
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
 * CRITICAL SECTION WRAPPERS
 * ========================================================================= */

void enter_critical(void)
{
#ifdef HOST_TEST
    __enter_critical();
#else
    __asm__ volatile (
        "svc %0\n"
        :
        : "I" (SVC_ENTER_CRITICAL)
    );
#endif
}

void exit_critical(void)
{
#ifdef HOST_TEST
    __exit_critical();
#else
    __asm__ volatile (
        "svc %0\n"
        :
        : "I" (SVC_EXIT_CRITICAL)
    );
#endif
}

/* ============================================================================
 * KERNEL INITIALIZATION WRAPPERS
 * ========================================================================= */

void os_init(void)
{
#ifdef HOST_TEST
    __os_init();
#else
    __asm__ volatile (
        "svc %0\n"
        :
        : "I" (SVC_OS_INIT)
    );
#endif
}

void os_start(void)
{
#ifdef HOST_TEST
    __os_start();
#else
    __asm__ volatile (
        "svc %0\n"
        :
        : "I" (SVC_OS_START)
    );
#endif
}

void* kernel_protected_data(uint16_t num_words)
{
#ifdef HOST_TEST
    return __kernel_protected_data(num_words);
#else
    void* res;
    __asm__ volatile (
        "mov r0, %1\n"
        "svc %2\n"
        "mov %0, r0\n"
        : "=r" (res)
        : "r" (num_words), "I" (SVC_KERNEL_PROTECTED_DATA)
        : "r0"
    );
    return res;
#endif
}

uint32_t* kernel_get_stack(uint8_t task_idx)
{
#ifdef HOST_TEST
    return __kernel_get_stack(task_idx);
#else
    uint32_t* stack;
    __asm__ volatile (
        "mov r0, %1\n"
        "svc %2\n"
        "mov %0, r0\n"
        : "=r" (stack)
        : "r" (task_idx), "I" (SVC_KERNEL_GET_STACK)
        : "r0"
    );
    return stack;
#endif
}

uint32_t* kernel_get_data(uint8_t task_idx)
{
#ifdef HOST_TEST
    return __kernel_get_data(task_idx);
#else
    uint32_t* data;
    __asm__ volatile (
        "mov r0, %1\n"
        "svc %2\n"
        "mov %0, r0\n"
        : "=r" (data)
        : "r" (task_idx), "I" (SVC_KERNEL_GET_DATA)
        : "r0"
    );
    return data;
#endif
}

/* ============================================================================
 * SCHEDULER WRAPPERS
 * ========================================================================= */

void os_yield(void)
{
#ifdef HOST_TEST
    __os_yield();
#else
    __asm__ volatile (
        "svc %0\n"
        :
        : "I" (SVC_OS_YIELD)
    );
#endif
}

uint32_t os_get_tick_count(void)
{
#ifdef HOST_TEST
    return __os_get_tick_count();
#else
    uint32_t res;
    __asm__ volatile (
        "svc %1\n"
        "mov %0, r0\n"
        : "=r" (res)
        : "I" (SVC_OS_GET_TICK_COUNT)
        : "r0"
    );
    return res;
#endif
}

const char* os_get_current_task_name(void)
{
#ifdef HOST_TEST
    return __os_get_current_task_name();
#else
    const char* res;
    __asm__ volatile (
        "svc %1\n"
        "mov %0, r0\n"
        : "=r" (res)
        : "I" (SVC_OS_GET_CURRENT_TASK_NAME)
        : "r0"
    );
    return res;
#endif
}

uint32_t task_active_sleep(uint32_t ticks)
{
#ifdef HOST_TEST
    return __task_active_sleep(ticks);
#else
    uint32_t res;
    __asm__ volatile (
        "mov r0, %1\n"
        "svc %2\n"
        "mov %0, r0\n"
        : "=r" (res)
        : "r" (ticks), "I" (SVC_TASK_ACTIVE_SLEEP)
        : "r0"
    );
    return res;
#endif
}

uint32_t task_blocking_sleep(uint32_t ticks)
{
#ifdef HOST_TEST
    return __task_blocking_sleep(ticks);
#else
    uint32_t res;
    __asm__ volatile (
        "mov r0, %1\n"
        "svc %2\n"
        "mov %0, r0\n"
        : "=r" (res)
        : "r" (ticks), "I" (SVC_TASK_BLOCKING_SLEEP)
        : "r0"
    );
    return res;
#endif
}

uint32_t task_busy_wait(uint32_t ticks)
{
#ifdef HOST_TEST
    return __task_busy_wait(ticks);
#else
    uint32_t res;
    __asm__ volatile (
        "mov r0, %1\n"
        "svc %2\n"
        "mov %0, r0\n"
        : "=r" (res)
        : "r" (ticks), "I" (SVC_TASK_BUSY_WAIT)
        : "r0"
    );
    return res;
#endif
}

uint8_t os_get_running_task_count(void)
{
#ifdef HOST_TEST
    return __os_get_running_task_count();
#else
    uint8_t count;
    __asm__ volatile (
        "svc %1\n"
        "mov %0, r0\n"
        : "=r" (count)
        : "I" (SVC_OS_GET_RUNNING_TASK_COUNT)
        : "r0"
    );
    return count;
#endif
}

uint32_t os_get_task_ticks_remaining(void)
{
#ifdef HOST_TEST
    return __os_get_task_ticks_remaining();
#else
    uint32_t ticks;
    __asm__ volatile (
        "svc %1\n"
        "mov %0, r0\n"
        : "=r" (ticks)
        : "I" (SVC_OS_GET_TASK_TICKS_REMAINING)
        : "r0"
    );
    return ticks;
#endif
}

/* ============================================================================
 * TASK MANAGEMENT WRAPPERS
 * ========================================================================= */

void os_register_task(void (*function)(void), const char *name)
{
#ifdef HOST_TEST
    __os_register_task(function, name);
#else
    __asm__ volatile (
        "mov r0, %0\n"
        "mov r1, %1\n"
        "svc %2\n"
        :
        : "r" (function), "r" (name), "I" (SVC_OS_REGISTER_TASK)
        : "r0", "r1"
    );
#endif
}

void os_exit_task(void)
{
#ifdef HOST_TEST
    __os_exit_task();
#else
    __asm__ volatile (
        "svc %0\n"
        :
        : "I" (SVC_OS_EXIT_TASK)
        :
    );
#endif
}

void os_kill_process(uint8_t task_index)
{
#ifdef HOST_TEST
    __os_kill_process(task_index);
#else
    __asm__ volatile (
        "mov r0, %0\n"
        "svc %1\n"
        :
        : "r" (task_index), "I" (SVC_OS_KILL_PROCESS)
        : "r0"
    );
#endif
}

void os_task_suicide(void)
{
#ifdef HOST_TEST
    __os_task_suicide();
#else
    __asm__ volatile (
        "svc %0\n"
        :
        : "I" (SVC_OS_TASK_SUICIDE)
        :
    );
#endif
}

/* ============================================================================
 * PIPE WRAPPERS
 * ========================================================================= */

bool pipe_init(uint8_t pipe_idx, uint8_t pipe_capacity_bytes)
{
#ifdef HOST_TEST
    return __pipe_init(pipe_idx, pipe_capacity_bytes);
#else
    bool res;
    __asm__ volatile (
        "mov r0, %1\n"
        "mov r1, %2\n"
        "svc %3\n"
        "mov %0, r0\n"
        : "=r" (res)
        : "r" (pipe_idx), "r" (pipe_capacity_bytes), "I" (SVC_PIPE_INIT)
        : "r0", "r1"
    );
    return res;
#endif
}

bool pipe_enqueue(uint8_t pipe_idx, uint8_t* message, uint8_t message_bytes)
{
#ifdef HOST_TEST
    return __pipe_enqueue(pipe_idx, message, message_bytes);
#else
    bool res;
    __asm__ volatile (
        "mov r0, %1\n"
        "mov r1, %2\n"
        "mov r2, %3\n"
        "svc %4\n"
        "mov %0, r0\n"
        : "=r" (res)
        : "r" (pipe_idx), "r" (message), "r" (message_bytes), "I" (SVC_PIPE_ENQUEUE)
        : "r0", "r1", "r2"
    );
    return res;
#endif
}

bool pipe_dequeue(uint8_t pipe_idx, uint8_t* message, uint8_t message_bytes)
{
#ifdef HOST_TEST
    return __pipe_dequeue(pipe_idx, message, message_bytes);
#else
    bool res;
    __asm__ volatile (
        "mov r0, %1\n"
        "mov r1, %2\n"
        "mov r2, %3\n"
        "svc %4\n"
        "mov %0, r0\n"
        : "=r" (res)
        : "r" (pipe_idx), "r" (message), "r" (message_bytes), "I" (SVC_PIPE_DEQUEUE)
        : "r0", "r1", "r2"
    );
    return res;
#endif
}

uint8_t pipe_get_count(uint8_t pipe_idx)
{
#ifdef HOST_TEST
    return __pipe_get_count(pipe_idx);
#else
    uint8_t count;
    __asm__ volatile (
        "mov r0, %1\n"
        "svc %2\n"
        "mov %0, r0\n"
        : "=r" (count)
        : "r" (pipe_idx), "I" (SVC_PIPE_GET_COUNT)
        : "r0"
    );
    return count;
#endif
}

uint8_t pipe_get_max_count(uint8_t pipe_idx)
{
#ifdef HOST_TEST
    return __pipe_get_max_count(pipe_idx);
#else
    uint8_t max_count;
    __asm__ volatile (
        "mov r0, %1\n"
        "svc %2\n"
        "mov %0, r0\n"
        : "=r" (max_count)
        : "r" (pipe_idx), "I" (SVC_PIPE_GET_MAX_COUNT)
        : "r0"
    );
    return max_count;
#endif
}

/* ============================================================================
 * SEMAPHORE WRAPPERS
 * ========================================================================= */

bool semaphore_init(uint8_t semaphore_idx, uint32_t semaphore_count)
{
#ifdef HOST_TEST
    return __semaphore_init(semaphore_idx, semaphore_count);
#else
    bool semaphore_init_succeeded;
    __asm__ volatile (
        "mov r0, %1\n"
        "mov r1, %2\n"
        "svc %3\n"
        "mov %0, r0\n"
        : "=r" (semaphore_init_succeeded)
        : "r" (semaphore_idx), "r" (semaphore_count), "I" (SVC_SEMAPHORE_INIT)
        : "r0", "r1"
    );
    return semaphore_init_succeeded;
#endif
}

bool semaphore_feed(uint8_t semaphore_idx)
{
#ifdef HOST_TEST
    return __semaphore_feed(semaphore_idx);
#else
    bool semaphore_feed_succeeded;
    __asm__ volatile (
        "mov r0, %1\n"
        "svc %2\n"
        "mov %0, r0\n"
        : "=r" (semaphore_feed_succeeded)
        : "r" (semaphore_idx), "I" (SVC_SEMAPHORE_FEED)
        : "r0"
    );
    return semaphore_feed_succeeded;
#endif
}

bool semaphore_consume(uint8_t semaphore_idx)
{
#ifdef HOST_TEST
    return __semaphore_consume(semaphore_idx);
#else
    bool semaphore_consume_succeeded;
    __asm__ volatile (
        "mov r0, %1\n"
        "svc %2\n"
        "mov %0, r0\n"
        : "=r" (semaphore_consume_succeeded)
        : "r" (semaphore_idx), "I" (SVC_SEMAPHORE_CONSUME)
        : "r0"
    );
    return semaphore_consume_succeeded;
#endif
}

uint32_t semaphore_get_count(uint8_t semaphore_idx)
{
#ifdef HOST_TEST
    return __semaphore_get_count(semaphore_idx);
#else
    uint32_t count;
    __asm__ volatile (
        "mov r0, %1\n"
        "svc %2\n"
        "mov %0, r0\n"
        : "=r" (count)
        : "r" (semaphore_idx), "I" (SVC_SEMAPHORE_GET_COUNT)
        : "r0"
    );
    return count;
#endif
}

uint32_t semaphore_get_max_count(uint8_t semaphore_idx)
{
#ifdef HOST_TEST
    return __semaphore_get_max_count(semaphore_idx);
#else
    uint32_t max_count;
    __asm__ volatile (
        "mov r0, %1\n"
        "svc %2\n"
        "mov %0, r0\n"
        : "=r" (max_count)
        : "r" (semaphore_idx), "I" (SVC_SEMAPHORE_GET_MAX_COUNT)
        : "r0"
    );
    return max_count;
#endif
}
