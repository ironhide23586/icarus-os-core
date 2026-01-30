/**
 * @file    unistd.h
 * @brief   ICARUS OS POSIX Standard Symbolic Constants and Types
 * @version 0.1.0
 *
 * @details POSIX.1-2017 compatible unistd interface.
 *          Provides sleep functions and process-related stubs.
 *
 * @see     IEEE Std 1003.1-2017 (POSIX.1)
 * @see     https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/unistd.h.html
 *
 * @author  Souham Biswas
 * @date    2026
 *
 * @copyright Copyright (c) 2026 ICARUS Project
 */

#ifndef ICARUS_POSIX_UNISTD_H
#define ICARUS_POSIX_UNISTD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "icarus/posix/types.h"

/* ============================================================================
 * POSIX VERSION MACROS
 * ========================================================================= */

/** @brief POSIX.1-2017 version */
#define _POSIX_VERSION          200809L

/** @brief POSIX.1-2017 C language binding */
#define _POSIX_C_SOURCE         200809L

/* ============================================================================
 * SUPPORTED POSIX OPTIONS
 * ========================================================================= */

/** @brief Semaphores supported */
#define _POSIX_SEMAPHORES       200809L

/** @brief Message queues supported */
#define _POSIX_MESSAGE_PASSING  200809L

/** @brief Threads supported */
#define _POSIX_THREADS          200809L

/** @brief Monotonic clock supported */
#define _POSIX_MONOTONIC_CLOCK  200809L

/* ============================================================================
 * SLEEP FUNCTIONS
 * ========================================================================= */

/**
 * @brief Sleep for specified seconds
 *
 * @param[in] seconds  Number of seconds to sleep
 *
 * @return Remaining seconds if interrupted, 0 if completed
 *
 * @note Maps to task_active_sleep(seconds * 1000)
 */
unsigned int sleep(unsigned int seconds);

/**
 * @brief Sleep for specified microseconds
 *
 * @param[in] usec  Microseconds to sleep (max 1,000,000)
 *
 * @retval 0   Success
 * @retval -1  Error (EINVAL if usec >= 1,000,000)
 *
 * @note Resolution is 1ms; values < 1000 sleep for 1ms
 */
int usleep(uint32_t usec);

/* ============================================================================
 * PROCESS FUNCTIONS (STUBS)
 * ========================================================================= */

/**
 * @brief Get process ID
 *
 * @return Always returns 1 (single process RTOS)
 */
static inline pid_t getpid(void) { return 1; }

/**
 * @brief Get parent process ID
 *
 * @return Always returns 0 (no parent in RTOS)
 */
static inline pid_t getppid(void) { return 0; }

/**
 * @brief Get user ID
 *
 * @return Always returns 0 (root)
 */
static inline uid_t getuid(void) { return 0; }

/**
 * @brief Get effective user ID
 *
 * @return Always returns 0 (root)
 */
static inline uid_t geteuid(void) { return 0; }

/**
 * @brief Get group ID
 *
 * @return Always returns 0 (root group)
 */
static inline gid_t getgid(void) { return 0; }

/**
 * @brief Get effective group ID
 *
 * @return Always returns 0 (root group)
 */
static inline gid_t getegid(void) { return 0; }

/* ============================================================================
 * STANDARD FILE DESCRIPTORS
 * ========================================================================= */

#define STDIN_FILENO    0       /**< Standard input */
#define STDOUT_FILENO   1       /**< Standard output */
#define STDERR_FILENO   2       /**< Standard error */

/* ============================================================================
 * MISCELLANEOUS CONSTANTS
 * ========================================================================= */

#ifndef NULL
#define NULL            ((void *)0)
#endif

/** @brief Null device path */
#define _PATH_DEVNULL   "/dev/null"

#ifdef __cplusplus
}
#endif

#endif /* ICARUS_POSIX_UNISTD_H */
