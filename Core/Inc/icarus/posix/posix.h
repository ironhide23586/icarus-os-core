/**
 * @file    posix.h
 * @brief   ICARUS OS POSIX API - Master Include Header
 * @version 0.1.0
 *
 * @details Single include point for all POSIX-compatible APIs.
 *
 * @par Supported POSIX Features:
 *      - Threads (pthread_*)
 *      - Semaphores (sem_*)
 *      - Message Queues (mq_*)
 *      - Time functions (clock_*, nanosleep, sleep, usleep)
 *
 * @par Compliance:
 *      Partial compliance with IEEE Std 1003.1-2017 (POSIX.1).
 *      See individual headers for specific limitations.
 *
 * @par Example Usage:
 * @code
 *     #include "icarus/posix/posix.h"
 *
 *     sem_t my_sem;
 *     pthread_t my_thread;
 *
 *     void thread_func(void) {
 *         sem_wait(&my_sem);
 *         // ... do work ...
 *         sem_post(&my_sem);
 *     }
 *
 *     int main(void) {
 *         sem_init(&my_sem, 0, 1);
 *         pthread_create(&my_thread, NULL, thread_func, NULL);
 *         pthread_join(my_thread, NULL);
 *         sem_destroy(&my_sem);
 *         return 0;
 *     }
 * @endcode
 *
 * @see     IEEE Std 1003.1-2017 (POSIX.1)
 * @see     https://pubs.opengroup.org/onlinepubs/9699919799/
 *
 * @author  Souham Biswas
 * @date    2026
 *
 * @copyright Copyright (c) 2026 ICARUS Project
 *            https://github.com/ironhide23586/icarus-os-core
 *            Licensed under MIT License
 */

#ifndef ICARUS_POSIX_H
#define ICARUS_POSIX_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * POSIX FEATURE TEST MACROS
 * ========================================================================= */

/** @brief Enable POSIX.1-2017 features */
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE     200809L
#endif

/* ============================================================================
 * POSIX HEADERS
 * ========================================================================= */

#include "icarus/posix/types.h"     /* POSIX type definitions */
#include "icarus/posix/errno.h"     /* Error numbers */
#include "icarus/posix/fcntl.h"     /* File control flags */
#include "icarus/posix/time.h"      /* Time functions */
#include "icarus/posix/unistd.h"    /* Standard symbolic constants */
#include "icarus/posix/pthread.h"   /* POSIX threads */
#include "icarus/posix/semaphore.h" /* POSIX semaphores */
#include "icarus/posix/mqueue.h"    /* POSIX message queues */

/* ============================================================================
 * ICARUS-SPECIFIC EXTENSIONS
 * ========================================================================= */

/**
 * @brief Initialize POSIX subsystem
 *
 * @details Must be called after os_init() and before using any POSIX APIs.
 *          Initializes errno and internal state.
 *
 * @note This is an ICARUS extension, not part of POSIX.
 */
void posix_init(void);

/**
 * @brief Get ICARUS task index from pthread_t
 *
 * @param[in] thread  POSIX thread ID
 *
 * @return ICARUS task index
 *
 * @note This is an ICARUS extension for debugging/interop.
 */
static inline uint8_t pthread_to_task_index(pthread_t thread)
{
    return (uint8_t)thread;
}

/**
 * @brief Get pthread_t from ICARUS task index
 *
 * @param[in] task_idx  ICARUS task index
 *
 * @return POSIX thread ID
 *
 * @note This is an ICARUS extension for debugging/interop.
 */
static inline pthread_t task_index_to_pthread(uint8_t task_idx)
{
    return (pthread_t)task_idx;
}

#ifdef __cplusplus
}
#endif

#endif /* ICARUS_POSIX_H */
