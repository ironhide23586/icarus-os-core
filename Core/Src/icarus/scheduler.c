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

#include <stddef.h>
#ifndef HOST_TEST
#include "stm32h7xx.h"
#else
#include "cmsis_gcc.h"
#endif

/* ============================================================================
 * SECTION PLACEMENT MACROS
 * ========================================================================= */

/* Defined centrally in icarus/config.h — included via icarus/kernel.h */

/* ============================================================================
 * TASK INFORMATION
 * ========================================================================= */

/**
 * @brief Privileged implementation of os_get_tick_count
 * @note  Internal function - use os_get_tick_count() wrapper
 */
ITCM_FUNC_PRIV uint32_t __os_get_tick_count(void) {
    return os_tick_count;
}

/**
 * @brief Privileged implementation of os_get_running_task_count
 * @note  Internal function - use os_get_running_task_count() wrapper
 */
uint8_t __os_get_running_task_count(void) {
    return running_task_count;
}

/**
 * @brief Privileged implementation of os_get_current_task_name
 * @note  Internal function - use os_get_current_task_name() wrapper
 */
const char *__os_get_current_task_name(void) {
    if (current_task_index < num_created_tasks &&
        task_list[current_task_index] != NULL) {
        return task_list[current_task_index]->name;
    }
    return "unknown";
}

/**
 * @brief Privileged implementation of os_get_task_ticks_remaining
 * @note  Internal function - use os_get_task_ticks_remaining() wrapper
 */
uint32_t __os_get_task_ticks_remaining(void) {
    return current_task_ticks_remaining;
}

/* ============================================================================
 * TASK SCHEDULING
 * ========================================================================= */

/**
 * @brief Privileged implementation of os_yield
 * @note  Internal function - use os_yield() wrapper
 */
ITCM_FUNC_PRIV void __os_yield(void) {
    current_task_ticks_remaining = ticks_per_task;
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

/**
 * @brief Privileged implementation of task_active_sleep
 * @note  Internal function - use task_active_sleep() wrapper
 */
ITCM_FUNC_PRIV uint32_t __task_active_sleep(uint32_t ticks) {
    task_list[current_task_index]->global_tick_paused = os_tick_count;
    task_list[current_task_index]->ticks_to_pause = ticks;
    task_list[current_task_index]->task_state = TASK_STATE_BLOCKED;
    __os_yield();
    return os_tick_count - task_list[current_task_index]->global_tick_paused;
}

/**
 * @brief Privileged implementation of task_blocking_sleep
 * @note  Internal function - use task_blocking_sleep() wrapper
 * @note  Spin-wait calls public SVC wrappers — only safe from thread mode
 */
uint32_t __task_blocking_sleep(uint32_t ticks) {
    enter_critical();
    uint32_t delta = task_busy_wait(ticks);
    exit_critical();
    return delta;
}

/**
 * @brief Privileged implementation of task_busy_wait
 * @note  Internal function - use task_busy_wait() wrapper
 */
uint32_t __task_busy_wait(uint32_t ticks) {
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
