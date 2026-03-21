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
#include "bsp/display.h"
#include "bsp/led.h"

#include <string.h>

/* ============================================================================
 * SECTION PLACEMENT MACROS
 * ========================================================================= */

/* Defined centrally in icarus/config.h — included via icarus/kernel.h */

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
 * KERNEL DATA STRUCTURES (DTCM_PRIV - privileged-only, zero wait state)
 * ========================================================================= */

DTCM_DATA_PRIV icarus_task_t* task_list[ICARUS_MAX_TASKS];
DTCM_DATA_PRIV icarus_semaphore_t* semaphore_list[ICARUS_MAX_SEMAPHORES];
DTCM_DATA_PRIV icarus_pipe_t* message_pipe_list[ICARUS_MAX_MESSAGE_QUEUES];

DTCM_DATA_PRIV static icarus_task_t task_pool[ICARUS_MAX_TASKS];
DTCM_DATA_PRIV static icarus_semaphore_t semaphore_pool[ICARUS_MAX_SEMAPHORES];
DTCM_DATA_PRIV static icarus_pipe_t message_pipe_pool[ICARUS_MAX_MESSAGE_QUEUES];
DTCM_DATA_PRIV int8_t cleanup_task_idx[ICARUS_MAX_TASKS];

/* ============================================================================
 * SCHEDULER STATE (DTCM_PRIV - privileged-only, zero wait state)
 * ========================================================================= */

DTCM_DATA_PRIV uint8_t current_task_index;
DTCM_DATA_PRIV uint8_t running_task_count;
DTCM_DATA_PRIV uint8_t num_created_tasks;
DTCM_DATA_PRIV volatile uint32_t current_task_ticks_remaining;
DTCM_DATA_PRIV volatile uint32_t ticks_per_task;
DTCM_DATA_PRIV uint32_t cpu_vregisters[16];
DTCM_DATA_PRIV volatile uint32_t os_tick_count;
DTCM_DATA_PRIV volatile uint8_t os_running;
DTCM_DATA_PRIV volatile uint8_t critical_stack_depth;
DTCM_DATA_PRIV volatile bool scheduler_enabled;
DTCM_DATA_PRIV int8_t current_cleanup_task_idx;

/* ============================================================================
 * STACK POOL (RAM_D1)
 * ========================================================================= */

static uint32_t stack_pool[ICARUS_MAX_TASKS][ICARUS_STACK_WORDS];


/* ============================================================================
 * DATA POOL (RAM_D1)
 * ========================================================================= */

static uint32_t data_pool[ICARUS_MAX_TASKS][ICARUS_DATA_WORDS];

/* Data pool allocation offsets — kernel metadata, protected in DTCM */
DTCM_DATA_PRIV static uint16_t data_pool_word_offsets[ICARUS_MAX_TASKS];

/* ============================================================================
 * EXTERNAL ASSEMBLY FUNCTIONS
 * ========================================================================= */

extern void start_cold_task(icarus_task_t *task);

/* ============================================================================
 * CRITICAL SECTION MANAGEMENT
 * ========================================================================= */

/**
 * @brief Privileged implementation of enter_critical
 * @note  Internal function - use enter_critical() wrapper
 */
ITCM_FUNC_PRIV void __enter_critical(void)
{
    scheduler_enabled = false;
    critical_stack_depth++;
}

/**
 * @brief Privileged implementation of exit_critical
 * @note  Internal function - use exit_critical() wrapper
 */
ITCM_FUNC_PRIV void __exit_critical(void)
{
    if (--critical_stack_depth == 0) {
        scheduler_enabled = true;
    }
}

/* ============================================================================
 * INTERNAL HELPER FUNCTIONS
 * ========================================================================= */

static inline ITCM_FUNC_PRIV void __init_sem(uint8_t semaphore_idx, uint32_t semaphore_count,
                              bool should_engage)
{
    semaphore_list[semaphore_idx]->count = semaphore_count;
    semaphore_list[semaphore_idx]->max_count = semaphore_count;
    semaphore_list[semaphore_idx]->tick_updated_at = os_tick_count;
    semaphore_list[semaphore_idx]->engaged = should_engage;
}

static inline ITCM_FUNC_PRIV void __init_pipe(uint8_t message_pipe_idx, uint8_t max_messages,
                               bool should_engage)
{
    message_pipe_list[message_pipe_idx]->count = 0;
    message_pipe_list[message_pipe_idx]->max_count = max_messages;
    message_pipe_list[message_pipe_idx]->enqueue_idx = 0;
    message_pipe_list[message_pipe_idx]->dequeue_idx = 0;
    message_pipe_list[message_pipe_idx]->tick_updated_at = os_tick_count;
    message_pipe_list[message_pipe_idx]->engaged = should_engage;

    for (uint16_t i = 0; i < ICARUS_MAX_MESSAGE_BYTES; i++) {
        message_pipe_list[message_pipe_idx]->buffer[i] = 0;
    }
}

/* ============================================================================
 * SYSTEM TASKS
 * ========================================================================= */

static void os_idle_task(void)
{
    // Simplified display_init for DTCM protection testing
    // printf("ICARUS OS - DTCM Protection Test\r\n");
    // fflush(stdout);
    display_init();
    while (1) {
        os_yield();
    }
}

static void os_heartbeat_task(void)
{
#if ICARUS_ENABLE_HEARTBEAT_VIS
    const char* task_name = os_get_current_task_name();
#endif

    while (1) {
        if (os_is_running()) {  // Use SVC wrapper instead of direct DTCM access
#if ICARUS_ENABLE_HEARTBEAT_VIS
            display_render_banner(ROW_HEARTBEAT, task_name, true);
            task_active_sleep(8);
            LED_On();
            task_active_sleep(ICARUS_HEARTBEAT_ON_TICKS - 1);
            display_render_banner(ROW_HEARTBEAT, task_name, false);
            task_active_sleep(8);
            LED_Off();
            task_active_sleep(ICARUS_HEARTBEAT_OFF_TICKS - 1);
#else
            LED_Blink(ICARUS_HEARTBEAT_ON_TICKS, ICARUS_HEARTBEAT_OFF_TICKS);
#endif
        } else {
#if ICARUS_ENABLE_HEARTBEAT_VIS
            display_render_banner(ROW_HEARTBEAT, task_name, false);
#endif
            task_active_sleep(100);
        }
    }
}

/* ============================================================================
 * KERNEL INITIALIZATION
 * ========================================================================= */

/**
 * @brief Privileged implementation of os_init
 * @note  Internal function - use os_init() wrapper
 */
ITCM_FUNC_PRIV void __os_init(void)
{
    uint8_t i;

    os_running = 0;
    ticks_per_task = ICARUS_TICKS_PER_TASK;
    running_task_count = 0;
    current_task_index = 0;
    current_cleanup_task_idx = -1;
    current_task_ticks_remaining = ticks_per_task;

    for (i = 0; i < ICARUS_MAX_TASKS; i++) {
        task_list[i] = &task_pool[i];
        cleanup_task_idx[i] = -1;
        data_pool_word_offsets[i] = 0;
    }

    for (i = 0; i < ICARUS_MAX_SEMAPHORES; i++) {
        semaphore_list[i] = &semaphore_pool[i];
        __init_sem(i, 0, false);
    }

    for (i = 0; i < ICARUS_MAX_MESSAGE_QUEUES; i++) {
        message_pipe_list[i] = &message_pipe_pool[i];
        __init_pipe(i, 0, false);
    }

    for (i = 0; i < 16; i++) {
        cpu_vregisters[i] = 0;
    }

    __os_register_task(os_idle_task, "ICARUS_KEEPALIVE_TASK");
    __os_register_task(os_heartbeat_task, ">ICARUS_HEARTBEAT<");

    scheduler_enabled = true;
}

/**
 * @brief Privileged implementation of os_start
 * @note  Internal function - use os_start() wrapper
 */
ITCM_FUNC_PRIV void __os_start(void)
{
    if (num_created_tasks == 0 || num_created_tasks > ICARUS_MAX_TASKS) {
        return;
    }
    start_cold_task(task_list[current_task_index]);
}

/* ============================================================================
 * STACK POOL ACCESS (for task.c)
 * ========================================================================= */

/**
 * @brief Privileged implementation of kernel_get_stack
 * @note  Internal function - use kernel_get_stack() wrapper
 */
ITCM_FUNC_PRIV uint32_t* __kernel_get_stack(uint8_t task_idx)
{
    return stack_pool[task_idx];
}

/* ============================================================================
 * DATA POOL ACCESS (for task.c)
 * ========================================================================= */

/**
 * @brief Privileged implementation of kernel_get_data
 * @note  Internal function - use kernel_get_data() wrapper
 */
ITCM_FUNC_PRIV uint32_t* __kernel_get_data(uint8_t task_idx)
{
    return data_pool[task_idx];
}




/**
 * @brief Privileged implementation of kernel_protected_data
 * @note  Internal function - use kernel_protected_data() wrapper
 * @note  Runs in SVC handler — already atomic, no critical section needed
 */
ITCM_FUNC_PRIV void* __kernel_protected_data(uint16_t num_words) {
    if (data_pool_word_offsets[current_task_index] + num_words > ICARUS_DATA_WORDS || num_words == 0)
        return NULL;
    uint16_t current_offset = data_pool_word_offsets[current_task_index];
    data_pool_word_offsets[current_task_index] += num_words;
    uint32_t *ret_ptr = &data_pool[current_task_index][current_offset];
    return (void*) ret_ptr;
}

/* ============================================================================
 * TASK METADATA READ GATES (for display/diagnostics)
 * ========================================================================= */

/**
 * @brief Privileged implementation of os_get_task_name
 * @note  Internal function - use os_get_task_name() wrapper
 * @note  Runs in SVC handler — reads task_list from DTCM in privileged mode
 * @note  Copies name to RAM_D1 buffer to allow unprivileged access
 */
ITCM_FUNC_PRIV const char* __os_get_task_name(uint8_t task_idx) {
    /* Static buffer in RAM_D1 (not DTCM) for unprivileged access */
#ifndef HOST_TEST
    static char name_buffer[ICARUS_MAX_TASK_NAME_LEN] __attribute__((section(".ram_d1")));
#else
    static char name_buffer[ICARUS_MAX_TASK_NAME_LEN];
#endif
    
    if (task_idx >= num_created_tasks || task_list[task_idx] == NULL) {
        return NULL;
    }
    
    /* Copy name from DTCM to RAM_D1 buffer */
    const char* src = task_list[task_idx]->name;
    for (uint8_t i = 0; i < ICARUS_MAX_TASK_NAME_LEN; i++) {
        name_buffer[i] = src[i];
        if (src[i] == '\0') break;
    }
    name_buffer[ICARUS_MAX_TASK_NAME_LEN - 1] = '\0';  /* Ensure null termination */
    
    return name_buffer;
}

/**
 * @brief Privileged implementation of os_get_num_created_tasks
 * @note  Internal function - use os_get_num_created_tasks() wrapper
 * @note  Runs in SVC handler — reads num_created_tasks from DTCM in privileged mode
 */
ITCM_FUNC_PRIV uint8_t __os_get_num_created_tasks(void) {
    return num_created_tasks;
}

/**
 * @brief Privileged implementation of os_is_running
 * @note  Internal function - use os_is_running() wrapper
 * @note  Runs in SVC handler — reads os_running from DTCM in privileged mode
 */
ITCM_FUNC_PRIV uint8_t __os_is_running(void) {
    return os_running;
}
