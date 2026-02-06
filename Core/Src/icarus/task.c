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

extern uint32_t* kernel_get_stack(uint8_t task_idx);

/* ============================================================================
 * TASK CREATION
 * ========================================================================= */

void os_create_task(icarus_task_t *task, void (*function)(void),
                    uint32_t *stack, uint32_t stack_size,
                    uint32_t *data, const char *name)
{
    if (running_task_count >= ICARUS_MAX_TASKS) {
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

    uint32_t *stack_top = stack + stack_size - 1;

    *(stack_top--) = 0x01000000;                      /* xPSR */
    *(stack_top--) = (uint32_t)(uintptr_t)function;   /* PC */
    *(stack_top--) = (uint32_t)(uintptr_t)os_exit_task; /* LR */
    *(stack_top--) = 0;                               /* R12 */
    *(stack_top--) = 0;                               /* R3 */
    *(stack_top--) = 0;                               /* R2 */
    *(stack_top--) = 0;                               /* R1 */
    *(stack_top)   = 0;                               /* R0 */

    task->stack_pointer = stack_top;
    task->data_pointer = data;
    num_created_tasks++;
}

void os_register_task(void (*function)(void), const char *name)
{
    os_create_task(task_list[num_created_tasks], function,
                   kernel_get_stack(num_created_tasks), 
                   ICARUS_STACK_WORDS, 
                   kernel_get_data(num_created_tasks), name);
}

/* ============================================================================
 * TASK LIFECYCLE
 * ========================================================================= */

void os_exit_task(void)
{
    task_list[current_task_index]->task_state = TASK_STATE_FINISHED;

    if (running_task_count > 0) {
        running_task_count--;
    }

    os_yield();
}

void os_kill_process(uint8_t task_index)
{
    if (task_index >= num_created_tasks || task_index < 2) {
        return;  /* Cannot kill system tasks */
    }

    task_list[task_index]->task_state = TASK_STATE_KILLED;

    if (running_task_count > 0) {
        running_task_count--;
    }

    if (task_index == current_task_index) {
        os_yield();
    }
}

void os_task_suicide(void)
{
    printf("[INFO] %s committed suicide\r\n",
           task_list[current_task_index]->name);
    os_kill_process(current_task_index);
}
