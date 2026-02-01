/**
 * @file    bsp_display.h
 * @brief   Board Support Package - Terminal Display Driver
 * @version 0.1.0
 *
 * @details ANSI terminal-based display for task visualization.
 *          Renders progress bars, status indicators, and message queues
 *          over USB CDC serial connection.
 *
 * @par Terminal Requirements:
 *      - ANSI escape code support
 *      - UTF-8 encoding (for box drawing characters)
 *      - Minimum 80x24 terminal size recommended
 *
 * @author  Souham Biswas
 * @date    2026
 *
 * @copyright Copyright (c) 2026 ICARUS Project
 */

#ifndef BSP_DISPLAY_H
#define BSP_DISPLAY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "icarus/icarus_config.h"

/* ============================================================================
 * DISPLAY CONFIGURATION
 * ========================================================================= */

/**
 * @defgroup DISPLAY_CONFIG Display Configuration
 * @brief    Terminal layout and timing parameters
 * @{
 */

/* Feature flags */
#define ENABLE_HEARTBEAT_VISUALIZATION  ICARUS_ENABLE_HEARTBEAT_VIS

/* Terminal row assignments (1-indexed) */
#define ROW_HEADER      1       /**< Header/logo start row */
#define ROW_SEPARATOR   9       /**< Separator line row */

#if ENABLE_HEARTBEAT_VISUALIZATION
#define ROW_HEARTBEAT   10      /**< Heartbeat indicator row */
#define ROW_TASK_A      11      /**< First task progress bar */
#define ROW_TASK_B      12      /**< Second task progress bar */
#define ROW_TASK_C      13      /**< Third task progress bar */
#else
#define ROW_TASK_A      11
#define ROW_TASK_B      12
#define ROW_TASK_C      13
#endif

/* Progress bar dimensions */
#define BAR_WIDTH       40      /**< Progress bar width in characters */
#define BAR_COL_START   6       /**< Column where bar starts */

/* Vertical bar (semaphore) dimensions */
#define VBAR_HEIGHT     10      /**< Vertical bar height in rows */
#define VBAR_COL        70      /**< Column position for vertical bar */

/* Task timing (in OS ticks) */
#define HEARTBEAT_ON_TICKS      ICARUS_HEARTBEAT_ON_TICKS
#define HEARTBEAT_OFF_TICKS     ICARUS_HEARTBEAT_OFF_TICKS
#define HEARTBEAT_PERIOD_TICKS  (HEARTBEAT_ON_TICKS + HEARTBEAT_OFF_TICKS)
#define TASK_A_PERIOD_TICKS     2000
#define TASK_B_PERIOD_TICKS     4000
#define TASK_C_PERIOD_TICKS     3000

/* Rendering timing */
#define RENDER_INTERVAL_TICKS   20      /**< Display update interval */
#define CYCLE_PAUSE_TICKS       100     /**< Pause between cycles */

/** @} */

/* ============================================================================
 * MESSAGE HISTORY TYPES
 * ========================================================================= */

/** @brief Maximum messages in history buffer */
#define MSG_HISTORY_LEN         8

/** @brief Maximum bytes per message to display */
#define MSG_HISTORY_MAX_BYTES   4

/**
 * @brief Single message history entry
 */
typedef struct {
    uint8_t data[MSG_HISTORY_MAX_BYTES];    /**< Message data */
    uint8_t len;                             /**< Actual message length */
    uint8_t source_id;                       /**< Producer ID (0xFF = unknown) */
    bool is_send;                            /**< true = sent, false = received */
} msg_history_entry_t;

/**
 * @brief Circular message history buffer
 */
typedef struct {
    msg_history_entry_t entries[MSG_HISTORY_LEN];
    uint8_t head;       /**< Next write position */
    uint8_t count;      /**< Number of valid entries */
} msg_history_t;

/* ============================================================================
 * DISPLAY API
 * ========================================================================= */

/**
 * @brief Initialize terminal display
 *
 * @details Clears screen, hides cursor, renders header/logo.
 *          Called once during system startup.
 */
void display_init(void);

/**
 * @brief Render horizontal progress bar
 *
 * @param[in] row           Terminal row (1-indexed)
 * @param[in] task_name     Task name to display
 * @param[in] elapsed_ticks Ticks elapsed in current period
 * @param[in] period_ticks  Total ticks for period
 */
void display_render_bar(uint8_t row, const char* task_name,
                        uint32_t elapsed_ticks, uint32_t period_ticks);

/**
 * @brief Render on/off banner (heartbeat indicator)
 *
 * @param[in] row       Terminal row (1-indexed)
 * @param[in] task_name Task name to display
 * @param[in] is_on     true = show banner, false = clear
 */
void display_render_banner(uint8_t row, const char* task_name, bool is_on);

/**
 * @brief Render vertical bar (semaphore fill level)
 *
 * @param[in] start_row Top row of bar (1-indexed)
 * @param[in] col       Column position
 * @param[in] count     Current count
 * @param[in] max_count Maximum count
 */
void display_render_vbar(uint8_t start_row, uint8_t col,
                         uint32_t count, uint32_t max_count);

/**
 * @brief Render message pipe visualization
 *
 * @param[in] start_row Top row of panel (1-indexed)
 * @param[in] col       Column position
 * @param[in] label     Short label (e.g., "Q1")
 * @param[in] count     Current byte count
 * @param[in] max_count Maximum capacity
 * @param[in] last_sent Last sent value
 * @param[in] last_recv Last received value
 * @param[in] show_sent Show sent indicator
 * @param[in] show_recv Show received indicator
 */
void display_render_pipe(uint8_t start_row, uint8_t col, const char* label,
                         uint8_t count, uint8_t max_count,
                         uint8_t last_sent, uint8_t last_recv,
                         bool show_sent, bool show_recv);

/**
 * @brief Render producer task bar with message indicator
 */
void display_render_producer(uint8_t row, const char* task_name,
                             uint32_t elapsed_ticks, uint32_t period_ticks,
                             uint8_t msg_value, bool show_msg);

/**
 * @brief Render consumer task bar with message indicator
 */
void display_render_consumer(uint8_t row, const char* task_name,
                             uint32_t elapsed_ticks, uint32_t period_ticks,
                             uint8_t msg_value, bool show_msg);

/* ============================================================================
 * MESSAGE HISTORY API
 * ========================================================================= */

/**
 * @brief Initialize message history buffer
 * @param[out] hist  History buffer to initialize
 */
void msg_history_init(msg_history_t* hist);

/**
 * @brief Add message to history
 *
 * @param[in,out] hist      History buffer
 * @param[in]     data      Message data
 * @param[in]     len       Message length
 * @param[in]     source_id Producer ID (0xFF if unknown)
 * @param[in]     is_send   true = send event, false = receive
 */
void msg_history_add(msg_history_t* hist, const uint8_t* data, uint8_t len,
                     uint8_t source_id, bool is_send);

/**
 * @brief Render message history as rolling window
 *
 * @param[in] row   Starting row
 * @param[in] col   Column position
 * @param[in] hist  History buffer
 * @param[in] label Display label
 */
void display_render_msg_history(uint8_t row, uint8_t col,
                                msg_history_t* hist, const char* label);

#ifdef __cplusplus
}
#endif

#endif /* BSP_DISPLAY_H */
