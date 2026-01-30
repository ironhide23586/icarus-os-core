/**
 * @file    task.h
 * @brief   ICARUS OS Task Management - Legacy Compatibility Header
 * @version 0.1.0
 *
 * @details This header provides backward compatibility with existing code.
 *          New code should include "icarus/icarus_task.h" directly.
 *
 * @deprecated Use "icarus/icarus_task.h" for new development
 *
 * @author  Souham Biswas
 * @date    2026
 *
 * @copyright Copyright (c) 2026 ICARUS Project
 *            https://github.com/ironhide23586/icarus-os-core
 *            Licensed under MIT License
 */

#ifndef __ICARUS_TASK_H__
#define __ICARUS_TASK_H__

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * INCLUDE NEW MODULAR HEADERS
 * ========================================================================= */

#include "icarus/icarus_config.h"
#include "icarus/icarus_types.h"
#include "icarus/icarus_task.h"

/* ============================================================================
 * LEGACY INCLUDES (for existing code compatibility)
 * ========================================================================= */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>

/* HAL and peripheral includes */
#include "main.h"
#include "bsp/bsp_led.h"
#include "usbd_cdc_if.h"
#include "usbd_def.h"
#include "cmsis_gcc.h"

/* ============================================================================
 * LEGACY DEFINES (mapped to new names)
 * ========================================================================= */

/* These are now defined in icarus_config.h and icarus_task.h */
/* Kept here for reference - actual definitions come from includes above */

/*
 * Configuration constants (from icarus_config.h):
 *   ICARUS_MAX_TASKS           -> MAX_TASKS
 *   ICARUS_MAX_SEMAPHORES      -> MAX_SEMAPHORES
 *   ICARUS_MAX_MESSAGE_QUEUES  -> MAX_MESSAGE_QUEUES
 *   ICARUS_MAX_MESSAGE_BYTES   -> MAX_MESSAGE_BUFFER_BYTES
 *   ICARUS_STACK_WORDS         -> STACK_WORDS
 *   ICARUS_MAX_TASK_NAME_LEN   -> MAX_TASK_NAME_LENGTH
 *
 * Type definitions (from icarus_types.h):
 *   icarus_task_t              -> task_t
 *   icarus_semaphore_t         -> semaphore_t
 *   icarus_pipe_t              -> message_pipe_t
 *   icarus_task_state_t        -> task_state_t
 *   icarus_task_priority_t     -> task_pri_t
 *
 * State constants:
 *   TASK_STATE_COLD            -> TASK_COLD
 *   TASK_STATE_RUNNING         -> TASK_RUNNING
 *   TASK_STATE_READY           -> TASK_READY
 *   TASK_STATE_BLOCKED         -> TASK_BLOCKED
 *   TASK_STATE_KILLED          -> TASK_KILLED
 *   TASK_STATE_FINISHED        -> TASK_FINISHED
 *
 * Priority constants:
 *   TASK_PRIORITY_LOW          -> PRI_LOW
 *   TASK_PRIORITY_MED          -> PRI_MED
 *   TASK_PRIORITY_HIGH         -> PRI_HIGH
 */

/* Additional legacy defines */
#define CPU_VREGISTERS_SIZE     16

#ifdef __cplusplus
}
#endif

#endif /* __ICARUS_TASK_H__ */
