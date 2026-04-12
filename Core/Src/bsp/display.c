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
 * @copyright Copyright 2025-2026 Souham Biswas
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
#define ANSI_GOTO(row, col) (void)printf("\033[%d;%dH", (row), (col))

/** @brief Show terminal cursor */
#define ANSI_SHOW_CURSOR()  (void)printf("\033[?25h")

/** @brief Hide terminal cursor */
#define ANSI_HIDE_CURSOR()  (void)printf("\033[?25l")

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
    if (hist == NULL) {
        return;
    }
    (void)memset(hist, 0, sizeof(msg_history_t));
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
    if ((hist == NULL) || (data == NULL) || (len == 0u)) {
        return;
    }

    // Clamp length to max
    uint8_t clamped_len = len;
    if (clamped_len > (uint8_t)MSG_HISTORY_MAX_BYTES) {
        clamped_len = (uint8_t)MSG_HISTORY_MAX_BYTES;
    }

    // Add entry at head position
    msg_history_entry_t* entry = &hist->entries[hist->head];
    (void)memcpy(entry->data, data, clamped_len);
    entry->len = clamped_len;
    entry->source_id = source_id;
    entry->is_send = is_send;

    // Advance head (circular)
    hist->head = (uint8_t)((uint8_t)(hist->head + 1u) % (uint8_t)MSG_HISTORY_LEN);
    if (hist->count < (uint8_t)MSG_HISTORY_LEN) {
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
    if (hist == NULL) {
        return;
    }
    
    // Header - use ASCII for compatibility
    ANSI_GOTO(row, col);
    (void)printf(ANSI_CYAN ANSI_BOLD "+---%s---+" ANSI_RESET, label);

    // Calculate starting index (oldest entry)
    uint8_t start_idx;
    if (hist->count < (uint8_t)MSG_HISTORY_LEN) {
        start_idx = 0u;
    } else {
        start_idx = hist->head;
    }

    // Render each history entry
    for (uint8_t i = 0u; i < (uint8_t)MSG_HISTORY_LEN; i++) {
        int goto_row = (int)row + 1 + (int)i;
        ANSI_GOTO(goto_row, col);

        if (i < hist->count) {
            uint8_t idx = (uint8_t)((start_idx + i) % (uint8_t)MSG_HISTORY_LEN);
            msg_history_entry_t* entry = &hist->entries[idx];

            if (entry->is_send) {
                (void)printf(ANSI_GREEN "|>P%d:", entry->source_id);
            } else {
                (void)printf(ANSI_MAGENTA "|<C%d:", entry->source_id);
            }

            if (entry->len == 1u) {
                (void)printf("%3d" ANSI_RESET "|", entry->data[0]);
            } else {
                for (uint8_t j = 0u; (j < entry->len) && (j < 3u); j++) {
                    (void)printf("%02X", entry->data[j]);
                }
                (void)printf(ANSI_RESET "|");
            }
        } else {
            (void)printf("|        |");
        }
    }

    // Footer
    {
        int footer_row = (int)row + (int)MSG_HISTORY_LEN + 1;
        ANSI_GOTO(footer_row, col);
    }
    (void)printf(ANSI_CYAN "+---------+" ANSI_RESET);
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
    uint32_t period = period_ticks;
    if (period == 0u) {
        period = 1u;
    }

    // Clamp elapsed to period to avoid overflow
    uint32_t elapsed = elapsed_ticks;
    if (elapsed > period) {
        elapsed = period;
    }

    // Calculate filled portion (0 to BAR_WIDTH)
    // cppcheck-suppress zerodivcond
    // period is guaranteed > 0 by guard above, but cppcheck can't track this
    uint32_t filled = (elapsed * (uint32_t)BAR_WIDTH) / period;
    if (filled > (uint32_t)BAR_WIDTH) {
        filled = (uint32_t)BAR_WIDTH;
    }

    // Move cursor to start of this task's row
    ANSI_GOTO(row, 1);

    // Print task name and opening bracket
    (void)printf("[%s] ", (task_name != NULL) ? task_name : "unknown");

    // Render bar: filled portion, then empty portion
    for (uint32_t i = 0u; i < (uint32_t)BAR_WIDTH; i++) {
        if (i < filled) {
            (void)printf("█");  // Full block for filled
        } else {
            (void)printf("─");  // Horizontal line for empty
        }
    }

    // Print period in ticks (right-aligned)
    (void)printf(" %4" PRIu32 " / %4" PRIu32 " ticks", elapsed, period);

    // Clear to end of line to avoid leftover characters
    (void)printf("\033[K");
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
        (void)printf("[%s] ", task_name);
        // Print a visual indicator (filled blocks)
        for (uint32_t i = 0u; i < ((uint32_t)BAR_WIDTH - 20u); i++) {
            (void)printf("★");
        }
        (void)printf(" [%s]", task_name);
        // Clear to end of line to remove any leftover characters
        (void)printf("\033[K");
    } else {
        // Clear the entire line when LED is off
        (void)printf("\033[K");  // Clear from cursor to end of line
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
    uint32_t mc = max_count;
    if (mc == 0u) {
        mc = 1u;
    }
    uint32_t cnt = count;
    if (cnt > mc) {
        cnt = mc;
    }

    uint32_t filled_rows = (cnt * (uint32_t)VBAR_HEIGHT) / mc;
    if (filled_rows > (uint32_t)VBAR_HEIGHT) {
        filled_rows = (uint32_t)VBAR_HEIGHT;
    }

    // Label
    {
        int label_row = (int)start_row - 1;
        ANSI_GOTO(label_row, col);
    }
    (void)printf(ANSI_CYAN "SEM" ANSI_RESET);

    // Top border
    ANSI_GOTO(start_row, col);
    (void)printf("+---+");

    // Bar rows (fill from bottom)
    for (uint8_t i = 0u; i < (uint8_t)VBAR_HEIGHT; i++) {
        int bar_row = (int)start_row + 1 + (int)i;
        ANSI_GOTO(bar_row, col);
        uint8_t row_from_bottom = (uint8_t)((uint8_t)VBAR_HEIGHT - 1u) - i;
        if ((uint32_t)row_from_bottom < filled_rows) {
            (void)printf(ANSI_GREEN "|###|" ANSI_RESET);
        } else {
            (void)printf("|   |");
        }
    }

    // Bottom border
    {
        int bottom_row = (int)start_row + (int)VBAR_HEIGHT + 1;
        ANSI_GOTO(bottom_row, col);
    }
    (void)printf("+---+");

    // Count label
    {
        int count_row = (int)start_row + (int)VBAR_HEIGHT + 2;
        ANSI_GOTO(count_row, col);
    }
    (void)printf("%2" PRIu32 "/%2" PRIu32, cnt, mc);
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
    uint8_t mc = max_count;
    if (mc == 0u) {
        mc = 1u;
    }
    uint8_t cnt = count;
    if (cnt > mc) {
        cnt = mc;
    }

    // Calculate fill percentage for horizontal bar
    uint8_t bar_width = 8u;
    uint8_t filled = (uint8_t)((uint8_t)(cnt * bar_width) / mc);

    // Draw label
    ANSI_GOTO(start_row, col);
    (void)printf("\033[36m%s\033[0m", label);  // Cyan label

    // Draw queue visualization: [████────]
    {
        int q_row = (int)start_row + 1;
        ANSI_GOTO(q_row, col);
    }
    (void)printf("[");
    for (uint8_t i = 0u; i < bar_width; i++) {
        if (i < filled) {
            (void)printf("\033[33m█\033[0m");  // Yellow filled
        } else {
            (void)printf("─");
        }
    }
    (void)printf("]");

    // Draw count
    {
        int cnt_row = (int)start_row + 2;
        ANSI_GOTO(cnt_row, col);
    }
    (void)printf("%2d/%2d", cnt, mc);

    // Draw sent indicator with arrow animation
    {
        int sent_row = (int)start_row + 3;
        ANSI_GOTO(sent_row, col);
    }
    if (show_sent) {
        (void)printf("\033[32m→%3d\033[0m", last_sent);  // Green arrow + value
    } else {
        (void)printf("     ");
    }

    // Draw received indicator
    {
        int recv_row = (int)start_row + 4;
        ANSI_GOTO(recv_row, col);
    }
    if (show_recv) {
        (void)printf("\033[35m←%3d\033[0m", last_recv);  // Magenta arrow + value
    } else {
        (void)printf("     ");
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
    uint32_t period = period_ticks;
    if (period == 0u) {
        period = 1u;
    }
    uint32_t elapsed = elapsed_ticks;
    if (elapsed > period) {
        elapsed = period;
    }

    uint32_t filled = (elapsed * (uint32_t)BAR_WIDTH) / period;
    if (filled > (uint32_t)BAR_WIDTH) {
        filled = (uint32_t)BAR_WIDTH;
    }

    ANSI_GOTO(row, 1);

    // Task name with producer color (green)
    (void)printf("\033[32m[%s]\033[0m ", (task_name != NULL) ? task_name : "unknown");

    // Progress bar
    for (uint32_t i = 0u; i < (uint32_t)BAR_WIDTH; i++) {
        if (i < filled) {
            (void)printf("\033[32m█\033[0m");  // Green filled
        } else {
            (void)printf("─");
        }
    }

    // Timing info
    (void)printf(" %4" PRIu32 "/%4" PRIu32, elapsed, period);

    // Message indicator with animation
    if (show_msg) {
        // Flash effect: show arrow and value
        (void)printf(" \033[32;1m→[%3d]\033[0m", msg_value);
    } else {
        (void)printf("        ");
    }

    (void)printf("\033[K");
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
    uint32_t period = period_ticks;
    if (period == 0u) {
        period = 1u;
    }
    uint32_t elapsed = elapsed_ticks;
    if (elapsed > period) {
        elapsed = period;
    }

    uint32_t filled = (elapsed * (uint32_t)BAR_WIDTH) / period;
    if (filled > (uint32_t)BAR_WIDTH) {
        filled = (uint32_t)BAR_WIDTH;
    }

    ANSI_GOTO(row, 1);

    // Task name with consumer color (magenta)
    (void)printf("\033[35m[%s]\033[0m ", (task_name != NULL) ? task_name : "unknown");

    // Progress bar
    for (uint32_t i = 0u; i < (uint32_t)BAR_WIDTH; i++) {
        if (i < filled) {
            (void)printf("\033[35m█\033[0m");  // Magenta filled
        } else {
            (void)printf("─");
        }
    }

    // Timing info
    (void)printf(" %4" PRIu32 "/%4" PRIu32, elapsed, period);

    // Message indicator with animation
    if (show_msg) {
        // Flash effect: show arrow and value
        (void)printf(" \033[35;1m←[%3d]\033[0m", msg_value);
    } else {
        (void)printf("        ");
    }

    (void)printf("\033[K");
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
    static uint8_t initialized = 0u;
    if (initialized != 0u) {
        return;
    }
    initialized = 1u;
    
    // Clear screen and move to top
    (void)printf("\033[2J");      // Clear entire screen
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
    (void)printf("\033[2K");
    (void)printf("┌──────────────────────────────────────────────────────────────┐");

    // ICARUS logo line 1
    ANSI_GOTO(ROW_HEADER + 1, 1);
    (void)printf("\033[2K");
    (void)printf("│   ██╗ ██████╗  █████╗ ██████╗ ██╗   ██╗ ██████╗              │");

    // ICARUS logo line 2
    ANSI_GOTO(ROW_HEADER + 2, 1);
    (void)printf("\033[2K");
    (void)printf("│   ██║██╔════╝ ██╔══██╗██╔══██╗██║   ██║██╔════╝              │");

    // ICARUS logo line 3
    ANSI_GOTO(ROW_HEADER + 3, 1);
    (void)printf("\033[2K");
    (void)printf("│   ██║██║      ███████║██████╔╝██║   ██║╚█████╗               │");

    // ICARUS logo line 4
    ANSI_GOTO(ROW_HEADER + 4, 1);
    (void)printf("\033[2K");
    (void)printf("│   ██║██║      ██╔══██║██╔══██╗██║   ██║ ╚═══██╗              │");

    // ICARUS logo line 5
    ANSI_GOTO(ROW_HEADER + 5, 1);
    (void)printf("\033[2K");
    (void)printf("│   ██║╚██████╗ ██║  ██║██║  ██║╚██████╔╝██████╔╝              │");

    // ICARUS logo line 6
    ANSI_GOTO(ROW_HEADER + 6, 1);
    (void)printf("\033[2K");
    (void)printf("│   ╚═╝ ╚═════╝ ╚═╝  ╚═╝╚═╝  ╚═╝ ╚═════╝ ╚═════╝               │");

    // Subtitle / metadata
    ANSI_GOTO(ROW_HEADER + 7, 1);
    (void)printf("\033[2K");
    (void)printf("│   Preemptive Kernel • ARMv7E-M • STM32H750                   │");

    // Bottom border
    ANSI_GOTO(ROW_SEPARATOR, 1);
    (void)printf("\033[2K");
    (void)printf("└──────────────────────────────────────────────────────────────┘");

    // TEST: Simple message after header
    (void)printf("\r\nDTCM Protection Active - System Running\r\n");
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
    uint8_t user_task_count = 0u;

    for (uint8_t i = 0u; (i < num_tasks) && (user_task_count < 3u); i++) {
        // Get task name via SVC (safe with DTCM priv-only)
        const char* name = os_get_task_name(i);

        if ((name != NULL) && (name[0] != '\0')) {
            // Skip system tasks (they start with "ICARUS_")
            if (strncmp(name, "ICARUS_", 7) != 0) {
                // This is a user task
                uint8_t disp_row = (uint8_t)((uint8_t)ROW_TASK_A + user_task_count);
                uint32_t period = (uint32_t)TASK_A_PERIOD_TICKS;
                if (user_task_count == 1u) {
                    period = (uint32_t)TASK_B_PERIOD_TICKS;
                } else if (user_task_count == 2u) {
                    period = (uint32_t)TASK_C_PERIOD_TICKS;
                } else {
                    /* keep default */
                }

                display_render_bar(disp_row, name, 0u, period);
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
