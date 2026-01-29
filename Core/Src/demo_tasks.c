/*
 * demo_tasks.c
 *
 *  Created on: Jan 29, 2026
 *      Author: Souham Biswas
 *      GitHub: https://github.com/ironhide23586/icarus-os-core
 */

#include "demo_tasks.h"
#include "kernel/task.h"
#include "display.h"

// Semaphore demo configuration
#define SEM_IDX           0      // Semaphore index to use
#define SEM_CAPACITY      10     // Semaphore capacity (max_count)
#define PRODUCER_DELAY    200    // Ticks between produce operations
#define CONSUMER_DELAY    190    // Ticks between consume operations

// Message queue demo configuration
#define PIPE_SS_IDX       0      // Single-Single pipe index
#define PIPE_SM_IDX       1      // Single-Multi pipe index
#define PIPE_MS_IDX       2      // Multi-Single pipe index
#define PIPE_MM_IDX       3      // Multi-Multi pipe index
#define PIPE_CAPACITY     16     // Pipe capacity in bytes
#define MSG_SIZE          1      // Message size in bytes

// Display rows for message queue demos (after existing tasks)
#define ROW_PIPE_SS_P     14     // Single-Single producer
#define ROW_PIPE_SS_C     15     // Single-Single consumer
#define ROW_PIPE_SM_P     16     // Single-Multi producer
#define ROW_PIPE_SM_C1    17     // Single-Multi consumer 1
#define ROW_PIPE_SM_C2    18     // Single-Multi consumer 2
#define ROW_PIPE_MS_P1    19     // Multi-Single producer 1
#define ROW_PIPE_MS_P2    20     // Multi-Single producer 2
#define ROW_PIPE_MS_C     21     // Multi-Single consumer
#define ROW_PIPE_MM_P1    22     // Multi-Multi producer 1
#define ROW_PIPE_MM_P2    23     // Multi-Multi producer 2
#define ROW_PIPE_MM_C1    24     // Multi-Multi consumer 1
#define ROW_PIPE_MM_C2    25     // Multi-Multi consumer 2

// Timing for message queue tasks
#define PIPE_SEND_DELAY   400    // Ticks between send operations
#define PIPE_RECV_DELAY   350    // Ticks between receive operations


// ============================================================================
// SEMAPHORE DEMO TASKS
// ============================================================================

static void producer_task(void) {
    const char* task_name = os_get_current_task_name();
    uint32_t items_produced = 0;
    
    while (1) {
        uint32_t period_start = os_get_tick_count();
        uint32_t elapsed;
        
        do {
            elapsed = os_get_tick_count() - period_start;
            display_render_bar(ROW_TASK_A, task_name, elapsed, PRODUCER_DELAY);
            display_render_vbar(ROW_HEARTBEAT, VBAR_COL, 
                               semaphore_get_count(SEM_IDX), 
                               semaphore_get_max_count(SEM_IDX));
            
            if (elapsed < PRODUCER_DELAY) {
                task_active_sleep(RENDER_INTERVAL_TICKS);
            }
            elapsed = os_get_tick_count() - period_start;
        } while (elapsed < PRODUCER_DELAY);
        
        semaphore_feed(SEM_IDX);
        items_produced++;
        
        display_render_vbar(ROW_HEARTBEAT, VBAR_COL,
                           semaphore_get_count(SEM_IDX),
                           semaphore_get_max_count(SEM_IDX));
    }
}

static void consumer_task(void) {
    const char* task_name = os_get_current_task_name();
    uint32_t items_consumed = 0;
    
    while (1) {
        uint32_t period_start = os_get_tick_count();
        uint32_t elapsed;
        
        do {
            elapsed = os_get_tick_count() - period_start;
            display_render_bar(ROW_TASK_B, task_name, elapsed, CONSUMER_DELAY);
            display_render_vbar(ROW_HEARTBEAT, VBAR_COL,
                               semaphore_get_count(SEM_IDX),
                               semaphore_get_max_count(SEM_IDX));
            
            if (elapsed < CONSUMER_DELAY) {
                task_active_sleep(RENDER_INTERVAL_TICKS);
            }
            elapsed = os_get_tick_count() - period_start;
        } while (elapsed < CONSUMER_DELAY);
        
        semaphore_consume(SEM_IDX);
        items_consumed++;
        
        display_render_vbar(ROW_HEARTBEAT, VBAR_COL,
                           semaphore_get_count(SEM_IDX),
                           semaphore_get_max_count(SEM_IDX));
    }
}

static void reference_task(void) {
    uint32_t elapsed;
    uint32_t remaining;
    const char* task_name = os_get_current_task_name();
    
    while (1) {
        uint32_t period_start_tick = os_get_tick_count();
        
        do {
            elapsed = os_get_tick_count() - period_start_tick;
            display_render_bar(ROW_TASK_C, task_name, elapsed, TASK_C_PERIOD_TICKS);
            
            remaining = (elapsed < TASK_C_PERIOD_TICKS) ? (TASK_C_PERIOD_TICKS - elapsed) : 0;
            
            if (remaining > 0) {
                uint32_t sleep_ticks = (remaining < RENDER_INTERVAL_TICKS) ? remaining : RENDER_INTERVAL_TICKS;
                task_active_sleep(sleep_ticks);
            }
            
            elapsed = os_get_tick_count() - period_start_tick;
        } while (elapsed < TASK_C_PERIOD_TICKS);
        
        display_render_bar(ROW_TASK_C, task_name, TASK_C_PERIOD_TICKS, TASK_C_PERIOD_TICKS);
        task_active_sleep(CYCLE_PAUSE_TICKS);
    }
}


// ============================================================================
// MESSAGE QUEUE DEMO TASKS
// ============================================================================

// --- Single Producer -> Single Consumer ---
static void pipe_ss_producer(void) {
    const char* task_name = os_get_current_task_name();
    uint8_t msg = 0;
    
    while (1) {
        uint32_t period_start = os_get_tick_count();
        uint32_t elapsed;
        
        do {
            elapsed = os_get_tick_count() - period_start;
            display_render_bar(ROW_PIPE_SS_P, task_name, elapsed, PIPE_SEND_DELAY);
            
            if (elapsed < PIPE_SEND_DELAY) {
                task_active_sleep(RENDER_INTERVAL_TICKS);
            }
            elapsed = os_get_tick_count() - period_start;
        } while (elapsed < PIPE_SEND_DELAY);
        
        pipe_enqueue(PIPE_SS_IDX, &msg, MSG_SIZE);
        msg++;
    }
}

static void pipe_ss_consumer(void) {
    const char* task_name = os_get_current_task_name();
    uint8_t msg;
    
    while (1) {
        uint32_t period_start = os_get_tick_count();
        uint32_t elapsed;
        
        do {
            elapsed = os_get_tick_count() - period_start;
            display_render_bar(ROW_PIPE_SS_C, task_name, elapsed, PIPE_RECV_DELAY);
            
            if (elapsed < PIPE_RECV_DELAY) {
                task_active_sleep(RENDER_INTERVAL_TICKS);
            }
            elapsed = os_get_tick_count() - period_start;
        } while (elapsed < PIPE_RECV_DELAY);
        
        pipe_dequeue(PIPE_SS_IDX, &msg, MSG_SIZE);
    }
}

// --- Single Producer -> Multiple Consumers ---
// Producer runs faster (150ms) to feed two consumers (350ms + 450ms)
#define PIPE_SM_SEND_DELAY  150

static void pipe_sm_producer(void) {
    const char* task_name = os_get_current_task_name();
    uint8_t msg = 0;
    
    while (1) {
        uint32_t period_start = os_get_tick_count();
        uint32_t elapsed;
        
        do {
            elapsed = os_get_tick_count() - period_start;
            display_render_bar(ROW_PIPE_SM_P, task_name, elapsed, PIPE_SM_SEND_DELAY);
            
            if (elapsed < PIPE_SM_SEND_DELAY) {
                task_active_sleep(RENDER_INTERVAL_TICKS);
            }
            elapsed = os_get_tick_count() - period_start;
        } while (elapsed < PIPE_SM_SEND_DELAY);
        
        pipe_enqueue(PIPE_SM_IDX, &msg, MSG_SIZE);
        msg++;
    }
}

static void pipe_sm_consumer1(void) {
    const char* task_name = os_get_current_task_name();
    uint8_t msg;
    
    while (1) {
        uint32_t period_start = os_get_tick_count();
        uint32_t elapsed;
        
        do {
            elapsed = os_get_tick_count() - period_start;
            display_render_bar(ROW_PIPE_SM_C1, task_name, elapsed, PIPE_RECV_DELAY);
            
            if (elapsed < PIPE_RECV_DELAY) {
                task_active_sleep(RENDER_INTERVAL_TICKS);
            }
            elapsed = os_get_tick_count() - period_start;
        } while (elapsed < PIPE_RECV_DELAY);
        
        pipe_dequeue(PIPE_SM_IDX, &msg, MSG_SIZE);
    }
}

static void pipe_sm_consumer2(void) {
    const char* task_name = os_get_current_task_name();
    uint8_t msg;
    
    while (1) {
        uint32_t period_start = os_get_tick_count();
        uint32_t elapsed;
        
        do {
            elapsed = os_get_tick_count() - period_start;
            display_render_bar(ROW_PIPE_SM_C2, task_name, elapsed, PIPE_RECV_DELAY + 100);
            
            if (elapsed < PIPE_RECV_DELAY + 100) {
                task_active_sleep(RENDER_INTERVAL_TICKS);
            }
            elapsed = os_get_tick_count() - period_start;
        } while (elapsed < PIPE_RECV_DELAY + 100);
        
        pipe_dequeue(PIPE_SM_IDX, &msg, MSG_SIZE);
    }
}


// --- Multiple Producers -> Single Consumer ---
static void pipe_ms_producer1(void) {
    const char* task_name = os_get_current_task_name();
    uint8_t msg = 0;
    
    while (1) {
        uint32_t period_start = os_get_tick_count();
        uint32_t elapsed;
        
        do {
            elapsed = os_get_tick_count() - period_start;
            display_render_bar(ROW_PIPE_MS_P1, task_name, elapsed, PIPE_SEND_DELAY);
            
            if (elapsed < PIPE_SEND_DELAY) {
                task_active_sleep(RENDER_INTERVAL_TICKS);
            }
            elapsed = os_get_tick_count() - period_start;
        } while (elapsed < PIPE_SEND_DELAY);
        
        pipe_enqueue(PIPE_MS_IDX, &msg, MSG_SIZE);
        msg++;
    }
}

static void pipe_ms_producer2(void) {
    const char* task_name = os_get_current_task_name();
    uint8_t msg = 100;
    
    while (1) {
        uint32_t period_start = os_get_tick_count();
        uint32_t elapsed;
        
        do {
            elapsed = os_get_tick_count() - period_start;
            display_render_bar(ROW_PIPE_MS_P2, task_name, elapsed, PIPE_SEND_DELAY + 150);
            
            if (elapsed < PIPE_SEND_DELAY + 150) {
                task_active_sleep(RENDER_INTERVAL_TICKS);
            }
            elapsed = os_get_tick_count() - period_start;
        } while (elapsed < PIPE_SEND_DELAY + 150);
        
        pipe_enqueue(PIPE_MS_IDX, &msg, MSG_SIZE);
        msg++;
    }
}

static void pipe_ms_consumer(void) {
    const char* task_name = os_get_current_task_name();
    uint8_t msg;
    
    while (1) {
        uint32_t period_start = os_get_tick_count();
        uint32_t elapsed;
        
        do {
            elapsed = os_get_tick_count() - period_start;
            display_render_bar(ROW_PIPE_MS_C, task_name, elapsed, PIPE_RECV_DELAY);
            
            if (elapsed < PIPE_RECV_DELAY) {
                task_active_sleep(RENDER_INTERVAL_TICKS);
            }
            elapsed = os_get_tick_count() - period_start;
        } while (elapsed < PIPE_RECV_DELAY);
        
        pipe_dequeue(PIPE_MS_IDX, &msg, MSG_SIZE);
    }
}


// --- Multiple Producers -> Multiple Consumers ---
static void pipe_mm_producer1(void) {
    const char* task_name = os_get_current_task_name();
    uint8_t msg = 0;
    
    while (1) {
        uint32_t period_start = os_get_tick_count();
        uint32_t elapsed;
        
        do {
            elapsed = os_get_tick_count() - period_start;
            display_render_bar(ROW_PIPE_MM_P1, task_name, elapsed, PIPE_SEND_DELAY);
            
            if (elapsed < PIPE_SEND_DELAY) {
                task_active_sleep(RENDER_INTERVAL_TICKS);
            }
            elapsed = os_get_tick_count() - period_start;
        } while (elapsed < PIPE_SEND_DELAY);
        
        pipe_enqueue(PIPE_MM_IDX, &msg, MSG_SIZE);
        msg++;
    }
}

static void pipe_mm_producer2(void) {
    const char* task_name = os_get_current_task_name();
    uint8_t msg = 200;
    
    while (1) {
        uint32_t period_start = os_get_tick_count();
        uint32_t elapsed;
        
        do {
            elapsed = os_get_tick_count() - period_start;
            display_render_bar(ROW_PIPE_MM_P2, task_name, elapsed, PIPE_SEND_DELAY + 200);
            
            if (elapsed < PIPE_SEND_DELAY + 200) {
                task_active_sleep(RENDER_INTERVAL_TICKS);
            }
            elapsed = os_get_tick_count() - period_start;
        } while (elapsed < PIPE_SEND_DELAY + 200);
        
        pipe_enqueue(PIPE_MM_IDX, &msg, MSG_SIZE);
        msg++;
    }
}

static void pipe_mm_consumer1(void) {
    const char* task_name = os_get_current_task_name();
    uint8_t msg;
    
    while (1) {
        uint32_t period_start = os_get_tick_count();
        uint32_t elapsed;
        
        do {
            elapsed = os_get_tick_count() - period_start;
            display_render_bar(ROW_PIPE_MM_C1, task_name, elapsed, PIPE_RECV_DELAY);
            
            if (elapsed < PIPE_RECV_DELAY) {
                task_active_sleep(RENDER_INTERVAL_TICKS);
            }
            elapsed = os_get_tick_count() - period_start;
        } while (elapsed < PIPE_RECV_DELAY);
        
        pipe_dequeue(PIPE_MM_IDX, &msg, MSG_SIZE);
    }
}

static void pipe_mm_consumer2(void) {
    const char* task_name = os_get_current_task_name();
    uint8_t msg;
    
    while (1) {
        uint32_t period_start = os_get_tick_count();
        uint32_t elapsed;
        
        do {
            elapsed = os_get_tick_count() - period_start;
            display_render_bar(ROW_PIPE_MM_C2, task_name, elapsed, PIPE_RECV_DELAY + 50);
            
            if (elapsed < PIPE_RECV_DELAY + 50) {
                task_active_sleep(RENDER_INTERVAL_TICKS);
            }
            elapsed = os_get_tick_count() - period_start;
        } while (elapsed < PIPE_RECV_DELAY + 50);
        
        pipe_dequeue(PIPE_MM_IDX, &msg, MSG_SIZE);
    }
}


// ============================================================================
// INITIALIZATION
// ============================================================================

void demo_tasks_init(void) {
    // Initialize semaphore for producer-consumer demo
    semaphore_init(SEM_IDX, SEM_CAPACITY);

    // Initialize message pipes for all demo configurations
    pipe_init(PIPE_SS_IDX, PIPE_CAPACITY);  // Single-Single
    pipe_init(PIPE_SM_IDX, PIPE_CAPACITY);  // Single-Multi
    pipe_init(PIPE_MS_IDX, PIPE_CAPACITY);  // Multi-Single
    pipe_init(PIPE_MM_IDX, PIPE_CAPACITY);  // Multi-Multi

    // Semaphore demo tasks
    os_register_task(producer_task, "producer");
    os_register_task(consumer_task, "consumer");
    os_register_task(reference_task, "reference");

    // Message queue demo: Single Producer -> Single Consumer
    os_register_task(pipe_ss_producer, "ss_prod");
    os_register_task(pipe_ss_consumer, "ss_cons");

    // Message queue demo: Single Producer -> Multiple Consumers
    os_register_task(pipe_sm_producer, "sm_prod");
    os_register_task(pipe_sm_consumer1, "sm_con1");
    os_register_task(pipe_sm_consumer2, "sm_con2");

    // Message queue demo: Multiple Producers -> Single Consumer
    os_register_task(pipe_ms_producer1, "ms_prd1");
    os_register_task(pipe_ms_producer2, "ms_prd2");
    os_register_task(pipe_ms_consumer, "ms_cons");

    // Message queue demo: Multiple Producers -> Multiple Consumers
    os_register_task(pipe_mm_producer1, "mm_prd1");
    os_register_task(pipe_mm_producer2, "mm_prd2");
    os_register_task(pipe_mm_consumer1, "mm_con1");
    os_register_task(pipe_mm_consumer2, "mm_con2");
}
