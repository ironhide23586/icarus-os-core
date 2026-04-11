/**
 * @file    scheduler.h
 * @brief   ICARUS Scheduler - Task Scheduling and Timing
 * @version 0.1.0
 *
 * @details Task scheduling, yielding, and sleep functions.
 *
 * @author  Souham Biswas
 * @date    2025
 *
 * @copyright Copyright 2025-2026 Souham Biswas
 *            https://github.com/ironhide23586/icarus-os-core
 *            Licensed under the Apache License, Version 2.0
 */

#ifndef ICARUS_SCHEDULER_H
#define ICARUS_SCHEDULER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "icarus/types.h"

/* ============================================================================
 * TASK SCHEDULING
 * ========================================================================= */

/**
 * @brief Voluntarily yield CPU to scheduler
 * @note  Placed in ITCM for zero wait-state execution
 */
void os_yield(void);

/**
 * @brief Sleep for specified ticks (cooperative)
 * @param ticks Number of ticks to sleep
 * @return Actual ticks elapsed
 * @note  Placed in ITCM for zero wait-state execution
 */
uint32_t task_active_sleep(uint32_t ticks);

/**
 * @brief Sleep with scheduler disabled (blocking)
 * @param ticks Number of ticks to wait
 * @return Actual ticks elapsed
 * @warning Blocks all other tasks!
 */
uint32_t task_blocking_sleep(uint32_t ticks);

/**
 * @brief Busy-wait for specified ticks
 * @param ticks Number of ticks to wait
 * @return Actual ticks elapsed
 */
uint32_t task_busy_wait(uint32_t ticks);

/* ============================================================================
 * TASK INFORMATION
 * ========================================================================= */

/**
 * @brief Get current system tick count
 * @return Ticks since os_start()
 * @note  Placed in ITCM for zero wait-state execution
 */
uint32_t os_get_tick_count(void);

/**
 * @brief Get number of active tasks
 * @return Count of tasks in RUNNING/READY/BLOCKED state
 */
uint8_t os_get_running_task_count(void);

/**
 * @brief Get name of currently executing task
 * @return Task name string or "unknown"
 */
const char* os_get_current_task_name(void);

/**
 * @brief Get remaining ticks in current time slice
 * @return Ticks remaining before preemption
 */
uint32_t os_get_task_ticks_remaining(void);

/**
 * @brief Get task name by index (safe from unprivileged mode)
 * @param task_idx Task index (0 to num_created_tasks-1)
 * @return Pointer to task name string, or NULL if invalid index
 * @note  Uses SVC to read task_list from DTCM in privileged mode
 */
const char* os_get_task_name(uint8_t task_idx);

/**
 * @brief Get number of created tasks (safe from unprivileged mode)
 * @return Number of tasks registered with os_register_task()
 * @note  Uses SVC to read num_created_tasks from DTCM in privileged mode
 */
uint8_t os_get_num_created_tasks(void);

/**
 * @brief Check if OS is running (safe from unprivileged mode)
 * @return 1 if OS is running, 0 otherwise
 * @note  Uses SVC to read os_running from DTCM in privileged mode
 */
uint8_t os_is_running(void);

/**
 * @brief  Get a task's current state (safe from unprivileged mode).
 * @param  task_idx  Task index (0 to num_created_tasks-1).
 * @return Task state, or TASK_STATE_FINISHED if index is invalid.
 * @note   Uses SVC to read task_list from DTCM in privileged mode.
 */
icarus_task_state_t os_get_task_state(uint8_t task_idx);

/**
 * @brief  Get a task's dispatch count (times scheduled to run).
 * @param  task_idx  Task index.
 * @return Dispatch count, or 0 if index is invalid.
 */
uint32_t os_get_task_dispatch_count(uint8_t task_idx);

/**
 * @brief  Get a task's stack high-water mark (minimum free words).
 * @param  task_idx  Task index.
 * @return Minimum free stack words observed, or 0 if invalid.
 * @note   The watermark is updated lazily — call os_update_stack_watermark()
 *         to force a scan.
 */
uint32_t os_get_stack_watermark(uint8_t task_idx);

/**
 * @brief  Scan a task's stack for the sentinel pattern and update
 *         the watermark field in the TCB.
 * @param  task_idx  Task index.
 */
void os_update_stack_watermark(uint8_t task_idx);

/* ============================================================================
 * PRIVILEGED IMPLEMENTATIONS (Internal - Do Not Call Directly)
 * ========================================================================= */

void __os_yield(void);
uint32_t __task_active_sleep(uint32_t ticks);
uint32_t __task_blocking_sleep(uint32_t ticks);
uint32_t __task_busy_wait(uint32_t ticks);
uint32_t __os_get_tick_count(void);
uint8_t __os_get_running_task_count(void);
const char* __os_get_current_task_name(void);
uint32_t __os_get_task_ticks_remaining(void);
const char* __os_get_task_name(uint8_t task_idx);
uint8_t __os_get_num_created_tasks(void);
uint8_t __os_is_running(void);
icarus_task_state_t __os_get_task_state(uint8_t task_idx);
uint32_t __os_get_task_dispatch_count(uint8_t task_idx);
uint32_t __os_get_stack_watermark(uint8_t task_idx);
void __os_update_stack_watermark(uint8_t task_idx);

#ifdef __cplusplus
}
#endif

#endif /* ICARUS_SCHEDULER_H */
