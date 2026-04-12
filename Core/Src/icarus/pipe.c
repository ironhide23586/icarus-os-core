/**
 * @file    pipe.c
 * @brief   ICARUS Message Pipe Implementation
 * @version 0.1.0
 *
 * @details Byte-stream message pipes for inter-task communication.
 *
 * @author  Souham Biswas
 * @date    2025
 *
 * @copyright Copyright 2025-2026 Souham Biswas
 *            https://github.com/ironhide23586/icarus-os-core
 *            Licensed under the Apache License, Version 2.0
 */

#include "icarus/pipe.h"
#include "icarus/kernel.h"
#include "icarus/scheduler.h"

/* ============================================================================
 * SECTION PLACEMENT MACROS
 * ========================================================================= */

/* Defined centrally in icarus/config.h — included via icarus/kernel.h */

/* ============================================================================
 * MESSAGE PIPE IMPLEMENTATION
 * ========================================================================= */

/**
 * @brief Privileged implementation of pipe_init
 * @note  Internal function - use pipe_init() wrapper
 * @note  Runs in SVC handler — already atomic, no critical section needed
 */
ITCM_FUNC bool __pipe_init(uint8_t pipe_idx, uint16_t pipe_capacity_bytes) {
    if ((pipe_idx < (uint8_t)ICARUS_MAX_MESSAGE_QUEUES) &&
        (pipe_capacity_bytes > 0u) &&
        (pipe_capacity_bytes <= (uint16_t)ICARUS_MAX_MESSAGE_BYTES)) {

        if (!message_pipe_list[pipe_idx]->engaged) {
            message_pipe_list[pipe_idx]->count = 0;
            message_pipe_list[pipe_idx]->max_count = pipe_capacity_bytes;
            message_pipe_list[pipe_idx]->enqueue_idx = 0;
            message_pipe_list[pipe_idx]->dequeue_idx = 0;
            message_pipe_list[pipe_idx]->tick_updated_at = os_tick_count;
            message_pipe_list[pipe_idx]->engaged = true;

            for (uint16_t i = 0u; i < (uint16_t)ICARUS_MAX_MESSAGE_BYTES; i++) {
                message_pipe_list[pipe_idx]->buffer[i] = 0;
            }

            return true;
        }
    }

    return false;
}

/**
 * @brief Privileged implementation of pipe_enqueue
 * @note  Internal function - use pipe_enqueue() wrapper
 * @note  Spin-wait uses pipe_can_enqueue() SVC gate — safe with DTCM priv-only
 */
ITCM_FUNC bool __pipe_enqueue(uint8_t pipe_idx, uint8_t *message,
                              uint8_t message_bytes) {
#ifdef HOST_TEST
    /* In host tests, check validity directly (no MPU) */
    if ((pipe_idx >= (uint8_t)ICARUS_MAX_MESSAGE_QUEUES) ||
        (!message_pipe_list[pipe_idx]->engaged) ||
        (message_bytes > message_pipe_list[pipe_idx]->max_count)) {
        return false;
    }
#else
    /* On target, index check only — gate checks engaged+size via SVC */
    if (pipe_idx >= ICARUS_MAX_MESSAGE_QUEUES) {
        return false;
    }
#endif

    /* pipe_can_enqueue checks engaged + free space via SVC (priv-safe) */
    while (!pipe_can_enqueue(pipe_idx, message_bytes)) {
        (void)task_active_sleep(1);
    }

    pipe_write_bytes(pipe_idx, message, message_bytes);
    return true;
}

/**
 * @brief Privileged implementation of pipe_dequeue
 * @note  Internal function - use pipe_dequeue() wrapper
 * @note  Spin-wait uses pipe_can_dequeue() SVC gate — safe with DTCM priv-only
 */
ITCM_FUNC bool __pipe_dequeue(uint8_t pipe_idx, uint8_t *message,
                              uint8_t message_bytes) {
#ifdef HOST_TEST
    /* In host tests, check validity directly (no MPU) */
    if ((pipe_idx >= (uint8_t)ICARUS_MAX_MESSAGE_QUEUES) ||
        (!message_pipe_list[pipe_idx]->engaged) ||
        (message_bytes > message_pipe_list[pipe_idx]->max_count)) {
        return false;
    }
#else
    /* On target, index check only — gate checks engaged+size via SVC */
    if (pipe_idx >= ICARUS_MAX_MESSAGE_QUEUES) {
        return false;
    }
#endif

    /* pipe_can_dequeue checks engaged + available bytes via SVC (priv-safe) */
    while (!pipe_can_dequeue(pipe_idx, message_bytes)) {
        (void)task_active_sleep(1);
    }

    pipe_read_bytes(pipe_idx, message, message_bytes);
    return true;
}

/**
 * @brief Privileged implementation of pipe_get_count
 * @note  Internal function - use pipe_get_count() wrapper
 */
ITCM_FUNC uint16_t __pipe_get_count(uint8_t pipe_idx) {
    if ((pipe_idx >= (uint8_t)ICARUS_MAX_MESSAGE_QUEUES) ||
        (!message_pipe_list[pipe_idx]->engaged)) {
        return 0;
    }
    return message_pipe_list[pipe_idx]->count;
}

/**
 * @brief Privileged implementation of pipe_get_max_count
 * @note  Internal function - use pipe_get_max_count() wrapper
 */
ITCM_FUNC uint16_t __pipe_get_max_count(uint8_t pipe_idx) {
    if ((pipe_idx >= (uint8_t)ICARUS_MAX_MESSAGE_QUEUES) ||
        (!message_pipe_list[pipe_idx]->engaged)) {
        return 0;
    }
    return message_pipe_list[pipe_idx]->max_count;
}

/**
 * @brief Privileged call gate: can pipe accept message_bytes bytes?
 * @note  Returns true if engaged and free space >= message_bytes
 *        Used by __pipe_enqueue spin loop to avoid direct DTCM reads
 *        from unprivileged thread mode once DTCM is priv-only.
 */
ITCM_FUNC bool __pipe_can_enqueue(uint8_t pipe_idx, uint8_t message_bytes) {
    if ((pipe_idx >= (uint8_t)ICARUS_MAX_MESSAGE_QUEUES) ||
        (!message_pipe_list[pipe_idx]->engaged)) {
        return false;
    }
    return (message_pipe_list[pipe_idx]->max_count -
            message_pipe_list[pipe_idx]->count) >= message_bytes;
}

/**
 * @brief Privileged call gate: can pipe deliver message_bytes bytes?
 * @note  Returns true if engaged and count >= message_bytes
 *        Used by __pipe_dequeue spin loop.
 */
ITCM_FUNC bool __pipe_can_dequeue(uint8_t pipe_idx, uint8_t message_bytes) {
    if ((pipe_idx >= (uint8_t)ICARUS_MAX_MESSAGE_QUEUES) ||
        (!message_pipe_list[pipe_idx]->engaged)) {
        return false;
    }
    return message_pipe_list[pipe_idx]->count >= message_bytes;
}

/**
 * @brief Privileged write gate: write bytes to pipe buffer
 * @note  Called after pipe_can_enqueue() spin loop exits
 *        Runs in privileged SVC handler — already atomic, no critical section needed
 */
ITCM_FUNC void __pipe_write_bytes(uint8_t pipe_idx, uint8_t *message, uint8_t message_bytes) {
    if ((pipe_idx >= (uint8_t)ICARUS_MAX_MESSAGE_QUEUES) ||
        (!message_pipe_list[pipe_idx]->engaged)) {
        return;
    }

    for (uint8_t i = 0; i < message_bytes; i++) {
        message_pipe_list[pipe_idx]->buffer[
            message_pipe_list[pipe_idx]->enqueue_idx] = message[i];
        message_pipe_list[pipe_idx]->enqueue_idx =
            (uint16_t)(((uint16_t)(message_pipe_list[pipe_idx]->enqueue_idx + 1u)) %
            message_pipe_list[pipe_idx]->max_count);
        message_pipe_list[pipe_idx]->count++;
    }

    message_pipe_list[pipe_idx]->tick_updated_at = os_tick_count;
}

/**
 * @brief Privileged write gate: read bytes from pipe buffer
 * @note  Called after pipe_can_dequeue() spin loop exits
 *        Runs in privileged SVC handler — already atomic, no critical section needed
 */
ITCM_FUNC void __pipe_read_bytes(uint8_t pipe_idx, uint8_t *message, uint8_t message_bytes) {
    if ((pipe_idx >= (uint8_t)ICARUS_MAX_MESSAGE_QUEUES) ||
        (!message_pipe_list[pipe_idx]->engaged)) {
        return;
    }

    for (uint8_t i = 0; i < message_bytes; i++) {
        message[i] = message_pipe_list[pipe_idx]->buffer[
            message_pipe_list[pipe_idx]->dequeue_idx];
        message_pipe_list[pipe_idx]->dequeue_idx =
            (uint16_t)(((uint16_t)(message_pipe_list[pipe_idx]->dequeue_idx + 1u)) %
            message_pipe_list[pipe_idx]->max_count);
        message_pipe_list[pipe_idx]->count--;
    }

    message_pipe_list[pipe_idx]->tick_updated_at = os_tick_count;
}
