/**
 * @file    semaphore.h
 * @brief   ICARUS Semaphore - Counting Semaphores
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

#ifndef ICARUS_SEMAPHORE_H
#define ICARUS_SEMAPHORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/* ============================================================================
 * SEMAPHORE API
 * ========================================================================= */

/**
 * @brief Initialize a counting semaphore
 * @param semaphore_idx   Semaphore index (0 to ICARUS_MAX_SEMAPHORES-1)
 * @param semaphore_count Initial and maximum count
 * @return true on success, false on error
 */
bool semaphore_init(uint8_t semaphore_idx, uint32_t semaphore_count);

/**
 * @brief Increment semaphore count (V operation / signal)
 * @param semaphore_idx Semaphore index
 * @return true on success
 * @note  Blocks if count == max_count
 * @note  Placed in ITCM for zero wait-state execution
 */
bool semaphore_feed(uint8_t semaphore_idx);

/**
 * @brief Decrement semaphore count (P operation / wait)
 * @param semaphore_idx Semaphore index
 * @return true on success
 * @note  Blocks if count == 0
 * @note  Placed in ITCM for zero wait-state execution
 */
bool semaphore_consume(uint8_t semaphore_idx);

/**
 * @brief Get current semaphore count
 * @param semaphore_idx Semaphore index
 * @return Current count, or 0 if invalid
 */
uint32_t semaphore_get_count(uint8_t semaphore_idx);

/**
 * @brief Get semaphore maximum count
 * @param semaphore_idx Semaphore index
 * @return Maximum count, or 0 if invalid
 */
uint32_t semaphore_get_max_count(uint8_t semaphore_idx);

/* ============================================================================
 * PRIVILEGED IMPLEMENTATIONS (Internal - Do Not Call Directly)
 * ========================================================================= */

bool __semaphore_init(uint8_t semaphore_idx, uint32_t semaphore_count);
bool __semaphore_feed(uint8_t semaphore_idx);
bool __semaphore_consume(uint8_t semaphore_idx);

#ifdef __cplusplus
}
#endif

#endif /* ICARUS_SEMAPHORE_H */
