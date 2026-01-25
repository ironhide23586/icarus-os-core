/*
 * mock_asm.c
 * Mock implementations of ARM assembly functions for host testing
 */

#include "mock_asm.h"
#include <stdio.h>
#include <stdlib.h>

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
void os_yield_pendsv(void) {
	// Mock: do nothing (context switching not needed for unit tests)
}
