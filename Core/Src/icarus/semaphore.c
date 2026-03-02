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

/* Defined centrally in icarus/config.h — included via icarus/kernel.h */

/* ============================================================================
 * SEMAPHORE IMPLEMENTATION
 * ========================================================================= */

/**
 * @brief Privileged implementation of semaphore_init
 * @note  Internal function - use semaphore_init() wrapper
 */
bool __semaphore_init(uint8_t semaphore_idx, uint32_t semaphore_count) {
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
 * @brief Privileged implementation of semaphore_feed
 * @note  Internal function - use semaphore_feed() wrapper
 * @note  Spin-wait uses sem_can_feed() SVC gate — safe with DTCM priv-only
 */
ITCM_FUNC bool __semaphore_feed(uint8_t semaphore_idx) {
#ifdef HOST_TEST
    /* In host tests, check validity directly (no MPU) */
    if (semaphore_idx >= ICARUS_MAX_SEMAPHORES ||
        !semaphore_list[semaphore_idx]->engaged) {
        return false;
    }
#else
    /* On target, index check only — gate checks engaged via SVC */
    if (semaphore_idx >= ICARUS_MAX_SEMAPHORES) {
        return false;
    }
#endif

    while (!sem_can_feed(semaphore_idx)) {
        task_active_sleep(1);
    }

    enter_critical();
    ++semaphore_list[semaphore_idx]->count;
    semaphore_list[semaphore_idx]->tick_updated_at = os_tick_count;
    exit_critical();

    return true;
}

/**
 * @brief Privileged implementation of semaphore_consume
 * @note  Internal function - use semaphore_consume() wrapper
 * @note  Spin-wait uses sem_can_consume() SVC gate — safe with DTCM priv-only
 */
ITCM_FUNC bool __semaphore_consume(uint8_t semaphore_idx) {
#ifdef HOST_TEST
    /* In host tests, check validity directly (no MPU) */
    if (semaphore_idx >= ICARUS_MAX_SEMAPHORES ||
        !semaphore_list[semaphore_idx]->engaged) {
        return false;
    }
#else
    /* On target, index check only — gate checks engaged via SVC */
    if (semaphore_idx >= ICARUS_MAX_SEMAPHORES) {
        return false;
    }
#endif

    while (!sem_can_consume(semaphore_idx)) {
        task_active_sleep(1);
    }

    enter_critical();
    --semaphore_list[semaphore_idx]->count;
    semaphore_list[semaphore_idx]->tick_updated_at = os_tick_count;
    exit_critical();

    return true;
}

/**
 * @brief Privileged implementation of semaphore_get_count
 * @note  Internal function - use semaphore_get_count() wrapper
 */
uint32_t __semaphore_get_count(uint8_t semaphore_idx) {
    if (semaphore_idx >= ICARUS_MAX_SEMAPHORES ||
        !semaphore_list[semaphore_idx]->engaged) {
        return 0;
    }
    return semaphore_list[semaphore_idx]->count;
}

/**
 * @brief Privileged implementation of semaphore_get_max_count
 * @note  Internal function - use semaphore_get_max_count() wrapper
 */
uint32_t __semaphore_get_max_count(uint8_t semaphore_idx) {
    if (semaphore_idx >= ICARUS_MAX_SEMAPHORES ||
        !semaphore_list[semaphore_idx]->engaged) {
        return 0;
    }
    return semaphore_list[semaphore_idx]->max_count;
}

/**
 * @brief Privileged call gate: can semaphore accept a feed?
 * @note  Returns true if engaged and count < max_count
 *        Used by __semaphore_feed spin loop to avoid direct DTCM reads
 *        from unprivileged thread mode once DTCM is priv-only.
 */
bool __sem_can_feed(uint8_t semaphore_idx) {
    if (semaphore_idx >= ICARUS_MAX_SEMAPHORES ||
        !semaphore_list[semaphore_idx]->engaged) {
        return false;
    }
    return semaphore_list[semaphore_idx]->count <
           semaphore_list[semaphore_idx]->max_count;
}

/**
 * @brief Privileged call gate: can semaphore be consumed?
 * @note  Returns true if engaged and count > 0
 *        Used by __semaphore_consume spin loop.
 */
bool __sem_can_consume(uint8_t semaphore_idx) {
    if (semaphore_idx >= ICARUS_MAX_SEMAPHORES ||
        !semaphore_list[semaphore_idx]->engaged) {
        return false;
    }
    return semaphore_list[semaphore_idx]->count > 0;
}
