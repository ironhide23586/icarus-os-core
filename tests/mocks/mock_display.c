/*
 * mock_display.c
 * Mock implementation of display functions for unit testing
 */

#include "mock_display.h"
#include <stdio.h>

// Mock implementations - do nothing, just satisfy linker
void display_render_bar(uint8_t row, const char* task_name, uint32_t elapsed_ticks, uint32_t period_ticks) {
	(void)row;
	(void)task_name;
	(void)elapsed_ticks;
	(void)period_ticks;
	// Mock: do nothing
}

void display_render_banner(uint8_t row, const char* task_name, bool is_on) {
	(void)row;
	(void)task_name;
	(void)is_on;
	// Mock: do nothing
}

void display_init(void) {
	// Mock: do nothing
}
