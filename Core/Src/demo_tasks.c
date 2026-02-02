/**
 * @file    demo_tasks.c
 * @brief   ICARUS (Intelligent Cooperative Architecture for Real-time 
 *          Unified Systems) OS Demonstration Tasks
 * @version 0.1.0
 *
 * @details This module provides demonstration tasks showcasing ICARUS OS
 *          kernel primitives including:
 *          - Counting semaphore producer-consumer patterns
 *          - Message pipe communication patterns:
 *            - Single Producer → Single Consumer (SS)
 *            - Single Producer → Multiple Consumers (SM)
 *            - Multiple Producers → Single Consumer (MS)
 *            - Multiple Producers → Multiple Consumers (MM)
 *
 * @par Visual Output:
 *      All demo tasks render progress bars and message history to the
 *      terminal display, providing real-time visualization of IPC
 *      operations and task scheduling.
 *
 * @see     icarus_task.h for kernel API documentation
 * @see     display.h for rendering functions
 *
 * @author  Souham Biswas
 * @date    2025
 *
 * @copyright Copyright 2025 Souham Biswas
 *            https://github.com/ironhide23586/icarus-os-core
 *            Licensed under the Apache License, Version 2.0
 */

#include "demo_tasks.h"
#include "icarus/icarus.h"
#include "bsp/display.h"

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
#define PIPE_CAPACITY     32     // Pipe capacity in bytes

// Message sizes for different demos
#define MSG_SIZE_SS       1      // Single byte for SS (simple sequential)
#define MSG_SIZE_SM       2      // 2 bytes for SM (shows distribution)
#define MSG_SIZE_MS       1      // 1 byte for MS (shows interleaving)
#define MSG_SIZE_MM       3      // 3 bytes for MM (shows full complexity)

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

// History display column positions (right side of screen)
#define HIST_COL_SS       78
#define HIST_COL_SM       92
#define HIST_COL_MS       106
#define HIST_COL_MM       120

// Timing for message queue tasks
#define PIPE_SEND_DELAY   400    // Ticks between send operations
#define PIPE_RECV_DELAY   350    // Ticks between receive operations

// Message flash duration (ticks to show the sent/received value)
#define MSG_FLASH_TICKS   150

// Global message history buffers (one per pipe)
static msg_history_t hist_ss;
static msg_history_t hist_sm;
static msg_history_t hist_ms;
static msg_history_t hist_mm;


// ============================================================================
// SEMAPHORE DEMO TASKS
// ============================================================================

/**
 * @brief   Producer task for semaphore demonstration
 *
 * @details Periodically feeds the semaphore at SEM_IDX, demonstrating
 *          the producer side of producer-consumer synchronization.
 *          Renders a progress bar and vertical semaphore level indicator.
 *
 * @note    Runs indefinitely with PRODUCER_DELAY tick period
 */
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

/**
 * @brief   Consumer task for semaphore demonstration
 *
 * @details Periodically consumes from the semaphore at SEM_IDX, demonstrating
 *          the consumer side of producer-consumer synchronization.
 *          Renders a progress bar and vertical semaphore level indicator.
 *
 * @note    Runs indefinitely with CONSUMER_DELAY tick period
 * @note    Slightly faster than producer to demonstrate blocking behavior
 */
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

/**
 * @brief   Reference task for timing visualization
 *
 * @details Provides a consistent timing reference by running at a fixed
 *          period (TASK_C_PERIOD_TICKS). Useful for visually comparing
 *          task execution timing against producer/consumer tasks.
 *
 * @note    Runs indefinitely with TASK_C_PERIOD_TICKS period
 */
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
// MESSAGE QUEUE DEMO TASKS - Single Producer -> Single Consumer
// Message: 1 byte sequential counter (0,1,2,3...)
// Demonstrates: FIFO ordering preserved
// ============================================================================

/**
 * @brief   Single-Single pipe producer task
 *
 * @details Sends sequential 1-byte messages to PIPE_SS_IDX, demonstrating
 *          basic FIFO message ordering. Each message is a counter value
 *          that increments with each send.
 *
 * @note    Demonstrates: FIFO ordering preserved in single-producer scenario
 */
static void pipe_ss_producer(void) {
    const char* task_name = os_get_current_task_name();
    uint8_t msg = 0;
    uint32_t send_tick = 0;
    
    while (1) {
        uint32_t period_start = os_get_tick_count();
        uint32_t elapsed;
        
        do {
            elapsed = os_get_tick_count() - period_start;
            bool show_flash = (os_get_tick_count() - send_tick) < MSG_FLASH_TICKS;
            display_render_producer(ROW_PIPE_SS_P, task_name, elapsed, PIPE_SEND_DELAY, 
                                   (uint8_t)(msg - 1), show_flash);
            display_render_msg_history(ROW_PIPE_SS_P, HIST_COL_SS, &hist_ss, "SS");
            
            if (elapsed < PIPE_SEND_DELAY) {
                task_active_sleep(RENDER_INTERVAL_TICKS);
            }
            elapsed = os_get_tick_count() - period_start;
        } while (elapsed < PIPE_SEND_DELAY);
        
        pipe_enqueue(PIPE_SS_IDX, &msg, MSG_SIZE_SS);
        msg_history_add(&hist_ss, &msg, MSG_SIZE_SS, 0, true);
        send_tick = os_get_tick_count();
        msg++;
    }
}

/**
 * @brief   Single-Single pipe consumer task
 *
 * @details Receives sequential 1-byte messages from PIPE_SS_IDX,
 *          verifying FIFO ordering is preserved.
 *
 * @note    Demonstrates: Messages received in same order as sent
 */
static void pipe_ss_consumer(void) {
    const char* task_name = os_get_current_task_name();
    uint8_t msg = 0;
    uint8_t last_msg = 0;
    uint32_t recv_tick = 0;
    
    while (1) {
        uint32_t period_start = os_get_tick_count();
        uint32_t elapsed;
        
        do {
            elapsed = os_get_tick_count() - period_start;
            bool show_flash = (os_get_tick_count() - recv_tick) < MSG_FLASH_TICKS;
            display_render_consumer(ROW_PIPE_SS_C, task_name, elapsed, PIPE_RECV_DELAY,
                                   last_msg, show_flash);
            
            if (elapsed < PIPE_RECV_DELAY) {
                task_active_sleep(RENDER_INTERVAL_TICKS);
            }
            elapsed = os_get_tick_count() - period_start;
        } while (elapsed < PIPE_RECV_DELAY);
        
        pipe_dequeue(PIPE_SS_IDX, &msg, MSG_SIZE_SS);
        msg_history_add(&hist_ss, &msg, MSG_SIZE_SS, 0, false);  // Consumer 0
        last_msg = msg;
        recv_tick = os_get_tick_count();
    }
}


// ============================================================================
// MESSAGE QUEUE DEMO TASKS - Single Producer -> Multiple Consumers
// Message: 2 bytes [seq_hi, seq_lo] - 16-bit counter
// Demonstrates: Messages distributed among consumers (each msg goes to ONE consumer)
// ============================================================================

#define PIPE_SM_SEND_DELAY  150  // Faster producer to feed two consumers

/**
 * @brief   Single-Multi pipe producer task
 *
 * @details Sends 2-byte messages (16-bit counter) to PIPE_SM_IDX at a
 *          faster rate to feed multiple consumers. Demonstrates message
 *          distribution among competing consumers.
 *
 * @note    Demonstrates: Messages distributed to whichever consumer dequeues first
 */
static void pipe_sm_producer(void) {
    const char* task_name = os_get_current_task_name();
    uint16_t seq = 0;
    uint8_t msg[2];
    uint32_t send_tick = 0;
    
    while (1) {
        uint32_t period_start = os_get_tick_count();
        uint32_t elapsed;
        
        do {
            elapsed = os_get_tick_count() - period_start;
            bool show_flash = (os_get_tick_count() - send_tick) < MSG_FLASH_TICKS;
            display_render_producer(ROW_PIPE_SM_P, task_name, elapsed, PIPE_SM_SEND_DELAY,
                                   (uint8_t)(seq & 0xFF), show_flash);
            display_render_msg_history(ROW_PIPE_SM_P, HIST_COL_SM, &hist_sm, "SM");
            
            if (elapsed < PIPE_SM_SEND_DELAY) {
                task_active_sleep(RENDER_INTERVAL_TICKS);
            }
            elapsed = os_get_tick_count() - period_start;
        } while (elapsed < PIPE_SM_SEND_DELAY);
        
        msg[0] = (uint8_t)(seq >> 8);    // High byte
        msg[1] = (uint8_t)(seq & 0xFF);  // Low byte
        pipe_enqueue(PIPE_SM_IDX, msg, MSG_SIZE_SM);
        msg_history_add(&hist_sm, msg, MSG_SIZE_SM, 0, true);
        send_tick = os_get_tick_count();
        seq++;
    }
}

/**
 * @brief   Single-Multi pipe consumer 1
 *
 * @details First consumer for PIPE_SM_IDX, competing with consumer 2
 *          for messages from the single producer.
 */
static void pipe_sm_consumer1(void) {
    const char* task_name = os_get_current_task_name();
    uint8_t msg[2];
    uint8_t last_lo = 0;
    uint32_t recv_tick = 0;
    
    while (1) {
        uint32_t period_start = os_get_tick_count();
        uint32_t elapsed;
        
        do {
            elapsed = os_get_tick_count() - period_start;
            bool show_flash = (os_get_tick_count() - recv_tick) < MSG_FLASH_TICKS;
            display_render_consumer(ROW_PIPE_SM_C1, task_name, elapsed, PIPE_RECV_DELAY,
                                   last_lo, show_flash);
            
            if (elapsed < PIPE_RECV_DELAY) {
                task_active_sleep(RENDER_INTERVAL_TICKS);
            }
            elapsed = os_get_tick_count() - period_start;
        } while (elapsed < PIPE_RECV_DELAY);
        
        pipe_dequeue(PIPE_SM_IDX, msg, MSG_SIZE_SM);
        msg_history_add(&hist_sm, msg, MSG_SIZE_SM, 0, false);  // Consumer 0
        last_lo = msg[1];
        recv_tick = os_get_tick_count();
    }
}

/**
 * @brief   Single-Multi pipe consumer 2
 *
 * @details Second consumer for PIPE_SM_IDX, competing with consumer 1
 *          for messages. Runs slightly slower to show distribution.
 */
static void pipe_sm_consumer2(void) {
    const char* task_name = os_get_current_task_name();
    uint8_t msg[2];
    uint8_t last_lo = 0;
    uint32_t recv_tick = 0;
    
    while (1) {
        uint32_t period_start = os_get_tick_count();
        uint32_t elapsed;
        
        do {
            elapsed = os_get_tick_count() - period_start;
            bool show_flash = (os_get_tick_count() - recv_tick) < MSG_FLASH_TICKS;
            display_render_consumer(ROW_PIPE_SM_C2, task_name, elapsed, PIPE_RECV_DELAY + 100,
                                   last_lo, show_flash);
            
            if (elapsed < PIPE_RECV_DELAY + 100) {
                task_active_sleep(RENDER_INTERVAL_TICKS);
            }
            elapsed = os_get_tick_count() - period_start;
        } while (elapsed < PIPE_RECV_DELAY + 100);
        
        pipe_dequeue(PIPE_SM_IDX, msg, MSG_SIZE_SM);
        msg_history_add(&hist_sm, msg, MSG_SIZE_SM, 1, false);  // Consumer 1
        last_lo = msg[1];
        recv_tick = os_get_tick_count();
    }
}


// ============================================================================
// MESSAGE QUEUE DEMO TASKS - Multiple Producers -> Single Consumer
// Message: 1 byte with producer ID encoded (P0: 0-99, P1: 100-199)
// Demonstrates: Messages from different producers interleaved, order per-producer preserved
// ============================================================================

/**
 * @brief   Multi-Single pipe producer 1
 *
 * @details First producer for PIPE_MS_IDX, sending values 0-99.
 *          Demonstrates interleaved messages from multiple producers.
 *
 * @note    Producer ID encoded in value range (0-99)
 */
static void pipe_ms_producer1(void) {
    const char* task_name = os_get_current_task_name();
    uint8_t msg = 0;  // Start at 0 (range 0-99)
    uint32_t send_tick = 0;
    
    while (1) {
        uint32_t period_start = os_get_tick_count();
        uint32_t elapsed;
        
        do {
            elapsed = os_get_tick_count() - period_start;
            bool show_flash = (os_get_tick_count() - send_tick) < MSG_FLASH_TICKS;
            display_render_producer(ROW_PIPE_MS_P1, task_name, elapsed, PIPE_SEND_DELAY,
                                   (uint8_t)(msg == 0 ? 99 : msg - 1), show_flash);
            display_render_msg_history(ROW_PIPE_MS_P1, HIST_COL_MS, &hist_ms, "MS");
            
            if (elapsed < PIPE_SEND_DELAY) {
                task_active_sleep(RENDER_INTERVAL_TICKS);
            }
            elapsed = os_get_tick_count() - period_start;
        } while (elapsed < PIPE_SEND_DELAY);
        
        pipe_enqueue(PIPE_MS_IDX, &msg, MSG_SIZE_MS);
        msg_history_add(&hist_ms, &msg, MSG_SIZE_MS, 0, true);  // Producer 0
        send_tick = os_get_tick_count();
        msg++;
        if (msg >= 100) msg = 0;  // Wrap to stay in 0-99 range
    }
}

/**
 * @brief   Multi-Single pipe producer 2
 *
 * @details Second producer for PIPE_MS_IDX, sending values 100-199.
 *          Runs at different rate to show interleaving.
 *
 * @note    Producer ID encoded in value range (100-199)
 */
static void pipe_ms_producer2(void) {
    const char* task_name = os_get_current_task_name();
    uint8_t msg = 100;  // Start at 100 (range 100-199)
    uint32_t send_tick = 0;
    
    while (1) {
        uint32_t period_start = os_get_tick_count();
        uint32_t elapsed;
        
        do {
            elapsed = os_get_tick_count() - period_start;
            bool show_flash = (os_get_tick_count() - send_tick) < MSG_FLASH_TICKS;
            display_render_producer(ROW_PIPE_MS_P2, task_name, elapsed, PIPE_SEND_DELAY + 150,
                                   (uint8_t)(msg == 100 ? 199 : msg - 1), show_flash);
            
            if (elapsed < PIPE_SEND_DELAY + 150) {
                task_active_sleep(RENDER_INTERVAL_TICKS);
            }
            elapsed = os_get_tick_count() - period_start;
        } while (elapsed < PIPE_SEND_DELAY + 150);
        
        pipe_enqueue(PIPE_MS_IDX, &msg, MSG_SIZE_MS);
        msg_history_add(&hist_ms, &msg, MSG_SIZE_MS, 1, true);  // Producer 1
        send_tick = os_get_tick_count();
        msg++;
        if (msg >= 200) msg = 100;  // Wrap to stay in 100-199 range
    }
}

/**
 * @brief   Multi-Single pipe consumer
 *
 * @details Single consumer for PIPE_MS_IDX, receiving interleaved
 *          messages from both producers.
 *
 * @note    Demonstrates: Per-producer ordering preserved despite interleaving
 */
static void pipe_ms_consumer(void) {
    const char* task_name = os_get_current_task_name();
    uint8_t msg = 0;
    uint8_t last_msg = 0;
    uint32_t recv_tick = 0;
    
    while (1) {
        uint32_t period_start = os_get_tick_count();
        uint32_t elapsed;
        
        do {
            elapsed = os_get_tick_count() - period_start;
            bool show_flash = (os_get_tick_count() - recv_tick) < MSG_FLASH_TICKS;
            display_render_consumer(ROW_PIPE_MS_C, task_name, elapsed, PIPE_RECV_DELAY,
                                   last_msg, show_flash);
            
            if (elapsed < PIPE_RECV_DELAY) {
                task_active_sleep(RENDER_INTERVAL_TICKS);
            }
            elapsed = os_get_tick_count() - period_start;
        } while (elapsed < PIPE_RECV_DELAY);
        
        pipe_dequeue(PIPE_MS_IDX, &msg, MSG_SIZE_MS);
        msg_history_add(&hist_ms, &msg, MSG_SIZE_MS, 0, false);  // Consumer 0
        last_msg = msg;
        recv_tick = os_get_tick_count();
    }
}


// ============================================================================
// MESSAGE QUEUE DEMO TASKS - Multiple Producers -> Multiple Consumers
// Message: 3 bytes [producer_id, seq_hi, seq_lo]
// Demonstrates: Full concurrent access with producer identification
// ============================================================================

/**
 * @brief   Multi-Multi pipe producer 1
 *
 * @details First producer for PIPE_MM_IDX, sending 3-byte messages
 *          with explicit producer ID (0) and 16-bit sequence number.
 */
static void pipe_mm_producer1(void) {
    const char* task_name = os_get_current_task_name();
    uint16_t seq = 0;
    uint8_t msg[3];
    uint32_t send_tick = 0;
    
    while (1) {
        uint32_t period_start = os_get_tick_count();
        uint32_t elapsed;
        
        do {
            elapsed = os_get_tick_count() - period_start;
            bool show_flash = (os_get_tick_count() - send_tick) < MSG_FLASH_TICKS;
            display_render_producer(ROW_PIPE_MM_P1, task_name, elapsed, PIPE_SEND_DELAY,
                                   (uint8_t)(seq & 0xFF), show_flash);
            display_render_msg_history(ROW_PIPE_MM_P1, HIST_COL_MM, &hist_mm, "MM");
            
            if (elapsed < PIPE_SEND_DELAY) {
                task_active_sleep(RENDER_INTERVAL_TICKS);
            }
            elapsed = os_get_tick_count() - period_start;
        } while (elapsed < PIPE_SEND_DELAY);
        
        msg[0] = 0;                       // Producer ID = 0
        msg[1] = (uint8_t)(seq >> 8);     // Seq high byte
        msg[2] = (uint8_t)(seq & 0xFF);   // Seq low byte
        pipe_enqueue(PIPE_MM_IDX, msg, MSG_SIZE_MM);
        msg_history_add(&hist_mm, msg, MSG_SIZE_MM, 0, true);
        send_tick = os_get_tick_count();
        seq++;
    }
}

/**
 * @brief   Multi-Multi pipe producer 2
 *
 * @details Second producer for PIPE_MM_IDX, sending 3-byte messages
 *          with explicit producer ID (1) and 16-bit sequence number.
 */
static void pipe_mm_producer2(void) {
    const char* task_name = os_get_current_task_name();
    uint16_t seq = 0;
    uint8_t msg[3];
    uint32_t send_tick = 0;
    
    while (1) {
        uint32_t period_start = os_get_tick_count();
        uint32_t elapsed;
        
        do {
            elapsed = os_get_tick_count() - period_start;
            bool show_flash = (os_get_tick_count() - send_tick) < MSG_FLASH_TICKS;
            display_render_producer(ROW_PIPE_MM_P2, task_name, elapsed, PIPE_SEND_DELAY + 200,
                                   (uint8_t)(seq & 0xFF), show_flash);
            
            if (elapsed < PIPE_SEND_DELAY + 200) {
                task_active_sleep(RENDER_INTERVAL_TICKS);
            }
            elapsed = os_get_tick_count() - period_start;
        } while (elapsed < PIPE_SEND_DELAY + 200);
        
        msg[0] = 1;                       // Producer ID = 1
        msg[1] = (uint8_t)(seq >> 8);     // Seq high byte
        msg[2] = (uint8_t)(seq & 0xFF);   // Seq low byte
        pipe_enqueue(PIPE_MM_IDX, msg, MSG_SIZE_MM);
        msg_history_add(&hist_mm, msg, MSG_SIZE_MM, 1, true);
        send_tick = os_get_tick_count();
        seq++;
    }
}

/**
 * @brief   Multi-Multi pipe consumer 1
 *
 * @details First consumer for PIPE_MM_IDX, competing with consumer 2
 *          for messages from both producers.
 */
static void pipe_mm_consumer1(void) {
    const char* task_name = os_get_current_task_name();
    uint8_t msg[3];
    uint8_t last_lo = 0;
    uint32_t recv_tick = 0;
    
    while (1) {
        uint32_t period_start = os_get_tick_count();
        uint32_t elapsed;
        
        do {
            elapsed = os_get_tick_count() - period_start;
            bool show_flash = (os_get_tick_count() - recv_tick) < MSG_FLASH_TICKS;
            display_render_consumer(ROW_PIPE_MM_C1, task_name, elapsed, PIPE_RECV_DELAY,
                                   last_lo, show_flash);
            
            if (elapsed < PIPE_RECV_DELAY) {
                task_active_sleep(RENDER_INTERVAL_TICKS);
            }
            elapsed = os_get_tick_count() - period_start;
        } while (elapsed < PIPE_RECV_DELAY);
        
        pipe_dequeue(PIPE_MM_IDX, msg, MSG_SIZE_MM);
        msg_history_add(&hist_mm, msg, MSG_SIZE_MM, 0, false);  // Consumer 0
        last_lo = msg[2];
        recv_tick = os_get_tick_count();
    }
}

/**
 * @brief   Multi-Multi pipe consumer 2
 *
 * @details Second consumer for PIPE_MM_IDX, competing with consumer 1
 *          for messages from both producers.
 */
static void pipe_mm_consumer2(void) {
    const char* task_name = os_get_current_task_name();
    uint8_t msg[3];
    uint8_t last_lo = 0;
    uint32_t recv_tick = 0;
    
    while (1) {
        uint32_t period_start = os_get_tick_count();
        uint32_t elapsed;
        
        do {
            elapsed = os_get_tick_count() - period_start;
            bool show_flash = (os_get_tick_count() - recv_tick) < MSG_FLASH_TICKS;
            display_render_consumer(ROW_PIPE_MM_C2, task_name, elapsed, PIPE_RECV_DELAY + 50,
                                   last_lo, show_flash);
            
            if (elapsed < PIPE_RECV_DELAY + 50) {
                task_active_sleep(RENDER_INTERVAL_TICKS);
            }
            elapsed = os_get_tick_count() - period_start;
        } while (elapsed < PIPE_RECV_DELAY + 50);
        
        pipe_dequeue(PIPE_MM_IDX, msg, MSG_SIZE_MM);
        msg_history_add(&hist_mm, msg, MSG_SIZE_MM, 1, false);  // Consumer 1
        last_lo = msg[2];
        recv_tick = os_get_tick_count();
    }
}


// ============================================================================
// INITIALIZATION
// ============================================================================

/**
 * @brief   Initialize and register all demonstration tasks
 *
 * @details Initializes the IPC primitives (semaphores, pipes) and registers
 *          all demo tasks with the kernel. Call this after os_init() and
 *          before os_start().
 *
 * @par Registered Tasks:
 *      | Task Name  | Type     | Description                    |
 *      |------------|----------|--------------------------------|
 *      | producer   | Sem Demo | Semaphore producer             |
 *      | consumer   | Sem Demo | Semaphore consumer             |
 *      | reference  | Timing   | Fixed-period reference         |
 *      | ss_prod    | Pipe SS  | Single→Single producer         |
 *      | ss_cons    | Pipe SS  | Single→Single consumer         |
 *      | sm_prod    | Pipe SM  | Single→Multi producer          |
 *      | sm_con1    | Pipe SM  | Single→Multi consumer 1        |
 *      | sm_con2    | Pipe SM  | Single→Multi consumer 2        |
 *      | ms_prd1    | Pipe MS  | Multi→Single producer 1        |
 *      | ms_prd2    | Pipe MS  | Multi→Single producer 2        |
 *      | ms_cons    | Pipe MS  | Multi→Single consumer          |
 *      | mm_prd1    | Pipe MM  | Multi→Multi producer 1         |
 *      | mm_prd2    | Pipe MM  | Multi→Multi producer 2         |
 *      | mm_con1    | Pipe MM  | Multi→Multi consumer 1         |
 *      | mm_con2    | Pipe MM  | Multi→Multi consumer 2         |
 *
 * @pre     os_init() must have been called
 *
 * @post    All demo semaphores and pipes initialized
 * @post    All demo tasks registered with kernel
 *
 * @see     os_init()
 * @see     os_start()
 */
void demo_tasks_init(void) {
    // Initialize message history buffers
    msg_history_init(&hist_ss);
    msg_history_init(&hist_sm);
    msg_history_init(&hist_ms);
    msg_history_init(&hist_mm);

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
