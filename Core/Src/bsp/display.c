/*
 * display.c
 *
 *  Created on: Jan 24, 2026
 *      Author: Souham Biswas
 *      GitHub: https://github.com/ironhide23586/icarus-os-core
 */

#include "bsp/display.h"
#include "kernel/task.h"
#include <string.h>
#include <inttypes.h>

// ANSI escape code helper: move cursor to row, col (1-indexed)
#define ANSI_GOTO(row, col) printf("\033[%d;%dH", (row), (col))
// ANSI escape code helpers: show/hide cursor
#define ANSI_SHOW_CURSOR()  printf("\033[?25h")
#define ANSI_HIDE_CURSOR()  printf("\033[?25l")

/**
 * @brief Render a progress bar on a fixed row
 * @param row: terminal row (1-indexed)
 * @param task_name: task name string (from TCB)
 * @param elapsed_ticks: ticks elapsed in current period
 * @param period_ticks: total ticks for this period
 */
void display_render_bar(uint8_t row, const char* task_name, uint32_t elapsed_ticks, uint32_t period_ticks) {
    // Guard against division by zero
    if (period_ticks == 0) {
        period_ticks = 1;
    }
    
    // Clamp elapsed to period to avoid overflow
    if (elapsed_ticks > period_ticks) {
        elapsed_ticks = period_ticks;
    }
    
    // Calculate filled portion (0 to BAR_WIDTH)
    // cppcheck-suppress zerodivcond
    // period_ticks is guaranteed > 0 by guard above, but cppcheck can't track this
    uint32_t filled = (elapsed_ticks * BAR_WIDTH) / period_ticks;
    if (filled > BAR_WIDTH) {
        filled = BAR_WIDTH;
    }
    
    // Move cursor to start of this task's row
    ANSI_GOTO(row, 1);
    
    // Print task name and opening bracket
    printf("[%s] ", task_name ? task_name : "unknown");
    
    // Render bar: filled portion, then empty portion
    for (uint32_t i = 0; i < BAR_WIDTH; i++) {
        if (i < filled) {
            printf("█");  // Full block for filled
        } else {
            printf("─");  // Horizontal line for empty
        }
    }
    
    // Print period in ticks (right-aligned)
    printf(" %4" PRIu32 " / %4" PRIu32 " ticks", elapsed_ticks, period_ticks);
    
    // Clear to end of line to avoid leftover characters
    printf("\033[K");
}

/**
 * @brief Render a simple flashing banner (on/off indicator)
 * @param row: terminal row (1-indexed)
 * @param task_name: task name string
 * @param is_on: true to show banner (LED on), false to clear (LED off)
 */
void display_render_banner(uint8_t row, const char* task_name, bool is_on) {
    // Move cursor to start of this task's row
    ANSI_GOTO(row, 1);
    
    if (is_on) {
        // Show banner when LED is on
        printf("[%s] ", task_name);
        // Print a visual indicator (filled blocks)
        for (uint32_t i = 0; i < BAR_WIDTH - 20; i++) {
            printf("★");
        }
        printf(" [%s]", task_name);
        // Clear to end of line to remove any leftover characters
        printf("\033[K");
    } else {
        // Clear the entire line when LED is off
        printf("\033[K");  // Clear from cursor to end of line
    }
}

/**
 * @brief Render a vertical bar showing semaphore fill level
 * @param start_row: top row of the vertical bar (1-indexed)
 * @param col: column position for the bar
 * @param count: current semaphore count
 * @param max_count: maximum semaphore capacity (init_count)
 */
void display_render_vbar(uint8_t start_row, uint8_t col, uint32_t count, uint32_t max_count) {
    // Guard against division by zero
    if (max_count == 0) {
        max_count = 1;
    }
    
    // Clamp count to max
    if (count > max_count) {
        count = max_count;
    }
    
    // Calculate filled rows (bottom-up fill)
    uint32_t filled_rows = (count * VBAR_HEIGHT) / max_count;
    if (filled_rows > VBAR_HEIGHT) {
        filled_rows = VBAR_HEIGHT;
    }
    
    // Draw top border
    ANSI_GOTO(start_row, col);
    printf("┌───┐");
    
    // Draw bar rows (top to bottom, fill from bottom)
    for (uint8_t i = 0; i < VBAR_HEIGHT; i++) {
        ANSI_GOTO(start_row + 1 + i, col);
        // Row is filled if it's in the bottom 'filled_rows' portion
        uint8_t row_from_bottom = VBAR_HEIGHT - 1 - i;
        if (row_from_bottom < filled_rows) {
            printf("│ █ │");
        } else {
            printf("│   │");
        }
    }
    
    // Draw bottom border
    ANSI_GOTO(start_row + VBAR_HEIGHT + 1, col);
    printf("└───┘");
    
    // Draw count label below
    ANSI_GOTO(start_row + VBAR_HEIGHT + 2, col);
    printf(" %2" PRIu32 "/%2" PRIu32, count, max_count);
}

/**
 * @brief Initialize terminal display - clear screen and print header
 */
void display_init(void) {
    static uint8_t initialized = 0;
    if (initialized) {
        return;
    }
    initialized = 1;
    
    // Clear screen and move to top
    printf("\033[2J");      // Clear entire screen
    ANSI_HIDE_CURSOR();    // Hide cursor
    ANSI_GOTO(ROW_HEADER, 1);
    
    // // Print header
    // printf("ICARUS OS v0.0.1 — Preemptive Kernel Scheduler Demo - ARMv7E-M");
    // ANSI_GOTO(ROW_SEPARATOR, 1);
    // printf("************************************************\n");

    // ─────────────────────────────────────────────
    // ICARUS GUI-style header with CORRECT ASCII logo
    // ─────────────────────────────────────────────

    // Top border
    ANSI_GOTO(ROW_HEADER, 1);
    printf("\033[2K");
    printf("┌──────────────────────────────────────────────────────────────┐");

    // ICARUS logo line 1
    ANSI_GOTO(ROW_HEADER + 1, 1);
    printf("\033[2K");
    printf("│   ██╗ ██████╗  █████╗ ██████╗ ██╗   ██╗ ██████╗              │");

    // ICARUS logo line 2
    ANSI_GOTO(ROW_HEADER + 2, 1);
    printf("\033[2K");
    printf("│   ██║██╔════╝ ██╔══██╗██╔══██╗██║   ██║██╔════╝              │");

    // ICARUS logo line 3
    ANSI_GOTO(ROW_HEADER + 3, 1);
    printf("\033[2K");
    printf("│   ██║██║      ███████║██████╔╝██║   ██║╚█████╗               │");

    // ICARUS logo line 4
    ANSI_GOTO(ROW_HEADER + 4, 1);
    printf("\033[2K");
    printf("│   ██║██║      ██╔══██║██╔══██╗██║   ██║ ╚═══██╗              │");

    // ICARUS logo line 5
    ANSI_GOTO(ROW_HEADER + 5, 1);
    printf("\033[2K");
    printf("│   ██║╚██████╗ ██║  ██║██║  ██║╚██████╔╝██████╔╝              │");

    // ICARUS logo line 6
    ANSI_GOTO(ROW_HEADER + 6, 1);
    printf("\033[2K");
    printf("│   ╚═╝ ╚═════╝ ╚═╝  ╚═╝╚═╝  ╚═╝ ╚═════╝ ╚═════╝               │");

    // Subtitle / metadata
    ANSI_GOTO(ROW_HEADER + 7, 1);
    printf("\033[2K");
    printf("│   Preemptive Kernel • ARMv7E-M • STM32H750                   │");

    // Bottom border
    ANSI_GOTO(ROW_SEPARATOR, 1);
    printf("\033[2K");
    printf("└──────────────────────────────────────────────────────────────┘");


    
    // Initialize heartbeat row (if enabled)
#if ENABLE_HEARTBEAT_VISUALIZATION
    display_render_banner(ROW_HEARTBEAT, "heartbeat", false);  // Start with banner off
#endif
    
    // Initialize task rows with empty bars using task names from TCB
    // Note: This assumes tasks are registered in order and we skip system tasks
    // System tasks are registered first (ICARUS_KEEPALIVE_TASK, ICARUS_HEARTBEART_TASK)
    // User tasks start from index 2
    extern task_t* task_list[MAX_TASKS];
    extern uint8_t num_created_tasks;
    
    uint8_t user_task_count = 0;
    for (uint8_t i = 0; i < num_created_tasks && user_task_count < 3; i++) {
        // Skip system tasks (they start with "ICARUS_")
        if (task_list[i] != NULL && task_list[i]->name[0] != '\0') {
            const char* name = task_list[i]->name;
            if (strncmp(name, "ICARUS_", 7) != 0) {
                // This is a user task
                uint8_t row = ROW_TASK_A + user_task_count;
                uint32_t period = TASK_A_PERIOD_TICKS;
                if (user_task_count == 1) period = TASK_B_PERIOD_TICKS;
                else if (user_task_count == 2) period = TASK_C_PERIOD_TICKS;
                
                display_render_bar(row, name, 0, period);
                user_task_count++;
            }
        }
    }
    
    // Move cursor below display area to avoid interference
#if ENABLE_HEARTBEAT_VISUALIZATION
    ANSI_GOTO(ROW_TASK_C + 2, 1);
#else
    ANSI_GOTO(ROW_TASK_C + 2, 1);  // Same position since rows shift when heartbeat is disabled
#endif
}
