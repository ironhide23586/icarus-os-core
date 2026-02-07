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
 * @copyright Copyright 2025 Souham Biswas
 *            https://github.com/ironhide23586/icarus-os-core
 *            Licensed under the Apache License, Version 2.0
 */

#include "icarus/pipe.h"
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
 * MESSAGE PIPE IMPLEMENTATION
 * ========================================================================= */

/**
 * @brief Privileged implementation of pipe_init
 * @note  Internal function - use pipe_init() wrapper
 */
bool __pipe_init(uint8_t pipe_idx, uint8_t pipe_capacity_bytes)
{
    __enter_critical();

    if (pipe_idx < ICARUS_MAX_MESSAGE_QUEUES &&
        pipe_capacity_bytes > 0 &&
        pipe_capacity_bytes <= ICARUS_MAX_MESSAGE_BYTES) {

        if (!message_pipe_list[pipe_idx]->engaged) {
            message_pipe_list[pipe_idx]->count = 0;
            message_pipe_list[pipe_idx]->max_count = pipe_capacity_bytes;
            message_pipe_list[pipe_idx]->enqueue_idx = 0;
            message_pipe_list[pipe_idx]->dequeue_idx = 0;
            message_pipe_list[pipe_idx]->tick_updated_at = os_tick_count;
            message_pipe_list[pipe_idx]->engaged = true;

            for (uint16_t i = 0; i < ICARUS_MAX_MESSAGE_BYTES; i++) {
                message_pipe_list[pipe_idx]->buffer[i] = 0;
            }

            __exit_critical();
            return true;
        }
    }

    __exit_critical();
    return false;
}

/**
 * @brief Public API for initializing a message pipe
 * @note  Will become SVC wrapper in privileged mode
 */
bool pipe_init(uint8_t pipe_idx, uint8_t pipe_capacity_bytes)
{
#ifdef HOST_TEST
    return __pipe_init(pipe_idx, pipe_capacity_bytes);
#else
    bool res;
    __asm__ volatile (
        "mov r0, %1\n"
        "mov r1, %2\n"
        "svc %3\n"
        "mov %0, r0\n"
        : "=r" (res)
        : "r" (pipe_idx), "r" (pipe_capacity_bytes), "I" (SVC_PIPE_INIT)
        : "r0", "r1"
    );
    return res;
#endif
}

/**
 * @brief Privileged implementation of pipe_enqueue
 * @note  Internal function - use pipe_enqueue() wrapper
 */
ITCM_FUNC bool __pipe_enqueue(uint8_t pipe_idx, uint8_t* message,
                            uint8_t message_bytes)
{
    if (pipe_idx >= ICARUS_MAX_MESSAGE_QUEUES ||
        !message_pipe_list[pipe_idx]->engaged ||
        message_bytes > message_pipe_list[pipe_idx]->max_count) {
        return false;
    }

    while ((message_pipe_list[pipe_idx]->max_count -
            message_pipe_list[pipe_idx]->count) < message_bytes) {
        __task_active_sleep(1);  // Call privileged function directly
        if ((message_pipe_list[pipe_idx]->max_count -
             message_pipe_list[pipe_idx]->count) >= message_bytes) {
            scheduler_enabled = false;
        }
    }

    __enter_critical();

    for (uint8_t i = 0; i < message_bytes; i++) {
        message_pipe_list[pipe_idx]->buffer[
            message_pipe_list[pipe_idx]->enqueue_idx] = message[i];
        message_pipe_list[pipe_idx]->enqueue_idx =
            (uint8_t)(message_pipe_list[pipe_idx]->enqueue_idx + 1) %
            message_pipe_list[pipe_idx]->max_count;
        message_pipe_list[pipe_idx]->count++;
    }

    message_pipe_list[pipe_idx]->tick_updated_at = os_tick_count;
    __exit_critical();

    return true;
}

/**
 * @brief Public API for enqueueing data to a message pipe
 * @note  Will become SVC wrapper in privileged mode
 */
bool pipe_enqueue(uint8_t pipe_idx, uint8_t* message,
                            uint8_t message_bytes)
{
#ifdef HOST_TEST
    return __pipe_enqueue(pipe_idx, message, message_bytes);
#else
    bool res;
    __asm__ volatile (
        "mov r0, %1\n"
        "mov r1, %2\n"
        "mov r2, %3\n"
        "svc %4\n"
        "mov %0, r0\n"
        : "=r" (res)
        : "r" (pipe_idx), "r" (message), "r" (message_bytes), "I" (SVC_PIPE_ENQUEUE)
        : "r0", "r1", "r2"
    );
    return res;
#endif
}

/**
 * @brief Privileged implementation of pipe_dequeue
 * @note  Internal function - use pipe_dequeue() wrapper
 */
ITCM_FUNC bool __pipe_dequeue(uint8_t pipe_idx, uint8_t* message,
                            uint8_t message_bytes)
{
    if (pipe_idx >= ICARUS_MAX_MESSAGE_QUEUES ||
        !message_pipe_list[pipe_idx]->engaged ||
        message_bytes > message_pipe_list[pipe_idx]->max_count) {
        return false;
    }

    while (message_pipe_list[pipe_idx]->count < message_bytes) {
        __task_active_sleep(1);  // Call privileged function directly
        if (message_pipe_list[pipe_idx]->count >= message_bytes) {
            scheduler_enabled = false;
        }
    }

    __enter_critical();

    for (uint8_t i = 0; i < message_bytes; i++) {
        message[i] = message_pipe_list[pipe_idx]->buffer[
            message_pipe_list[pipe_idx]->dequeue_idx];
        message_pipe_list[pipe_idx]->dequeue_idx =
            (uint8_t)(message_pipe_list[pipe_idx]->dequeue_idx + 1) %
            message_pipe_list[pipe_idx]->max_count;
        message_pipe_list[pipe_idx]->count--;
    }

    message_pipe_list[pipe_idx]->tick_updated_at = os_tick_count;
    __exit_critical();

    return true;
}

/**
 * @brief Public API for dequeueing data from a message pipe
 * @note  Will become SVC wrapper in privileged mode
 */
bool pipe_dequeue(uint8_t pipe_idx, uint8_t* message,
                            uint8_t message_bytes)
{
#ifdef HOST_TEST
    return __pipe_dequeue(pipe_idx, message, message_bytes);
#else
    bool res;
    __asm__ volatile (
        "mov r0, %1\n"
        "mov r1, %2\n"
        "mov r2, %3\n"
        "svc %4\n"
        "mov %0, r0\n"
        : "=r" (res)
        : "r" (pipe_idx), "r" (message), "r" (message_bytes), "I" (SVC_PIPE_DEQUEUE)
        : "r0", "r1", "r2"
    );
    return res;
#endif
}

uint8_t pipe_get_count(uint8_t pipe_idx)
{
    if (pipe_idx >= ICARUS_MAX_MESSAGE_QUEUES ||
        !message_pipe_list[pipe_idx]->engaged) {
        return 0;
    }
    return message_pipe_list[pipe_idx]->count;
}

uint8_t pipe_get_max_count(uint8_t pipe_idx)
{
    if (pipe_idx >= ICARUS_MAX_MESSAGE_QUEUES ||
        !message_pipe_list[pipe_idx]->engaged) {
        return 0;
    }
    return message_pipe_list[pipe_idx]->max_count;
}
