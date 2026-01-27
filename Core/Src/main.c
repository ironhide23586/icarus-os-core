/*
 * main.c
 *
 *  Created on: Jan 23, 2026
 *      Author: Souham Biswas
 *      GitHub: https://github.com/ironhide23586/icarus-os-core
 */


#include "main.h"
#include "retarget_hal.h"
#include "kernel/task.h"
#include "display.h"


// ANSI escape code helper: show cursor (used in error path)
#define ANSI_SHOW_CURSOR()  printf("\033[?25h")

// Semaphore demo configuration
#define SEM_IDX           0      // Semaphore index to use
#define SEM_CAPACITY      10     // Semaphore capacity (init_count)
#define PRODUCER_DELAY    500    // Ticks between produce operations
#define CONSUMER_DELAY    300   // Ticks between consume operations (slower = buffer fills up)

// Producer task - feeds the semaphore
static void producer_task(void) {
    const char* task_name = os_get_current_task_name();
    uint32_t items_produced = 0;
    
    while (1) {
        // Show progress bar for this produce cycle
        uint32_t period_start = os_get_tick_count();
        uint32_t elapsed;
        
        // Animate progress bar while waiting
        do {
            elapsed = os_get_tick_count() - period_start;
            display_render_bar(ROW_TASK_A, task_name, elapsed, PRODUCER_DELAY);
            
            // Update vertical bar with current semaphore state
            display_render_vbar(ROW_HEARTBEAT, VBAR_COL, 
                               semaphore_get_count(SEM_IDX), 
                               semaphore_get_init_count(SEM_IDX));
            
            if (elapsed < PRODUCER_DELAY) {
                task_active_sleep(RENDER_INTERVAL_TICKS);
            }
            elapsed = os_get_tick_count() - period_start;
        } while (elapsed < PRODUCER_DELAY);
        
        // Feed the semaphore (may block if full)
        semaphore_feed(SEM_IDX);
        items_produced++;
        
        // Update display after feed
        display_render_vbar(ROW_HEARTBEAT, VBAR_COL,
                           semaphore_get_count(SEM_IDX),
                           semaphore_get_init_count(SEM_IDX));
    }
}

// Consumer task - consumes from the semaphore
static void consumer_task(void) {
    const char* task_name = os_get_current_task_name();
    uint32_t items_consumed = 0;
    
    while (1) {
        // Show progress bar for this consume cycle
        uint32_t period_start = os_get_tick_count();
        uint32_t elapsed;
        
        // Animate progress bar while waiting
        do {
            elapsed = os_get_tick_count() - period_start;
            display_render_bar(ROW_TASK_B, task_name, elapsed, CONSUMER_DELAY);
            
            // Update vertical bar with current semaphore state
            display_render_vbar(ROW_HEARTBEAT, VBAR_COL,
                               semaphore_get_count(SEM_IDX),
                               semaphore_get_init_count(SEM_IDX));
            
            if (elapsed < CONSUMER_DELAY) {
                task_active_sleep(RENDER_INTERVAL_TICKS);
            }
            elapsed = os_get_tick_count() - period_start;
        } while (elapsed < CONSUMER_DELAY);
        
        // Consume from the semaphore (may block if empty)
        semaphore_consume(SEM_IDX);
        items_consumed++;
        
        // Update display after consume
        display_render_vbar(ROW_HEARTBEAT, VBAR_COL,
                           semaphore_get_count(SEM_IDX),
                           semaphore_get_init_count(SEM_IDX));
    }
}

// Reference task - just shows time passing (unchanged behavior)
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




int main(void)
{
  hal_init();
  os_init();

  // Initialize semaphore for producer-consumer demo
  semaphore_init(SEM_IDX, SEM_CAPACITY);

  os_register_task(producer_task, "producer");
  os_register_task(consumer_task, "consumer");
  os_register_task(reference_task, "reference");

  os_start();

  // Error path: restore cursor before error message
  ANSI_SHOW_CURSOR();
  printf("ERROR: Should not be here\r\n");
  while (1) {
    LED_Blink(50, 900);  // Short blink, long off
  }

}
