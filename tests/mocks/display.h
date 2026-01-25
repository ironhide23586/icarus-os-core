/*
 * mock_display.h
 * Mock for display.h to isolate kernel tests from display dependencies
 * This file replaces Core/Inc/bsp/display.h during testing
 */

#ifndef DISPLAY_H_
#define DISPLAY_H_

#include <stdint.h>
#include <stdbool.h>

// Mock display functions - these are stubs for testing
// The real display functions are in Core/Src/bsp/display.c

void display_render_bar(uint8_t row, const char* task_name, uint32_t elapsed_ticks, uint32_t period_ticks);
void display_render_banner(uint8_t row, const char* task_name, bool is_on);
void display_init(void);

// Constants from real display.h (needed by kernel)
#define ROW_TASK_A 11
#define ROW_TASK_B 12
#define ROW_TASK_C 13

#endif /* DISPLAY_H_ */
