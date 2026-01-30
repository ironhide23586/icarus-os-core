/**
 * @file    icarus_types.h
 * @brief   ICARUS OS Core Type Definitions
 * @version 0.1.0
 *
 * @details Defines all fundamental data types used throughout the ICARUS kernel.
 *          This includes task control blocks, synchronization primitives, and
 *          inter-process communication structures.
 *
 * @note    All structures are designed for ARM Cortex-M7 alignment requirements.
 *
 * @see     ARMv7-M Architecture Reference Manual for alignment requirements
 * @see     docs/do178c/design/SDD.md Section 4.2 - Data Structures
 *
 * @author  Souham Biswas
 * @date    2026
 *
 * @copyright Copyright (c) 2026 ICARUS Project
 *            https://github.com/ironhide23586/icarus-os-core
 *            Licensed under MIT License
 */

#ifndef ICARUS_TYPES_H
#define ICARUS_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "icarus_config.h"

/* ============================================================================
 * TASK STATE DEFINITIONS
 * ========================================================================= */

/**
 * @brief Task state enumeration
 *
 * State transition diagram:
 * @verbatim
 *                    ┌─────────┐
 *                    │  COLD   │ (Initial state after creation)
 *                    └────┬────┘
 *                         │ task_start()
 *                         ▼
 *     ┌──────────────►┌─────────┐◄──────────────┐
 *     │               │ RUNNING │               │
 *     │               └────┬────┘               │
 *     │                    │                    │
 *     │ scheduler          │ os_yield() /       │ scheduler
 *     │ preemption         │ time slice         │ dispatch
 *     │                    ▼                    │
 *     │               ┌─────────┐               │
 *     └───────────────│  READY  │───────────────┘
 *                     └────┬────┘
 *                          │ task_active_sleep()
 *                          ▼
 *                     ┌─────────┐
 *                     │ BLOCKED │ (Waiting for timeout/event)
 *                     └────┬────┘
 *                          │ timeout expired
 *                          ▼
 *                     ┌─────────┐
 *                     │  READY  │
 *                     └─────────┘
 *
 *     Any state ──► KILLED (os_kill_process)
 *     Any state ──► FINISHED (os_exit_task)
 * @endverbatim
 *
 * @see docs/do178c/design/SDD.md Section 4.3 - Task State Machine
 */
typedef enum {
    TASK_STATE_COLD     = 0,    /**< Task created but never started */
    TASK_STATE_RUNNING  = 1,    /**< Task currently executing on CPU */
    TASK_STATE_READY    = 2,    /**< Task ready to run, waiting for CPU */
    TASK_STATE_BLOCKED  = 3,    /**< Task waiting for timeout or event */
    TASK_STATE_KILLED   = 4,    /**< Task terminated by another task */
    TASK_STATE_FINISHED = 5     /**< Task completed execution normally */
} icarus_task_state_t;

/**
 * @brief Task priority levels
 *
 * @note Current implementation uses round-robin within priority levels.
 *       Future versions may implement strict priority scheduling.
 */
typedef enum {
    TASK_PRIORITY_LOW   = 0,    /**< Background/idle priority */
    TASK_PRIORITY_MED   = 1,    /**< Normal application priority */
    TASK_PRIORITY_HIGH  = 2     /**< Time-critical priority */
} icarus_task_priority_t;

/* ============================================================================
 * TASK CONTROL BLOCK (TCB)
 * ========================================================================= */

/**
 * @brief Task Control Block (TCB)
 *
 * @details The TCB contains all information needed to manage a task's execution.
 *          Field ordering is critical for assembly code compatibility.
 *
 * @warning Do NOT reorder fields without updating context_switch.s offsets!
 *          Assembly code uses hardcoded offsets:
 *          - stack_pointer: offset 12
 *          - task_state:    offset 16
 *          - global_tick_paused: offset 20
 *          - ticks_to_pause: offset 24
 *
 * @see Core/Src/kernel/context_switch.s for assembly offset usage
 */
typedef struct {
    /* ---- Fields accessed by C code only ---- */
    void (*function)(void);                     /**< Task entry point function pointer */
    uint32_t *stack_base;                       /**< Base address of task stack */
    uint32_t stack_size;                        /**< Stack size in 32-bit words */

    /* ---- Fields accessed by assembly (offsets hardcoded) ---- */
    uint32_t *stack_pointer;                    /**< Current stack pointer (offset 12) */
    icarus_task_state_t task_state;             /**< Current task state (offset 16) */
    uint32_t global_tick_paused;                /**< Tick count when blocked (offset 20) */
    uint32_t ticks_to_pause;                    /**< Ticks to remain blocked (offset 24) */

    /* ---- Additional metadata ---- */
    icarus_task_priority_t task_priority;       /**< Task priority level */
    char name[ICARUS_MAX_TASK_NAME_LEN];        /**< Human-readable task name */
} icarus_task_t;

/* ============================================================================
 * SYNCHRONIZATION PRIMITIVES
 * ========================================================================= */

/**
 * @brief Counting Semaphore
 *
 * @details Implements a counting semaphore for resource management and
 *          task synchronization. Supports both binary (max_count=1) and
 *          counting (max_count>1) semaphore semantics.
 *
 * @note Memory footprint: 16 bytes per semaphore
 *
 * @see docs/do178c/design/SDD.md Section 4.4 - Synchronization Primitives
 */
typedef struct {
    bool engaged;               /**< true if semaphore is initialized and active */
    uint32_t count;             /**< Current semaphore count */
    uint32_t max_count;         /**< Maximum count (initial value) */
    uint32_t tick_updated_at;   /**< Last modification timestamp (for debugging) */
} icarus_semaphore_t;

/**
 * @brief Message Pipe (Queue)
 *
 * @details Implements a circular buffer message queue for inter-task
 *          communication. Supports variable-length messages up to
 *          ICARUS_MAX_MESSAGE_BYTES.
 *
 * @note Memory footprint: ~140 bytes per pipe (with 128-byte buffer)
 *
 * @see docs/do178c/design/SDD.md Section 4.5 - Inter-Process Communication
 */
typedef struct {
    bool engaged;               /**< true if pipe is initialized and active */
    uint8_t count;              /**< Current number of bytes in buffer */
    uint8_t max_count;          /**< Maximum buffer capacity in bytes */
    uint8_t enqueue_idx;        /**< Write index (circular) */
    uint8_t dequeue_idx;        /**< Read index (circular) */
    uint32_t tick_updated_at;   /**< Last modification timestamp */
    uint8_t buffer[ICARUS_MAX_MESSAGE_BYTES];  /**< Circular message buffer */
} icarus_pipe_t;

#ifdef __cplusplus
}
#endif

#endif /* ICARUS_TYPES_H */
