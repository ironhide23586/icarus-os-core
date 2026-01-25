/*
 * main.c
 *
 *  Created on: Jan 23, 2026
 *      Author: souhamb
 */


#include "main.h"
#include "retarget_hal.h"
#include "kernel/task.h"
#include "display.h"


// ANSI escape code helper: show cursor (used in error path)
#define ANSI_SHOW_CURSOR()  printf("\033[?25h")

void test_task_a(void) {
    uint32_t period_start_tick;
    uint32_t elapsed;
    uint32_t remaining;
    const char* task_name = os_get_current_task_name();
    
    while (1) {
        // Start a new period
        period_start_tick = os_get_tick_count();
        
        // Render progress bar continuously during sleep period
        // This loop allows preemption - other tasks can run between renders
        do {
            elapsed = os_get_tick_count() - period_start_tick;
            display_render_bar(ROW_TASK_A, task_name, elapsed, TASK_A_PERIOD_TICKS);
            
            // Calculate remaining time in period
            remaining = (elapsed < TASK_A_PERIOD_TICKS) ? (TASK_A_PERIOD_TICKS - elapsed) : 0;
            
            // Sleep for render interval, but not longer than remaining period
            if (remaining > 0) {
                uint32_t sleep_ticks = (remaining < RENDER_INTERVAL_TICKS) ? remaining : RENDER_INTERVAL_TICKS;
                task_active_sleep(sleep_ticks);
            }
            
            // Recalculate elapsed after sleep
            elapsed = os_get_tick_count() - period_start_tick;
        } while (elapsed < TASK_A_PERIOD_TICKS);
        
        // Final render to show completed bar
        display_render_bar(ROW_TASK_A, task_name, TASK_A_PERIOD_TICKS, TASK_A_PERIOD_TICKS);
        
        // Brief pause before next cycle
        task_active_sleep(CYCLE_PAUSE_TICKS);
    }
}

void test_task_b(void) {
    uint32_t period_start_tick;
    uint32_t elapsed;
    uint32_t remaining;
    const char* task_name = os_get_current_task_name();
    
    while (1) {
        // Start a new period
        period_start_tick = os_get_tick_count();
        
        // Render progress bar continuously during sleep period
        do {
            elapsed = os_get_tick_count() - period_start_tick;
            display_render_bar(ROW_TASK_B, task_name, elapsed, TASK_B_PERIOD_TICKS);
            
            remaining = (elapsed < TASK_B_PERIOD_TICKS) ? (TASK_B_PERIOD_TICKS - elapsed) : 0;
            
            if (remaining > 0) {
                uint32_t sleep_ticks = (remaining < RENDER_INTERVAL_TICKS) ? remaining : RENDER_INTERVAL_TICKS;
                task_active_sleep(sleep_ticks);
            }
            
            elapsed = os_get_tick_count() - period_start_tick;
        } while (elapsed < TASK_B_PERIOD_TICKS);
        
        // Final render to show completed bar
        display_render_bar(ROW_TASK_B, task_name, TASK_B_PERIOD_TICKS, TASK_B_PERIOD_TICKS);
        
        // Brief pause before next cycle
        task_active_sleep(CYCLE_PAUSE_TICKS);
    }
}

void test_task_c(void) {
    uint32_t period_start_tick;
    uint32_t elapsed;
    uint32_t remaining;
    const char* task_name = os_get_current_task_name();
    
    while (1) {
        // Start a new period
        period_start_tick = os_get_tick_count();
        
        // Render progress bar continuously during sleep period
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
        
        // Final render to show completed bar
        display_render_bar(ROW_TASK_C, task_name, TASK_C_PERIOD_TICKS, TASK_C_PERIOD_TICKS);
        
        // Brief pause before next cycle
        task_active_sleep(CYCLE_PAUSE_TICKS);
    }
}




int main(void)
{
  hal_init();
  os_init();

  os_register_task(test_task_a, "task_a");
  os_register_task(test_task_b, "task_b");
  os_register_task(test_task_c, "task_c");

  os_start();

  // Error path: restore cursor before error message
  ANSI_SHOW_CURSOR();
  printf("ERROR: Should not be here\r\n");
  while (1) {
    LED_Blink(50, 900);  // Short blink, long off
  }

}
