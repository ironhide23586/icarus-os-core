/**
 * @file    pthread.h
 * @brief   ICARUS OS POSIX Threads API
 * @version 0.1.0
 *
 * @details POSIX.1-2017 (IEEE Std 1003.1) compatible thread interface.
 *          Maps to native ICARUS kernel primitives.
 *
 * @par Supported Features:
 *      - Thread creation and termination (pthread_create, pthread_exit)
 *      - Thread joining (pthread_join)
 *      - Thread attributes (pthread_attr_*)
 *      - Mutexes (pthread_mutex_*)
 *      - Condition variables (pthread_cond_*)
 *
 * @par Limitations:
 *      - Thread cancellation not supported
 *      - Thread-specific data (TLS) not supported
 *      - Scheduling policies limited to round-robin
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

#ifndef ICARUS_POSIX_PTHREAD_H
#define ICARUS_POSIX_PTHREAD_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include "icarus/posix/types.h"
#include "icarus/posix/errno.h"

/* ============================================================================
 * THREAD TYPES
 * ========================================================================= */

/**
 * @brief Thread identifier
 * @note  Maps to ICARUS task index
 */
typedef uint8_t pthread_t;

/**
 * @brief Thread attributes
 */
typedef struct {
    size_t      stacksize;      /**< Stack size in bytes */
    int         detachstate;    /**< Detach state */
    int         schedpolicy;    /**< Scheduling policy */
    int         priority;       /**< Thread priority */
} pthread_attr_t;

/* Detach state values */
#define PTHREAD_CREATE_JOINABLE     0
#define PTHREAD_CREATE_DETACHED     1

/* Scheduling policies */
#define SCHED_OTHER                 0   /**< Default (round-robin) */
#define SCHED_FIFO                  1   /**< First-in-first-out */
#define SCHED_RR                    2   /**< Round-robin */

/* ============================================================================
 * THREAD ATTRIBUTE FUNCTIONS
 * ========================================================================= */

/**
 * @brief Initialize thread attributes to default values
 *
 * @param[out] attr  Attribute object to initialize
 *
 * @retval 0       Success
 * @retval EINVAL  Invalid argument
 *
 * @par Default Values:
 *      - stacksize: ICARUS_STACK_WORDS * 4
 *      - detachstate: PTHREAD_CREATE_JOINABLE
 *      - schedpolicy: SCHED_RR
 *      - priority: 0 (normal)
 */
int pthread_attr_init(pthread_attr_t *attr);

/**
 * @brief Destroy thread attributes object
 *
 * @param[in,out] attr  Attribute object to destroy
 *
 * @retval 0       Success
 * @retval EINVAL  Invalid argument
 */
int pthread_attr_destroy(pthread_attr_t *attr);

/**
 * @brief Set stack size attribute
 *
 * @param[in,out] attr       Attribute object
 * @param[in]     stacksize  Stack size in bytes
 *
 * @retval 0       Success
 * @retval EINVAL  Invalid argument or size too small
 */
int pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize);

/**
 * @brief Get stack size attribute
 *
 * @param[in]  attr       Attribute object
 * @param[out] stacksize  Stack size in bytes
 *
 * @retval 0       Success
 * @retval EINVAL  Invalid argument
 */
int pthread_attr_getstacksize(const pthread_attr_t *attr, size_t *stacksize);

/**
 * @brief Set detach state attribute
 *
 * @param[in,out] attr         Attribute object
 * @param[in]     detachstate  PTHREAD_CREATE_JOINABLE or PTHREAD_CREATE_DETACHED
 *
 * @retval 0       Success
 * @retval EINVAL  Invalid argument
 */
int pthread_attr_setdetachstate(pthread_attr_t *attr, int detachstate);

/**
 * @brief Get detach state attribute
 *
 * @param[in]  attr         Attribute object
 * @param[out] detachstate  Detach state value
 *
 * @retval 0       Success
 * @retval EINVAL  Invalid argument
 */
int pthread_attr_getdetachstate(const pthread_attr_t *attr, int *detachstate);

/* ============================================================================
 * THREAD CREATION AND TERMINATION
 * ========================================================================= */

/**
 * @brief Create a new thread
 *
 * @param[out] thread       Thread identifier
 * @param[in]  attr         Thread attributes (NULL for defaults)
 * @param[in]  start_routine Thread entry function
 * @param[in]  arg          Argument passed to start_routine
 *
 * @retval 0       Success
 * @retval EAGAIN  Insufficient resources (MAX_TASKS reached)
 * @retval EINVAL  Invalid attributes
 *
 * @note The start_routine signature differs from POSIX:
 *       POSIX: void *(*start_routine)(void *)
 *       ICARUS: void (*start_routine)(void)
 *       Use pthread_create_ex() for full POSIX compatibility.
 */
int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                   void (*start_routine)(void), void *arg);

/**
 * @brief Create a new thread with custom name (ICARUS extension)
 *
 * @param[out] thread        Thread identifier
 * @param[in]  attr          Thread attributes (NULL for defaults)
 * @param[in]  start_routine Thread entry function
 * @param[in]  arg           Argument passed to start_routine
 * @param[in]  name          Thread name for debugging
 *
 * @retval 0       Success
 * @retval EAGAIN  Insufficient resources
 * @retval EINVAL  Invalid attributes
 *
 * @note ICARUS extension - not part of POSIX standard
 */
int pthread_create_named(pthread_t *thread, const pthread_attr_t *attr,
                         void (*start_routine)(void), void *arg,
                         const char *name);

/**
 * @brief Terminate calling thread
 *
 * @param[in] retval  Return value (ignored in current implementation)
 *
 * @note This function does not return
 */
void pthread_exit(void *retval) __attribute__((noreturn));

/**
 * @brief Wait for thread termination
 *
 * @param[in]  thread  Thread to wait for
 * @param[out] retval  Thread return value (always NULL in current impl)
 *
 * @retval 0        Success
 * @retval EINVAL   Invalid thread
 * @retval ESRCH    Thread not found
 * @retval EDEADLK  Deadlock detected (joining self)
 */
int pthread_join(pthread_t thread, void **retval);

/**
 * @brief Detach a thread
 *
 * @param[in] thread  Thread to detach
 *
 * @retval 0       Success
 * @retval EINVAL  Invalid thread
 * @retval ESRCH   Thread not found
 */
int pthread_detach(pthread_t thread);

/**
 * @brief Get calling thread's ID
 *
 * @return Thread identifier of calling thread
 */
pthread_t pthread_self(void);

/**
 * @brief Compare thread IDs
 *
 * @param[in] t1  First thread ID
 * @param[in] t2  Second thread ID
 *
 * @retval Non-zero  Thread IDs are equal
 * @retval 0         Thread IDs are different
 */
int pthread_equal(pthread_t t1, pthread_t t2);

/**
 * @brief Yield processor to another thread
 *
 * @retval 0  Always succeeds
 *
 * @note Maps to os_yield()
 */
int sched_yield(void);

#ifdef __cplusplus
}
#endif

#endif /* ICARUS_POSIX_PTHREAD_H */
