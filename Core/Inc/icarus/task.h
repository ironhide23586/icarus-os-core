/**
 * @file    task.h
 * @brief   ICARUS Task Management - Creation and Lifecycle
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

#ifndef ICARUS_TASK_H
#define ICARUS_TASK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "icarus/types.h"

/* ============================================================================
 * TASK CREATION
 * ========================================================================= */

/**
 * @brief Register a task with automatic stack allocation
 * @param function Task entry point
 * @param name     Task name (max ICARUS_MAX_TASK_NAME_LEN chars)
 */
void os_register_task(void (*function)(void), const char *name);

/**
 * @brief Create a task with explicit stack allocation
 * @param task       TCB to initialize
 * @param function   Task entry point
 * @param stack      Stack memory (8-byte aligned)
 * @param stack_size Stack size in 32-bit words
 * @param name       Task name
 */
void os_create_task(icarus_task_t *task, void (*function)(void),
                    uint32_t *stack, uint32_t stack_size, 
                    uint32_t *data, const char *name);

/* ============================================================================
 * TASK LIFECYCLE
 * ========================================================================= */

/**
 * @brief Terminate current task normally
 * @note  Called automatically if task function returns
 */
void os_exit_task(void);

/**
 * @brief Terminate another task by index
 * @param task_index Index of task to kill
 */
void os_kill_process(uint8_t task_index);

/**
 * @brief Terminate current task (suicide)
 */
void os_task_suicide(void);

/* ============================================================================
 * PRIVILEGED IMPLEMENTATIONS (Internal - Do Not Call Directly)
 * ========================================================================= */

void __os_register_task(void (*function)(void), const char *name);
void __os_exit_task(void);
void __os_kill_process(uint8_t task_index);
void __os_task_suicide(void);

#ifdef __cplusplus
}
#endif

#endif /* ICARUS_TASK_H */
