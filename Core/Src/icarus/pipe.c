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
 * @note  Internal function - use pipe_init() wrapper from svc.c
 */
bool __pipe_init(uint8_t pipe_idx, uint8_t pipe_capacity_bytes)
{
    __enter_critical();

    if (pipe_idx < ICARUS_MAX_MESSAGE_QUEUES &&
        pipe_capacity_bytes > 0 &&
        pipe_capacity_bytes <= ICARUS_MAX_MESSAGE_BYTES) {

        if (!__message_pipe_list[pipe_idx]->engaged) {
            __message_pipe_list[pipe_idx]->count = 0;
            __message_pipe_list[pipe_idx]->max_count = pipe_capacity_bytes;
            __message_pipe_list[pipe_idx]->enqueue_idx = 0;
            __message_pipe_list[pipe_idx]->dequeue_idx = 0;
            __message_pipe_list[pipe_idx]->tick_updated_at = __os_tick_count;
            __message_pipe_list[pipe_idx]->engaged = true;

            for (uint16_t _i = 0; _i < ICARUS_MAX_MESSAGE_BYTES; _i++) {
                __message_pipe_list[pipe_idx]->buffer[_i] = 0;
            }

            __exit_critical();
            return true;
        }
    }

    __exit_critical();
    return false;
}

/**
 * @brief Privileged implementation of pipe_enqueue
 * @note  Internal function - use pipe_enqueue() wrapper from svc.c
 */
ITCM_FUNC bool __pipe_enqueue(uint8_t pipe_idx, uint8_t* message,
                            uint8_t message_bytes)
{
    if (pipe_idx >= ICARUS_MAX_MESSAGE_QUEUES ||
        !__message_pipe_list[pipe_idx]->engaged ||
        message_bytes > __message_pipe_list[pipe_idx]->max_count) {
        return false;
    }

    while ((__message_pipe_list[pipe_idx]->max_count -
            __message_pipe_list[pipe_idx]->count) < message_bytes) {
        __task_active_sleep(1);  // Call privileged function directly
        if ((__message_pipe_list[pipe_idx]->max_count -
             __message_pipe_list[pipe_idx]->count) >= message_bytes) {
            __scheduler_enabled = false;
        }
    }

    __enter_critical();

    for (uint8_t _i = 0; _i < message_bytes; _i++) {
        __message_pipe_list[pipe_idx]->buffer[
            __message_pipe_list[pipe_idx]->enqueue_idx] = message[_i];
        __message_pipe_list[pipe_idx]->enqueue_idx =
            (uint8_t)(__message_pipe_list[pipe_idx]->enqueue_idx + 1) %
            __message_pipe_list[pipe_idx]->max_count;
        __message_pipe_list[pipe_idx]->count++;
    }

    __message_pipe_list[pipe_idx]->tick_updated_at = __os_tick_count;
    __exit_critical();

    return true;
}

/**
 * @brief Privileged implementation of pipe_dequeue
 * @note  Internal function - use pipe_dequeue() wrapper from svc.c
 */
ITCM_FUNC bool __pipe_dequeue(uint8_t pipe_idx, uint8_t* message,
                            uint8_t message_bytes)
{
    if (pipe_idx >= ICARUS_MAX_MESSAGE_QUEUES ||
        !__message_pipe_list[pipe_idx]->engaged ||
        message_bytes > __message_pipe_list[pipe_idx]->max_count) {
        return false;
    }

    while (__message_pipe_list[pipe_idx]->count < message_bytes) {
        __task_active_sleep(1);  // Call privileged function directly
        if (__message_pipe_list[pipe_idx]->count >= message_bytes) {
            __scheduler_enabled = false;
        }
    }

    __enter_critical();

    for (uint8_t _i = 0; _i < message_bytes; _i++) {
        message[_i] = __message_pipe_list[pipe_idx]->buffer[
            __message_pipe_list[pipe_idx]->dequeue_idx];
        __message_pipe_list[pipe_idx]->dequeue_idx =
            (uint8_t)(__message_pipe_list[pipe_idx]->dequeue_idx + 1) %
            __message_pipe_list[pipe_idx]->max_count;
        __message_pipe_list[pipe_idx]->count--;
    }

    __message_pipe_list[pipe_idx]->tick_updated_at = __os_tick_count;
    __exit_critical();

    return true;
}

uint8_t __pipe_get_count(uint8_t pipe_idx)
{
    if (pipe_idx >= ICARUS_MAX_MESSAGE_QUEUES ||
        !__message_pipe_list[pipe_idx]->engaged) {
        return 0;
    }
    return __message_pipe_list[pipe_idx]->count;
}

uint8_t __pipe_get_max_count(uint8_t pipe_idx)
{
    if (pipe_idx >= ICARUS_MAX_MESSAGE_QUEUES ||
        !__message_pipe_list[pipe_idx]->engaged) {
        return 0;
    }
    return __message_pipe_list[pipe_idx]->max_count;
}
