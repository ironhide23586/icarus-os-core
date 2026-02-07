/**
 * @file    task.c
 * @brief   ICARUS Task Management Implementation
 * @version 0.1.0
 *
 * @details Task creation, registration, and lifecycle management.
 *
 * @author  Souham Biswas
 * @date    2025
 *
 * @copyright Copyright 2025 Souham Biswas
 *            https://github.com/ironhide23586/icarus-os-core
 *            Licensed under the Apache License, Version 2.0
 */

#include "icarus/task.h"
#include "icarus/kernel.h"
#include "icarus/scheduler.h"

#include <string.h>
#include <stdio.h>

/* ============================================================================
 * EXTERNAL FUNCTIONS
 * ========================================================================= */

extern uint32_t* __kernel_get_stack(uint8_t task_idx);
extern uint32_t* __kernel_get_data(uint8_t task_idx);

/* ============================================================================
 * EXTERNAL KERNEL DATA
 * ========================================================================= */

extern DTCM_DATA int8_t __cleanup_task_idx[ICARUS_MAX_TASKS];
extern DTCM_DATA int8_t __current_cleanup_task_idx;

/* ============================================================================
 * TASK CREATION (PRIVILEGED INTERNAL)
 * ========================================================================= */

/* Privileged internal function - called by __os_register_task() only */
static void __os_create_task(icarus_task_t *task, void (*function)(void),
                             uint32_t *stack, uint32_t stack_size,
                             uint32_t *data, const char *name)
{
    if (__running_task_count >= ICARUS_MAX_TASKS) {
        return;
    }

    task->function = function;
    task->stack_base = stack;
    task->stack_size = stack_size;
    task->task_state = TASK_STATE_COLD;
    task->task_priority = TASK_PRIORITY_LOW;
    task->global_tick_paused = 0;
    task->ticks_to_pause = 0;

    strncpy(task->name, name, ICARUS_MAX_TASK_NAME_LEN);
    task->name[ICARUS_MAX_TASK_NAME_LEN - 1] = '\0';

    uint32_t *_stack_top = stack + stack_size - 1;

    *(_stack_top--) = 0x01000000;                      /* xPSR */
    *(_stack_top--) = (uint32_t)(uintptr_t)function;   /* PC */
    *(_stack_top--) = (uint32_t)(uintptr_t)os_exit_task; /* LR */
    *(_stack_top--) = 0;                               /* R12 */
    *(_stack_top--) = 0;                               /* R3 */
    *(_stack_top--) = 0;                               /* R2 */
    *(_stack_top--) = 0;                               /* R1 */
    *(_stack_top)   = 0;                               /* R0 */

    task->stack_pointer = _stack_top;
    task->data_pointer = data;
    __num_created_tasks++;
}

/* Privileged implementation - called via SVC from os_register_task() */
void __os_register_task(void (*function)(void), const char *name)
{
    __os_create_task(__task_list[__num_created_tasks], function,
                     __kernel_get_stack(__num_created_tasks), 
                     ICARUS_STACK_WORDS, 
                     __kernel_get_data(__num_created_tasks), name);
}




/* ============================================================================
 * TASK LIFECYCLE
 * ========================================================================= */

/* Privileged implementation - called via SVC from os_exit_task() */
void __os_exit_task(void)
{
    __task_list[__current_task_index]->task_state = TASK_STATE_FINISHED;

    if (__running_task_count > 0) {
        __running_task_count--;
    }

    // Add to cleanup queue
    if (__current_cleanup_task_idx < (ICARUS_MAX_TASKS - 1)) {
        __cleanup_task_idx[++__current_cleanup_task_idx] = (int8_t)__current_task_index;
    }

    __os_yield();  // Call privileged function directly (no SVC from kernel code)
}

/* Privileged implementation - called via SVC from os_kill_process() */
void __os_kill_process(uint8_t task_index)
{
    if (task_index >= __num_created_tasks || task_index == 0) {
        return;  /* Cannot kill task 0 (idle task) or invalid indices */
    }

    __task_list[task_index]->task_state = TASK_STATE_KILLED;

    if (__running_task_count > 0) {
        __running_task_count--;
    }

    // Add to cleanup queue
    if (__current_cleanup_task_idx < (ICARUS_MAX_TASKS - 1)) {
        __cleanup_task_idx[++__current_cleanup_task_idx] = (int8_t)task_index;
    }

    if (task_index == __current_task_index) {
        __os_yield();  // Call privileged function directly (no SVC from kernel code)
    }
}

/* Privileged implementation - called via SVC from os_task_suicide() */
void __os_task_suicide(void)
{
    printf("[INFO] %s committed suicide\r\n",
           __task_list[__current_task_index]->name);
    __os_kill_process(__current_task_index);
}
