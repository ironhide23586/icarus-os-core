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
 * TASK SCHEDULING
 * ========================================================================= */

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
        os_tick_count++;
#endif
        delta = os_tick_count - st;
        if (delta >= ticks) {
            break;
        }
    }
    return delta;
}
