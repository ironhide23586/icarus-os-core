/**
 * @file    kernel.c
 * @brief   ICARUS Kernel Core Implementation
 * @version 0.1.0
 *
 * @details Core kernel state, data structures, and initialization.
 *
 * @author  Souham Biswas
 * @date    2025
 *
 * @copyright Copyright 2025 Souham Biswas
 *            https://github.com/ironhide23586/icarus-os-core
 *            Licensed under the Apache License, Version 2.0
 */

#include "icarus/kernel.h"
#include "icarus/task.h"
#include "icarus/scheduler.h"
#include "icarus/svc.h"
#include "bsp/display.h"
#include "bsp/led.h"

#include <string.h>

/* ============================================================================
 * SECTION PLACEMENT MACROS
 * ========================================================================= */

#ifndef HOST_TEST
#define ITCM_FUNC __attribute__((section(".itcm")))
#define DTCM_DATA __attribute__((section(".dtcm")))
#else
#define ITCM_FUNC
#define DTCM_DATA
#endif

/* ============================================================================
 * COMPILE-TIME STRUCTURE VALIDATION
 * ========================================================================= */

#ifndef SKIP_STATIC_ASSERTS
#include <stddef.h>
_Static_assert(offsetof(icarus_task_t, stack_pointer) == 12,
               "TCB stack_pointer offset mismatch - update context_switch.s");
_Static_assert(offsetof(icarus_task_t, task_state) == 16,
               "TCB task_state offset mismatch - update context_switch.s");
_Static_assert(offsetof(icarus_task_t, global_tick_paused) == 20,
               "TCB global_tick_paused offset mismatch - update context_switch.s");
_Static_assert(offsetof(icarus_task_t, ticks_to_pause) == 24,
               "TCB ticks_to_pause offset mismatch - update context_switch.s");
#endif

/* ============================================================================
 * KERNEL DATA STRUCTURES (DTCM - Zero Wait State)
 * ========================================================================= */

DTCM_DATA icarus_task_t* __task_list[ICARUS_MAX_TASKS];
DTCM_DATA icarus_semaphore_t* __semaphore_list[ICARUS_MAX_SEMAPHORES];
DTCM_DATA icarus_pipe_t* __message_pipe_list[ICARUS_MAX_MESSAGE_QUEUES];

DTCM_DATA static icarus_task_t __task_pool[ICARUS_MAX_TASKS];
DTCM_DATA static icarus_semaphore_t __semaphore_pool[ICARUS_MAX_SEMAPHORES];
DTCM_DATA static icarus_pipe_t __message_pipe_pool[ICARUS_MAX_MESSAGE_QUEUES];
DTCM_DATA int8_t __cleanup_task_idx[ICARUS_MAX_TASKS];

/* ============================================================================
 * SCHEDULER STATE (DTCM - Zero Wait State)
 * ========================================================================= */

DTCM_DATA uint8_t __current_task_index;
DTCM_DATA uint8_t __running_task_count;
DTCM_DATA uint8_t __num_created_tasks;
DTCM_DATA volatile uint32_t __current_task_ticks_remaining;
DTCM_DATA volatile uint32_t __ticks_per_task;
DTCM_DATA uint32_t __cpu_vregisters[16];
DTCM_DATA volatile uint32_t __os_tick_count;
DTCM_DATA volatile uint8_t __os_running;
DTCM_DATA volatile uint8_t __critical_stack_depth;
DTCM_DATA volatile bool __scheduler_enabled;
DTCM_DATA int8_t __current_cleanup_task_idx;

/* ============================================================================
 * STACK POOL (RAM_D1)
 * ========================================================================= */

static uint32_t __stack_pool[ICARUS_MAX_TASKS][ICARUS_STACK_WORDS];


/* ============================================================================
 * DATA POOL (RAM_D1)
 * ========================================================================= */

static uint32_t __data_pool[ICARUS_MAX_TASKS][ICARUS_DATA_WORDS];
static uint16_t __data_pool_word_offsets[ICARUS_MAX_TASKS];

/* ============================================================================
 * EXTERNAL ASSEMBLY FUNCTIONS
 * ========================================================================= */

extern void __start_cold_task(icarus_task_t *task);

/* ============================================================================
 * CRITICAL SECTION MANAGEMENT
 * ========================================================================= */

/* Privileged implementation - called via SVC from enter_critical() */
void __enter_critical(void) {
    __scheduler_enabled = false;
    __critical_stack_depth++;
}

/* Privileged implementation - called via SVC from exit_critical() */
void __exit_critical(void) {
    if (--__critical_stack_depth == 0) {
        __scheduler_enabled = true;
    }
}

/* ============================================================================
 * INTERNAL HELPER FUNCTIONS
 * ========================================================================= */

static inline void __init_sem(uint8_t semaphore_idx, uint32_t semaphore_count,
                              bool should_engage) {
    __semaphore_list[semaphore_idx]->count = semaphore_count;
    __semaphore_list[semaphore_idx]->max_count = semaphore_count;
    __semaphore_list[semaphore_idx]->tick_updated_at = __os_tick_count;
    __semaphore_list[semaphore_idx]->engaged = should_engage;
}

static inline void __init_pipe(uint8_t message_pipe_idx, uint8_t max_messages,
                               bool should_engage) {
    __message_pipe_list[message_pipe_idx]->count = 0;
    __message_pipe_list[message_pipe_idx]->max_count = max_messages;
    __message_pipe_list[message_pipe_idx]->enqueue_idx = 0;
    __message_pipe_list[message_pipe_idx]->dequeue_idx = 0;
    __message_pipe_list[message_pipe_idx]->tick_updated_at = __os_tick_count;
    __message_pipe_list[message_pipe_idx]->engaged = should_engage;

    for (uint16_t _i = 0; _i < ICARUS_MAX_MESSAGE_BYTES; _i++) {
        __message_pipe_list[message_pipe_idx]->buffer[_i] = 0;
    }
}

/* ============================================================================
 * SYSTEM TASKS
 * ========================================================================= */

static void os_idle_task(void)
{
    display_init();
    while (1) {
        __os_yield();  // Call privileged function directly
    }
}

static void os_heartbeat_task(void)
{
#if ICARUS_ENABLE_HEARTBEAT_VIS
    const char* _task_name = __os_get_current_task_name();  // Call privileged function directly
#endif

    while (1) {
        if (__os_running) {
#if ICARUS_ENABLE_HEARTBEAT_VIS
            display_render_banner(ROW_HEARTBEAT, _task_name, true);
            __task_active_sleep(8);  // Call privileged function directly
            LED_On();
            __task_active_sleep(ICARUS_HEARTBEAT_ON_TICKS - 1);
            display_render_banner(ROW_HEARTBEAT, _task_name, false);
            __task_active_sleep(8);
            LED_Off();
            __task_active_sleep(ICARUS_HEARTBEAT_OFF_TICKS - 1);
#else
            LED_Blink(ICARUS_HEARTBEAT_ON_TICKS, ICARUS_HEARTBEAT_OFF_TICKS);
#endif
        } else {
#if ICARUS_ENABLE_HEARTBEAT_VIS
            display_render_banner(ROW_HEARTBEAT, _task_name, false);
#endif
            __task_active_sleep(100);
        }
    }
}

/* ============================================================================
 * KERNEL INITIALIZATION
 * ========================================================================= */

/* Privileged implementation - called via SVC from os_init() */
void __os_init(void) {
    hal_init();
    uint8_t _i;

    __os_running = 0;
    __ticks_per_task = ICARUS_TICKS_PER_TASK;
    __running_task_count = 0;
    __current_task_index = 0;
    __current_cleanup_task_idx = -1;
    __current_task_ticks_remaining = __ticks_per_task;

    for (_i = 0; _i < ICARUS_MAX_TASKS; _i++) {
        __task_list[_i] = &__task_pool[_i];
        __cleanup_task_idx[_i] = -1;
        __data_pool_word_offsets[_i] = 0;
    }

    for (_i = 0; _i < ICARUS_MAX_SEMAPHORES; _i++) {
        __semaphore_list[_i] = &__semaphore_pool[_i];
        __init_sem(_i, 0, false);
    }

    for (_i = 0; _i < ICARUS_MAX_MESSAGE_QUEUES; _i++) {
        __message_pipe_list[_i] = &__message_pipe_pool[_i];
        __init_pipe(_i, 0, false);
    }

    for (_i = 0; _i < 16; _i++) {
        __cpu_vregisters[_i] = 0;
    }

    __os_register_task(os_idle_task, "ICARUS_KEEPALIVE_TASK");
    __os_register_task(os_heartbeat_task, ">ICARUS_HEARTBEAT<");

    __scheduler_enabled = true;
}

/* Privileged implementation - called via SVC from os_start() */
void __os_start(void) {
    if ((__num_created_tasks == 0) || (__num_created_tasks > ICARUS_MAX_TASKS)) {
        return;
    }
    
    __start_cold_task(__task_list[__current_task_index]);
}

/* ============================================================================
 * STACK POOL ACCESS (for task.c)
 * ========================================================================= */

uint32_t* __kernel_get_stack(uint8_t task_idx) {
    return __stack_pool[task_idx];
}


/* ============================================================================
 * DATA POOL ACCESS (for task.c)
 * ========================================================================= */

uint32_t* __kernel_get_data(uint8_t task_idx) {
    return __data_pool[task_idx];
}

/* Privileged implementation - called via SVC from kernel_protected_data() */
void* __kernel_protected_data(uint16_t num_words) {
    if ((__data_pool_word_offsets[__current_task_index] + num_words > ICARUS_DATA_WORDS) ||
        (num_words == 0)) {
        return NULL;
    }
    
    __enter_critical();
    uint16_t _current_offset = __data_pool_word_offsets[__current_task_index];
    __data_pool_word_offsets[__current_task_index] += num_words;
    uint32_t *_ret_ptr = &__data_pool[__current_task_index][_current_offset];
    __exit_critical();
    
    return (void*) _ret_ptr;
}
