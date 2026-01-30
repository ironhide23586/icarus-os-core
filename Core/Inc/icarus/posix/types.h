/**
 * @file    types.h
 * @brief   ICARUS OS POSIX Type Definitions
 * @version 0.1.0
 *
 * @details POSIX.1-2017 compatible type definitions.
 *
 * @see     IEEE Std 1003.1-2017 (POSIX.1)
 * @see     https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/sys_types.h.html
 *
 * @author  Souham Biswas
 * @date    2026
 *
 * @copyright Copyright (c) 2026 ICARUS Project
 */

#ifndef ICARUS_POSIX_TYPES_H
#define ICARUS_POSIX_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

/* ============================================================================
 * POSIX TYPE DEFINITIONS
 * ========================================================================= */

/** 
 * @brief Signed size type (for return values that can be -1) 
 * @note  Only define if not already defined by newlib/system headers
 */
#ifndef _SSIZE_T_DECLARED
#ifndef __ssize_t_defined
typedef int32_t     ssize_t;
#define __ssize_t_defined
#endif
#define _SSIZE_T_DECLARED
#endif

/** @brief Microseconds type for usleep() */
typedef uint32_t    useconds_t;

/** @brief Process ID type */
typedef int16_t     pid_t;

/** @brief User ID type */
typedef uint16_t    uid_t;

/** @brief Group ID type */
typedef uint16_t    gid_t;

/** @brief File offset type */
typedef int32_t     off_t;

/** @brief File mode type */
typedef uint16_t    mode_t;

/** @brief Device ID type */
typedef uint16_t    dev_t;

/** @brief Inode number type */
typedef uint32_t    ino_t;

/** @brief Link count type */
typedef uint16_t    nlink_t;

/** @brief Block size type */
typedef int32_t     blksize_t;

/** @brief Block count type */
typedef int32_t     blkcnt_t;

/** @brief Clock tick type */
typedef uint32_t    clock_t;

/** @brief Time type (seconds since epoch) */
typedef int32_t     time_t;

/** @brief Microseconds type */
typedef int32_t     suseconds_t;

/** @brief Timer ID type */
typedef int32_t     timer_t;

/** @brief Clock ID type */
typedef int32_t     clockid_t;

/* Clock IDs */
#define CLOCK_REALTIME              0   /**< System-wide realtime clock */
#define CLOCK_MONOTONIC             1   /**< Monotonic clock (OS ticks) */
#define CLOCK_PROCESS_CPUTIME_ID    2   /**< Process CPU time */
#define CLOCK_THREAD_CPUTIME_ID     3   /**< Thread CPU time */

#ifdef __cplusplus
}
#endif

#endif /* ICARUS_POSIX_TYPES_H */
