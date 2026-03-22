/**
 * @file    display.c
 * @brief   ICARUS OS Terminal Display Implementation
 * @version 0.1.0
 *
 * @details Implements ANSI terminal-based visualization for real-time kernel
 *          monitoring. Provides progress bars, semaphore indicators, message
 *          queue visualizations, and rolling message history displays.
 *
 * @par Terminal Requirements:
 *      - ANSI escape code support (VT100 compatible)
 *      - UTF-8 encoding for box drawing characters
 *      - Minimum 80x24 terminal size (100x40 recommended)
 *      - 115200 baud USB CDC connection
 *
 * @par Color Coding:
 *      - Green: Producers, successful operations
 *      - Magenta: Consumers, receive operations
 *      - Cyan: Headers, labels, informational
 *      - Yellow: Warnings, queue fill indicators
 *
 * @see     docs/do178c/design/SDD.md Section 6 - Display Subsystem
 *
 * @author  Souham Biswas
 * @date    2025
 *
 * @copyright Copyright 2025 Souham Biswas
 *            https://github.com/ironhide23586/icarus-os-core
 *            Licensed under the Apache License, Version 2.0
 */

#include "bsp/display.h"
#include "icarus/kernel.h"
#include "icarus/scheduler.h"
#include <string.h>
#include <inttypes.h>

/* ============================================================================
 * ANSI ESCAPE CODE MACROS
 * ========================================================================= */

/**
 * @defgroup ANSI_CODES ANSI Escape Codes
 * @brief    Terminal control sequences for cursor and color manipulation
 * @{
 */

/** @brief Move cursor to specified row and column (1-indexed) */
#define ANSI_GOTO(row, col) printf("\033[%d;%dH", (row), (col))

/** @brief Show terminal cursor */
#define ANSI_SHOW_CURSOR()  printf("\033[?25h")

/** @brief Hide terminal cursor */
#define ANSI_HIDE_CURSOR()  printf("\033[?25l")

/** @brief Reset all text attributes */
#define ANSI_RESET      "\033[0m"

/** @brief Bold text attribute */
#define ANSI_BOLD       "\033[1m"

/** @brief Dim text attribute */
#define ANSI_DIM        "\033[2m"

/** @brief Green foreground color */
#define ANSI_GREEN      "\033[32m"

/** @brief Yellow foreground color */
#define ANSI_YELLOW     "\033[33m"

/** @brief Magenta foreground color */
#define ANSI_MAGENTA    "\033[35m"

/** @brief Cyan foreground color */
#define ANSI_CYAN       "\033[36m"

/** @brief White foreground color */
#define ANSI_WHITE      "\033[37m"

/** @brief Green background color */
#define ANSI_BG_GREEN   "\033[42m"

/** @brief Magenta background color */
#define ANSI_BG_MAGENTA "\033[45m"

/** @} */ /* End of ANSI_CODES */

/* ============================================================================
 * MESSAGE HISTORY IMPLEMENTATION
 * ========================================================================= */

/**
 * @brief   Initialize a message history buffer
 *
 * @details Clears all entries and resets head/count to zero.
 *          Must be called before using the history buffer.
 *
 * @param[out] hist  Pointer to history buffer to initialize
 *
 * @pre     hist != NULL
 * @post    hist->count == 0, hist->head == 0
 */
void msg_history_init(msg_history_t* hist) {
    if (hist == NULL) return;
    memset(hist, 0, sizeof(msg_history_t));
}

/**
 * @brief   Add a message to the history buffer
 *
 * @details Adds a new entry at the head position using circular buffer
 *          semantics. Oldest entries are overwritten when buffer is full.
 *
 * @param[in,out] hist      Pointer to history buffer
 * @param[in]     data      Message data bytes
 * @param[in]     len       Number of bytes in message (clamped to MSG_HISTORY_MAX_BYTES)
 * @param[in]     source_id Producer/consumer ID (0-254, 0xFF = unknown)
 * @param[in]     is_send   true = send event, false = receive event
 *
 * @pre     hist != NULL
 * @pre     data != NULL
 * @pre     len > 0
 */
void msg_history_add(msg_history_t* hist, const uint8_t* data, uint8_t len, 
                     uint8_t source_id, bool is_send) {
    if (hist == NULL || data == NULL || len == 0) return;
    
    // Clamp length to max
    if (len > MSG_HISTORY_MAX_BYTES) {
        len = MSG_HISTORY_MAX_BYTES;
    }
    
    // Add entry at head position
    msg_history_entry_t* entry = &hist->entries[hist->head];
    memcpy(entry->data, data, len);
    entry->len = len;
    entry->source_id = source_id;
    entry->is_send = is_send;
    
    // Advance head (circular)
    hist->head = (uint8_t)(hist->head + 1) % MSG_HISTORY_LEN;
    if (hist->count < MSG_HISTORY_LEN) {
        hist->count++;
    }
}

/**
 * @brief   Render message history as a rolling window display
 *
 * @details Displays the last MSG_HISTORY_LEN messages in a bordered panel.
 *          Uses ASCII box drawing characters for terminal compatibility.
 *          Color-codes entries: green for sends, magenta for receives.
 *
 * @param[in] row   Starting terminal row (1-indexed)
 * @param[in] col   Column position for left edge of panel
 * @param[in] hist  Pointer to history buffer to render
 * @param[in] label Short label for panel header (e.g., "SS", "SM")
 *
 * @par Display Format:
 * @verbatim
 *     +---SS---+
 *     |>P0: 42|   <- Producer 0 sent 42
 *     |<C0: 42|   <- Consumer 0 received 42
 *     |>P0: 43|
 *     +---------+
 * @endverbatim
 */
void display_render_msg_history(uint8_t row, uint8_t col, msg_history_t* hist, const char* label) {
    if (hist == NULL) return;
    
    // Header - use ASCII for compatibility
    ANSI_GOTO(row, col);
    printf(ANSI_CYAN ANSI_BOLD "+---%s---+" ANSI_RESET, label);
    
    // Calculate starting index (oldest entry)
    uint8_t start_idx;
    if (hist->count < MSG_HISTORY_LEN) {
        start_idx = 0;
    } else {
        start_idx = hist->head;
    }
    
    // Render each history entry
    for (uint8_t i = 0; i < MSG_HISTORY_LEN; i++) {
        ANSI_GOTO(row + 1 + i, col);
        
        if (i < hist->count) {
            uint8_t idx = (start_idx + i) % MSG_HISTORY_LEN;
            msg_history_entry_t* entry = &hist->entries[idx];
            
            if (entry->is_send) {
                printf(ANSI_GREEN "|>P%d:", entry->source_id);
            } else {
                printf(ANSI_MAGENTA "|<C%d:", entry->source_id);
            }
            
            if (entry->len == 1) {
                printf("%3d" ANSI_RESET "|", entry->data[0]);
            } else {
                for (uint8_t j = 0; j < entry->len && j < 3; j++) {
                    printf("%02X", entry->data[j]);
                }
                printf(ANSI_RESET "|");
            }
        } else {
            printf("|        |");
        }
    }
    
    // Footer
    ANSI_GOTO(row + MSG_HISTORY_LEN + 1, col);
    printf(ANSI_CYAN "+---------+" ANSI_RESET);
}

/**
 * @brief   Render a horizontal progress bar for task visualization
 *
 * @details Displays a fixed-width progress bar showing task execution progress
 *          within its current period. Uses Unicode block characters for smooth
 *          fill animation.
 *
 * @param[in] row           Terminal row (1-indexed)
 * @param[in] task_name     Task name string (from TCB, max 16 chars displayed)
 * @param[in] elapsed_ticks Ticks elapsed in current period
 * @param[in] period_ticks  Total ticks for this period (must be > 0)
 *
 * @par Display Format:
 * @verbatim
 *     [task_name] ████████████────────────────  160 / 2000 ticks
 * @endverbatim
 *
 * @note    Clears to end of line to prevent display artifacts
 * @note    Division by zero is guarded (period_ticks forced to 1 if 0)
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
 * @brief   Render a flashing banner indicator (heartbeat display)
 *
 * @details Displays or clears a star-filled banner synchronized with the
 *          heartbeat LED. Used to provide visual confirmation that the
 *          kernel scheduler is running.
 *
 * @param[in] row       Terminal row (1-indexed)
 * @param[in] task_name Task name to display at banner edges
 * @param[in] is_on     true = show banner (LED on), false = clear line (LED off)
 *
 * @par Display Format (when on):
 * @verbatim
 *     [>ICARUS_HEARTBEAT<] ★★★★★★★★★★★★★★★★★★★★ [>ICARUS_HEARTBEAT<]
 * @endverbatim
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
 * @brief   Render a vertical bar showing semaphore fill level
 *
 * @details Displays a vertical bar chart representing semaphore count as a
 *          percentage of maximum capacity. Bar fills from bottom to top.
 *          Uses ASCII box drawing for terminal compatibility.
 *
 * @param[in] start_row Top row of the vertical bar (1-indexed)
 * @param[in] col       Column position for left edge
 * @param[in] count     Current semaphore count
 * @param[in] max_count Maximum semaphore capacity
 *
 * @par Display Format:
 * @verbatim
 *       SEM
 *     +---+
 *     |###|  <- Filled portion (green)
 *     |###|
 *     |   |  <- Empty portion
 *     |   |
 *     +---+
 *      3/10  <- Current/max count
 * @endverbatim
 *
 * @note    Division by zero is guarded (max_count forced to 1 if 0)
 */
void display_render_vbar(uint8_t start_row, uint8_t col, uint32_t count, uint32_t max_count) {
    if (max_count == 0) {
        max_count = 1;
    }
    if (count > max_count) {
        count = max_count;
    }
    
    uint32_t filled_rows = (count * VBAR_HEIGHT) / max_count;
    if (filled_rows > VBAR_HEIGHT) {
        filled_rows = VBAR_HEIGHT;
    }
    
    // Label
    ANSI_GOTO(start_row - 1, col);
    printf(ANSI_CYAN "SEM" ANSI_RESET);
    
    // Top border
    ANSI_GOTO(start_row, col);
    printf("+---+");
    
    // Bar rows (fill from bottom)
    for (uint8_t i = 0; i < VBAR_HEIGHT; i++) {
        ANSI_GOTO(start_row + 1 + i, col);
        uint8_t row_from_bottom = VBAR_HEIGHT - 1 - i;
        if (row_from_bottom < filled_rows) {
            printf(ANSI_GREEN "|###|" ANSI_RESET);
        } else {
            printf("|   |");
        }
    }
    
    // Bottom border
    ANSI_GOTO(start_row + VBAR_HEIGHT + 1, col);
    printf("+---+");
    
    // Count label
    ANSI_GOTO(start_row + VBAR_HEIGHT + 2, col);
    printf("%2" PRIu32 "/%2" PRIu32, count, max_count);
}

/**
 * @brief   Render a message queue (pipe) visualization panel
 *
 * @details Displays a compact panel showing pipe fill level and recent
 *          send/receive activity. Includes horizontal fill bar and
 *          directional arrows for message flow indication.
 *
 * @param[in] start_row Top row of the panel (1-indexed)
 * @param[in] col       Column position for left edge
 * @param[in] label     Short label for the queue (e.g., "Q1", "PIPE0")
 * @param[in] count     Current byte count in pipe
 * @param[in] max_count Maximum pipe capacity in bytes
 * @param[in] last_sent Last message value sent (0-255)
 * @param[in] last_recv Last message value received (0-255)
 * @param[in] show_sent true to display sent indicator with animation
 * @param[in] show_recv true to display received indicator with animation
 *
 * @par Display Format:
 * @verbatim
 *     PIPE0
 *     [████────]
 *      12/32
 *     →123      <- Green arrow + sent value
 *     ← 45      <- Magenta arrow + received value
 * @endverbatim
 */
void display_render_pipe(uint8_t start_row, uint8_t col, const char* label,
                         uint8_t count, uint8_t max_count,
                         uint8_t last_sent, uint8_t last_recv,
                         bool show_sent, bool show_recv) {
    // Guard against division by zero
    if (max_count == 0) {
        max_count = 1;
    }
    if (count > max_count) {
        count = max_count;
    }
    
    // Calculate fill percentage for horizontal bar
    uint8_t bar_width = 8;
    uint8_t filled = (uint8_t) (count * bar_width) / max_count;
    
    // Draw label
    ANSI_GOTO(start_row, col);
    printf("\033[36m%s\033[0m", label);  // Cyan label
    
    // Draw queue visualization: [████────] 
    ANSI_GOTO(start_row + 1, col);
    printf("[");
    for (uint8_t i = 0; i < bar_width; i++) {
        if (i < filled) {
            printf("\033[33m█\033[0m");  // Yellow filled
        } else {
            printf("─");
        }
    }
    printf("]");
    
    // Draw count
    ANSI_GOTO(start_row + 2, col);
    printf("%2d/%2d", count, max_count);
    
    // Draw sent indicator with arrow animation
    ANSI_GOTO(start_row + 3, col);
    if (show_sent) {
        printf("\033[32m→%3d\033[0m", last_sent);  // Green arrow + value
    } else {
        printf("     ");
    }
    
    // Draw received indicator
    ANSI_GOTO(start_row + 4, col);
    if (show_recv) {
        printf("\033[35m←%3d\033[0m", last_recv);  // Magenta arrow + value
    } else {
        printf("     ");
    }
}

/**
 * @brief   Render a producer task progress bar with message indicator
 *
 * @details Displays a green-colored progress bar for producer tasks with
 *          an animated arrow indicator showing the last sent message value.
 *
 * @param[in] row           Terminal row (1-indexed)
 * @param[in] task_name     Task name string
 * @param[in] elapsed_ticks Ticks elapsed in current period
 * @param[in] period_ticks  Total ticks for this period
 * @param[in] msg_value     Last message value sent (0-255)
 * @param[in] show_msg      true to show message indicator with flash effect
 *
 * @par Display Format:
 * @verbatim
 *     [producer] ████████████────────  160/2000 →[42]
 * @endverbatim
 */
void display_render_producer(uint8_t row, const char* task_name, 
                             uint32_t elapsed_ticks, uint32_t period_ticks,
                             uint8_t msg_value, bool show_msg) {
    // Guard against division by zero
    if (period_ticks == 0) {
        period_ticks = 1;
    }
    if (elapsed_ticks > period_ticks) {
        elapsed_ticks = period_ticks;
    }
    
    uint32_t filled = (elapsed_ticks * BAR_WIDTH) / period_ticks;
    if (filled > BAR_WIDTH) {
        filled = BAR_WIDTH;
    }
    
    ANSI_GOTO(row, 1);
    
    // Task name with producer color (green)
    printf("\033[32m[%s]\033[0m ", task_name ? task_name : "unknown");
    
    // Progress bar
    for (uint32_t i = 0; i < BAR_WIDTH; i++) {
        if (i < filled) {
            printf("\033[32m█\033[0m");  // Green filled
        } else {
            printf("─");
        }
    }
    
    // Timing info
    printf(" %4" PRIu32 "/%4" PRIu32, elapsed_ticks, period_ticks);
    
    // Message indicator with animation
    if (show_msg) {
        // Flash effect: show arrow and value
        printf(" \033[32;1m→[%3d]\033[0m", msg_value);
    } else {
        printf("        ");
    }
    
    printf("\033[K");
}

/**
 * @brief   Render a consumer task progress bar with message indicator
 *
 * @details Displays a magenta-colored progress bar for consumer tasks with
 *          an animated arrow indicator showing the last received message value.
 *
 * @param[in] row           Terminal row (1-indexed)
 * @param[in] task_name     Task name string
 * @param[in] elapsed_ticks Ticks elapsed in current period
 * @param[in] period_ticks  Total ticks for this period
 * @param[in] msg_value     Last message value received (0-255)
 * @param[in] show_msg      true to show message indicator with flash effect
 *
 * @par Display Format:
 * @verbatim
 *     [consumer] ████████████────────  160/2000 ←[42]
 * @endverbatim
 */
void display_render_consumer(uint8_t row, const char* task_name,
                             uint32_t elapsed_ticks, uint32_t period_ticks,
                             uint8_t msg_value, bool show_msg) {
    // Guard against division by zero
    if (period_ticks == 0) {
        period_ticks = 1;
    }
    if (elapsed_ticks > period_ticks) {
        elapsed_ticks = period_ticks;
    }
    
    uint32_t filled = (elapsed_ticks * BAR_WIDTH) / period_ticks;
    if (filled > BAR_WIDTH) {
        filled = BAR_WIDTH;
    }
    
    ANSI_GOTO(row, 1);
    
    // Task name with consumer color (magenta)
    printf("\033[35m[%s]\033[0m ", task_name ? task_name : "unknown");
    
    // Progress bar
    for (uint32_t i = 0; i < BAR_WIDTH; i++) {
        if (i < filled) {
            printf("\033[35m█\033[0m");  // Magenta filled
        } else {
            printf("─");
        }
    }
    
    // Timing info
    printf(" %4" PRIu32 "/%4" PRIu32, elapsed_ticks, period_ticks);
    
    // Message indicator with animation
    if (show_msg) {
        // Flash effect: show arrow and value
        printf(" \033[35;1m←[%3d]\033[0m", msg_value);
    } else {
        printf("        ");
    }
    
    printf("\033[K");
}

/**
 * @brief   Initialize terminal display subsystem
 *
 * @details Performs one-time initialization of the terminal display:
 *          - Clears screen and hides cursor
 *          - Renders ICARUS ASCII art logo header
 *          - Initializes task progress bar rows
 *          - Sets up heartbeat visualization (if enabled)
 *
 * @pre     USB CDC must be initialized and connected
 * @post    Terminal ready for real-time updates
 *
 * @note    This function is idempotent - subsequent calls are no-ops
 * @note    Called automatically by os_idle_task on first run
 *
 * @par Logo Display:
 * @verbatim
 *     ┌──────────────────────────────────────────────────────────────┐
 *     │   ██╗ ██████╗  █████╗ ██████╗ ██╗   ██╗ ██████╗              │
 *     │   ██║██╔════╝ ██╔══██╗██╔══██╗██║   ██║██╔════╝              │
 *     │   ██║██║      ███████║██████╔╝██║   ██║╚█████╗               │
 *     │   ██║██║      ██╔══██║██╔══██╗██║   ██║ ╚═══██╗              │
 *     │   ██║╚██████╗ ██║  ██║██║  ██║╚██████╔╝██████╔╝              │
 *     │   ╚═╝ ╚═════╝ ╚═╝  ╚═╝╚═╝  ╚═╝ ╚═════╝ ╚═════╝               │
 *     │   Preemptive Kernel • ARMv7E-M • STM32H750                   │
 *     └──────────────────────────────────────────────────────────────┘
 * @endverbatim
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

    // TEST: Simple message after header
    printf("\r\nDTCM Protection Active - System Running\r\n");
    return;  // TEMPORARY: Skip task iteration to isolate issue

    
    // Initialize heartbeat row (if enabled)
#if ENABLE_HEARTBEAT_VISUALIZATION
    display_render_banner(ROW_HEARTBEAT, "heartbeat", false);  // Start with banner off
#endif
    
    // Initialize task rows with empty bars using task names from TCB
    // Note: This assumes tasks are registered in order and we skip system tasks
    // System tasks are registered first (ICARUS_KEEPALIVE_TASK, ICARUS_HEARTBEAT_TASK)
    // User tasks start from index 2
    
    uint8_t num_tasks = os_get_num_created_tasks();
    uint8_t user_task_count = 0;
    
    for (uint8_t i = 0; i < num_tasks && user_task_count < 3; i++) {
        // Get task name via SVC (safe with DTCM priv-only)
        const char* name = os_get_task_name(i);
        
        if (name != NULL && name[0] != '\0') {
            // Skip system tasks (they start with "ICARUS_")
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
