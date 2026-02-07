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
 * @note  This is the only public API for task creation
 */
void os_register_task(void (*function)(void), const char *name);

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

#ifdef __cplusplus
}
#endif

#endif /* ICARUS_TASK_H */
