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
 * @copyright Copyright 2025 Souham Biswas
 *            https://github.com/ironhide23586/icarus-os-core
 *            Licensed under the Apache License, Version 2.0
 */

#ifndef ICARUS_SCHEDULER_H
#define ICARUS_SCHEDULER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "icarus/svc.h"
#include <stdint.h>

/* ============================================================================
 * SECTION PLACEMENT MACROS
 * ========================================================================= */

#ifndef HOST_TEST
#define ITCM_FUNC_USER __attribute__((section(".itcm")))
#define ITCM_FUNC_PRIV __attribute__((section(".itcm.privileged")))
#else
#define ITCM_FUNC_USER
#define ITCM_FUNC_PRIV
#endif

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

/* ============================================================================
 * PRIVILEGED IMPLEMENTATIONS (Internal - Do Not Call Directly)
 * ========================================================================= */

ITCM_FUNC_PRIV void __os_yield(void);
ITCM_FUNC_PRIV uint32_t __task_active_sleep(uint32_t ticks);
uint32_t __task_blocking_sleep(uint32_t ticks);
uint32_t __task_busy_wait(uint32_t ticks);
ITCM_FUNC_PRIV uint32_t __os_get_tick_count(void);
uint8_t __os_get_running_task_count(void);
const char* __os_get_current_task_name(void);
uint32_t __os_get_task_ticks_remaining(void);

#ifdef __cplusplus
}
#endif

#endif /* ICARUS_SCHEDULER_H */
