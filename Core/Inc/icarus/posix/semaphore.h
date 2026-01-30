/**
 * @file    semaphore.h
 * @brief   ICARUS OS POSIX Semaphores API
 * @version 0.1.0
 *
 * @details POSIX.1-2017 (IEEE Std 1003.1) compatible semaphore interface.
 *          Maps to native ICARUS counting semaphores.
 *
 * @par Supported Features:
 *      - Unnamed semaphores (sem_init, sem_destroy)
 *      - Wait operations (sem_wait, sem_trywait, sem_timedwait)
 *      - Post operation (sem_post)
 *      - Value query (sem_getvalue)
 *
 * @par Limitations:
 *      - Named semaphores (sem_open, sem_close, sem_unlink) not supported
 *      - Process-shared semaphores not supported (single process RTOS)
 *
 * @see     IEEE Std 1003.1-2017 (POSIX.1)
 * @see     https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/semaphore.h.html
 *
 * @author  Souham Biswas
 * @date    2026
 *
 * @copyright Copyright (c) 2026 ICARUS Project
 */

#ifndef ICARUS_POSIX_SEMAPHORE_H
#define ICARUS_POSIX_SEMAPHORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "icarus/posix/types.h"
#include "icarus/posix/errno.h"
#include "icarus/posix/time.h"

/* ============================================================================
 * SEMAPHORE TYPES
 * ========================================================================= */

/**
 * @brief Semaphore type
 *
 * @note Internal structure - do not access members directly
 */
typedef struct {
    uint8_t     __idx;          /**< Internal: kernel semaphore index */
    uint8_t     __valid;        /**< Internal: initialization flag */
} sem_t;

/** @brief Failed semaphore initialization indicator */
#define SEM_FAILED      ((sem_t *)0)

/** @brief Maximum semaphore value */
#define SEM_VALUE_MAX   UINT32_MAX

/* ============================================================================
 * SEMAPHORE FUNCTIONS
 * ========================================================================= */

/**
 * @brief Initialize an unnamed semaphore
 *
 * @param[out] sem      Semaphore to initialize
 * @param[in]  pshared  Process-shared flag (must be 0, not supported)
 * @param[in]  value    Initial semaphore value
 *
 * @retval 0   Success
 * @retval -1  Error (check errno)
 *
 * @par Errors:
 *      - EINVAL: pshared != 0 or value > SEM_VALUE_MAX
 *      - ENOSPC: No semaphore slots available
 */
int sem_init(sem_t *sem, int pshared, unsigned int value);

/**
 * @brief Destroy an unnamed semaphore
 *
 * @param[in,out] sem  Semaphore to destroy
 *
 * @retval 0   Success
 * @retval -1  Error (check errno)
 *
 * @par Errors:
 *      - EINVAL: Invalid semaphore
 *      - EBUSY: Threads waiting on semaphore
 */
int sem_destroy(sem_t *sem);

/**
 * @brief Lock (decrement) a semaphore
 *
 * @details Decrements the semaphore. If the value is zero, blocks until
 *          the semaphore becomes positive.
 *
 * @param[in] sem  Semaphore to lock
 *
 * @retval 0   Success
 * @retval -1  Error (check errno)
 *
 * @par Errors:
 *      - EINVAL: Invalid semaphore
 *      - EINTR: Interrupted by signal (not applicable in ICARUS)
 *
 * @note Maps to semaphore_consume()
 */
int sem_wait(sem_t *sem);

/**
 * @brief Try to lock a semaphore without blocking
 *
 * @param[in] sem  Semaphore to lock
 *
 * @retval 0   Success (semaphore was decremented)
 * @retval -1  Error (check errno)
 *
 * @par Errors:
 *      - EINVAL: Invalid semaphore
 *      - EAGAIN: Semaphore value is zero (would block)
 */
int sem_trywait(sem_t *sem);

/**
 * @brief Lock a semaphore with timeout
 *
 * @param[in] sem      Semaphore to lock
 * @param[in] abstime  Absolute timeout (CLOCK_REALTIME)
 *
 * @retval 0   Success
 * @retval -1  Error (check errno)
 *
 * @par Errors:
 *      - EINVAL: Invalid semaphore or timeout
 *      - ETIMEDOUT: Timeout expired
 */
int sem_timedwait(sem_t *sem, const struct timespec *abstime);

/**
 * @brief Unlock (increment) a semaphore
 *
 * @details Increments the semaphore, potentially waking a blocked thread.
 *
 * @param[in] sem  Semaphore to unlock
 *
 * @retval 0   Success
 * @retval -1  Error (check errno)
 *
 * @par Errors:
 *      - EINVAL: Invalid semaphore
 *      - EOVERFLOW: Semaphore value would exceed SEM_VALUE_MAX
 *
 * @note Maps to semaphore_feed()
 */
int sem_post(sem_t *sem);

/**
 * @brief Get semaphore value
 *
 * @param[in]  sem   Semaphore to query
 * @param[out] sval  Current value
 *
 * @retval 0   Success
 * @retval -1  Error (check errno)
 *
 * @par Errors:
 *      - EINVAL: Invalid semaphore
 *
 * @note If threads are blocked, *sval may be zero or negative
 *       (negative indicates number of waiters - not supported in ICARUS)
 */
int sem_getvalue(sem_t *sem, int *sval);

#ifdef __cplusplus
}
#endif

#endif /* ICARUS_POSIX_SEMAPHORE_H */
