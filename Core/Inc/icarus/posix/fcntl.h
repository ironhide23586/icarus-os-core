/**
 * @file    fcntl.h
 * @brief   ICARUS OS POSIX File Control Definitions
 * @version 0.1.0
 *
 * @details POSIX.1-2017 compatible file control flags.
 *          Used primarily for message queue open flags.
 *
 * @see     IEEE Std 1003.1-2017 (POSIX.1)
 *
 * @author  Souham Biswas
 * @date    2026
 *
 * @copyright Copyright (c) 2026 ICARUS Project
 */

#ifndef ICARUS_POSIX_FCNTL_H
#define ICARUS_POSIX_FCNTL_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * FILE ACCESS MODES
 * ========================================================================= */

#define O_RDONLY        0x0000      /**< Open for reading only */
#define O_WRONLY        0x0001      /**< Open for writing only */
#define O_RDWR          0x0002      /**< Open for reading and writing */
#define O_ACCMODE       0x0003      /**< Mask for access mode bits */

/* ============================================================================
 * FILE CREATION FLAGS
 * ========================================================================= */

#define O_CREAT         0x0100      /**< Create file if it doesn't exist */
#define O_EXCL          0x0200      /**< Exclusive create (fail if exists) */
#define O_NOCTTY        0x0400      /**< Don't assign controlling terminal */
#define O_TRUNC         0x0800      /**< Truncate to zero length */

/* ============================================================================
 * FILE STATUS FLAGS
 * ========================================================================= */

#define O_APPEND        0x1000      /**< Append mode */
#define O_NONBLOCK      0x2000      /**< Non-blocking mode */
#define O_SYNC          0x4000      /**< Synchronous writes */
#define O_DSYNC         0x8000      /**< Synchronous data writes */

#ifdef __cplusplus
}
#endif

#endif /* ICARUS_POSIX_FCNTL_H */
