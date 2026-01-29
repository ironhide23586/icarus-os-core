/*
 * display.h
 *
 *  Created on: Jan 24, 2026
 *      Author: Souham Biswas
 *      GitHub: https://github.com/ironhide23586/icarus-os-core
 */

#ifndef DISPLAY_H_
#define DISPLAY_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

/* Exported constants --------------------------------------------------------*/
// Feature flags
#define ENABLE_HEARTBEAT_VISUALIZATION  1  // Set to 1 to enable, 0 to disable heartbeat visualization

// Terminal row assignments for fixed-position rendering
#define ROW_HEADER    1
#define ROW_SEPARATOR 9
#if ENABLE_HEARTBEAT_VISUALIZATION
#define ROW_HEARTBEAT 10
#define ROW_TASK_A    11
#define ROW_TASK_B    12
#define ROW_TASK_C    13
#else
#define ROW_TASK_A    11
#define ROW_TASK_B    12
#define ROW_TASK_C    13
#endif

#define BAR_WIDTH     40
#define BAR_COL_START 6  // Column where bar starts (after "[A] ")

// Vertical bar configuration (for semaphore visualization)
#define VBAR_HEIGHT   10  // Height of vertical bar in rows
#define VBAR_COL      70  // Column position for vertical bar

// Task period durations (in ticks) - longer = slower, more visible animation
#define HEARTBEAT_ON_TICKS   437   // Heartbeat LED on duration (437ms)
#define HEARTBEAT_OFF_TICKS  479   // Heartbeat LED off duration (479ms)
#define HEARTBEAT_PERIOD_TICKS (HEARTBEAT_ON_TICKS + HEARTBEAT_OFF_TICKS)  // Total period (916ms)
#define TASK_A_PERIOD_TICKS  2000  // Task A period duration
#define TASK_B_PERIOD_TICKS  4000  // Task B period duration (2x slower than A)
#define TASK_C_PERIOD_TICKS  3000  // Task C period duration

// Rendering and timing configuration
#define RENDER_INTERVAL_TICKS  20  // How often to update the display (in ticks)
#define CYCLE_PAUSE_TICKS      100 // Pause between cycles (in ticks)

/* Exported functions prototypes ---------------------------------------------*/
/**
 * @brief Initialize terminal display - clear screen and print header
 */
void display_init(void);

/**
 * @brief Render a progress bar on a fixed row
 * @param row: terminal row (1-indexed)
 * @param task_name: task name string (from TCB)
 * @param elapsed_ticks: ticks elapsed in current period
 * @param period_ticks: total ticks for this period
 */
void display_render_bar(uint8_t row, const char* task_name, uint32_t elapsed_ticks, uint32_t period_ticks);

/**
 * @brief Render a simple flashing banner (on/off indicator)
 * @param row: terminal row (1-indexed)
 * @param task_name: task name string
 * @param is_on: true to show banner (LED on), false to clear (LED off)
 */
void display_render_banner(uint8_t row, const char* task_name, bool is_on);

/**
 * @brief Render a vertical bar showing semaphore fill level
 * @param start_row: top row of the vertical bar (1-indexed)
 * @param col: column position for the bar
 * @param count: current semaphore count
 * @param max_count: maximum semaphore capacity (init_count)
 */
void display_render_vbar(uint8_t start_row, uint8_t col, uint32_t count, uint32_t max_count);

/**
 * @brief Render a message queue visualization panel
 * @param start_row: top row of the panel (1-indexed)
 * @param col: column position for the panel
 * @param label: short label for the queue (e.g., "SS", "SM")
 * @param count: current message count in queue
 * @param max_count: maximum queue capacity
 * @param last_sent: last message value sent (0-255)
 * @param last_recv: last message value received (0-255)
 * @param show_sent: true if last_sent is valid
 * @param show_recv: true if last_recv is valid
 */
void display_render_pipe(uint8_t start_row, uint8_t col, const char* label,
                         uint8_t count, uint8_t max_count,
                         uint8_t last_sent, uint8_t last_recv,
                         bool show_sent, bool show_recv);

/**
 * @brief Render a producer task bar with sent message indicator
 * @param row: terminal row (1-indexed)
 * @param task_name: task name string
 * @param elapsed_ticks: ticks elapsed in current period
 * @param period_ticks: total ticks for this period
 * @param msg_value: last message value sent
 * @param show_msg: true to show message value with animation
 */
void display_render_producer(uint8_t row, const char* task_name, 
                             uint32_t elapsed_ticks, uint32_t period_ticks,
                             uint8_t msg_value, bool show_msg);

/**
 * @brief Render a consumer task bar with received message indicator
 * @param row: terminal row (1-indexed)
 * @param task_name: task name string
 * @param elapsed_ticks: ticks elapsed in current period
 * @param period_ticks: total ticks for this period
 * @param msg_value: last message value received
 * @param show_msg: true to show message value with animation
 */
void display_render_consumer(uint8_t row, const char* task_name,
                             uint32_t elapsed_ticks, uint32_t period_ticks,
                             uint8_t msg_value, bool show_msg);

// Message history configuration
#define MSG_HISTORY_LEN     8    // Number of messages to show in history
#define MSG_HISTORY_MAX_BYTES 4  // Max bytes per message to display

/**
 * @brief Message history entry for rolling window display
 */
typedef struct {
    uint8_t data[MSG_HISTORY_MAX_BYTES];  // Message data
    uint8_t len;                           // Actual message length
    uint8_t source_id;                     // Producer ID (0, 1, etc.) or 0xFF for unknown
    bool is_send;                          // true = sent, false = received
} msg_history_entry_t;

/**
 * @brief Message history buffer for a pipe
 */
typedef struct {
    msg_history_entry_t entries[MSG_HISTORY_LEN];
    uint8_t head;       // Next write position
    uint8_t count;      // Number of valid entries
} msg_history_t;

/**
 * @brief Initialize a message history buffer
 * @param hist: pointer to history buffer
 */
void msg_history_init(msg_history_t* hist);

/**
 * @brief Add a message to history
 * @param hist: pointer to history buffer
 * @param data: message data
 * @param len: message length in bytes
 * @param source_id: producer ID (use 0xFF if unknown)
 * @param is_send: true if this is a send event, false for receive
 */
void msg_history_add(msg_history_t* hist, const uint8_t* data, uint8_t len, 
                     uint8_t source_id, bool is_send);

/**
 * @brief Render message history as a rolling window
 * @param row: starting row for display
 * @param col: column position
 * @param hist: pointer to history buffer
 * @param label: label for the history (e.g., "SS", "SM")
 */
void display_render_msg_history(uint8_t row, uint8_t col, msg_history_t* hist, const char* label);

#ifdef __cplusplus
}
#endif

#endif /* DISPLAY_H_ */
