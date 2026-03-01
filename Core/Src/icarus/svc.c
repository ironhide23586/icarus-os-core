/**
 * @file    svc.c
 * @brief   ICARUS Supervisor Call (SVC) Implementation
 * @version 0.1.0
 *
 * @details SVC handler and wrapper functions for MPU-protected kernel calls.
 *
 *          TEMPORARY: All wrappers are direct calls to __func for debugging.
 *          SVC inline assembly and SVC_Handler_C dispatcher are preserved
 *          but disabled behind ENABLE_SVC_DISPATCH (not yet defined).
 *
 * @author  Souham Biswas
 * @date    2025
 *
 * @copyright Copyright 2025 Souham Biswas
 *            https://github.com/ironhide23586/icarus-os-core
 *            Licensed under the Apache License, Version 2.0
 */

#include "icarus/svc.h"
#include "icarus/kernel.h"
#include "icarus/task.h"
#include "icarus/scheduler.h"
#include "icarus/pipe.h"
#include "icarus/semaphore.h"

/* ============================================================================
 * ALL WRAPPERS - Direct calls (SVC dispatch disabled for debugging)
 * ========================================================================= */

void enter_critical(void)              { __enter_critical(); }
void exit_critical(void)               { __exit_critical(); }
void os_init(void)                     { __os_init(); }
void os_start(void)                    { __os_start(); }
void os_yield(void)                    { __os_yield(); }
void os_exit_task(void)                { __os_exit_task(); }
void os_task_suicide(void)             { __os_task_suicide(); }

void* kernel_protected_data(uint16_t num_words) {
    return __kernel_protected_data(num_words);
}

uint32_t* kernel_get_stack(uint8_t task_idx) {
    return __kernel_get_stack(task_idx);
}
uint32_t* kernel_get_data(uint8_t task_idx) {
    return __kernel_get_data(task_idx);
}
uint32_t os_get_tick_count(void) {
    return __os_get_tick_count();
}
const char* os_get_current_task_name(void) {
    return __os_get_current_task_name();
}
uint32_t task_active_sleep(uint32_t ticks) {
    return __task_active_sleep(ticks);
}
uint32_t task_blocking_sleep(uint32_t ticks) {
    return __task_blocking_sleep(ticks);
}
uint32_t task_busy_wait(uint32_t ticks) {
    return __task_busy_wait(ticks);
}
uint8_t os_get_running_task_count(void) {
    return __os_get_running_task_count();
}
uint32_t os_get_task_ticks_remaining(void) {
    return __os_get_task_ticks_remaining();
}
void os_register_task(void (*function)(void), const char *name) {
    __os_register_task(function, name);
}
void os_kill_process(uint8_t task_index) {
    __os_kill_process(task_index);
}
bool pipe_init(uint8_t pipe_idx, uint8_t pipe_capacity_bytes) {
    return __pipe_init(pipe_idx, pipe_capacity_bytes);
}
bool pipe_enqueue(uint8_t pipe_idx, uint8_t* message, uint8_t message_bytes) {
    return __pipe_enqueue(pipe_idx, message, message_bytes);
}
bool pipe_dequeue(uint8_t pipe_idx, uint8_t* message, uint8_t message_bytes) {
    return __pipe_dequeue(pipe_idx, message, message_bytes);
}
uint8_t pipe_get_count(uint8_t pipe_idx) {
    return __pipe_get_count(pipe_idx);
}
uint8_t pipe_get_max_count(uint8_t pipe_idx) {
    return __pipe_get_max_count(pipe_idx);
}
bool semaphore_init(uint8_t semaphore_idx, uint32_t semaphore_count) {
    return __semaphore_init(semaphore_idx, semaphore_count);
}
bool semaphore_feed(uint8_t semaphore_idx) {
    return __semaphore_feed(semaphore_idx);
}
bool semaphore_consume(uint8_t semaphore_idx) {
    return __semaphore_consume(semaphore_idx);
}
uint32_t semaphore_get_count(uint8_t semaphore_idx) {
    return __semaphore_get_count(semaphore_idx);
}
uint32_t semaphore_get_max_count(uint8_t semaphore_idx) {
    return __semaphore_get_max_count(semaphore_idx);
}
