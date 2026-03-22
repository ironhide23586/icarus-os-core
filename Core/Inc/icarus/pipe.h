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
 * SVC CALL GATES (MPU Protection)
 * ========================================================================= */

/**
 * @brief Check if pipe can accept data (SVC call gate)
 * @param pipe_idx Pipe index
 * @param message_bytes Number of bytes to enqueue
 * @return true if free space >= message_bytes and engaged
 * @note  Uses SVC to read message_pipe_list from DTCM in privileged mode
 */
bool pipe_can_enqueue(uint8_t pipe_idx, uint8_t message_bytes);

/**
 * @brief Check if pipe has data available (SVC call gate)
 * @param pipe_idx Pipe index
 * @param message_bytes Number of bytes to dequeue
 * @return true if count >= message_bytes and engaged
 * @note  Uses SVC to read message_pipe_list from DTCM in privileged mode
 */
bool pipe_can_dequeue(uint8_t pipe_idx, uint8_t message_bytes);

/**
 * @brief Write bytes to pipe (SVC call gate)
 * @param pipe_idx Pipe index
 * @param message Pointer to message data
 * @param message_bytes Number of bytes to write
 * @note  Uses SVC to write message_pipe_list in DTCM from privileged mode
 * @note  Called after pipe_can_enqueue() spin loop exits
 */
void pipe_write_bytes(uint8_t pipe_idx, uint8_t *message, uint8_t message_bytes);

/**
 * @brief Read bytes from pipe (SVC call gate)
 * @param pipe_idx Pipe index
 * @param message Pointer to buffer for message data
 * @param message_bytes Number of bytes to read
 * @note  Uses SVC to read message_pipe_list in DTCM from privileged mode
 * @note  Called after pipe_can_dequeue() spin loop exits
 */
void pipe_read_bytes(uint8_t pipe_idx, uint8_t *message, uint8_t message_bytes);

/* ============================================================================
 * PRIVILEGED IMPLEMENTATIONS (Internal - Do Not Call Directly)
 * ========================================================================= */

bool __pipe_init(uint8_t pipe_idx, uint8_t pipe_capacity_bytes);
bool __pipe_enqueue(uint8_t pipe_idx, uint8_t* message, uint8_t message_bytes);
bool __pipe_dequeue(uint8_t pipe_idx, uint8_t* message, uint8_t message_bytes);
uint8_t __pipe_get_count(uint8_t pipe_idx);
uint8_t __pipe_get_max_count(uint8_t pipe_idx);
bool __pipe_can_enqueue(uint8_t pipe_idx, uint8_t message_bytes);
bool __pipe_can_dequeue(uint8_t pipe_idx, uint8_t message_bytes);
void __pipe_write_bytes(uint8_t pipe_idx, uint8_t *message, uint8_t message_bytes);
void __pipe_read_bytes(uint8_t pipe_idx, uint8_t *message, uint8_t message_bytes);

#ifdef __cplusplus
}
#endif

#endif /* ICARUS_PIPE_H */
