/*
 * stress_test.h
 *
 *  Created on: Jan 29, 2026
 *      Author: Souham Biswas
 *      GitHub: https://github.com/ironhide23586/icarus-os-core
 *
 *  Stress test suite for ICARUS OS kernel primitives
 */

#ifndef STRESS_TEST_H_
#define STRESS_TEST_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

// ============================================================================
// STRESS TEST CONFIGURATION
// ============================================================================

// Semaphore stress test config
#define STRESS_SEM_IDX_BASE       4      // Start after demo semaphores
#define STRESS_SEM_COUNT          4      // Number of semaphores to stress
#define STRESS_SEM_CAPACITY       5      // Small capacity = more contention

// Pipe stress test config  
#define STRESS_PIPE_IDX_BASE      4      // Start after demo pipes
#define STRESS_PIPE_COUNT         4      // Number of pipes to stress
#define STRESS_PIPE_CAPACITY      8      // Small capacity = more blocking (was 16)

// Timing config (aggressive = more stress)
#define STRESS_FAST_DELAY         5      // Very fast operations (was 10)
#define STRESS_MED_DELAY          25     // Medium speed (was 50)
#define STRESS_SLOW_DELAY         50     // Slower operations (was 100)
#define STRESS_BURST_COUNT        8      // Operations per burst (was 5)

// Display rows for stress test visualization
#define ROW_STRESS_HEADER         27
#define ROW_STRESS_SEM_START      28
#define ROW_STRESS_PIPE_START     33
#define ROW_STRESS_STATS          38

// Statistics tracking
typedef struct {
    volatile uint32_t sem_feeds;
    volatile uint32_t sem_consumes;
    volatile uint32_t sem_contentions;
    volatile uint32_t pipe_sends;
    volatile uint32_t pipe_recvs;
    volatile uint32_t pipe_full_waits;
    volatile uint32_t pipe_empty_waits;
    volatile uint32_t task_yields;
    volatile uint32_t task_sleeps;
    volatile uint32_t max_sem_wait_ticks;
    volatile uint32_t max_pipe_wait_ticks;
    // Verification error counters
    volatile uint32_t seq_errors;        // Out-of-order messages
    volatile uint32_t data_errors;       // Data corruption detected
    volatile uint32_t overflow_errors;   // Semaphore/pipe overflow
    volatile uint32_t underflow_errors;  // Semaphore/pipe underflow
} stress_stats_t;

extern stress_stats_t g_stress_stats;

// ============================================================================
// PUBLIC API
// ============================================================================

/**
 * @brief Initialize and register all stress test tasks
 * 
 * Creates multiple competing tasks that hammer:
 * - Semaphores with rapid feed/consume cycles
 * - Message pipes with burst send/receive patterns
 * - Task scheduler with rapid yields and sleeps
 */
void stress_test_init(void);

/**
 * @brief Get current stress test statistics
 * @return Pointer to global stats structure
 */
stress_stats_t* stress_test_get_stats(void);

/**
 * @brief Reset all stress test statistics to zero
 */
void stress_test_reset_stats(void);

#ifdef __cplusplus
}
#endif

#endif /* STRESS_TEST_H_ */
