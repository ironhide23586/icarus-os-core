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
#include "icarus/svc.h"

/* ============================================================================
 * SECTION PLACEMENT MACROS
 * ========================================================================= */

#ifndef HOST_TEST
#define ITCM_FUNC_USER __attribute__((section(".itcm")))
#define ITCM_FUNC_PRIV __attribute__((section(".itcm.privileged")))
#else
#define ITCM_FUNC_USER
#define ITCM_FUNC_PRIV
#endif

/* ============================================================================
 * SEMAPHORE IMPLEMENTATION
 * ========================================================================= */

/* Privileged implementation - called via SVC from semaphore_init() */
ITCM_FUNC_PRIV bool __semaphore_init(uint8_t semaphore_idx, uint32_t semaphore_count)
{
    __enter_critical();

    if (semaphore_idx < ICARUS_MAX_SEMAPHORES && semaphore_count > 0) {
        if (!__semaphore_list[semaphore_idx]->engaged) {
            __semaphore_list[semaphore_idx]->count = semaphore_count;
            __semaphore_list[semaphore_idx]->max_count = semaphore_count;
            __semaphore_list[semaphore_idx]->tick_updated_at = __os_tick_count;
            __semaphore_list[semaphore_idx]->engaged = true;
            __exit_critical();
            return true;
        }
    }

    __exit_critical();
    return false;
}

/* Privileged implementation - called via SVC from semaphore_feed() */
ITCM_FUNC_PRIV bool __semaphore_feed(uint8_t semaphore_idx)
{
    if (semaphore_idx >= ICARUS_MAX_SEMAPHORES ||
        !__semaphore_list[semaphore_idx]->engaged) {
        return false;
    }

    while (__semaphore_list[semaphore_idx]->count >=
           __semaphore_list[semaphore_idx]->max_count) {
        __task_active_sleep(1);  // Call privileged function directly
    }

    __enter_critical();
    ++__semaphore_list[semaphore_idx]->count;
    __semaphore_list[semaphore_idx]->tick_updated_at = __os_tick_count;
    __exit_critical();

    return true;
}

/* Privileged implementation - called via SVC from semaphore_consume() */
ITCM_FUNC_PRIV bool __semaphore_consume(uint8_t semaphore_idx)
{
    if (semaphore_idx >= ICARUS_MAX_SEMAPHORES ||
        !__semaphore_list[semaphore_idx]->engaged) {
        return false;
    }

    while (__semaphore_list[semaphore_idx]->count == 0) {
        __task_active_sleep(1);  // Call privileged function directly
    }

    __enter_critical();
    --__semaphore_list[semaphore_idx]->count;
    __semaphore_list[semaphore_idx]->tick_updated_at = __os_tick_count;
    __exit_critical();

    return true;
}

ITCM_FUNC_PRIV uint32_t __semaphore_get_count(uint8_t semaphore_idx)
{
    if (semaphore_idx >= ICARUS_MAX_SEMAPHORES ||
        !__semaphore_list[semaphore_idx]->engaged) {
        return 0;
    }
    return __semaphore_list[semaphore_idx]->count;
}

ITCM_FUNC_PRIV uint32_t __semaphore_get_max_count(uint8_t semaphore_idx)
{
    if (semaphore_idx >= ICARUS_MAX_SEMAPHORES ||
        !__semaphore_list[semaphore_idx]->engaged) {
        return 0;
    }
    return __semaphore_list[semaphore_idx]->max_count;
}
