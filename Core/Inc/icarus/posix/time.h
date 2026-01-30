/**
 * @file    time.h
 * @brief   ICARUS OS POSIX Time Functions
 * @version 0.1.0
 *
 * @details POSIX.1-2017 compatible time interface.
 *
 * @see     IEEE Std 1003.1-2017 (POSIX.1)
 * @see     https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/time.h.html
 *
 * @author  Souham Biswas
 * @date    2026
 *
 * @copyright Copyright (c) 2026 ICARUS Project
 */

#ifndef ICARUS_POSIX_TIME_H
#define ICARUS_POSIX_TIME_H

#ifdef __cplusplus
extern "C" {
#endif

#include "icarus/posix/types.h"

/* ============================================================================
 * TIME STRUCTURES
 * ========================================================================= */

/**
 * @brief Time specification with nanosecond precision
 */
struct timespec {
    time_t  tv_sec;     /**< Seconds */
    long    tv_nsec;    /**< Nanoseconds (0 to 999,999,999) */
};

/**
 * @brief Time value with microsecond precision
 */
struct timeval {
    time_t      tv_sec;     /**< Seconds */
    suseconds_t tv_usec;    /**< Microseconds (0 to 999,999) */
};

/**
 * @brief Timer specification for interval timers
 */
struct itimerspec {
    struct timespec it_interval;    /**< Timer interval */
    struct timespec it_value;       /**< Initial expiration */
};

/* ============================================================================
 * TIME FUNCTIONS
 * ========================================================================= */

/**
 * @brief Get current time
 *
 * @param[in]  clk_id  Clock identifier (CLOCK_REALTIME or CLOCK_MONOTONIC)
 * @param[out] tp      Time specification
 *
 * @retval 0   Success
 * @retval -1  Error (check errno)
 *
 * @note CLOCK_MONOTONIC maps to os_get_tick_count() with 1ms resolution
 */
int clock_gettime(clockid_t clk_id, struct timespec *tp);

/**
 * @brief Get clock resolution
 *
 * @param[in]  clk_id  Clock identifier
 * @param[out] res     Resolution specification
 *
 * @retval 0   Success
 * @retval -1  Error (check errno)
 *
 * @note ICARUS has 1ms (1,000,000 ns) resolution
 */
int clock_getres(clockid_t clk_id, struct timespec *res);

/**
 * @brief Sleep for specified duration
 *
 * @param[in]  rqtp   Requested sleep time
 * @param[out] rmtp   Remaining time if interrupted (may be NULL)
 *
 * @retval 0   Success (slept for full duration)
 * @retval -1  Error or interrupted (check errno)
 *
 * @note Maps to task_active_sleep()
 */
int nanosleep(const struct timespec *rqtp, struct timespec *rmtp);

/**
 * @brief Sleep for specified number of seconds
 *
 * @param[in] seconds  Seconds to sleep
 *
 * @return Remaining seconds if interrupted, 0 if completed
 */
unsigned int sleep(unsigned int seconds);

/**
 * @brief Sleep for specified number of microseconds
 *
 * @param[in] usec  Microseconds to sleep
 *
 * @retval 0   Success
 * @retval -1  Error (check errno)
 */
int usleep(uint32_t usec);

/* ============================================================================
 * HELPER MACROS
 * ========================================================================= */

/** @brief Nanoseconds per second */
#define NSEC_PER_SEC    1000000000L

/** @brief Nanoseconds per millisecond */
#define NSEC_PER_MSEC   1000000L

/** @brief Nanoseconds per microsecond */
#define NSEC_PER_USEC   1000L

/** @brief Microseconds per second */
#define USEC_PER_SEC    1000000L

/** @brief Milliseconds per second */
#define MSEC_PER_SEC    1000L

/**
 * @brief Convert timespec to milliseconds
 */
#define TIMESPEC_TO_MS(ts) \
    ((uint32_t)((ts)->tv_sec * MSEC_PER_SEC + (ts)->tv_nsec / NSEC_PER_MSEC))

/**
 * @brief Convert milliseconds to timespec
 */
#define MS_TO_TIMESPEC(ms, ts) do { \
    (ts)->tv_sec = (ms) / MSEC_PER_SEC; \
    (ts)->tv_nsec = ((ms) % MSEC_PER_SEC) * NSEC_PER_MSEC; \
} while (0)

#ifdef __cplusplus
}
#endif

#endif /* ICARUS_POSIX_TIME_H */
