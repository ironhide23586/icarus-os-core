/**
 * @file    scheduler.c
 * @brief   ICARUS Scheduler Implementation
 * @version 0.1.0
 *
 * @details Task scheduling, yielding, and sleep functions.
 *
 * @author  Souham Biswas
 * @date    2025
 *
 * @copyright Copyright 2025 Souham Biswas
 *            https://github.com/ironhide23586/icarus-os-core
 *            Licensed under the Apache License, Version 2.0
 */

#include "icarus/scheduler.h"
#include "icarus/kernel.h"
#include "bsp/config.h"

#include <stddef.h>  // For NULL
#ifndef HOST_TEST
#include "stm32h7xx.h"  // For SCB and SCB_ICSR_PENDSVSET_Msk
#else
#include "cmsis_gcc.h"  // For SCB mock during testing
#endif

/* ============================================================================
 * SECTION PLACEMENT MACROS
 * ========================================================================= */

#ifndef HOST_TEST
#define ITCM_FUNC __attribute__((section(".itcm")))
#else
#define ITCM_FUNC
#endif

/* ============================================================================
 * TASK INFORMATION
 * ========================================================================= */

/**
 * @brief Privileged implementation of os_get_tick_count
 * @note  Internal function - use os_get_tick_count() wrapper
 */
ITCM_FUNC uint32_t __os_get_tick_count(void)
{
    return os_tick_count;
}

/**
 * @brief Public API for getting tick count
 * @note  Will become SVC wrapper in privileged mode
 */
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

uint8_t os_get_running_task_count(void)
{
    return running_task_count;
}

/**
 * @brief Privileged implementation of os_get_current_task_name
 * @note  Internal function - use os_get_current_task_name() wrapper
 */
const char* __os_get_current_task_name(void)
{
    if (current_task_index < num_created_tasks &&
        task_list[current_task_index] != NULL) {
        return task_list[current_task_index]->name;
    }
    return "unknown";
}

/**
 * @brief Public API for getting current task name
 * @note  Will become SVC wrapper in privileged mode
 */
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

uint32_t os_get_task_ticks_remaining(void)
{
    return current_task_ticks_remaining;
}

/* ============================================================================
 * TASK SCHEDULING
 * ========================================================================= */

/**
 * @brief Privileged implementation of os_yield
 * @note  Internal function - use os_yield() wrapper
 */
ITCM_FUNC void __os_yield(void)
{
    current_task_ticks_remaining = ticks_per_task;
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

/**
 * @brief Public API for yielding CPU
 * @note  Will become SVC wrapper in privileged mode
 */
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

/**
 * @brief Privileged implementation of task_active_sleep
 * @note  Internal function - use task_active_sleep() wrapper
 */
ITCM_FUNC uint32_t __task_active_sleep(uint32_t ticks)
{
    task_list[current_task_index]->global_tick_paused = os_tick_count;
    task_list[current_task_index]->ticks_to_pause = ticks;
    task_list[current_task_index]->task_state = TASK_STATE_BLOCKED;
    __os_yield();
    return os_tick_count - task_list[current_task_index]->global_tick_paused;
}

/**
 * @brief Public API for active sleep (with context switch)
 * @note  Will become SVC wrapper in privileged mode
 */
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

/**
 * @brief Privileged implementation of task_blocking_sleep
 * @note  Internal function - use task_blocking_sleep() wrapper
 */
uint32_t __task_blocking_sleep(uint32_t ticks)
{
    __enter_critical();
    uint32_t delta = task_busy_wait(ticks);
    __exit_critical();
    return delta;
}

/**
 * @brief Public API for blocking sleep (busy-wait)
 * @note  Will become SVC wrapper in privileged mode
 */
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
    uint32_t st = os_tick_count;
    uint32_t delta;

    while (1) {
#ifdef HOST_TEST
        os_tick_count++;
#endif
        delta = os_tick_count - st;
        if (delta >= ticks) {
            break;
        }
    }
    return delta;
}
