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
 * @note  Internal function - use os_get_tick_count() wrapper from svc.c
 */
ITCM_FUNC uint32_t __os_get_tick_count(void)
{
    return __os_tick_count;
}

uint8_t __os_get_running_task_count(void)
{
    return __running_task_count;
}

/**
 * @brief Privileged implementation of os_get_current_task_name
 * @note  Internal function - use os_get_current_task_name() wrapper from svc.c
 */
const char* __os_get_current_task_name(void)
{
    if (__current_task_index < __num_created_tasks &&
        __task_list[__current_task_index] != NULL) {
        return __task_list[__current_task_index]->name;
    }
    return "unknown";
}

uint32_t __os_get_task_ticks_remaining(void)
{
    return __current_task_ticks_remaining;
}

/* ============================================================================
 * TASK SCHEDULING
 * ========================================================================= */

/**
 * @brief Privileged implementation of os_yield
 * @note  Internal function - use os_yield() wrapper from svc.c
 */
ITCM_FUNC void __os_yield(void)
{
    __current_task_ticks_remaining = __ticks_per_task;
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

/**
 * @brief Privileged implementation of task_active_sleep
 * @note  Internal function - use task_active_sleep() wrapper from svc.c
 */
ITCM_FUNC uint32_t __task_active_sleep(uint32_t ticks)
{
    __task_list[__current_task_index]->global_tick_paused = __os_tick_count;
    __task_list[__current_task_index]->ticks_to_pause = ticks;
    __task_list[__current_task_index]->task_state = TASK_STATE_BLOCKED;
    __os_yield();
    return __os_tick_count - __task_list[__current_task_index]->global_tick_paused;
}

/**
 * @brief Privileged implementation of task_blocking_sleep
 * @note  Internal function - use task_blocking_sleep() wrapper from svc.c
 */
uint32_t __task_blocking_sleep(uint32_t ticks)
{
    __enter_critical();
    uint32_t _delta = __task_busy_wait(ticks);
    __exit_critical();
    return _delta;
}

uint32_t __task_busy_wait(uint32_t ticks)
{
    uint32_t _st = __os_tick_count;
    uint32_t _delta;

    while (1) {
#ifdef HOST_TEST
        __os_tick_count++;
#endif
        _delta = __os_tick_count - _st;
        if (_delta >= ticks) {
            break;
        }
    }
    return _delta;
}
