/**
 * @file    config.h
 * @brief   ICARUS OS Configuration Header
 * @version 0.1.0
 *
 * @details Central configuration file for ICARUS Real-Time Operating System.
 *          All tunable parameters and feature flags are defined here.
 *
 * @note    Modify this file to customize ICARUS for your target application.
 *
 * @see     docs/do178c/design/SDD.md for Software Design Document
 * @see     docs/do178c/requirements/SRS.md for Software Requirements Specification
 *
 * @author  Souham Biswas
 * @date    2025
 *
 * @copyright Copyright 2025 Souham Biswas
 *            https://github.com/ironhide23586/icarus-os-core
 *            Licensed under the Apache License, Version 2.0
 */

#ifndef ICARUS_CONFIG_H
#define ICARUS_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * VERSION INFORMATION
 * ========================================================================= */

#define ICARUS_VERSION_MAJOR    0
#define ICARUS_VERSION_MINOR    1
#define ICARUS_VERSION_PATCH    0
#define ICARUS_VERSION_STRING   "0.1.0"

/* ============================================================================
 * KERNEL CONFIGURATION
 * ========================================================================= */

/**
 * @defgroup KERNEL_CONFIG Kernel Configuration
 * @brief    Core kernel parameters
 * @{
 */

/**
 * @brief Maximum number of concurrent tasks
 * @note  Each task requires STACK_WORDS * 4 bytes of stack memory
 *        With MAX_TASKS=128 and STACK_WORDS=512: 256KB stack pool
 * @see   STM32H750 has 512KB RAM_D1 available for stacks
 */
#define ICARUS_MAX_TASKS            128

/**
 * @brief Maximum number of semaphores
 * @note  Each semaphore uses 16 bytes in DTCM
 *        64 semaphores = 1KB DTCM usage
 */
#define ICARUS_MAX_SEMAPHORES       64

/**
 * @brief Maximum number of message queues (pipes)
 * @note  Each pipe uses ~140 bytes in DTCM
 *        64 pipes = ~9KB DTCM usage
 */
#define ICARUS_MAX_MESSAGE_QUEUES   64

/**
 * @brief Maximum bytes per message in a pipe
 */
#define ICARUS_MAX_MESSAGE_BYTES    128

/**
 * @brief Stack size per task in 32-bit words
 * @note  256 words = 1KB per task
 *        Must be even for 8-byte alignment (ARM ABI requirement)
 */
#define ICARUS_STACK_WORDS          256

/**
 * @brief Data region size per task in 32-bit words
 * @note  512 words = 2KB per task
 *        Must be even for 8-byte alignment (ARM ABI requirement)
 */
#define ICARUS_DATA_WORDS           512 

/**
 * @brief Number of SysTick interrupts per task time slice
 * @note  At 1ms SysTick, 50 = 50ms time slice
 */
#define ICARUS_TICKS_PER_TASK       50

/**
 * @brief Maximum length of task name string (including null terminator)
 */
#define ICARUS_MAX_TASK_NAME_LEN    32

/** @} */ /* End of KERNEL_CONFIG */

/* ============================================================================
 * MEMORY CONFIGURATION
 * ========================================================================= */

/**
 * @defgroup MEMORY_CONFIG Memory Configuration
 * @brief    Memory region assignments for STM32H750
 * @{
 */

/**
 * @brief Enable DTCM placement for kernel data structures
 * @note  DTCM provides zero wait-state access at 480MHz
 * @see   STM32H750 Reference Manual RM0433, Section 2.3.2
 */
#define ICARUS_USE_DTCM             1

/**
 * @brief Enable ITCM placement for hot-path code
 * @note  ITCM provides zero wait-state instruction fetch
 * @see   STM32H750 Reference Manual RM0433, Section 2.3.1
 */
#define ICARUS_USE_ITCM             1

/** @} */ /* End of MEMORY_CONFIG */

/* ============================================================================
 * DEBUG CONFIGURATION
 * ========================================================================= */

/**
 * @defgroup DEBUG_CONFIG Debug Configuration
 * @brief    Debug and diagnostic options
 * @{
 */

/**
 * @brief Enable runtime assertions
 */
#define ICARUS_ENABLE_ASSERTS       1

/**
 * @brief Enable kernel statistics collection
 */
#define ICARUS_ENABLE_STATS         0

/**
 * @brief Print buffer size for USB CDC output
 * @note  Must not exceed 64 bytes for USB FS CDC packet sizing
 */
#define ICARUS_PRINT_BUFFER_BYTES   64

/**
 * @brief Maximum retries for print operations
 */
#define ICARUS_MAX_PRINT_RETRIES    4

/** @} */ /* End of DEBUG_CONFIG */

/* ============================================================================
 * DISPLAY CONFIGURATION
 * ========================================================================= */

/**
 * @defgroup DISPLAY_CONFIG Display Configuration
 * @brief    Terminal display and visualization options
 * @{
 */

/**
 * @brief Enable heartbeat LED visualization on terminal
 */
#define ICARUS_ENABLE_HEARTBEAT_VIS 1

/**
 * @brief Heartbeat LED on duration in ticks
 */
#define ICARUS_HEARTBEAT_ON_TICKS   437

/**
 * @brief Heartbeat LED off duration in ticks
 */
#define ICARUS_HEARTBEAT_OFF_TICKS  479

/** @} */ /* End of DISPLAY_CONFIG */

/* ============================================================================
 * COMPILE-TIME VALIDATION
 * ========================================================================= */

#if (ICARUS_STACK_WORDS % 2) != 0
#error "ICARUS_STACK_WORDS must be even for 8-byte stack alignment (ARM ABI)"
#endif

#if (ICARUS_PRINT_BUFFER_BYTES > 64)
#error "ICARUS_PRINT_BUFFER_BYTES must be <= 64 for USB FS CDC packet sizing"
#endif

#if (ICARUS_MAX_TASKS > 255)
#error "ICARUS_MAX_TASKS must be <= 255 (uint8_t index)"
#endif

#ifdef __cplusplus
}
#endif

#endif /* ICARUS_CONFIG_H */
