/*
 * mock_asm.c
 * Mock implementations of ARM assembly functions for host testing
 */

#include "mock_asm.h"
#include <stdio.h>
#include <stdlib.h>

// Track yield calls for testing
static uint32_t yield_call_count = 0;

// External reference to os_tick_count for tick simulation
extern volatile uint32_t os_tick_count;

// Mock: start_cold_task
// In real system, this sets up PSP and branches to task function
// For testing, we just mark the task as running
void start_cold_task(task_t *task) {
	if (task == NULL) {
		return;
	}
	// Mark task as running (simplified for testing)
	task->task_state = TASK_RUNNING;
	// In real system, this would switch to the task's stack and call the function
	// For testing, we don't actually execute the task function
}

// Mock: os_yield_trampoline
// In real system, this is called from context switch
void os_yield_trampoline(void) {
	// Mock: do nothing
}

// Mock: os_yield_pendsv
// In real system, this performs the actual context switch
// For testing, we track calls and can simulate task state changes
void os_yield_pendsv(void) {
	yield_call_count++;
	// Mock: In a real context switch, blocked tasks would be checked
	// For testing, we just track that yield was called
}

// Test helper: simulate tick advancement
void test_advance_ticks(uint32_t ticks) {
	os_tick_count += ticks;
}

// Test helper: get number of times os_yield_pendsv was called
uint32_t test_get_yield_count(void) {
	return yield_call_count;
}

// Test helper: reset yield count
void test_reset_yield_count(void) {
	yield_call_count = 0;
}
