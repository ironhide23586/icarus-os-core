/*
 * display.h
 *
 *  Created on: Jan 24, 2026
 *      Author: souhamb
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

#ifdef __cplusplus
}
#endif

#endif /* DISPLAY_H_ */
