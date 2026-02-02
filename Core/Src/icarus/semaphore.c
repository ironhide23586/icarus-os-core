/**
 * @file    semaphore.c
 * @brief   ICARUS Semaphore Implementation
 * @version 0.1.0
 *
 * @details Counting semaphore implementation for task synchronization.
 *
 * @author  Souham Biswas
 * @date    2025
 *
 * @copyright Copyright 2025 Souham Biswas
 *            https://github.com/ironhide23586/icarus-os-core
 *            Licensed under the Apache License, Version 2.0
 */

#include "icarus/semaphore.h"
#include "icarus/kernel.h"
#include "icarus/scheduler.h"

/* ============================================================================
 * SECTION PLACEMENT MACROS
 * ========================================================================= */

#ifndef HOST_TEST
#define ITCM_FUNC __attribute__((section(".itcm")))
#else
#define ITCM_FUNC
#endif

/* ============================================================================
 * SEMAPHORE IMPLEMENTATION
 * ========================================================================= */

bool semaphore_init(uint8_t semaphore_idx, uint32_t semaphore_count)
{
    enter_critical();

    if (semaphore_idx < ICARUS_MAX_SEMAPHORES && semaphore_count > 0) {
        if (!semaphore_list[semaphore_idx]->engaged) {
            semaphore_list[semaphore_idx]->count = semaphore_count;
            semaphore_list[semaphore_idx]->max_count = semaphore_count;
            semaphore_list[semaphore_idx]->tick_updated_at = os_tick_count;
            semaphore_list[semaphore_idx]->engaged = true;
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
