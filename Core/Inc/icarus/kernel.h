/**
 * @file    kernel.h
 * @brief   ICARUS Kernel Core - State and Initialization
 * @version 0.1.0
 *
 * @details Core kernel state variables, data structures, and initialization.
 *          This is the foundation layer that other kernel modules depend on.
 *
 * @author  Souham Biswas
 * @date    2025
 *
 * @copyright Copyright 2025 Souham Biswas
 *            https://github.com/ironhide23586/icarus-os-core
 *            Licensed under the Apache License, Version 2.0
 */

#ifndef ICARUS_KERNEL_H
#define ICARUS_KERNEL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "bsp/retarget_hal.h"
#include "icarus/config.h"
#include "icarus/types.h"
#include "icarus/svc.h"

/* ============================================================================
 * SECTION PLACEMENT MACROS
 * ========================================================================= */

#ifndef HOST_TEST
#define DTCM_DATA_USER __attribute__((section(".dtcm")))
#define DTCM_DATA_PRIV __attribute__((section(".dtcm.privileged")))
#else
#define DTCM_DATA_USER
#define DTCM_DATA_PRIV
#endif

/* ============================================================================
 * KERNEL STATE (extern declarations)
 * ========================================================================= */

/** @brief Array of pointers to all task control blocks */
extern DTCM_DATA_PRIV icarus_task_t* __task_list[ICARUS_MAX_TASKS];

/** @brief Array of pointers to all semaphores */
extern DTCM_DATA_PRIV icarus_semaphore_t* __semaphore_list[ICARUS_MAX_SEMAPHORES];

/** @brief Array of pointers to all message pipes */
extern DTCM_DATA_PRIV icarus_pipe_t* __message_pipe_list[ICARUS_MAX_MESSAGE_QUEUES];

/** @brief Index of currently executing task */
extern DTCM_DATA_PRIV uint8_t __current_task_index;

/** @brief Count of tasks in active states */
extern DTCM_DATA_PRIV uint8_t __running_task_count;

/** @brief Total number of created tasks */
extern DTCM_DATA_PRIV uint8_t __num_created_tasks;

/** @brief Ticks remaining in current task's time slice */
extern DTCM_DATA_PRIV volatile uint32_t __current_task_ticks_remaining;

/** @brief Configured ticks per time slice */
extern DTCM_DATA_PRIV volatile uint32_t __ticks_per_task;

/** @brief System tick counter */
extern DTCM_DATA_PRIV volatile uint32_t __os_tick_count;

/** @brief Flag indicating OS is running */
extern DTCM_DATA_PRIV volatile uint8_t __os_running;

/** @brief Scheduler enable flag */
extern DTCM_DATA_PRIV volatile bool __scheduler_enabled;

/** @brief Critical section nesting depth */
extern DTCM_DATA_PRIV volatile uint8_t __critical_stack_depth;

/** @brief CPU virtual registers for context switching */
extern DTCM_DATA_PRIV uint32_t __cpu_vregisters[16];

/** @brief Cleanup task index array */
extern DTCM_DATA_PRIV int8_t __cleanup_task_idx[ICARUS_MAX_TASKS];

/** @brief Current cleanup task index */
extern DTCM_DATA_PRIV int8_t __current_cleanup_task_idx;

/* ============================================================================
 * KERNEL INITIALIZATION
 * ========================================================================= */

/**
 * @brief Initialize the ICARUS kernel
 *
 * @details Must be called before any other kernel function. Initializes:
 *          - Task pool and task list
 *          - Semaphore pool
 *          - Message pipe pool
 *          - System tasks (idle, heartbeat)
 *
 * @pre     HAL must be initialized (hal_init() called)
 * @post    Kernel is ready for task registration
 */
void os_init(void);

/**
 * @brief Start the ICARUS scheduler
 *
 * @details Begins task execution. This function does not return.
 *
 * @pre     os_init() must have been called
 * @pre     At least one task must be registered
 *
 * @warning This function never returns!
 */
void os_start(void);

/* Forward declarations for scheduler functions used by kernel */
void os_yield(void);
uint32_t task_active_sleep(uint32_t ticks);
const char* os_get_current_task_name(void);

/* ============================================================================
 * CRITICAL SECTION API
 * ========================================================================= */

/**
 * @brief Enter critical section (disable scheduler)
 * @note  Supports nesting
 */
void enter_critical(void);

/**
 * @brief Exit critical section (re-enable scheduler if outermost)
 */
void exit_critical(void);

/**
 * @brief Get stack pointer for task index (internal use)
 * @param task_idx Task index
 * @return Pointer to stack memory
 */
uint32_t* kernel_get_stack(uint8_t task_idx);

/**
 * @brief Get data pointer for task index (internal use)
 * @param task_idx Task index
 * @return Pointer to data memory
 */
uint32_t* kernel_get_data(uint8_t task_idx);

/**
 * @brief Allocate MPU-protected data for current task
 * @param num_words Number of 32-bit words to allocate
 * @return Pointer to allocated memory, or NULL on error
 * @note  Memory is allocated from task's protected data pool
 * @note  Allocations are permanent for task lifetime
 * @note  Returns NULL if num_words is 0 or exceeds available space
 */
void* kernel_protected_data(uint16_t num_words);

/* ============================================================================
 * PRIVILEGED IMPLEMENTATIONS (Internal - Do Not Call Directly)
 * ========================================================================= */

void __enter_critical(void);
void __exit_critical(void);
void __os_init(void);
void __os_start(void);
void* __kernel_protected_data(uint16_t num_words);
uint32_t* __kernel_get_stack(uint8_t task_idx);
uint32_t* __kernel_get_data(uint8_t task_idx);

#ifdef __cplusplus
}
#endif

#endif /* ICARUS_KERNEL_H */
