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
    (void)arg0;  /* unused for now */

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
        case SVC_TASK_BLOCKING_SLEEP:
            stack_frame[0] = __task_blocking_sleep(arg0);
            break;
        case SVC_TASK_BUSY_WAIT:
            stack_frame[0] = __task_busy_wait(arg0);
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

void os_exit_task(void)                { __os_exit_task(); }
void os_task_suicide(void)             { __os_task_suicide(); }

void* kernel_protected_data(uint16_t num_words) {
    return __kernel_protected_data(num_words);
}

uint32_t* kernel_get_stack(uint8_t task_idx) {
    return __kernel_get_stack(task_idx);
}
uint32_t* kernel_get_data(uint8_t task_idx) {
    return __kernel_get_data(task_idx);
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
    return __task_active_sleep(ticks);
}
uint32_t task_blocking_sleep(uint32_t ticks) {
    return __task_blocking_sleep(ticks);
}
uint32_t task_busy_wait(uint32_t ticks) {
    return __task_busy_wait(ticks);
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
    __os_register_task(function, name);
}
void os_kill_process(uint8_t task_index) {
    __os_kill_process(task_index);
}
bool pipe_init(uint8_t pipe_idx, uint8_t pipe_capacity_bytes) {
    return __pipe_init(pipe_idx, pipe_capacity_bytes);
}
bool pipe_enqueue(uint8_t pipe_idx, uint8_t* message, uint8_t message_bytes) {
    return __pipe_enqueue(pipe_idx, message, message_bytes);
}
bool pipe_dequeue(uint8_t pipe_idx, uint8_t* message, uint8_t message_bytes) {
    return __pipe_dequeue(pipe_idx, message, message_bytes);
}
uint8_t pipe_get_count(uint8_t pipe_idx) {
    return __pipe_get_count(pipe_idx);
}
uint8_t pipe_get_max_count(uint8_t pipe_idx) {
    return __pipe_get_max_count(pipe_idx);
}
bool semaphore_init(uint8_t semaphore_idx, uint32_t semaphore_count) {
    return __semaphore_init(semaphore_idx, semaphore_count);
}
bool semaphore_feed(uint8_t semaphore_idx) {
    return __semaphore_feed(semaphore_idx);
}
bool semaphore_consume(uint8_t semaphore_idx) {
    return __semaphore_consume(semaphore_idx);
}
uint32_t semaphore_get_count(uint8_t semaphore_idx) {
    return __semaphore_get_count(semaphore_idx);
}
uint32_t semaphore_get_max_count(uint8_t semaphore_idx) {
    return __semaphore_get_max_count(semaphore_idx);
}
