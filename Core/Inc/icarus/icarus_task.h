/**
 * @file    icarus_task.h
 * @brief   ICARUS OS Task Management API
 * @version 0.1.0
 *
 * @details Public API for task creation, scheduling, and lifecycle management.
 *          This is the primary interface for application code to interact with
 *          the ICARUS kernel.
 *
 * @par Example Usage:
 * @code
 *     // Register a task with the kernel
 *     os_register_task(my_task_function, "MyTask");
 *
 *     // Start the scheduler
 *     os_start();
 *
 *     // Inside a task: yield CPU voluntarily
 *     os_yield();
 *
 *     // Inside a task: sleep for 100 ticks
 *     task_active_sleep(100);
 * @endcode
 *
 * @see docs/do178c/design/SDD.md Section 5 - Task Management
 * @see docs/do178c/requirements/SRS.md Section 3.1 - Task Requirements
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
#include <stdbool.h>
#include "icarus_config.h"
#include "icarus_types.h"

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
 *
 * @note    This function registers two system tasks:
 *          - ICARUS_KEEPALIVE_TASK (idle task)
 *          - ICARUS_HEARTBEAT_TASK (LED heartbeat)
 */
void os_init(void);

/**
 * @brief Start the ICARUS scheduler
 *
 * @details Begins task execution. This function does not return under normal
 *          operation. The first registered task will begin execution.
 *
 * @pre     os_init() must have been called
 * @pre     At least one task must be registered
 *
 * @warning This function enables interrupts and never returns!
 */
void os_start(void);

/* ============================================================================
 * TASK CREATION AND REGISTRATION
 * ========================================================================= */

/**
 * @brief Register a task with automatic stack allocation
 *
 * @details Simplified task registration using the kernel's internal stack pool.
 *          This is the recommended method for most applications.
 *
 * @param[in] function  Task entry point (must not return, use os_exit_task())
 * @param[in] name      Human-readable task name (max ICARUS_MAX_TASK_NAME_LEN-1 chars)
 *
 * @pre     os_init() must have been called
 * @pre     num_created_tasks < ICARUS_MAX_TASKS
 *
 * @par Example:
 * @code
 *     void my_task(void) {
 *         while (1) {
 *             // Task work here
 *             task_active_sleep(100);
 *         }
 *     }
 *
 *     os_register_task(my_task, "MyTask");
 * @endcode
 */
void os_register_task(void (*function)(void), const char *name);

/**
 * @brief Create a task with explicit stack allocation
 *
 * @details Advanced task creation with user-provided stack. Use this when
 *          you need control over stack placement or size.
 *
 * @param[out] task       Pointer to task control block to initialize
 * @param[in]  function   Task entry point function
 * @param[in]  stack      Pointer to stack memory (must be 8-byte aligned)
 * @param[in]  stack_size Stack size in 32-bit words
 * @param[in]  name       Human-readable task name
 *
 * @pre     task != NULL
 * @pre     function != NULL
 * @pre     stack != NULL and 8-byte aligned
 * @pre     stack_size >= 64 (minimum viable stack)
 */
void os_create_task(icarus_task_t *task, void (*function)(void),
                    uint32_t *stack, uint32_t stack_size, const char *name);

/* ============================================================================
 * TASK SCHEDULING AND CONTROL
 * ========================================================================= */

/**
 * @brief Voluntarily yield CPU to scheduler
 *
 * @details Triggers a context switch to the next ready task. The current
 *          task is placed in READY state and will be scheduled again.
 *
 * @note    This function is placed in ITCM for zero wait-state execution.
 *
 * @par Implementation:
 *          Sets PendSV pending bit to trigger context switch at lowest
 *          interrupt priority, ensuring clean stack state.
 *
 * @see     ARMv7-M Architecture Reference Manual, Section B1.5.14 (PendSV)
 */
void os_yield(void);

/**
 * @brief Sleep for specified number of ticks (non-blocking)
 *
 * @details Blocks the current task for the specified duration while allowing
 *          other tasks to run. This is the preferred sleep method.
 *
 * @param[in] ticks  Number of SysTick intervals to sleep
 *
 * @return Actual number of ticks elapsed (may be >= requested due to scheduling)
 *
 * @note    This function is placed in ITCM for zero wait-state execution.
 *
 * @par Example:
 * @code
 *     // Sleep for approximately 100ms (at 1ms tick rate)
 *     uint32_t actual = task_active_sleep(100);
 * @endcode
 */
uint32_t task_active_sleep(uint32_t ticks);

/**
 * @brief Sleep with interrupts disabled (blocking)
 *
 * @details Busy-waits for the specified duration with scheduler disabled.
 *          Use sparingly as this blocks all other tasks.
 *
 * @param[in] ticks  Number of SysTick intervals to wait
 *
 * @return Actual number of ticks elapsed
 *
 * @warning This function disables the scheduler! Use only when absolutely
 *          necessary (e.g., hardware timing requirements).
 */
uint32_t task_blocking_sleep(uint32_t ticks);

/**
 * @brief Busy-wait for specified ticks (must be in critical section)
 *
 * @details Low-level busy-wait loop. Caller must already be in critical section.
 *
 * @param[in] ticks  Number of ticks to wait
 *
 * @return Actual number of ticks elapsed
 *
 * @pre     Must be called from within a critical section
 */
uint32_t task_busy_wait(uint32_t ticks);

/* ============================================================================
 * TASK LIFECYCLE
 * ========================================================================= */

/**
 * @brief Terminate current task normally
 *
 * @details Marks the current task as FINISHED and yields to scheduler.
 *          The task's resources are queued for cleanup.
 *
 * @note    This is automatically called if a task function returns.
 *
 * @warning Do not call from interrupt context!
 */
void os_exit_task(void);

/**
 * @brief Terminate another task
 *
 * @details Forcibly terminates the specified task. The task is marked as
 *          KILLED and will not be scheduled again.
 *
 * @param[in] task_index  Index of task to kill (0 to num_created_tasks-1)
 *
 * @note    Cannot kill system tasks (index 0 and 1)
 * @note    If killing the current task, this triggers a context switch
 */
void os_kill_process(uint8_t task_index);

/**
 * @brief Terminate current task (suicide)
 *
 * @details Convenience function to kill the currently running task.
 *          Equivalent to os_kill_process(current_task_index).
 */
void os_task_suicide(void);

/* ============================================================================
 * TASK INFORMATION
 * ========================================================================= */

/**
 * @brief Get current system tick count
 *
 * @return Number of SysTick interrupts since os_start()
 *
 * @note    This function is placed in ITCM for zero wait-state execution.
 * @note    Tick count wraps at UINT32_MAX (~49 days at 1ms tick)
 */
uint32_t os_get_tick_count(void);

/**
 * @brief Get number of active tasks
 *
 * @return Count of tasks in RUNNING, READY, or BLOCKED state
 */
uint8_t os_get_running_task_count(void);

/**
 * @brief Get name of currently executing task
 *
 * @return Pointer to task name string, or "unknown" if invalid
 */
const char* os_get_current_task_name(void);

/**
 * @brief Get remaining ticks in current time slice
 *
 * @return Ticks remaining before preemption
 */
uint32_t os_get_task_ticks_remaining(void);

/* ============================================================================
 * SEMAPHORE API
 * ========================================================================= */

/**
 * @brief Initialize a counting semaphore
 *
 * @param[in] semaphore_idx  Semaphore index (0 to ICARUS_MAX_SEMAPHORES-1)
 * @param[in] semaphore_count  Initial count (also sets max_count)
 *
 * @retval true   Semaphore initialized successfully
 * @retval false  Invalid index, count=0, or semaphore already in use
 */
bool semaphore_init(uint8_t semaphore_idx, uint32_t semaphore_count);

/**
 * @brief Increment semaphore count (V operation / signal)
 *
 * @details Blocks if count == max_count until space is available.
 *
 * @param[in] semaphore_idx  Semaphore index
 *
 * @retval true   Operation successful
 * @retval false  Invalid index or semaphore not initialized
 *
 * @note    This function is placed in ITCM for zero wait-state execution.
 */
bool semaphore_feed(uint8_t semaphore_idx);

/**
 * @brief Decrement semaphore count (P operation / wait)
 *
 * @details Blocks if count == 0 until semaphore is signaled.
 *
 * @param[in] semaphore_idx  Semaphore index
 *
 * @retval true   Operation successful
 * @retval false  Invalid index or semaphore not initialized
 *
 * @note    This function is placed in ITCM for zero wait-state execution.
 */
bool semaphore_consume(uint8_t semaphore_idx);

/**
 * @brief Get current semaphore count
 *
 * @param[in] semaphore_idx  Semaphore index
 *
 * @return Current count, or 0 if invalid/uninitialized
 */
uint32_t semaphore_get_count(uint8_t semaphore_idx);

/**
 * @brief Get semaphore maximum count
 *
 * @param[in] semaphore_idx  Semaphore index
 *
 * @return Maximum count, or 0 if invalid/uninitialized
 */
uint32_t semaphore_get_max_count(uint8_t semaphore_idx);

/* ============================================================================
 * MESSAGE PIPE API
 * ========================================================================= */

/**
 * @brief Initialize a message pipe
 *
 * @param[in] pipe_idx  Pipe index (0 to ICARUS_MAX_MESSAGE_QUEUES-1)
 * @param[in] pipe_capacity_bytes  Maximum bytes the pipe can hold
 *
 * @retval true   Pipe initialized successfully
 * @retval false  Invalid index, capacity=0, or pipe already in use
 */
bool pipe_init(uint8_t pipe_idx, uint8_t pipe_capacity_bytes);

/**
 * @brief Enqueue message to pipe
 *
 * @details Blocks if insufficient space until room is available.
 *
 * @param[in] pipe_idx       Pipe index
 * @param[in] message        Pointer to message data
 * @param[in] message_bytes  Number of bytes to enqueue
 *
 * @retval true   Message enqueued successfully
 * @retval false  Invalid parameters or pipe not initialized
 *
 * @note    This function is placed in ITCM for zero wait-state execution.
 */
bool pipe_enqueue(uint8_t pipe_idx, uint8_t* message, uint8_t message_bytes);

/**
 * @brief Dequeue message from pipe
 *
 * @details Blocks if insufficient data until message is available.
 *
 * @param[in]  pipe_idx       Pipe index
 * @param[out] message        Buffer to receive message data
 * @param[in]  message_bytes  Number of bytes to dequeue
 *
 * @retval true   Message dequeued successfully
 * @retval false  Invalid parameters or pipe not initialized
 *
 * @note    This function is placed in ITCM for zero wait-state execution.
 */
bool pipe_dequeue(uint8_t pipe_idx, uint8_t* message, uint8_t message_bytes);

/**
 * @brief Get current byte count in pipe
 *
 * @param[in] pipe_idx  Pipe index
 *
 * @return Current byte count, or 0 if invalid/uninitialized
 */
uint8_t pipe_get_count(uint8_t pipe_idx);

/**
 * @brief Get pipe capacity
 *
 * @param[in] pipe_idx  Pipe index
 *
 * @return Maximum capacity in bytes, or 0 if invalid/uninitialized
 */
uint8_t pipe_get_max_count(uint8_t pipe_idx);

#ifdef __cplusplus
}
#endif

#endif /* ICARUS_TASK_H */
