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

/**
 * @brief Privileged implementation of semaphore_init
 * @note  Internal function - use semaphore_init() wrapper
 */
bool __semaphore_init(uint8_t semaphore_idx, uint32_t semaphore_count)
{
    __enter_critical();

    if (semaphore_idx < ICARUS_MAX_SEMAPHORES && semaphore_count > 0) {
        if (!semaphore_list[semaphore_idx]->engaged) {
            semaphore_list[semaphore_idx]->count = semaphore_count;
            semaphore_list[semaphore_idx]->max_count = semaphore_count;
            semaphore_list[semaphore_idx]->tick_updated_at = os_tick_count;
            semaphore_list[semaphore_idx]->engaged = true;
            __exit_critical();
            return true;
        }
    }

    __exit_critical();
    return false;
}

/**
 * @brief Public API for initializing a semaphore
 * @note  Will become SVC wrapper in privileged mode
 */
bool semaphore_init(uint8_t semaphore_idx, uint32_t semaphore_count)
{
    return __semaphore_init(semaphore_idx, semaphore_count);
}

/**
 * @brief Privileged implementation of semaphore_feed
 * @note  Internal function - use semaphore_feed() wrapper
 */
ITCM_FUNC bool __semaphore_feed(uint8_t semaphore_idx)
{
    if (semaphore_idx >= ICARUS_MAX_SEMAPHORES ||
        !semaphore_list[semaphore_idx]->engaged) {
        return false;
    }

    while (semaphore_list[semaphore_idx]->count >=
           semaphore_list[semaphore_idx]->max_count) {
        task_active_sleep(1);
    }

    __enter_critical();
    ++semaphore_list[semaphore_idx]->count;
    semaphore_list[semaphore_idx]->tick_updated_at = os_tick_count;
    __exit_critical();

    return true;
}

/**
 * @brief Public API for feeding (incrementing) a semaphore
 * @note  Will become SVC wrapper in privileged mode
 */
bool semaphore_feed(uint8_t semaphore_idx)
{
    return __semaphore_feed(semaphore_idx);
}

/**
 * @brief Privileged implementation of semaphore_consume
 * @note  Internal function - use semaphore_consume() wrapper
 */
ITCM_FUNC bool __semaphore_consume(uint8_t semaphore_idx)
{
    if (semaphore_idx >= ICARUS_MAX_SEMAPHORES ||
        !semaphore_list[semaphore_idx]->engaged) {
        return false;
    }

    while (semaphore_list[semaphore_idx]->count == 0) {
        task_active_sleep(1);
    }

    __enter_critical();
    --semaphore_list[semaphore_idx]->count;
    semaphore_list[semaphore_idx]->tick_updated_at = os_tick_count;
    __exit_critical();

    return true;
}

/**
 * @brief Public API for consuming (decrementing) a semaphore
 * @note  Will become SVC wrapper in privileged mode
 */
bool semaphore_consume(uint8_t semaphore_idx)
{
    return __semaphore_consume(semaphore_idx);
}

/**
 * @brief Privileged implementation of semaphore_get_count
 * @note  Internal function - use semaphore_get_count() wrapper
 */
uint32_t __semaphore_get_count(uint8_t semaphore_idx)
{
    if (semaphore_idx >= ICARUS_MAX_SEMAPHORES ||
        !semaphore_list[semaphore_idx]->engaged) {
        return 0;
    }
    return semaphore_list[semaphore_idx]->count;
}

/**
 * @brief Public API for getting semaphore count
 * @note  Will become SVC wrapper in privileged mode
 */
uint32_t semaphore_get_count(uint8_t semaphore_idx)
{
    return __semaphore_get_count(semaphore_idx);
}

/**
 * @brief Privileged implementation of semaphore_get_max_count
 * @note  Internal function - use semaphore_get_max_count() wrapper
 */
uint32_t __semaphore_get_max_count(uint8_t semaphore_idx)
{
    if (semaphore_idx >= ICARUS_MAX_SEMAPHORES ||
        !semaphore_list[semaphore_idx]->engaged) {
        return 0;
    }
    return semaphore_list[semaphore_idx]->max_count;
}

/**
 * @brief Public API for getting semaphore max count
 * @note  Will become SVC wrapper in privileged mode
 */
uint32_t semaphore_get_max_count(uint8_t semaphore_idx)
{
    return __semaphore_get_max_count(semaphore_idx);
}
