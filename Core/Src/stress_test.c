/*
 * stress_test.c
 *
 *  Created on: Jan 29, 2026
 *      Author: Souham Biswas
 *      GitHub: https://github.com/ironhide23586/icarus-os-core
 *
 *  Stress test suite for ICARUS OS kernel primitives
 *  
 *  This module creates high-contention scenarios to test:
 *  - Semaphore operations under heavy load
 *  - Message pipe throughput and blocking behavior
 *  - Task scheduler fairness and responsiveness
 *  - Critical section handling
 */

#include "stress_test.h"
#include "icarus/icarus_task.h"
#include "bsp/display.h"

// Global statistics
stress_stats_t g_stress_stats = {0};

// Display row assignments for stress test visualization
// Demo tasks use rows 10-26, history panels on right side
// Stress tests go below everything
#define STRESS_ROW_HEADER     42
#define STRESS_ROW_SEM_0      44   // sem_hammer_fast_0
#define STRESS_ROW_SEM_1      46   // sem_stress_med_0
#define STRESS_ROW_SEM_2      48   // sem_stress_slow
#define STRESS_ROW_SEM_3      50   // sem_mutex_stress_0
#define STRESS_ROW_PIPE_0     52   // pipe_flood
#define STRESS_ROW_PIPE_1     55   // pipe_multi_prod (needs 4 rows)
#define STRESS_ROW_PIPE_2     60   // pipe_varsz
#define STRESS_ROW_SCHED      63   // scheduler stress
#define STRESS_ROW_STATS      67   // statistics line 1
#define STRESS_ROW_STATS2     68   // statistics line 2

// Vertical bar columns for semaphore visualization (far right)
#define STRESS_VBAR_COL_0     100
#define STRESS_VBAR_COL_1     106
#define STRESS_VBAR_COL_2     112
#define STRESS_VBAR_COL_3     118

// ANSI helpers
#define ANSI_GOTO(row, col) printf("\033[%d;%dH", (row), (col))
#define ANSI_CLEAR_LINE()   printf("\033[K")

// ============================================================================
// SEMAPHORE STRESS TASKS
// Multiple tasks competing for the same semaphores
// ============================================================================

// Semaphore hammer - rapid feed/consume on sem 4
static void sem_hammer_fast_0(void) {
    const char* task_name = os_get_current_task_name();
    const uint8_t sem_idx = STRESS_SEM_IDX_BASE;
    uint32_t ops = 0;
    
    while (1) {
        uint32_t period_start = os_get_tick_count();
        
        // Burst feed
        for (uint8_t i = 0; i < STRESS_BURST_COUNT; i++) {
            uint32_t start = os_get_tick_count();
            semaphore_feed(sem_idx);
            uint32_t wait = os_get_tick_count() - start;
            if (wait > g_stress_stats.max_sem_wait_ticks) {
                g_stress_stats.max_sem_wait_ticks = wait;
            }
            g_stress_stats.sem_feeds++;
            ops++;
        }
        
        // Render progress bar
        uint32_t elapsed = os_get_tick_count() - period_start;
        display_render_producer(STRESS_ROW_SEM_0, task_name, elapsed, STRESS_FAST_DELAY * 2,
                               (uint8_t)(ops & 0xFF), true);
        
        // Render semaphore vbar
        display_render_vbar(STRESS_ROW_HEADER + 3, STRESS_VBAR_COL_0,
                           semaphore_get_count(sem_idx),
                           semaphore_get_max_count(sem_idx));
        
        task_active_sleep(STRESS_FAST_DELAY);
        g_stress_stats.task_sleeps++;
        
        // Burst consume
        for (uint8_t i = 0; i < STRESS_BURST_COUNT; i++) {
            uint32_t start = os_get_tick_count();
            semaphore_consume(sem_idx);
            uint32_t wait = os_get_tick_count() - start;
            if (wait > g_stress_stats.max_sem_wait_ticks) {
                g_stress_stats.max_sem_wait_ticks = wait;
            }
            g_stress_stats.sem_consumes++;
            ops++;
        }
        
        task_active_sleep(STRESS_FAST_DELAY);
        g_stress_stats.task_sleeps++;
    }
}

// Competing hammer on same semaphore
static void sem_hammer_fast_1(void) {
    const char* task_name = os_get_current_task_name();
    const uint8_t sem_idx = STRESS_SEM_IDX_BASE;
    uint32_t ops = 0;
    
    while (1) {
        uint32_t period_start = os_get_tick_count();
        
        // Opposite pattern - consume first, then feed
        for (uint8_t i = 0; i < STRESS_BURST_COUNT; i++) {
            semaphore_consume(sem_idx);
            g_stress_stats.sem_consumes++;
            ops++;
        }
        
        uint32_t elapsed = os_get_tick_count() - period_start;
        display_render_consumer(STRESS_ROW_SEM_0 + 1, task_name, elapsed, STRESS_FAST_DELAY * 2 + 6,
                               (uint8_t)(ops & 0xFF), true);
        
        task_active_sleep(STRESS_FAST_DELAY + 3);
        g_stress_stats.task_sleeps++;
        
        for (uint8_t i = 0; i < STRESS_BURST_COUNT; i++) {
            semaphore_feed(sem_idx);
            g_stress_stats.sem_feeds++;
            ops++;
        }
        
        task_active_sleep(STRESS_FAST_DELAY + 3);
        g_stress_stats.task_sleeps++;
    }
}

// Medium speed semaphore stress on sem 5
static void sem_stress_med_0(void) {
    const char* task_name = os_get_current_task_name();
    const uint8_t sem_idx = STRESS_SEM_IDX_BASE + 1;
    uint32_t ops = 0;
    
    while (1) {
        uint32_t period_start = os_get_tick_count();
        uint32_t count = semaphore_get_count(sem_idx);
        uint32_t max = semaphore_get_max_count(sem_idx);
        
        if (count < max / 2) {
            for (uint8_t i = 0; i < 3; i++) {
                semaphore_feed(sem_idx);
                g_stress_stats.sem_feeds++;
                ops++;
            }
        } else {
            for (uint8_t i = 0; i < 3; i++) {
                semaphore_consume(sem_idx);
                g_stress_stats.sem_consumes++;
                ops++;
            }
        }
        
        uint32_t elapsed = os_get_tick_count() - period_start;
        display_render_bar(STRESS_ROW_SEM_1, task_name, elapsed, STRESS_MED_DELAY);
        display_render_vbar(STRESS_ROW_HEADER + 3, STRESS_VBAR_COL_1,
                           semaphore_get_count(sem_idx),
                           semaphore_get_max_count(sem_idx));
        
        task_active_sleep(STRESS_MED_DELAY);
        g_stress_stats.task_sleeps++;
    }
}

// Competing medium speed task
static void sem_stress_med_1(void) {
    const char* task_name = os_get_current_task_name();
    const uint8_t sem_idx = STRESS_SEM_IDX_BASE + 1;
    uint32_t ops = 0;
    
    while (1) {
        uint32_t period_start = os_get_tick_count();
        
        semaphore_consume(sem_idx);
        g_stress_stats.sem_consumes++;
        ops++;
        
        os_yield();
        g_stress_stats.task_yields++;
        
        semaphore_feed(sem_idx);
        g_stress_stats.sem_feeds++;
        ops++;
        
        uint32_t elapsed = os_get_tick_count() - period_start;
        display_render_bar(STRESS_ROW_SEM_1 + 1, task_name, elapsed, STRESS_MED_DELAY + 7);
        
        task_active_sleep(STRESS_MED_DELAY + 7);
        g_stress_stats.task_sleeps++;
    }
}

// Slow but steady semaphore stress on sem 6
static void sem_stress_slow(void) {
    const char* task_name = os_get_current_task_name();
    const uint8_t sem_idx = STRESS_SEM_IDX_BASE + 2;
    uint8_t pattern = 0;
    
    while (1) {
        uint32_t period_start = os_get_tick_count();
        
        switch (pattern % 4) {
            case 0:
                semaphore_feed(sem_idx);
                semaphore_feed(sem_idx);
                g_stress_stats.sem_feeds += 2;
                break;
            case 1:
                semaphore_consume(sem_idx);
                g_stress_stats.sem_consumes++;
                break;
            case 2:
                semaphore_feed(sem_idx);
                g_stress_stats.sem_feeds++;
                break;
            case 3:
                semaphore_consume(sem_idx);
                semaphore_consume(sem_idx);
                g_stress_stats.sem_consumes += 2;
                break;
        }
        pattern++;
        
        uint32_t elapsed = os_get_tick_count() - period_start;
        display_render_bar(STRESS_ROW_SEM_2, task_name, elapsed, STRESS_SLOW_DELAY);
        display_render_vbar(STRESS_ROW_HEADER + 3, STRESS_VBAR_COL_2,
                           semaphore_get_count(sem_idx),
                           semaphore_get_max_count(sem_idx));
        
        task_active_sleep(STRESS_SLOW_DELAY);
        g_stress_stats.task_sleeps++;
    }
}

// Binary semaphore stress (mutex-like) on sem 7
static void sem_mutex_stress_0(void) {
    const char* task_name = os_get_current_task_name();
    const uint8_t sem_idx = STRESS_SEM_IDX_BASE + 3;
    volatile uint32_t critical_counter = 0;
    uint32_t ops = 0;
    
    while (1) {
        uint32_t period_start = os_get_tick_count();
        
        semaphore_consume(sem_idx);
        g_stress_stats.sem_consumes++;
        
        for (volatile int i = 0; i < 100; i++) {
            critical_counter++;
        }
        
        semaphore_feed(sem_idx);
        g_stress_stats.sem_feeds++;
        ops++;
        
        uint32_t elapsed = os_get_tick_count() - period_start;
        display_render_producer(STRESS_ROW_SEM_3, task_name, elapsed, STRESS_FAST_DELAY,
                               (uint8_t)(ops & 0xFF), true);
        display_render_vbar(STRESS_ROW_HEADER + 3, STRESS_VBAR_COL_3,
                           semaphore_get_count(sem_idx),
                           semaphore_get_max_count(sem_idx));
        
        os_yield();
        g_stress_stats.task_yields++;
    }
}

static void sem_mutex_stress_1(void) {
    const char* task_name = os_get_current_task_name();
    const uint8_t sem_idx = STRESS_SEM_IDX_BASE + 3;
    volatile uint32_t critical_counter = 0;
    uint32_t ops = 0;
    
    while (1) {
        uint32_t period_start = os_get_tick_count();
        
        semaphore_consume(sem_idx);
        g_stress_stats.sem_consumes++;
        
        for (volatile int i = 0; i < 50; i++) {
            critical_counter++;
        }
        
        semaphore_feed(sem_idx);
        g_stress_stats.sem_feeds++;
        ops++;
        
        uint32_t elapsed = os_get_tick_count() - period_start;
        display_render_consumer(STRESS_ROW_SEM_3 + 1, task_name, elapsed, STRESS_FAST_DELAY,
                               (uint8_t)(ops & 0xFF), true);
        
        task_active_sleep(STRESS_FAST_DELAY);
        g_stress_stats.task_sleeps++;
    }
}


// ============================================================================
// MESSAGE PIPE STRESS TASKS
// High-throughput message passing with various patterns
// ============================================================================

// Fast pipe flood - pipe 4
static void pipe_flood_sender(void) {
    const char* task_name = os_get_current_task_name();
    const uint8_t pipe_idx = STRESS_PIPE_IDX_BASE;
    uint8_t msg[4] = {0xDE, 0xAD, 0xBE, 0xEF};
    uint32_t seq = 0;
    uint32_t send_tick = 0;
    
    while (1) {
        uint32_t period_start = os_get_tick_count();
        
        msg[0] = (uint8_t)(seq >> 24);
        msg[1] = (uint8_t)(seq >> 16);
        msg[2] = (uint8_t)(seq >> 8);
        msg[3] = (uint8_t)(seq);
        
        uint32_t start = os_get_tick_count();
        pipe_enqueue(pipe_idx, msg, 4);
        uint32_t wait = os_get_tick_count() - start;
        
        if (wait > 1) {
            g_stress_stats.pipe_full_waits++;
        }
        if (wait > g_stress_stats.max_pipe_wait_ticks) {
            g_stress_stats.max_pipe_wait_ticks = wait;
        }
        
        g_stress_stats.pipe_sends++;
        send_tick = os_get_tick_count();
        seq++;
        
        uint32_t elapsed = os_get_tick_count() - period_start;
        bool show_flash = (os_get_tick_count() - send_tick) < 50;
        display_render_producer(STRESS_ROW_PIPE_0, task_name, elapsed, STRESS_FAST_DELAY,
                               (uint8_t)(seq & 0xFF), show_flash);
        
        // No sleep - flood as fast as possible to create backpressure
        os_yield();
        g_stress_stats.task_yields++;
    }
}

static void pipe_flood_receiver(void) {
    const char* task_name = os_get_current_task_name();
    const uint8_t pipe_idx = STRESS_PIPE_IDX_BASE;
    uint8_t msg[4];
    uint32_t expected_seq = 0;
    uint32_t recv_tick = 0;
    
    while (1) {
        uint32_t period_start = os_get_tick_count();
        
        uint32_t start = os_get_tick_count();
        pipe_dequeue(pipe_idx, msg, 4);
        uint32_t wait = os_get_tick_count() - start;
        
        if (wait > 1) {
            g_stress_stats.pipe_empty_waits++;
        }
        
        uint32_t recv_seq = ((uint32_t)msg[0] << 24) | ((uint32_t)msg[1] << 16) |
                           ((uint32_t)msg[2] << 8) | msg[3];
        
        // VERIFY: Check sequence ordering
        if (recv_seq != expected_seq) {
            g_stress_stats.seq_errors++;
            expected_seq = recv_seq;  // Resync
        }
        expected_seq++;
        recv_tick = os_get_tick_count();
        
        g_stress_stats.pipe_recvs++;
        
        uint32_t elapsed = os_get_tick_count() - period_start;
        bool show_flash = (os_get_tick_count() - recv_tick) < 50;
        display_render_consumer(STRESS_ROW_PIPE_0 + 1, task_name, elapsed, STRESS_FAST_DELAY + 4,
                               (uint8_t)(recv_seq & 0xFF), show_flash);
        
        // Slower than sender to create backpressure
        task_active_sleep(STRESS_FAST_DELAY * 2);
        g_stress_stats.task_sleeps++;
    }
}

// Multi-producer pipe stress - pipe 5
static void pipe_multi_prod_0(void) {
    const char* task_name = os_get_current_task_name();
    const uint8_t pipe_idx = STRESS_PIPE_IDX_BASE + 1;
    uint8_t msg[2] = {0x00, 0};
    uint32_t send_tick = 0;
    
    while (1) {
        uint32_t period_start = os_get_tick_count();
        
        msg[1]++;
        pipe_enqueue(pipe_idx, msg, 2);
        g_stress_stats.pipe_sends++;
        send_tick = os_get_tick_count();
        
        uint32_t elapsed = os_get_tick_count() - period_start;
        bool show_flash = (os_get_tick_count() - send_tick) < 80;
        display_render_producer(STRESS_ROW_PIPE_1, task_name, elapsed, STRESS_MED_DELAY,
                               msg[1], show_flash);
        
        task_active_sleep(STRESS_MED_DELAY);
        g_stress_stats.task_sleeps++;
    }
}

static void pipe_multi_prod_1(void) {
    const char* task_name = os_get_current_task_name();
    const uint8_t pipe_idx = STRESS_PIPE_IDX_BASE + 1;
    uint8_t msg[2] = {0x01, 0};
    uint32_t send_tick = 0;
    
    while (1) {
        uint32_t period_start = os_get_tick_count();
        
        msg[1]++;
        pipe_enqueue(pipe_idx, msg, 2);
        g_stress_stats.pipe_sends++;
        send_tick = os_get_tick_count();
        
        uint32_t elapsed = os_get_tick_count() - period_start;
        bool show_flash = (os_get_tick_count() - send_tick) < 80;
        display_render_producer(STRESS_ROW_PIPE_1 + 1, task_name, elapsed, STRESS_MED_DELAY + 11,
                               msg[1], show_flash);
        
        task_active_sleep(STRESS_MED_DELAY + 11);
        g_stress_stats.task_sleeps++;
    }
}

static void pipe_multi_prod_2(void) {
    const char* task_name = os_get_current_task_name();
    const uint8_t pipe_idx = STRESS_PIPE_IDX_BASE + 1;
    uint8_t msg[2] = {0x02, 0};
    uint32_t send_tick = 0;
    
    while (1) {
        uint32_t period_start = os_get_tick_count();
        
        msg[1]++;
        pipe_enqueue(pipe_idx, msg, 2);
        g_stress_stats.pipe_sends++;
        send_tick = os_get_tick_count();
        
        uint32_t elapsed = os_get_tick_count() - period_start;
        bool show_flash = (os_get_tick_count() - send_tick) < 80;
        display_render_producer(STRESS_ROW_PIPE_1 + 2, task_name, elapsed, STRESS_MED_DELAY + 23,
                               msg[1], show_flash);
        
        task_active_sleep(STRESS_MED_DELAY + 23);
        g_stress_stats.task_sleeps++;
    }
}

static void pipe_multi_cons(void) {
    const char* task_name = os_get_current_task_name();
    const uint8_t pipe_idx = STRESS_PIPE_IDX_BASE + 1;
    uint8_t msg[2];
    uint8_t last_seq[3] = {0, 0, 0};
    uint32_t recv_tick = 0;
    
    while (1) {
        uint32_t period_start = os_get_tick_count();
        
        pipe_dequeue(pipe_idx, msg, 2);
        g_stress_stats.pipe_recvs++;
        recv_tick = os_get_tick_count();
        
        // VERIFY: Check per-producer sequence ordering
        uint8_t prod_id = msg[0];
        if (prod_id < 3) {
            uint8_t expected = (uint8_t)(last_seq[prod_id] + 1);
            if (msg[1] != expected && last_seq[prod_id] != 0) {
                g_stress_stats.seq_errors++;  // Per-producer out of order
            }
            last_seq[prod_id] = msg[1];
        } else {
            g_stress_stats.data_errors++;  // Invalid producer ID
        }
        
        uint32_t elapsed = os_get_tick_count() - period_start;
        bool show_flash = (os_get_tick_count() - recv_tick) < 50;
        display_render_consumer(STRESS_ROW_PIPE_1 + 3, task_name, elapsed, STRESS_MED_DELAY / 3,
                               msg[1], show_flash);
        
        task_active_sleep(STRESS_MED_DELAY / 3);
        g_stress_stats.task_sleeps++;
    }
}

// Variable size message stress - pipe 6
static void pipe_varsz_sender(void) {
    const char* task_name = os_get_current_task_name();
    const uint8_t pipe_idx = STRESS_PIPE_IDX_BASE + 2;
    uint8_t msg[8];
    uint8_t size_pattern = 0;
    uint32_t send_tick = 0;
    
    while (1) {
        uint32_t period_start = os_get_tick_count();
        
        uint8_t msg_size = 1 << (size_pattern % 4);
        if (msg_size > 8) msg_size = 8;
        
        for (uint8_t i = 0; i < msg_size; i++) {
            msg[i] = (uint8_t)(size_pattern + i);
        }
        
        pipe_enqueue(pipe_idx, msg, msg_size);
        g_stress_stats.pipe_sends++;
        send_tick = os_get_tick_count();
        size_pattern++;
        
        uint32_t elapsed = os_get_tick_count() - period_start;
        bool show_flash = (os_get_tick_count() - send_tick) < 100;
        display_render_producer(STRESS_ROW_PIPE_2, task_name, elapsed, STRESS_SLOW_DELAY,
                               msg[0], show_flash);
        
        task_active_sleep(STRESS_SLOW_DELAY);
        g_stress_stats.task_sleeps++;
    }
}

static void pipe_varsz_receiver(void) {
    const char* task_name = os_get_current_task_name();
    const uint8_t pipe_idx = STRESS_PIPE_IDX_BASE + 2;
    uint8_t msg[8];
    uint8_t size_pattern = 0;
    uint32_t recv_tick = 0;
    
    while (1) {
        uint32_t period_start = os_get_tick_count();
        
        uint8_t msg_size = 1 << (size_pattern % 4);
        if (msg_size > 8) msg_size = 8;
        
        pipe_dequeue(pipe_idx, msg, msg_size);
        g_stress_stats.pipe_recvs++;
        recv_tick = os_get_tick_count();
        
        // VERIFY: Check data pattern integrity
        for (uint8_t i = 0; i < msg_size; i++) {
            uint8_t expected = (uint8_t)(size_pattern + i);
            if (msg[i] != expected) {
                g_stress_stats.data_errors++;
                break;  // Only count once per message
            }
        }
        size_pattern++;
        
        uint32_t elapsed = os_get_tick_count() - period_start;
        bool show_flash = (os_get_tick_count() - recv_tick) < 100;
        display_render_consumer(STRESS_ROW_PIPE_2 + 1, task_name, elapsed, STRESS_SLOW_DELAY + 5,
                               msg[0], show_flash);
        
        task_active_sleep(STRESS_SLOW_DELAY + 5);
        g_stress_stats.task_sleeps++;
    }
}


// ============================================================================
// SCHEDULER STRESS TASKS
// Rapid context switching and yield patterns
// ============================================================================

// Yield spammer - tests scheduler fairness
static void yield_spammer(void) {
    const char* task_name = os_get_current_task_name();
    uint32_t yield_count = 0;
    
    while (1) {
        uint32_t period_start = os_get_tick_count();
        
        for (uint8_t i = 0; i < 10; i++) {
            os_yield();
            g_stress_stats.task_yields++;
            yield_count++;
        }
        
        uint32_t elapsed = os_get_tick_count() - period_start;
        display_render_bar(STRESS_ROW_SCHED, task_name, elapsed, 20);
        
        task_active_sleep(1);
        g_stress_stats.task_sleeps++;
    }
}

// Mixed sleep patterns
static void sleep_pattern_task(void) {
    const char* task_name = os_get_current_task_name();
    uint8_t pattern = 0;
    uint32_t sleep_times[5] = {1, 5, 20, 50, 100};
    
    while (1) {
        uint32_t period_start = os_get_tick_count();
        uint32_t sleep_time = sleep_times[pattern % 5];
        
        uint32_t elapsed = os_get_tick_count() - period_start;
        display_render_bar(STRESS_ROW_SCHED + 1, task_name, elapsed, sleep_time);
        
        task_active_sleep(sleep_time);
        g_stress_stats.task_sleeps++;
        pattern++;
    }
}

// CPU hog - tests preemption
static void cpu_hog_task(void) {
    const char* task_name = os_get_current_task_name();
    volatile uint32_t counter = 0;
    uint32_t hog_cycles = 0;
    
    while (1) {
        uint32_t period_start = os_get_tick_count();
        
        // Burn CPU cycles without yielding
        for (volatile uint32_t i = 0; i < 10000; i++) {
            counter++;
        }
        hog_cycles++;
        
        uint32_t elapsed = os_get_tick_count() - period_start;
        display_render_bar(STRESS_ROW_SCHED + 2, task_name, elapsed, 50);
        
        os_yield();
        g_stress_stats.task_yields++;
    }
}

// ============================================================================
// STATISTICS DISPLAY TASK
// ============================================================================

static void stats_display_task(void) {
    while (1) {
        // Line 1: Operation counts
        ANSI_GOTO(STRESS_ROW_STATS, 1);
        printf("\033[1;36m");  // Bold cyan
        printf("STRESS: ");
        printf("\033[0;32msem_f=%lu \033[0;35msem_c=%lu ", 
               g_stress_stats.sem_feeds, g_stress_stats.sem_consumes);
        printf("\033[0;33mpipe_s=%lu \033[0;34mpipe_r=%lu ",
               g_stress_stats.pipe_sends, g_stress_stats.pipe_recvs);
        printf("\033[0;37myields=%lu sleeps=%lu",
               g_stress_stats.task_yields, g_stress_stats.task_sleeps);
        printf("\033[0m");
        ANSI_CLEAR_LINE();
        
        // Line 2: Wait stats
        ANSI_GOTO(STRESS_ROW_STATS2, 1);
        printf("\033[1;33m");  // Bold yellow
        printf("WAITS: ");
        printf("\033[0msem_max=%lu pipe_max=%lu full=%lu empty=%lu",
               g_stress_stats.max_sem_wait_ticks,
               g_stress_stats.max_pipe_wait_ticks,
               g_stress_stats.pipe_full_waits,
               g_stress_stats.pipe_empty_waits);
        ANSI_CLEAR_LINE();
        
        // Line 3: VERIFICATION ERRORS (red if any errors)
        ANSI_GOTO(STRESS_ROW_STATS2 + 1, 1);
        uint32_t total_errors = g_stress_stats.seq_errors + 
                                g_stress_stats.data_errors +
                                g_stress_stats.overflow_errors +
                                g_stress_stats.underflow_errors;
        if (total_errors > 0) {
            printf("\033[1;31m");  // Bold red for errors
            printf("ERRORS: seq=%lu data=%lu overflow=%lu underflow=%lu  [FAIL]",
                   g_stress_stats.seq_errors,
                   g_stress_stats.data_errors,
                   g_stress_stats.overflow_errors,
                   g_stress_stats.underflow_errors);
        } else {
            printf("\033[1;32m");  // Bold green for pass
            printf("VERIFY: seq=0 data=0 overflow=0 underflow=0  [PASS]");
        }
        printf("\033[0m");
        ANSI_CLEAR_LINE();
        
        task_active_sleep(500);
        g_stress_stats.task_sleeps++;
    }
}

// ============================================================================
// HEADER DISPLAY
// ============================================================================

static void stress_header_init(void) {
    ANSI_GOTO(STRESS_ROW_HEADER, 1);
    printf("\033[1;33m");  // Bold yellow
    printf("═══════════════════════════════════════════════════════════════════════════════");
    ANSI_GOTO(STRESS_ROW_HEADER + 1, 1);
    printf("  ⚡ STRESS TEST ACTIVE ⚡  Semaphores: %d  Pipes: %d  Tasks: 18",
           STRESS_SEM_COUNT, STRESS_PIPE_COUNT);
    printf("\033[0m");
    ANSI_GOTO(STRESS_ROW_HEADER + 2, 1);
    printf("\033[1;33m");
    printf("═══════════════════════════════════════════════════════════════════════════════");
    printf("\033[0m");
}

// ============================================================================
// INITIALIZATION
// ============================================================================

stress_stats_t* stress_test_get_stats(void) {
    return &g_stress_stats;
}

void stress_test_reset_stats(void) {
    g_stress_stats.sem_feeds = 0;
    g_stress_stats.sem_consumes = 0;
    g_stress_stats.sem_contentions = 0;
    g_stress_stats.pipe_sends = 0;
    g_stress_stats.pipe_recvs = 0;
    g_stress_stats.pipe_full_waits = 0;
    g_stress_stats.pipe_empty_waits = 0;
    g_stress_stats.task_yields = 0;
    g_stress_stats.task_sleeps = 0;
    g_stress_stats.max_sem_wait_ticks = 0;
    g_stress_stats.max_pipe_wait_ticks = 0;
    // Reset verification error counters
    g_stress_stats.seq_errors = 0;
    g_stress_stats.data_errors = 0;
    g_stress_stats.overflow_errors = 0;
    g_stress_stats.underflow_errors = 0;
}

void stress_test_init(void) {
    // Reset statistics
    stress_test_reset_stats();
    
    // Initialize stress test semaphores
    semaphore_init(STRESS_SEM_IDX_BASE, STRESS_SEM_CAPACITY);      // Fast hammer
    semaphore_init(STRESS_SEM_IDX_BASE + 1, STRESS_SEM_CAPACITY);  // Medium stress
    semaphore_init(STRESS_SEM_IDX_BASE + 2, STRESS_SEM_CAPACITY);  // Slow stress
    semaphore_init(STRESS_SEM_IDX_BASE + 3, 1);                    // Binary/mutex
    
    // Initialize stress test pipes
    pipe_init(STRESS_PIPE_IDX_BASE, STRESS_PIPE_CAPACITY);         // Flood test
    pipe_init(STRESS_PIPE_IDX_BASE + 1, STRESS_PIPE_CAPACITY);     // Multi-producer
    pipe_init(STRESS_PIPE_IDX_BASE + 2, STRESS_PIPE_CAPACITY);     // Variable size
    pipe_init(STRESS_PIPE_IDX_BASE + 3, STRESS_PIPE_CAPACITY);     // Reserved
    
    // Print header
    stress_header_init();
    
    // Register semaphore stress tasks (7 tasks)
    os_register_task(sem_hammer_fast_0, "sem_ham0");
    os_register_task(sem_hammer_fast_1, "sem_ham1");
    os_register_task(sem_stress_med_0, "sem_med0");
    os_register_task(sem_stress_med_1, "sem_med1");
    os_register_task(sem_stress_slow, "sem_slow");
    os_register_task(sem_mutex_stress_0, "sem_mtx0");
    os_register_task(sem_mutex_stress_1, "sem_mtx1");
    
    // Register pipe stress tasks (8 tasks)
    os_register_task(pipe_flood_sender, "pf_send");
    os_register_task(pipe_flood_receiver, "pf_recv");
    os_register_task(pipe_multi_prod_0, "pm_prd0");
    os_register_task(pipe_multi_prod_1, "pm_prd1");
    os_register_task(pipe_multi_prod_2, "pm_prd2");
    os_register_task(pipe_multi_cons, "pm_cons");
    os_register_task(pipe_varsz_sender, "pv_send");
    os_register_task(pipe_varsz_receiver, "pv_recv");
    
    // Register scheduler stress tasks (3 tasks)
    os_register_task(yield_spammer, "yielder");
    os_register_task(sleep_pattern_task, "sleeper");
    os_register_task(cpu_hog_task, "cpu_hog");
    
    // Register stats display task (1 task)
    os_register_task(stats_display_task, "stats");
}
