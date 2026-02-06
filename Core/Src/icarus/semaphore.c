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
