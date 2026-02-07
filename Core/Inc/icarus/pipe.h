/**
 * @file    pipe.h
 * @brief   ICARUS Pipe - Message Pipes (Circular Buffer Queues)
 * @version 0.1.0
 *
 * @details Byte-stream message pipes for inter-task communication.
 *
 * @author  Souham Biswas
 * @date    2025
 *
 * @copyright Copyright 2025 Souham Biswas
 *            https://github.com/ironhide23586/icarus-os-core
 *            Licensed under the Apache License, Version 2.0
 */

#ifndef ICARUS_PIPE_H
#define ICARUS_PIPE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "icarus/svc.h"
#include <stdint.h>
#include <stdbool.h>

/* ============================================================================
 * MESSAGE PIPE API
 * ========================================================================= */

/**
 * @brief Initialize a message pipe
 * @param pipe_idx            Pipe index (0 to ICARUS_MAX_MESSAGE_QUEUES-1)
 * @param pipe_capacity_bytes Maximum bytes the pipe can hold
 * @return true on success, false on error
 */
bool pipe_init(uint8_t pipe_idx, uint8_t pipe_capacity_bytes);

/**
 * @brief Enqueue bytes to pipe
 * @param pipe_idx      Pipe index
 * @param message       Data to enqueue
 * @param message_bytes Number of bytes
 * @return true on success
 * @note  Blocks if insufficient space
 * @note  Placed in ITCM for zero wait-state execution
 */
bool pipe_enqueue(uint8_t pipe_idx, uint8_t* message, uint8_t message_bytes);

/**
 * @brief Dequeue bytes from pipe
 * @param pipe_idx      Pipe index
 * @param message       Buffer to receive data
 * @param message_bytes Number of bytes to read
 * @return true on success
 * @note  Blocks if insufficient data
 * @note  Placed in ITCM for zero wait-state execution
 */
bool pipe_dequeue(uint8_t pipe_idx, uint8_t* message, uint8_t message_bytes);

/**
 * @brief Get current byte count in pipe
 * @param pipe_idx Pipe index
 * @return Current count, or 0 if invalid
 */
uint8_t pipe_get_count(uint8_t pipe_idx);

/**
 * @brief Get pipe capacity
 * @param pipe_idx Pipe index
 * @return Maximum capacity, or 0 if invalid
 */
uint8_t pipe_get_max_count(uint8_t pipe_idx);

/* ============================================================================
 * PRIVILEGED IMPLEMENTATIONS (Internal - Do Not Call Directly)
 * ========================================================================= */

bool __pipe_init(uint8_t pipe_idx, uint8_t pipe_capacity_bytes);
bool __pipe_enqueue(uint8_t pipe_idx, uint8_t* message, uint8_t message_bytes);
bool __pipe_dequeue(uint8_t pipe_idx, uint8_t* message, uint8_t message_bytes);

#ifdef __cplusplus
}
#endif

#endif /* ICARUS_PIPE_H */
