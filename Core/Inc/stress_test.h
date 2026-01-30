/**
 * @file    stress_test.h
 * @brief   ICARUS OS Stress Test Suite Interface
 * @version 0.1.0
 *
 * @details Provides high-contention stress testing for kernel primitives.
 *          Creates multiple competing tasks that hammer semaphores, pipes,
 *          and the scheduler to verify correctness under load.
 *
 * @par Test Categories:
 *      - Semaphore stress: Rapid feed/consume with contention
 *      - Pipe stress: High-throughput message passing
 *      - Scheduler stress: Rapid yields and variable sleep patterns
 *
 * @par Verification:
 *      The stress test tracks error counters for:
 *      - Sequence errors (out-of-order messages)
 *      - Data corruption
 *      - Overflow/underflow conditions
 *
 * @see     docs/do178c/verification/test_traceability.md
 *
 * @author  Souham Biswas
 * @date    2026
 *
 * @copyright Copyright (c) 2026 ICARUS Project
 *            https://github.com/ironhide23586/icarus-os-core
 *            Licensed under MIT License
 */

#ifndef STRESS_TEST_H_
#define STRESS_TEST_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/* ============================================================================
 * STRESS TEST CONFIGURATION
 * ========================================================================= */

/**
 * @defgroup STRESS_CONFIG Stress Test Configuration
 * @brief    Tunable parameters for stress testing intensity
 * @{
 */

/** @brief Starting semaphore index for stress tests (after demo semaphores) */
#define STRESS_SEM_IDX_BASE       4

/** @brief Number of semaphores used in stress testing */
#define STRESS_SEM_COUNT          4

/** @brief Semaphore capacity (smaller = more contention) */
#define STRESS_SEM_CAPACITY       5

/** @brief Starting pipe index for stress tests (after demo pipes) */
#define STRESS_PIPE_IDX_BASE      4

/** @brief Number of pipes used in stress testing */
#define STRESS_PIPE_COUNT         4

/** @brief Pipe capacity in bytes (smaller = more blocking) */
#define STRESS_PIPE_CAPACITY      8

/** @brief Delay for fast stress operations (ticks) */
#define STRESS_FAST_DELAY         5

/** @brief Delay for medium stress operations (ticks) */
#define STRESS_MED_DELAY          25

/** @brief Delay for slow stress operations (ticks) */
#define STRESS_SLOW_DELAY         50

/** @brief Number of operations per burst */
#define STRESS_BURST_COUNT        8

/** @} */ /* End of STRESS_CONFIG */

/* ============================================================================
 * DISPLAY CONFIGURATION
 * ========================================================================= */

/** @brief Terminal row for stress test header */
#define ROW_STRESS_HEADER         27

/** @brief Starting row for semaphore stress display */
#define ROW_STRESS_SEM_START      28

/** @brief Starting row for pipe stress display */
#define ROW_STRESS_PIPE_START     33

/** @brief Row for statistics display */
#define ROW_STRESS_STATS          38

/* ============================================================================
 * STATISTICS STRUCTURE
 * ========================================================================= */

/**
 * @brief   Stress test statistics tracking structure
 *
 * @details Tracks operation counts and error conditions during stress testing.
 *          All fields are volatile for safe access from multiple tasks.
 */
typedef struct {
    volatile uint32_t sem_feeds;          /**< Total semaphore feed operations */
    volatile uint32_t sem_consumes;       /**< Total semaphore consume operations */
    volatile uint32_t sem_contentions;    /**< Semaphore contention events */
    volatile uint32_t pipe_sends;         /**< Total pipe send operations */
    volatile uint32_t pipe_recvs;         /**< Total pipe receive operations */
    volatile uint32_t pipe_full_waits;    /**< Times sender blocked on full pipe */
    volatile uint32_t pipe_empty_waits;   /**< Times receiver blocked on empty pipe */
    volatile uint32_t task_yields;        /**< Total voluntary yields */
    volatile uint32_t task_sleeps;        /**< Total sleep operations */
    volatile uint32_t max_sem_wait_ticks; /**< Maximum semaphore wait time */
    volatile uint32_t max_pipe_wait_ticks;/**< Maximum pipe wait time */
    volatile uint32_t seq_errors;         /**< Out-of-order message errors */
    volatile uint32_t data_errors;        /**< Data corruption errors */
    volatile uint32_t overflow_errors;    /**< Overflow condition errors */
    volatile uint32_t underflow_errors;   /**< Underflow condition errors */
} stress_stats_t;

/** @brief Global stress test statistics (accessible from all tasks) */
extern stress_stats_t g_stress_stats;

/* ============================================================================
 * STRESS TEST API
 * ========================================================================= */

/**
 * @brief   Initialize and register all stress test tasks
 *
 * @details Creates 19 competing tasks that stress test:
 *          - 4 semaphores with various access patterns
 *          - 4 message pipes with different throughput profiles
 *          - Scheduler with rapid yields and variable sleeps
 *
 * @pre     os_init() must have been called
 * @pre     demo_tasks_init() should be called first (uses indices 0-3)
 * @post    Stress test tasks registered and ready for scheduling
 *
 * @note    Stress testing significantly increases CPU load
 */
void stress_test_init(void);

/**
 * @brief   Get pointer to stress test statistics
 *
 * @return  Pointer to global statistics structure
 */
stress_stats_t* stress_test_get_stats(void);

/**
 * @brief   Reset all stress test statistics to zero
 *
 * @details Clears all operation counters and error counters.
 *          Useful for starting a fresh measurement period.
 */
void stress_test_reset_stats(void);

#ifdef __cplusplus
}
#endif

#endif /* STRESS_TEST_H_ */
