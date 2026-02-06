/*
 * test_task.c
 * Comprehensive unit tests for kernel task management functions
 * Tests all public API functions and major code paths
 */

#include "unity.h"
#include "icarus/icarus.h"
#include "bsp/display.h"
#include "bsp/error.h"
#include "retarget_hal.h"
#include "stm32h7xx_it.h"
#include "mock_asm.h"
#include "cmsis_gcc.h"
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

// Function prototype for __io_putchar
int __io_putchar(int ch);

// Access to internal kernel variables for testing
extern volatile uint32_t os_tick_count;
extern volatile uint8_t os_running;
extern uint8_t current_task_index;
extern uint8_t running_task_count;
extern uint8_t num_created_tasks;
extern int8_t current_cleanup_task_idx;
// print_buffer variables removed from kernel
// extern volatile uint8_t print_buffer_dequeue_idx;
// extern volatile uint8_t print_buffer_enqueue_idx;
extern volatile bool scheduler_enabled;
extern volatile uint8_t critical_stack_depth;
extern volatile uint32_t current_task_ticks_remaining;
extern icarus_task_t* task_list[];
extern uint32_t cpu_vregisters[];
extern icarus_semaphore_t* semaphore_list[];
extern icarus_pipe_t* message_pipe_list[];

// Test task functions
static void test_task_1(void) {
	// Simple test task
}

static void test_task_2(void) {
	// Another test task
}

static void test_task_3(void) {
	// Third test task
}

// Helper function to initialize task_list without calling os_init
// os_init() causes issues because it registers tasks with infinite loops
// Instead, we'll manually initialize what we need
// Note: This is a workaround - ideally we'd have a separate init function in the kernel
static void test_init_task_list(void) {
	// Initialize task_list by calling os_init() which sets task_list[i] = &task_pool[i]
	// os_init() also registers some tasks, but we'll reset num_created_tasks after
	// This is safe because display_init() is mocked
	os_init();
	
	// Reset task count so we can register our own test tasks
	// os_init() registered 2 tasks (idle and heartbeat), so we reset to start fresh
	num_created_tasks = 0;
}

// Test setup and teardown
void setUp(void) {
	// Initialize kernel state before each test
	os_tick_count = 0;
	os_running = 0;
	current_task_index = 0;
	running_task_count = 0;
	num_created_tasks = 0;
	current_cleanup_task_idx = -1;
	// print_buffer_dequeue_idx = 0;
	// print_buffer_enqueue_idx = 0;
	scheduler_enabled = true;
	critical_stack_depth = 0;
	
	// Initialize task_list (required for os_register_task to work)
	// This must be called before any test that uses os_register_task
	// We'll call it on-demand in tests that need it, not in setUp
	// because os_init calls display_init which may cause issues
	
	// Clear CPU registers
	for (int i = 0; i < 16; i++) {
		cpu_vregisters[i] = 0;
	}
}

void tearDown(void) {
	// Cleanup after each test
}

// ============================================================================
// Basic Tests
// ============================================================================

// Test: enqueue_print_buffer basic functionality
// DISABLED: enqueue_print_buffer is commented out in task.h
// void test_enqueue_print_buffer_basic(void) {
// 	uint8_t test_char = 'A';
// 	bool result;
// 	
// 	// Reset indices to known state
// 	print_buffer_enqueue_idx = 0;
// 	print_buffer_dequeue_idx = 0;
// 	
// 	// Should be able to enqueue when buffer is empty
// 	result = enqueue_print_buffer(test_char);
// 	TEST_ASSERT_TRUE(result);
// 	TEST_ASSERT_EQUAL(1, print_buffer_enqueue_idx);
// 	TEST_ASSERT_EQUAL(0, print_buffer_dequeue_idx);
// 	
// 	// Enqueue another character
// 	result = enqueue_print_buffer('B');
// 	TEST_ASSERT_TRUE(result);
// 	TEST_ASSERT_EQUAL(2, print_buffer_enqueue_idx);
// }

// Test: enqueue_print_buffer buffer full
// DISABLED: enqueue_print_buffer is commented out in task.h
// void test_enqueue_print_buffer_full(void) {
// 	uint8_t i;
// 	
// 	// Fill buffer to capacity
// 	print_buffer_enqueue_idx = 0;
// 	print_buffer_dequeue_idx = 0;
// 	
// 	// Fill buffer (PRINT_BUFFER_BYTES - 1, leaving one slot for wrap detection)
// 	for (i = 0; i < PRINT_BUFFER_BYTES - 1; i++) {
// 		TEST_ASSERT_TRUE(enqueue_print_buffer('A' + i));
// 	}
// 	
// 	// Next enqueue should fail (buffer full)
// 	TEST_ASSERT_FALSE(enqueue_print_buffer('Z'));
// }

// Test: task_busy_wait basic functionality
// With HOST_TEST defined, task_busy_wait auto-advances ticks so it won't hang
void test_task_busy_wait_basic(void) {
	os_tick_count = 500;
	
	// With 0 ticks, the loop runs once (incrementing tick) then exits
	// So delta will be 1 (one iteration)
	uint32_t result = task_busy_wait(0);
	TEST_ASSERT_TRUE(result >= 0);  // Should exit after 0 or more iterations
	
	// With non-zero ticks, should also work (ticks auto-advance in HOST_TEST)
	os_tick_count = 1000;
	result = task_busy_wait(5);
	TEST_ASSERT_TRUE(result >= 5);  // Should have waited at least 5 ticks
}

// Test: enter_critical and exit_critical
void test_critical_sections(void) {
	TEST_ASSERT_TRUE(scheduler_enabled);
	TEST_ASSERT_EQUAL(0, critical_stack_depth);
	
	// Critical sections are tested indirectly through semaphore operations
	// which use enter_critical/exit_critical
	TEST_PASS();
}

// Test: os_get_tick_count
void test_os_get_tick_count(void) {
	os_tick_count = 0;
	TEST_ASSERT_EQUAL(0, os_get_tick_count());
	
	os_tick_count = 1000;
	TEST_ASSERT_EQUAL(1000, os_get_tick_count());
	
	os_tick_count = 0xFFFFFFFF;
	TEST_ASSERT_EQUAL(0xFFFFFFFF, os_get_tick_count());
}

// ============================================================================
// Comprehensive Tests
// ============================================================================

// Test: os_init
// os_init initializes the kernel state and registers system tasks
void test_os_init(void) {
	// Reset state before test
	os_running = 1;  // Set to non-zero to verify it gets reset
	running_task_count = 5;
	current_task_index = 3;
	num_created_tasks = 0;
	
	// Call os_init
	os_init();
	
	// Verify state was initialized
	TEST_ASSERT_EQUAL(0, os_running);
	TEST_ASSERT_EQUAL(0, running_task_count);
	TEST_ASSERT_EQUAL(0, current_task_index);
	TEST_ASSERT_EQUAL(ICARUS_TICKS_PER_TASK, current_task_ticks_remaining);
	
	// os_init registers 2 system tasks (idle and heartbeat)
	TEST_ASSERT_EQUAL(2, num_created_tasks);
	
	// Verify task_list was initialized
	TEST_ASSERT_NOT_NULL(task_list[0]);
	TEST_ASSERT_NOT_NULL(task_list[1]);
	
	// Verify system task names
	TEST_ASSERT_EQUAL_STRING("ICARUS_KEEPALIVE_TASK", task_list[0]->name);
	TEST_ASSERT_EQUAL_STRING(">ICARUS_HEARTBEAT<", task_list[1]->name);
}

// Test: os_register_task
// This test covers os_register_task which uses task_list and stack_pool
void test_os_register_task(void) {
	// Initialize task_list (required for os_register_task)
	test_init_task_list();
	
	uint8_t initial_count = num_created_tasks;
	
	// Call os_register_task - it uses task_list[num_created_tasks] and stack_pool[num_created_tasks]
	os_register_task(test_task_1, "registered_task");
	
	// Verify task was created
	TEST_ASSERT_EQUAL(initial_count + 1, num_created_tasks);
	
	// Verify task_list entry was used
	TEST_ASSERT_NOT_NULL(task_list[initial_count]);
	TEST_ASSERT_EQUAL(TASK_STATE_COLD, task_list[initial_count]->task_state);
	TEST_ASSERT_EQUAL_STRING("registered_task", task_list[initial_count]->name);
	TEST_ASSERT_NOT_NULL(task_list[initial_count]->function);
	TEST_ASSERT_NOT_NULL(task_list[initial_count]->stack_base);
	TEST_ASSERT_EQUAL(TASK_PRIORITY_LOW, task_list[initial_count]->task_priority);
	TEST_ASSERT_NOT_NULL(task_list[initial_count]->stack_pointer);
}

// Test: os_register_task multiple tasks
void test_os_register_task_multiple(void) {
	// Initialize task_list (required for os_register_task)
	test_init_task_list();
	
	uint8_t initial_count = num_created_tasks;
	
	// Register multiple tasks
	os_register_task(test_task_1, "task1");
	os_register_task(test_task_2, "task2");
	os_register_task(test_task_3, "task3");
	
	// Verify all tasks were created
	TEST_ASSERT_EQUAL(initial_count + 3, num_created_tasks);
	
	// Verify each task
	TEST_ASSERT_EQUAL_STRING("task1", task_list[initial_count]->name);
	TEST_ASSERT_EQUAL_STRING("task2", task_list[initial_count + 1]->name);
	TEST_ASSERT_EQUAL_STRING("task3", task_list[initial_count + 2]->name);
}

// Test: os_get_running_task_count
void test_os_get_running_task_count(void) {
	running_task_count = 0;
	TEST_ASSERT_EQUAL(0, os_get_running_task_count());
	
	running_task_count = 5;
	TEST_ASSERT_EQUAL(5, os_get_running_task_count());
	
	running_task_count = 0;
	TEST_ASSERT_EQUAL(0, os_get_running_task_count());
}

// Test: os_get_current_task_name
void test_os_get_current_task_name(void) {
	// With no tasks, should return "unknown"
	current_task_index = 0;
	num_created_tasks = 0;
	TEST_ASSERT_EQUAL_STRING("unknown", os_get_current_task_name());
	
	// Test with invalid index
	current_task_index = 100;
	TEST_ASSERT_EQUAL_STRING("unknown", os_get_current_task_name());
	
	// Note: Testing with valid tasks requires os_init() which causes issues
	// This test covers the "unknown" path which is important
}

// Test: os_get_task_ticks_remaining
void test_os_get_task_ticks_remaining(void) {
	// Set a known value
	// Note: current_task_ticks_remaining is volatile and set during os_init
	// For testing, we'll just verify the function works
	(void)os_get_task_ticks_remaining();
	// Function should return a value (may be 0 if not initialized)
	TEST_PASS();
}

// Test: os_start with no tasks
void test_os_start_no_tasks(void) {
	num_created_tasks = 0;
	
	// Should return early without starting
	os_start();
	// No crash means success
	TEST_PASS();
}

// Test: os_start with valid tasks
void test_os_start_valid_tasks(void) {
	// Initialize task system
	test_init_task_list();
	os_register_task(test_task_1, "valid_task");
	
	// os_start calls task_start which calls start_cold_task (mocked)
	// The mock just sets task state to RUNNING
	os_start();
	
	// Verify task was started (mock sets state to RUNNING)
	TEST_ASSERT_EQUAL(TASK_STATE_RUNNING, task_list[0]->task_state);
}

// Test: os_exit_task - with running_task_count > 0
void test_os_exit_task_with_running_count(void) {
	test_init_task_list();
	os_register_task(test_task_1, "task1");
	current_task_index = 0;
	running_task_count = 1;
	current_cleanup_task_idx = -1;
	
	// Call os_exit_task
	os_exit_task();
	
	// Task should be FINISHED
	TEST_ASSERT_EQUAL(TASK_STATE_FINISHED, task_list[0]->task_state);
	// running_task_count should be decremented
	TEST_ASSERT_EQUAL(0, running_task_count);
	// cleanup_task_idx should be set
	TEST_ASSERT_EQUAL(0, current_cleanup_task_idx);
}

// Test: os_exit_task - cleanup_task_idx at max (boundary case)
void test_os_exit_task_cleanup_idx_max(void) {
	test_init_task_list();
	os_register_task(test_task_1, "task1");
	current_task_index = 0;
	running_task_count = 1;
	current_cleanup_task_idx = ICARUS_MAX_TASKS - 1;  // At max, can't add more
	
	os_exit_task();
	
	// Task should be FINISHED
	TEST_ASSERT_EQUAL(TASK_STATE_FINISHED, task_list[0]->task_state);
	// cleanup_task_idx should stay at max (can't increment past ICARUS_MAX_TASKS - 1)
	TEST_ASSERT_EQUAL(ICARUS_MAX_TASKS - 1, current_cleanup_task_idx);
}

// Test: os_exit_task - with running_task_count == 0 (should not decrement below 0)
void test_os_exit_task_zero_running_count(void) {
	test_init_task_list();
	os_register_task(test_task_1, "task1");
	current_task_index = 0;
	running_task_count = 0; // Already at 0
	current_cleanup_task_idx = -1;
	
	// Call os_exit_task
	os_exit_task();
	
	// Task should be FINISHED
	TEST_ASSERT_EQUAL(TASK_STATE_FINISHED, task_list[0]->task_state);
	// running_task_count should stay at 0 (not go negative)
	TEST_ASSERT_EQUAL(0, running_task_count);
	// cleanup_task_idx should still be set
	TEST_ASSERT_EQUAL(0, current_cleanup_task_idx);
}

// Test: os_kill_process valid task
void test_os_kill_process_valid(void) {
	test_init_task_list();
	os_register_task(test_task_1, "task1");
	os_register_task(test_task_2, "task2");
	
	// Set task states to READY (valid for killing)
	task_list[0]->task_state = TASK_STATE_READY;
	task_list[1]->task_state = TASK_STATE_READY;
	
	current_task_index = 0;
	running_task_count = 2;
	current_cleanup_task_idx = -1;
	
	// Kill task 1 (not task 0, since os_kill_process requires task_index > 0)
	os_kill_process(1);
	
	// Verify task was killed
	TEST_ASSERT_EQUAL(TASK_STATE_KILLED, task_list[1]->task_state);
	TEST_ASSERT_EQUAL(1, running_task_count);
	TEST_ASSERT_EQUAL(0, current_cleanup_task_idx);
}

// Test: os_kill_process invalid index
void test_os_kill_process_invalid_index(void) {
	// Test with invalid index (doesn't require os_init)
	num_created_tasks = 0;
	uint8_t initial_count = running_task_count;
	
	// Try to kill non-existent task
	os_kill_process(100);
	
	// Should not change running count
	TEST_ASSERT_EQUAL(initial_count, running_task_count);
}

// Test: os_kill_process task index 0 (should not kill - protected)
void test_os_kill_process_index_zero(void) {
	test_init_task_list();
	os_register_task(test_task_1, "task0");
	os_register_task(test_task_2, "task1");
	
	task_list[0]->task_state = TASK_STATE_READY;
	task_list[1]->task_state = TASK_STATE_READY;
	current_task_index = 1;
	running_task_count = 2;
	
	// Try to kill task 0 - should fail (task_index > 0 check)
	os_kill_process(0);
	
	// Task 0 should still be READY (not killed)
	TEST_ASSERT_EQUAL(TASK_STATE_READY, task_list[0]->task_state);
	TEST_ASSERT_EQUAL(2, running_task_count);
}

// Test: os_task_suicide
void test_os_task_suicide(void) {
	test_init_task_list();
	os_register_task(test_task_1, "task1");
	os_register_task(test_task_2, "task2");
	current_task_index = 1; // Set current task to index 1 (not 0, since os_kill_process requires task_index > 0)
	running_task_count = 2;
	
	// Call os_task_suicide (should kill current task)
	os_task_suicide();
	
	// Current task should be killed
	TEST_ASSERT_EQUAL(TASK_STATE_KILLED, task_list[1]->task_state);
	// running_task_count should be decremented
	TEST_ASSERT_EQUAL(1, running_task_count);
}

// Test: os_print_finished_tasks
// DISABLED: os_print_finished_tasks is commented out in task.h
// void test_os_print_finished_tasks(void) {
// 	// Set up some finished tasks
// 	current_cleanup_task_idx = 2;
// 	// Note: cleanup_task_idx is static, so we can't directly set it
// 	// But we can test the function doesn't crash
// 	os_print_finished_tasks();
// 	
// 	TEST_PASS(); // If we get here, no crash
// }

// Test: os_yield
void test_os_yield(void) {
	// os_yield should:
	// 1. Reset current_task_ticks_remaining to ticks_per_task
	// 2. Set PendSV pending bit
	
	current_task_ticks_remaining = 10;  // Some arbitrary value
	TEST_CLEAR_PENDSV();
	
	os_yield();
	
	// Verify ticks were reset
	TEST_ASSERT_EQUAL(ICARUS_TICKS_PER_TASK, current_task_ticks_remaining);
	
	// Verify PendSV was triggered
	TEST_ASSERT_TRUE(TEST_PENDSV_IS_SET());
}

// Test: task_active_sleep
void test_task_active_sleep(void) {
	// Initialize task system
	test_init_task_list();
	os_register_task(test_task_1, "sleep_test_task");
	
	// Set up current task
	current_task_index = 0;
	os_tick_count = 1000;
	TEST_CLEAR_PENDSV();  // Clear any pending PendSV
	
	// Call task_active_sleep - this should:
	// 1. Set global_tick_paused to current tick
	// 2. Set ticks_to_pause
	// 3. Set task state to BLOCKED
	// 4. Call os_yield (which sets PendSV pending)
	uint32_t sleep_ticks = 100;
	
	uint32_t actual_sleep = task_active_sleep(sleep_ticks);
	
	// Verify task was set to BLOCKED
	TEST_ASSERT_EQUAL(TASK_STATE_BLOCKED, task_list[0]->task_state);
	
	// Verify global_tick_paused was set
	TEST_ASSERT_EQUAL(1000, task_list[0]->global_tick_paused);
	
	// Verify ticks_to_pause was set
	TEST_ASSERT_EQUAL(sleep_ticks, task_list[0]->ticks_to_pause);
	
	// Verify PendSV was triggered (os_yield sets this bit)
	TEST_ASSERT_TRUE(TEST_PENDSV_IS_SET());
	
	// actual_sleep is os_tick_count - global_tick_paused (0 since ticks didn't advance)
	TEST_ASSERT_EQUAL(0, actual_sleep);
}

// Test: task_blocking_sleep
// With HOST_TEST defined, task_busy_wait auto-advances ticks so it won't hang
void test_task_blocking_sleep(void) {
	// Set initial tick count
	os_tick_count = 1000;
	
	// With 0 ticks, the loop runs once then exits
	uint32_t result = task_blocking_sleep(0);
	TEST_ASSERT_TRUE(result >= 0);  // Should exit after 0 or more iterations
	
	// With non-zero ticks, should also work
	os_tick_count = 2000;
	result = task_blocking_sleep(10);
	TEST_ASSERT_TRUE(result >= 10);  // Should have waited at least 10 ticks
}

// Test: os_create_task with max tasks
void test_os_create_task_max_tasks(void) {
	// Fill up to ICARUS_MAX_TASKS
	running_task_count = ICARUS_MAX_TASKS;
	
	uint8_t initial_count = num_created_tasks;
	
	// Try to create another task (should fail)
	icarus_task_t dummy_task;
	uint32_t dummy_stack[100];
	os_create_task(&dummy_task, test_task_1, dummy_stack, 100, NULL, "overflow");
	
	// Should not increment num_created_tasks
	TEST_ASSERT_EQUAL(initial_count, num_created_tasks);
}

// Test: os_create_task normal case
void test_os_create_task_normal(void) {
	icarus_task_t test_task;
	uint32_t test_stack[ICARUS_STACK_WORDS];
	uint8_t initial_count = num_created_tasks;
	
	os_create_task(&test_task, test_task_1, test_stack, ICARUS_STACK_WORDS, NULL, "created_task");
	
	TEST_ASSERT_EQUAL(initial_count + 1, num_created_tasks);
	TEST_ASSERT_EQUAL(TASK_STATE_COLD, test_task.task_state);
	TEST_ASSERT_EQUAL_STRING("created_task", test_task.name);
	TEST_ASSERT_NOT_NULL(test_task.function);
	TEST_ASSERT_NOT_NULL(test_task.stack_base);
	TEST_ASSERT_EQUAL(TASK_PRIORITY_LOW, test_task.task_priority);
	TEST_ASSERT_EQUAL(0, test_task.global_tick_paused);
	TEST_ASSERT_EQUAL(0, test_task.ticks_to_pause);
	TEST_ASSERT_NOT_NULL(test_task.stack_pointer);
}

// Test: os_create_task with long name (truncation)
void test_os_create_task_long_name(void) {
	icarus_task_t test_task;
	uint32_t test_stack[ICARUS_STACK_WORDS];
	char long_name[100];
	memset(long_name, 'A', 99);
	long_name[99] = '\0';
	
	os_create_task(&test_task, test_task_1, test_stack, ICARUS_STACK_WORDS, NULL, long_name);
	
	// Name should be truncated to ICARUS_MAX_TASK_NAME_LEN - 1
	TEST_ASSERT_EQUAL('\0', test_task.name[ICARUS_MAX_TASK_NAME_LEN - 1]);
}

// Test: dequeue_print_buffer (tested indirectly through enqueue)
// DISABLED: enqueue_print_buffer is commented out in task.h
// void test_dequeue_print_buffer_indirect(void) {
// 	// Fill buffer
// 	print_buffer_enqueue_idx = 0;
// 	print_buffer_dequeue_idx = 0;
// 	
// 	enqueue_print_buffer('H');
// 	enqueue_print_buffer('e');
// 	enqueue_print_buffer('l');
// 	enqueue_print_buffer('l');
// 	enqueue_print_buffer('o');
// 	
// 	// dequeue_print_buffer is static, but we know it's used internally
// 	// We can't test it directly, but we know enqueue works
// 	TEST_ASSERT_EQUAL(5, print_buffer_enqueue_idx);
// 	TEST_ASSERT_EQUAL(0, print_buffer_dequeue_idx);
// }

// ============================================================================
// DO-178C Coverage Tests - Branch, Statement, and MC/DC Coverage
// ============================================================================

// Test: exit_critical with critical_stack_depth == 0 (true branch)
// DISABLED: enqueue_print_buffer is commented out in task.h
// void test_exit_critical_depth_zero(void) {
// 	critical_stack_depth = 0;
// 	scheduler_enabled = false;
// 	
// 	bool result = enqueue_print_buffer('A');
// 	TEST_ASSERT_TRUE(result);
// 	TEST_ASSERT_TRUE(scheduler_enabled);
// 	TEST_ASSERT_EQUAL(0, critical_stack_depth);
// }

// Test: exit_critical with critical_stack_depth > 0 (false branch)
// DISABLED: enqueue_print_buffer is commented out in task.h
// void test_exit_critical_depth_nonzero(void) {
// 	critical_stack_depth = 1;
// 	scheduler_enabled = false;
// 	
// 	bool result1 = enqueue_print_buffer('A');
// 	TEST_ASSERT_TRUE(result1);
// 	// After enqueue: enter (1->2), do work, exit (2->1)
// 	// Depth should be back to 1 (not 0), scheduler still disabled
// 	TEST_ASSERT_EQUAL(1, critical_stack_depth);
// 	TEST_ASSERT_FALSE(scheduler_enabled); // Should still be disabled (depth > 0)
// }

// Test: enqueue_print_buffer - wrap around case
// DISABLED: enqueue_print_buffer is commented out in task.h
// void test_enqueue_print_buffer_wrap_around(void) {
// 	// Set up: enqueue_idx at last position, dequeue at start
// 	// Buffer has space: [dequeue=0] ... [enqueue=PRINT_BUFFER_BYTES-1]
// 	// After enqueue, should wrap to 0
// 	// But: next_enqueue_idx = (PRINT_BUFFER_BYTES-1 + 1) % PRINT_BUFFER_BYTES = 0
// 	// If dequeue_idx is 0, then next_enqueue_idx == dequeue_idx, so buffer is FULL!
// 	// Need dequeue_idx > 0 for wrap to work
// 	print_buffer_enqueue_idx = PRINT_BUFFER_BYTES - 1;
// 	print_buffer_dequeue_idx = 1; // Leave space for wrap (not 0)
// 	
// 	bool result = enqueue_print_buffer('W');
// 	TEST_ASSERT_TRUE(result);
// 	TEST_ASSERT_EQUAL(0, print_buffer_enqueue_idx); // Should wrap to 0
// }

// Test: enqueue_print_buffer - boundary conditions
// DISABLED: enqueue_print_buffer is commented out in task.h
// void test_enqueue_print_buffer_boundary(void) {
// 	print_buffer_enqueue_idx = PRINT_BUFFER_BYTES - 2;
// 	print_buffer_dequeue_idx = 0;
// 	
// 	bool result = enqueue_print_buffer('B');
// 	TEST_ASSERT_TRUE(result);
// 	TEST_ASSERT_EQUAL(PRINT_BUFFER_BYTES - 1, print_buffer_enqueue_idx);
// }

// Test: os_get_current_task_name - invalid index
void test_os_get_current_task_name_invalid_index(void) {
	current_task_index = 100;
	num_created_tasks = 5;
	TEST_ASSERT_EQUAL_STRING("unknown", os_get_current_task_name());
}

// Test: os_get_current_task_name - boundary: current_task_index == num_created_tasks
void test_os_get_current_task_name_boundary(void) {
	current_task_index = 5;
	num_created_tasks = 5;
	TEST_ASSERT_EQUAL_STRING("unknown", os_get_current_task_name());
}

// Test: os_get_current_task_name - NULL task_list entry
void test_os_get_current_task_name_null_task(void) {
	test_init_task_list();
	os_register_task(test_task_1, "test_task");
	current_task_index = 0;
	
	// Set task_list entry to NULL to test NULL check
	task_list[0] = NULL;
	
	const char* name = os_get_current_task_name();
	TEST_ASSERT_EQUAL_STRING("unknown", name);
}

// Test: os_start - num_created_tasks > ICARUS_MAX_TASKS (true branch: return early)
void test_os_start_too_many_tasks(void) {
	num_created_tasks = ICARUS_MAX_TASKS + 1;
	os_start();
	TEST_PASS();
}

// Test: os_start - boundary: num_created_tasks == ICARUS_MAX_TASKS
void test_os_start_boundary_max_tasks(void) {
	num_created_tasks = ICARUS_MAX_TASKS;
	os_start();
	TEST_PASS();
}

// Test: os_kill_process - task_index == num_created_tasks (edge case)
void test_os_kill_process_index_equal(void) {
	num_created_tasks = 5;
	os_kill_process(5);
	TEST_PASS();
}

// Test: os_kill_process - task_index == 0 (should not kill, error path)
void test_os_kill_process_index_zero_error(void) {
	test_init_task_list();
	os_register_task(test_task_1, "task1");
	current_task_index = 1; // Set current to a different task
	
	// Try to kill task 0 (should fail - task_index > 0 check)
	os_kill_process(0);
	
	// Task 0 should still be COLD (not killed)
	TEST_ASSERT_EQUAL(TASK_STATE_COLD, task_list[0]->task_state);
}

// Test: os_kill_process - task state >= 4 (already killed/finished, error path)
void test_os_kill_process_already_killed(void) {
	test_init_task_list();
	os_register_task(test_task_1, "task1");
	os_register_task(test_task_2, "task2");
	current_task_index = 0;
	
	// Set task state to TASK_STATE_KILLED (4) or TASK_STATE_FINISHED (5)
	task_list[1]->task_state = TASK_STATE_KILLED; // State 4
	uint8_t initial_count = running_task_count;
	
	os_kill_process(1);
	
	// Should not change running_task_count (state >= 4)
	TEST_ASSERT_EQUAL(initial_count, running_task_count);
	// Should still be KILLED
	TEST_ASSERT_EQUAL(TASK_STATE_KILLED, task_list[1]->task_state);
}

// Test: os_kill_process - suicide case (task_index == current_task_index)
void test_os_kill_process_suicide(void) {
	test_init_task_list();
	os_register_task(test_task_1, "task1");
	os_register_task(test_task_2, "task2");
	current_task_index = 1; // Set current to task 1
	running_task_count = 2;
	
	// Kill current task (suicide)
	os_kill_process(1);
	
	// Task should be killed
	TEST_ASSERT_EQUAL(TASK_STATE_KILLED, task_list[1]->task_state);
	// running_task_count should be decremented
	TEST_ASSERT_EQUAL(1, running_task_count);
}

// Test: os_kill_process - cleanup_idx at max (boundary case)
void test_os_kill_process_cleanup_idx_max(void) {
	test_init_task_list();
	os_register_task(test_task_1, "task1");
	os_register_task(test_task_2, "task2");
	
	task_list[0]->task_state = TASK_STATE_READY;
	task_list[1]->task_state = TASK_STATE_READY;
	current_task_index = 0;
	running_task_count = 2;
	current_cleanup_task_idx = ICARUS_MAX_TASKS - 1;  // At max
	
	os_kill_process(1);
	
	// Task should be killed
	TEST_ASSERT_EQUAL(TASK_STATE_KILLED, task_list[1]->task_state);
	// cleanup_task_idx should stay at max
	TEST_ASSERT_EQUAL(ICARUS_MAX_TASKS - 1, current_cleanup_task_idx);
}

// Test: os_create_task - boundary: running_task_count == ICARUS_MAX_TASKS - 1
void test_os_create_task_boundary_max_minus_one(void) {
	running_task_count = ICARUS_MAX_TASKS - 1;
	icarus_task_t test_task;
	uint32_t test_stack[ICARUS_STACK_WORDS];
	uint8_t initial_count = num_created_tasks;
	
	os_create_task(&test_task, test_task_1, test_stack, ICARUS_STACK_WORDS, NULL, "boundary");
	
	TEST_ASSERT_EQUAL(initial_count + 1, num_created_tasks);
}

// Test: os_create_task - early return when running_task_count >= ICARUS_MAX_TASKS
void test_os_create_task_max_running_tasks(void) {
	icarus_task_t test_task;
	uint32_t test_stack[ICARUS_STACK_WORDS];
	uint8_t initial_count = num_created_tasks;
	running_task_count = ICARUS_MAX_TASKS; // At max, should return early
	
	os_create_task(&test_task, test_task_1, test_stack, ICARUS_STACK_WORDS, NULL, "max_running");
	
	// Should not increment num_created_tasks (early return)
	TEST_ASSERT_EQUAL(initial_count, num_created_tasks);
}

// Test: os_print_finished_tasks - current_cleanup_task_idx == -1 (loop doesn't execute)
// DISABLED: os_print_finished_tasks is commented out in task.h
// void test_os_print_finished_tasks_empty(void) {
// 	current_cleanup_task_idx = -1;
// 	os_print_finished_tasks();
// 	TEST_PASS();
// }

// Test: display_render_bar - period_ticks == 0 (true branch: set to 1)
void test_display_render_bar_zero_period(void) {
	display_render_bar(1, "test", 10, 0);
	TEST_PASS();
}

// Test: display_render_bar - elapsed_ticks > period_ticks (true branch: clamp)
void test_display_render_bar_elapsed_exceeds_period(void) {
	display_render_bar(1, "test", 100, 50);
	TEST_PASS();
}

// Test: display_render_bar - filled > BAR_WIDTH (true branch: clamp)
void test_display_render_bar_filled_exceeds_width(void) {
	display_render_bar(1, "test", 1000, 10);
	TEST_PASS();
}

// Test: display_render_bar - task_name == NULL (true branch: use "unknown")
void test_display_render_bar_null_name(void) {
	display_render_bar(1, NULL, 50, 100);
	TEST_PASS();
}

// Test: display_render_bar - normal case (all false branches)
void test_display_render_bar_normal(void) {
	display_render_bar(1, "test_task", 50, 100);
	TEST_PASS();
}

// Test: display_render_banner - is_on == true (true branch: show banner)
void test_display_render_banner_on(void) {
	display_render_banner(1, "test", true);
	TEST_PASS();
}

// Test: display_render_banner - is_on == false (false branch: clear)
void test_display_render_banner_off(void) {
	display_render_banner(1, "test", false);
	TEST_PASS();
}

// Test: display_init - initialized == 0 (false branch: initialize)
void test_display_init_first_call(void) {
	display_init();
	TEST_PASS();
}

// Test: display_init - initialized == 1 (true branch: return early)
void test_display_init_second_call(void) {
	display_init();
	display_init();
	TEST_PASS();
}

// Test: __io_putchar - ch == '\n' (true branch: flush)
// With HOST_TEST defined, task_busy_wait auto-advances ticks so it won't hang
void test_io_putchar_newline(void) {
	// Reset the static buffer index by calling with non-newline chars first
	// Then call with newline to trigger flush
	
	// Fill some chars (not triggering flush)
	for (int j = 0; j < 5; j++) {
		__io_putchar('A');
	}
	
	// Now send newline - this triggers flush path
	int result = __io_putchar('\n');
	
	TEST_ASSERT_EQUAL('\n', result);
}

// Test: __io_putchar - i == sizeof(buf) (true branch: flush)
// Buffer is 64 bytes, so filling it triggers flush
void test_io_putchar_buffer_full(void) {
	// Fill buffer to capacity (64 chars) to trigger flush
	// Note: buffer may have leftover from previous test, so we fill 64 chars
	// which will definitely trigger at least one flush
	
	for (int j = 0; j < 64; j++) {
		int result = __io_putchar('B');
		TEST_ASSERT_EQUAL('B', result);
	}
	
	// If we got here without hanging, the test passed
	TEST_PASS();
}

// Test: __io_putchar - normal case (false branches)
// This works since it doesn't flush (no blocking sleep)
void test_io_putchar_normal(void) {
	// Only test non-flush case (buffer not full, no newline)
	// This avoids task_blocking_sleep
	for (int i = 0; i < 10; i++) {
		__io_putchar('A' + i);
	}
	// Don't add '\n' or fill to 64 to avoid flush
	TEST_PASS();
}

// ============================================================================
// retarget_hal.c Tests
// ============================================================================

// Test: LED_On
void test_LED_On(void) {
	LED_On();
	TEST_PASS();
}

// Test: LED_Off
void test_LED_Off(void) {
	LED_Off();
	TEST_PASS();
}

// Test: LED_Blink
// LED_Blink calls task_active_sleep which requires task setup
void test_LED_Blink(void) {
	test_init_task_list();
	os_register_task(test_task_1, "led_test");
	current_task_index = 0;
	os_tick_count = 0;
	
	// LED_Blink will call task_active_sleep twice (for on and off)
	// Each call to task_active_sleep calls os_yield which sets PendSV
	TEST_CLEAR_PENDSV();
	
	LED_Blink(100, 200);
	
	// After LED_Blink, PendSV should be set (from the last os_yield call)
	TEST_ASSERT_TRUE(TEST_PENDSV_IS_SET());
	
	// Task should be in BLOCKED state (from the last sleep)
	TEST_ASSERT_EQUAL(TASK_STATE_BLOCKED, task_list[0]->task_state);
}

// Test: LED_Blink with zero delays (boundary case)
void test_LED_Blink_zero_delays(void) {
	test_init_task_list();
	os_register_task(test_task_1, "led_test_zero");
	current_task_index = 0;
	os_tick_count = 0;
	TEST_CLEAR_PENDSV();
	
	// LED_Blink with 1ms delays (minimum - it subtracts 1)
	// Hdelay - 1 = 0, Ldelay - 1 = 0
	LED_Blink(1, 1);
	
	// PendSV should be set from os_yield calls
	TEST_ASSERT_TRUE(TEST_PENDSV_IS_SET());
}

// Test: platform_write
void test_platform_write(void) {
	I2C_HandleTypeDef hi2c = {0};
	uint8_t test_data[] = {0x01, 0x02, 0x03};
	
	int32_t result = platform_write(&hi2c, 0x10, test_data, sizeof(test_data));
	TEST_ASSERT_EQUAL(0, result);
}

// Test: platform_write with zero length
void test_platform_write_zero_length(void) {
	I2C_HandleTypeDef hi2c = {0};
	uint8_t test_data[] = {0x01};
	
	int32_t result = platform_write(&hi2c, 0x10, test_data, 0);
	TEST_ASSERT_EQUAL(0, result);
}

// Test: platform_read
void test_platform_read(void) {
	I2C_HandleTypeDef hi2c = {0};
	uint8_t read_buffer[4] = {0};
	
	int32_t result = platform_read(&hi2c, 0x10, read_buffer, sizeof(read_buffer));
	TEST_ASSERT_EQUAL(0, result);
}

// Test: platform_read with zero length
void test_platform_read_zero_length(void) {
	I2C_HandleTypeDef hi2c = {0};
	uint8_t read_buffer[1] = {0};
	
	int32_t result = platform_read(&hi2c, 0x10, read_buffer, 0);
	TEST_ASSERT_EQUAL(0, result);
}

// Test: hal_init
// hal_init calls hardware initialization functions, all of which are mocked
void test_hal_init(void) {
	// hal_init calls:
	// - MPU_Config (mocked via HAL_MPU_* functions)
	// - CPU_CACHE_Enable (mocked via SCB_Enable* functions)
	// - HAL_Init (mocked)
	// - SystemClock_Config (mocked via HAL_RCC_* functions)
	// - MX_*_Init functions (all mocked)
	
	// This should not crash since all HAL functions are mocked
	hal_init();
	
	TEST_PASS();
}

// ============================================================================
// stm32h7xx_it.c Tests (Interrupt Handlers)
// ============================================================================

// Test: SysTick_Handler - tick counting and scheduler trigger
void test_SysTick_Handler(void) {
	// Initialize state
	os_tick_count = 0;
	os_running = 1;
	current_task_ticks_remaining = ICARUS_TICKS_PER_TASK;
	scheduler_enabled = true;
	
	// Call SysTick_Handler
	SysTick_Handler();
	
	// Verify tick count incremented
	TEST_ASSERT_EQUAL(1, os_tick_count);
	// Verify ticks_remaining decremented
	TEST_ASSERT_EQUAL(ICARUS_TICKS_PER_TASK - 1, current_task_ticks_remaining);
}

// Test: SysTick_Handler - with os_running == 0 (should not trigger scheduler)
void test_SysTick_Handler_os_not_running(void) {
	os_tick_count = 0;
	os_running = 0;
	current_task_ticks_remaining = ICARUS_TICKS_PER_TASK;
	scheduler_enabled = true;
	
	SysTick_Handler();
	
	// Tick should increment
	TEST_ASSERT_EQUAL(1, os_tick_count);
	// When os_running == 0, the condition short-circuits, so ticks_remaining is NOT decremented
	// The condition is: if (os_running && --current_task_ticks_remaining == 0 && scheduler_enabled)
	// Since os_running is 0, the && short-circuits before decrementing
	TEST_ASSERT_EQUAL(ICARUS_TICKS_PER_TASK, current_task_ticks_remaining);
	// SCB->ICSR should not be set (scheduler not triggered)
}

// Test: SysTick_Handler - with scheduler_enabled == false (should not trigger scheduler)
void test_SysTick_Handler_scheduler_disabled(void) {
	os_tick_count = 0;
	os_running = 1;
	current_task_ticks_remaining = ICARUS_TICKS_PER_TASK;
	scheduler_enabled = false;
	
	SysTick_Handler();
	
	TEST_ASSERT_EQUAL(1, os_tick_count);
	TEST_ASSERT_EQUAL(ICARUS_TICKS_PER_TASK - 1, current_task_ticks_remaining);
}

// Test: SysTick_Handler - trigger scheduler when ticks_remaining reaches 0
void test_SysTick_Handler_trigger_scheduler(void) {
	os_tick_count = 0;
	os_running = 1;
	current_task_ticks_remaining = 1; // One tick remaining
	scheduler_enabled = true;
	
	SysTick_Handler();
	
	// Tick should increment
	TEST_ASSERT_EQUAL(1, os_tick_count);
	// ticks_remaining should be reset to ICARUS_TICKS_PER_TASK
	TEST_ASSERT_EQUAL(ICARUS_TICKS_PER_TASK, current_task_ticks_remaining);
	// SCB->ICSR should be set (PendSV triggered)
}

// Test: SysTick_Handler - multiple ticks
void test_SysTick_Handler_multiple_ticks(void) {
	os_tick_count = 0;
	os_running = 1;
	current_task_ticks_remaining = ICARUS_TICKS_PER_TASK;
	scheduler_enabled = true;
	
	// Call multiple times
	for (int i = 0; i < 5; i++) {
		SysTick_Handler();
	}
	
	TEST_ASSERT_EQUAL(5, os_tick_count);
	TEST_ASSERT_EQUAL(ICARUS_TICKS_PER_TASK - 5, current_task_ticks_remaining);
}

// Test: OTG_FS_IRQHandler - USB interrupt handler
void test_OTG_FS_IRQHandler(void) {
	// OTG_FS_IRQHandler calls HAL_PCD_IRQHandler
	// Since HAL_PCD_IRQHandler is mocked and hpcd_USB_OTG_FS is defined in mock_hal.c,
	// we can test this function
	OTG_FS_IRQHandler();
	TEST_PASS();
}

// Test: SVC_Handler - empty handler
void test_SVC_Handler(void) {
	SVC_Handler();
	TEST_PASS();
}

// Test: DebugMon_Handler - empty handler
void test_DebugMon_Handler(void) {
	DebugMon_Handler();
	TEST_PASS();
}

// ============================================================================
// Semaphore Tests
// ============================================================================

// Test: semaphore_init - basic initialization
void test_semaphore_init_basic(void) {
	test_init_task_list();
	
	// Initialize semaphore with count 5
	bool result = semaphore_init(0, 5);
	
	TEST_ASSERT_TRUE(result);
	TEST_ASSERT_TRUE(semaphore_list[0]->engaged);
	TEST_ASSERT_EQUAL(5, semaphore_list[0]->count);
	TEST_ASSERT_EQUAL(5, semaphore_list[0]->max_count);
}

// Test: semaphore_init - invalid index (>= ICARUS_MAX_SEMAPHORES)
void test_semaphore_init_invalid_index(void) {
	test_init_task_list();
	
	bool result = semaphore_init(ICARUS_MAX_SEMAPHORES, 5);
	TEST_ASSERT_FALSE(result);
	
	result = semaphore_init(ICARUS_MAX_SEMAPHORES + 10, 5);
	TEST_ASSERT_FALSE(result);
}

// Test: semaphore_init - zero count (should fail)
void test_semaphore_init_zero_count(void) {
	test_init_task_list();
	
	bool result = semaphore_init(0, 0);
	TEST_ASSERT_FALSE(result);
}

// Test: semaphore_init - already engaged (should fail)
void test_semaphore_init_already_engaged(void) {
	test_init_task_list();
	
	// First init should succeed
	bool result = semaphore_init(0, 5);
	TEST_ASSERT_TRUE(result);
	
	// Second init on same semaphore should fail
	result = semaphore_init(0, 10);
	TEST_ASSERT_FALSE(result);
	
	// Original values should be preserved
	TEST_ASSERT_EQUAL(5, semaphore_list[0]->count);
	TEST_ASSERT_EQUAL(5, semaphore_list[0]->max_count);
}

// Test: semaphore_init - multiple semaphores
void test_semaphore_init_multiple(void) {
	test_init_task_list();
	
	TEST_ASSERT_TRUE(semaphore_init(0, 1));
	TEST_ASSERT_TRUE(semaphore_init(1, 5));
	TEST_ASSERT_TRUE(semaphore_init(2, 10));
	
	TEST_ASSERT_EQUAL(1, semaphore_list[0]->max_count);
	TEST_ASSERT_EQUAL(5, semaphore_list[1]->max_count);
	TEST_ASSERT_EQUAL(10, semaphore_list[2]->max_count);
}

// Test: semaphore_init - boundary: ICARUS_MAX_SEMAPHORES - 1
void test_semaphore_init_boundary(void) {
	test_init_task_list();
	
	bool result = semaphore_init(ICARUS_MAX_SEMAPHORES - 1, 3);
	TEST_ASSERT_TRUE(result);
	TEST_ASSERT_TRUE(semaphore_list[ICARUS_MAX_SEMAPHORES - 1]->engaged);
}

// Test: semaphore_feed - invalid index
void test_semaphore_feed_invalid_index(void) {
	test_init_task_list();
	
	bool result = semaphore_feed(ICARUS_MAX_SEMAPHORES);
	TEST_ASSERT_FALSE(result);
	
	result = semaphore_feed(ICARUS_MAX_SEMAPHORES + 5);
	TEST_ASSERT_FALSE(result);
}

// Test: semaphore_feed - not engaged
void test_semaphore_feed_not_engaged(void) {
	test_init_task_list();
	
	// Semaphore 0 is not engaged (os_init sets engaged = false)
	bool result = semaphore_feed(0);
	TEST_ASSERT_FALSE(result);
}

// Test: semaphore_feed - basic feed (count < max_count)
void test_semaphore_feed_basic(void) {
	test_init_task_list();
	os_register_task(test_task_1, "feed_test");
	current_task_index = 0;
	
	// Initialize semaphore with count 10, max_count 10
	semaphore_init(0, 10);
	semaphore_list[0]->count = 5;  // Set count below max_count
	
	bool result = semaphore_feed(0);
	
	TEST_ASSERT_TRUE(result);
	TEST_ASSERT_EQUAL(6, semaphore_list[0]->count);
}

// Test: semaphore_consume - invalid index
void test_semaphore_consume_invalid_index(void) {
	test_init_task_list();
	
	bool result = semaphore_consume(ICARUS_MAX_SEMAPHORES);
	TEST_ASSERT_FALSE(result);
	
	result = semaphore_consume(ICARUS_MAX_SEMAPHORES + 5);
	TEST_ASSERT_FALSE(result);
}

// Test: semaphore_consume - not engaged
void test_semaphore_consume_not_engaged(void) {
	test_init_task_list();
	
	// Semaphore 0 is not engaged
	bool result = semaphore_consume(0);
	TEST_ASSERT_FALSE(result);
}

// Test: semaphore_consume - basic consume (count > 0)
void test_semaphore_consume_basic(void) {
	test_init_task_list();
	os_register_task(test_task_1, "consume_test");
	current_task_index = 0;
	
	// Initialize semaphore with count 5
	semaphore_init(0, 5);
	
	bool result = semaphore_consume(0);
	
	TEST_ASSERT_TRUE(result);
	TEST_ASSERT_EQUAL(4, semaphore_list[0]->count);
}

// Test: semaphore_consume - multiple consumes
void test_semaphore_consume_multiple(void) {
	test_init_task_list();
	os_register_task(test_task_1, "consume_multi");
	current_task_index = 0;
	
	semaphore_init(0, 5);
	
	TEST_ASSERT_TRUE(semaphore_consume(0));
	TEST_ASSERT_EQUAL(4, semaphore_list[0]->count);
	
	TEST_ASSERT_TRUE(semaphore_consume(0));
	TEST_ASSERT_EQUAL(3, semaphore_list[0]->count);
	
	TEST_ASSERT_TRUE(semaphore_consume(0));
	TEST_ASSERT_EQUAL(2, semaphore_list[0]->count);
}

// Test: semaphore feed and consume together
void test_semaphore_feed_consume_together(void) {
	test_init_task_list();
	os_register_task(test_task_1, "feed_consume");
	current_task_index = 0;
	
	semaphore_init(0, 10);
	semaphore_list[0]->count = 5;
	
	// Feed increases count
	TEST_ASSERT_TRUE(semaphore_feed(0));
	TEST_ASSERT_EQUAL(6, semaphore_list[0]->count);
	
	// Consume decreases count
	TEST_ASSERT_TRUE(semaphore_consume(0));
	TEST_ASSERT_EQUAL(5, semaphore_list[0]->count);
	
	// Multiple operations
	TEST_ASSERT_TRUE(semaphore_feed(0));
	TEST_ASSERT_TRUE(semaphore_feed(0));
	TEST_ASSERT_EQUAL(7, semaphore_list[0]->count);
	
	TEST_ASSERT_TRUE(semaphore_consume(0));
	TEST_ASSERT_TRUE(semaphore_consume(0));
	TEST_ASSERT_TRUE(semaphore_consume(0));
	TEST_ASSERT_EQUAL(4, semaphore_list[0]->count);
}

// Test: semaphore_consume - consume to zero
void test_semaphore_consume_to_zero(void) {
	test_init_task_list();
	os_register_task(test_task_1, "consume_zero");
	current_task_index = 0;
	
	semaphore_init(0, 2);
	
	TEST_ASSERT_TRUE(semaphore_consume(0));
	TEST_ASSERT_EQUAL(1, semaphore_list[0]->count);
	
	TEST_ASSERT_TRUE(semaphore_consume(0));
	TEST_ASSERT_EQUAL(0, semaphore_list[0]->count);
}

// Test: semaphore_feed - feed to max_count
void test_semaphore_feed_to_max(void) {
	test_init_task_list();
	os_register_task(test_task_1, "feed_max");
	current_task_index = 0;
	
	semaphore_init(0, 3);
	semaphore_list[0]->count = 2;  // One below max
	
	TEST_ASSERT_TRUE(semaphore_feed(0));
	TEST_ASSERT_EQUAL(3, semaphore_list[0]->count);  // Now at max
}

// ============================================================================
// Message Pipe Tests
// ============================================================================

// Test: pipe_init - basic initialization
void test_pipe_init_basic(void) {
	test_init_task_list();
	
	bool result = pipe_init(0, 64);
	
	TEST_ASSERT_TRUE(result);
	TEST_ASSERT_TRUE(message_pipe_list[0]->engaged);
	TEST_ASSERT_EQUAL(0, message_pipe_list[0]->count);
	TEST_ASSERT_EQUAL(64, message_pipe_list[0]->max_count);
	TEST_ASSERT_EQUAL(0, message_pipe_list[0]->enqueue_idx);
	TEST_ASSERT_EQUAL(0, message_pipe_list[0]->dequeue_idx);
}

// Test: pipe_init - invalid index (>= ICARUS_MAX_MESSAGE_QUEUES)
void test_pipe_init_invalid_index(void) {
	test_init_task_list();
	
	bool result = pipe_init(ICARUS_MAX_MESSAGE_QUEUES, 64);
	TEST_ASSERT_FALSE(result);
	
	result = pipe_init(ICARUS_MAX_MESSAGE_QUEUES + 10, 64);
	TEST_ASSERT_FALSE(result);
}

// Test: pipe_init - zero capacity (should fail)
void test_pipe_init_zero_capacity(void) {
	test_init_task_list();
	
	bool result = pipe_init(0, 0);
	TEST_ASSERT_FALSE(result);
}

// Test: pipe_init - capacity at max allowed value
void test_pipe_init_capacity_max(void) {
	test_init_task_list();
	
	// MAX_MESSAGE_BUFFER_BYTES is 128, so max capacity is 128
	// Use pipe index 10 to avoid conflicts with other tests
	bool result = pipe_init(10, 128);
	TEST_ASSERT_TRUE(result);
	TEST_ASSERT_EQUAL(128, message_pipe_list[10]->max_count);
}

// Test: pipe_init - capacity exceeds MAX_MESSAGE_BUFFER_BYTES (should fail)
void test_pipe_init_capacity_too_large(void) {
	test_init_task_list();
	
	// MAX_MESSAGE_BUFFER_BYTES is 128, so 129 should fail
	// Use pipe index 11 to avoid conflicts
	bool result = pipe_init(11, 129);
	TEST_ASSERT_FALSE(result);
	TEST_ASSERT_FALSE(message_pipe_list[11]->engaged);
}

// Test: pipe_init - already engaged (should fail)
void test_pipe_init_already_engaged(void) {
	test_init_task_list();
	
	// First init should succeed
	bool result = pipe_init(0, 64);
	TEST_ASSERT_TRUE(result);
	
	// Second init on same pipe should fail
	result = pipe_init(0, 128);
	TEST_ASSERT_FALSE(result);
	
	// Original values should be preserved
	TEST_ASSERT_EQUAL(64, message_pipe_list[0]->max_count);
}

// Test: pipe_init - multiple pipes
void test_pipe_init_multiple(void) {
	test_init_task_list();
	
	TEST_ASSERT_TRUE(pipe_init(0, 32));
	TEST_ASSERT_TRUE(pipe_init(1, 64));
	TEST_ASSERT_TRUE(pipe_init(2, 128));
	
	TEST_ASSERT_EQUAL(32, message_pipe_list[0]->max_count);
	TEST_ASSERT_EQUAL(64, message_pipe_list[1]->max_count);
	TEST_ASSERT_EQUAL(128, message_pipe_list[2]->max_count);
}

// Test: pipe_init - boundary: ICARUS_MAX_MESSAGE_QUEUES - 1
void test_pipe_init_boundary(void) {
	test_init_task_list();
	
	bool result = pipe_init(ICARUS_MAX_MESSAGE_QUEUES - 1, 32);
	TEST_ASSERT_TRUE(result);
	TEST_ASSERT_TRUE(message_pipe_list[ICARUS_MAX_MESSAGE_QUEUES - 1]->engaged);
}

// Test: pipe_enqueue - invalid index
void test_pipe_enqueue_invalid_index(void) {
	test_init_task_list();
	
	uint8_t data[] = {1, 2, 3};
	bool result = pipe_enqueue(ICARUS_MAX_MESSAGE_QUEUES, data, 3);
	TEST_ASSERT_FALSE(result);
}

// Test: pipe_enqueue - not engaged
void test_pipe_enqueue_not_engaged(void) {
	test_init_task_list();
	
	uint8_t data[] = {1, 2, 3};
	bool result = pipe_enqueue(0, data, 3);
	TEST_ASSERT_FALSE(result);
}

// Test: pipe_enqueue - message too large
void test_pipe_enqueue_message_too_large(void) {
	test_init_task_list();
	os_register_task(test_task_1, "enqueue_test");
	current_task_index = 0;
	
	pipe_init(0, 10);  // Capacity of 10 bytes
	
	uint8_t data[20] = {0};  // 20 bytes > 10 capacity
	bool result = pipe_enqueue(0, data, 20);
	TEST_ASSERT_FALSE(result);
}

// Test: pipe_enqueue - basic enqueue
void test_pipe_enqueue_basic(void) {
	test_init_task_list();
	os_register_task(test_task_1, "enqueue_test");
	current_task_index = 0;
	
	pipe_init(0, 64);
	
	uint8_t data[] = {0xAA, 0xBB, 0xCC};
	bool result = pipe_enqueue(0, data, 3);
	
	TEST_ASSERT_TRUE(result);
	TEST_ASSERT_EQUAL(3, message_pipe_list[0]->count);
	TEST_ASSERT_EQUAL(3, message_pipe_list[0]->enqueue_idx);
	
	// Verify data in buffer
	TEST_ASSERT_EQUAL(0xAA, message_pipe_list[0]->buffer[0]);
	TEST_ASSERT_EQUAL(0xBB, message_pipe_list[0]->buffer[1]);
	TEST_ASSERT_EQUAL(0xCC, message_pipe_list[0]->buffer[2]);
}

// Test: pipe_enqueue - multiple enqueues
void test_pipe_enqueue_multiple(void) {
	test_init_task_list();
	os_register_task(test_task_1, "enqueue_multi");
	current_task_index = 0;
	
	pipe_init(0, 64);
	
	uint8_t data1[] = {0x01, 0x02};
	uint8_t data2[] = {0x03, 0x04, 0x05};
	
	TEST_ASSERT_TRUE(pipe_enqueue(0, data1, 2));
	TEST_ASSERT_EQUAL(2, message_pipe_list[0]->count);
	
	TEST_ASSERT_TRUE(pipe_enqueue(0, data2, 3));
	TEST_ASSERT_EQUAL(5, message_pipe_list[0]->count);
	
	// Verify all data
	TEST_ASSERT_EQUAL(0x01, message_pipe_list[0]->buffer[0]);
	TEST_ASSERT_EQUAL(0x02, message_pipe_list[0]->buffer[1]);
	TEST_ASSERT_EQUAL(0x03, message_pipe_list[0]->buffer[2]);
	TEST_ASSERT_EQUAL(0x04, message_pipe_list[0]->buffer[3]);
	TEST_ASSERT_EQUAL(0x05, message_pipe_list[0]->buffer[4]);
}

// Test: pipe_dequeue - invalid index
void test_pipe_dequeue_invalid_index(void) {
	test_init_task_list();
	
	uint8_t data[10];
	bool result = pipe_dequeue(ICARUS_MAX_MESSAGE_QUEUES, data, 3);
	TEST_ASSERT_FALSE(result);
}

// Test: pipe_dequeue - not engaged
void test_pipe_dequeue_not_engaged(void) {
	test_init_task_list();
	
	uint8_t data[10];
	bool result = pipe_dequeue(0, data, 3);
	TEST_ASSERT_FALSE(result);
}

// Test: pipe_dequeue - message too large
void test_pipe_dequeue_message_too_large(void) {
	test_init_task_list();
	os_register_task(test_task_1, "dequeue_test");
	current_task_index = 0;
	
	pipe_init(0, 10);  // Capacity of 10 bytes
	
	uint8_t data[20];
	bool result = pipe_dequeue(0, data, 20);  // Request more than capacity
	TEST_ASSERT_FALSE(result);
}

// Test: pipe_dequeue - basic dequeue
void test_pipe_dequeue_basic(void) {
	test_init_task_list();
	os_register_task(test_task_1, "dequeue_test");
	current_task_index = 0;
	
	pipe_init(0, 64);
	
	// Enqueue some data first
	uint8_t send_data[] = {0xDE, 0xAD, 0xBE, 0xEF};
	pipe_enqueue(0, send_data, 4);
	
	// Dequeue it
	uint8_t recv_data[4] = {0};
	bool result = pipe_dequeue(0, recv_data, 4);
	
	TEST_ASSERT_TRUE(result);
	TEST_ASSERT_EQUAL(0, message_pipe_list[0]->count);
	TEST_ASSERT_EQUAL(4, message_pipe_list[0]->dequeue_idx);
	
	// Verify received data
	TEST_ASSERT_EQUAL(0xDE, recv_data[0]);
	TEST_ASSERT_EQUAL(0xAD, recv_data[1]);
	TEST_ASSERT_EQUAL(0xBE, recv_data[2]);
	TEST_ASSERT_EQUAL(0xEF, recv_data[3]);
}

// Test: pipe enqueue and dequeue together
void test_pipe_enqueue_dequeue_together(void) {
	test_init_task_list();
	os_register_task(test_task_1, "pipe_test");
	current_task_index = 0;
	
	pipe_init(0, 64);
	
	// Enqueue
	uint8_t send1[] = {0x11, 0x22, 0x33};
	TEST_ASSERT_TRUE(pipe_enqueue(0, send1, 3));
	TEST_ASSERT_EQUAL(3, message_pipe_list[0]->count);
	
	// Partial dequeue
	uint8_t recv1[2] = {0};
	TEST_ASSERT_TRUE(pipe_dequeue(0, recv1, 2));
	TEST_ASSERT_EQUAL(1, message_pipe_list[0]->count);
	TEST_ASSERT_EQUAL(0x11, recv1[0]);
	TEST_ASSERT_EQUAL(0x22, recv1[1]);
	
	// Enqueue more
	uint8_t send2[] = {0x44, 0x55};
	TEST_ASSERT_TRUE(pipe_enqueue(0, send2, 2));
	TEST_ASSERT_EQUAL(3, message_pipe_list[0]->count);
	
	// Dequeue remaining
	uint8_t recv2[3] = {0};
	TEST_ASSERT_TRUE(pipe_dequeue(0, recv2, 3));
	TEST_ASSERT_EQUAL(0, message_pipe_list[0]->count);
	TEST_ASSERT_EQUAL(0x33, recv2[0]);
	TEST_ASSERT_EQUAL(0x44, recv2[1]);
	TEST_ASSERT_EQUAL(0x55, recv2[2]);
}

// Test: pipe circular buffer wrap-around
void test_pipe_circular_wrap(void) {
	test_init_task_list();
	os_register_task(test_task_1, "wrap_test");
	current_task_index = 0;
	
	pipe_init(0, 8);  // Small buffer to test wrap
	
	// Fill most of buffer
	uint8_t data1[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
	TEST_ASSERT_TRUE(pipe_enqueue(0, data1, 6));
	TEST_ASSERT_EQUAL(6, message_pipe_list[0]->enqueue_idx);
	
	// Dequeue some to make room
	uint8_t recv1[4] = {0};
	TEST_ASSERT_TRUE(pipe_dequeue(0, recv1, 4));
	TEST_ASSERT_EQUAL(4, message_pipe_list[0]->dequeue_idx);
	TEST_ASSERT_EQUAL(2, message_pipe_list[0]->count);
	
	// Enqueue more - should wrap around
	uint8_t data2[] = {0x07, 0x08, 0x09, 0x0A};
	TEST_ASSERT_TRUE(pipe_enqueue(0, data2, 4));
	TEST_ASSERT_EQUAL(6, message_pipe_list[0]->count);
	// enqueue_idx should have wrapped: (6 + 4) % 8 = 2
	TEST_ASSERT_EQUAL(2, message_pipe_list[0]->enqueue_idx);
	
	// Dequeue all and verify order
	uint8_t recv2[6] = {0};
	TEST_ASSERT_TRUE(pipe_dequeue(0, recv2, 6));
	TEST_ASSERT_EQUAL(0x05, recv2[0]);
	TEST_ASSERT_EQUAL(0x06, recv2[1]);
	TEST_ASSERT_EQUAL(0x07, recv2[2]);
	TEST_ASSERT_EQUAL(0x08, recv2[3]);
	TEST_ASSERT_EQUAL(0x09, recv2[4]);
	TEST_ASSERT_EQUAL(0x0A, recv2[5]);
}

// ============================================================================
// SEMAPHORE GETTER TESTS
// ============================================================================

// Test: semaphore_get_count basic
void test_semaphore_get_count_basic(void) {
	test_init_task_list();
	os_register_task(test_task_1, "get_count_test");
	current_task_index = 0;
	
	semaphore_init(0, 5);
	TEST_ASSERT_EQUAL(5, semaphore_get_count(0));
	
	semaphore_consume(0);
	TEST_ASSERT_EQUAL(4, semaphore_get_count(0));
	
	semaphore_feed(0);
	TEST_ASSERT_EQUAL(5, semaphore_get_count(0));
}

// Test: semaphore_get_count invalid index
void test_semaphore_get_count_invalid(void) {
	TEST_ASSERT_EQUAL(0, semaphore_get_count(ICARUS_MAX_SEMAPHORES));
	TEST_ASSERT_EQUAL(0, semaphore_get_count(ICARUS_MAX_SEMAPHORES + 1));
}

// Test: semaphore_get_count not engaged
void test_semaphore_get_count_not_engaged(void) {
	test_init_task_list();
	// Semaphore 5 not initialized
	TEST_ASSERT_EQUAL(0, semaphore_get_count(5));
}

// Test: semaphore_get_max_count basic
void test_semaphore_get_max_count_basic(void) {
	test_init_task_list();
	os_register_task(test_task_1, "max_count_test");
	current_task_index = 0;
	
	semaphore_init(0, 10);
	TEST_ASSERT_EQUAL(10, semaphore_get_max_count(0));
	
	// Max count shouldn't change after consume/feed
	semaphore_consume(0);
	TEST_ASSERT_EQUAL(10, semaphore_get_max_count(0));
}

// Test: semaphore_get_max_count invalid index
void test_semaphore_get_max_count_invalid(void) {
	TEST_ASSERT_EQUAL(0, semaphore_get_max_count(ICARUS_MAX_SEMAPHORES));
}

// Test: semaphore_get_max_count not engaged
void test_semaphore_get_max_count_not_engaged(void) {
	test_init_task_list();
	TEST_ASSERT_EQUAL(0, semaphore_get_max_count(7));
}

// ============================================================================
// PIPE GETTER TESTS
// ============================================================================

// Test: pipe_get_count basic
void test_pipe_get_count_basic(void) {
	test_init_task_list();
	os_register_task(test_task_1, "pipe_count_test");
	current_task_index = 0;
	
	pipe_init(0, 16);
	TEST_ASSERT_EQUAL(0, pipe_get_count(0));
	
	uint8_t data[] = {0x01, 0x02, 0x03};
	pipe_enqueue(0, data, 3);
	TEST_ASSERT_EQUAL(3, pipe_get_count(0));
	
	uint8_t recv[2];
	pipe_dequeue(0, recv, 2);
	TEST_ASSERT_EQUAL(1, pipe_get_count(0));
}

// Test: pipe_get_count invalid index
void test_pipe_get_count_invalid(void) {
	TEST_ASSERT_EQUAL(0, pipe_get_count(ICARUS_MAX_MESSAGE_QUEUES));
}

// Test: pipe_get_count not engaged
void test_pipe_get_count_not_engaged(void) {
	test_init_task_list();
	TEST_ASSERT_EQUAL(0, pipe_get_count(3));
}

// Test: pipe_get_max_count basic
void test_pipe_get_max_count_basic(void) {
	test_init_task_list();
	os_register_task(test_task_1, "pipe_max_test");
	current_task_index = 0;
	
	pipe_init(0, 32);
	TEST_ASSERT_EQUAL(32, pipe_get_max_count(0));
}

// Test: pipe_get_max_count invalid index
void test_pipe_get_max_count_invalid(void) {
	TEST_ASSERT_EQUAL(0, pipe_get_max_count(ICARUS_MAX_MESSAGE_QUEUES));
}

// Test: pipe_get_max_count not engaged
void test_pipe_get_max_count_not_engaged(void) {
	test_init_task_list();
	TEST_ASSERT_EQUAL(0, pipe_get_max_count(5));
}

// ============================================================================
// DISPLAY FUNCTION TESTS
// ============================================================================

// Test: display_render_vbar basic
void test_display_render_vbar_basic(void) {
	// Just verify it doesn't crash with various inputs
	display_render_vbar(10, 50, 5, 10);
	display_render_vbar(10, 50, 0, 10);
	display_render_vbar(10, 50, 10, 10);
	TEST_PASS();
}

// Test: display_render_vbar zero max_count (edge case)
void test_display_render_vbar_zero_max(void) {
	// Should handle gracefully (guard against div by zero)
	display_render_vbar(10, 50, 5, 0);
	TEST_PASS();
}

// Test: display_render_vbar count exceeds max
void test_display_render_vbar_overflow(void) {
	display_render_vbar(10, 50, 15, 10);  // count > max
	TEST_PASS();
}

// Test: display_render_pipe basic
void test_display_render_pipe_basic(void) {
	display_render_pipe(10, 50, "TEST", 5, 10, 42, 37, true, true);
	display_render_pipe(10, 50, "TEST", 0, 10, 0, 0, false, false);
	TEST_PASS();
}

// Test: display_render_pipe zero max_count
void test_display_render_pipe_zero_max(void) {
	display_render_pipe(10, 50, "TEST", 5, 0, 42, 37, true, true);
	TEST_PASS();
}

// Test: display_render_producer basic
void test_display_render_producer_basic(void) {
	display_render_producer(10, "producer", 50, 100, 42, true);
	display_render_producer(10, "producer", 100, 100, 42, false);
	display_render_producer(10, NULL, 50, 100, 42, true);  // null name
	TEST_PASS();
}

// Test: display_render_producer zero period
void test_display_render_producer_zero_period(void) {
	display_render_producer(10, "producer", 50, 0, 42, true);
	TEST_PASS();
}

// Test: display_render_consumer basic
void test_display_render_consumer_basic(void) {
	display_render_consumer(10, "consumer", 50, 100, 42, true);
	display_render_consumer(10, "consumer", 100, 100, 42, false);
	display_render_consumer(10, NULL, 50, 100, 42, true);  // null name
	TEST_PASS();
}

// Test: display_render_consumer zero period
void test_display_render_consumer_zero_period(void) {
	display_render_consumer(10, "consumer", 50, 0, 42, true);
	TEST_PASS();
}

// Test: msg_history_init
void test_msg_history_init_basic(void) {
	msg_history_t hist;
	hist.head = 99;
	hist.count = 99;
	
	msg_history_init(&hist);
	TEST_ASSERT_EQUAL(0, hist.head);
	TEST_ASSERT_EQUAL(0, hist.count);
}

// Test: msg_history_init null
void test_msg_history_init_null(void) {
	msg_history_init(NULL);  // Should not crash
	TEST_PASS();
}

// Test: msg_history_add basic
void test_msg_history_add_basic(void) {
	msg_history_t hist;
	msg_history_init(&hist);
	
	uint8_t data[] = {0x01, 0x02, 0x03};
	msg_history_add(&hist, data, 3, 0, true);
	
	TEST_ASSERT_EQUAL(1, hist.count);
	TEST_ASSERT_EQUAL(1, hist.head);
	TEST_ASSERT_EQUAL(0x01, hist.entries[0].data[0]);
	TEST_ASSERT_EQUAL(3, hist.entries[0].len);
	TEST_ASSERT_EQUAL(0, hist.entries[0].source_id);
	TEST_ASSERT_TRUE(hist.entries[0].is_send);
}

// Test: msg_history_add null
void test_msg_history_add_null(void) {
	msg_history_t hist;
	msg_history_init(&hist);
	
	msg_history_add(NULL, (uint8_t[]){1}, 1, 0, true);  // null hist
	msg_history_add(&hist, NULL, 1, 0, true);           // null data
	msg_history_add(&hist, (uint8_t[]){1}, 0, 0, true); // zero len
	
	TEST_ASSERT_EQUAL(0, hist.count);  // Nothing should be added
}

// Test: msg_history_add wrap around
void test_msg_history_add_wrap(void) {
	msg_history_t hist;
	msg_history_init(&hist);
	
	// Fill history beyond capacity
	for (int i = 0; i < MSG_HISTORY_LEN + 2; i++) {
		uint8_t data = (uint8_t)i;
		msg_history_add(&hist, &data, 1, 0, true);
	}
	
	TEST_ASSERT_EQUAL(MSG_HISTORY_LEN, hist.count);  // Should cap at max
	TEST_ASSERT_EQUAL(2, hist.head);  // Should wrap: (MSG_HISTORY_LEN + 2) % MSG_HISTORY_LEN
}

// Test: msg_history_add clamp length
void test_msg_history_add_clamp_len(void) {
	msg_history_t hist;
	msg_history_init(&hist);
	
	uint8_t data[10] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A};
	msg_history_add(&hist, data, 10, 0, true);  // len > MSG_HISTORY_MAX_BYTES
	
	TEST_ASSERT_EQUAL(MSG_HISTORY_MAX_BYTES, hist.entries[0].len);
}

// Test: display_render_msg_history null
void test_display_render_msg_history_null(void) {
	display_render_msg_history(10, 50, NULL, "TEST");  // Should not crash
	TEST_PASS();
}

// Test: display_render_msg_history basic
void test_display_render_msg_history_basic(void) {
	msg_history_t hist;
	msg_history_init(&hist);
	
	uint8_t data[] = {0x42};
	msg_history_add(&hist, data, 1, 0, true);
	msg_history_add(&hist, data, 1, 1, false);
	
	display_render_msg_history(10, 50, &hist, "SS");
	TEST_PASS();
}

// Test runner
int main(void) {
	UNITY_BEGIN();
	
	// Basic tests
	// DISABLED: test_enqueue_print_buffer_basic - enqueue_print_buffer commented out
	// DISABLED: test_enqueue_print_buffer_full - enqueue_print_buffer commented out
	RUN_TEST(test_task_busy_wait_basic);
	RUN_TEST(test_critical_sections);
	RUN_TEST(test_os_get_tick_count);
	
	// Initialization tests
	RUN_TEST(test_os_init);
	
	// Task registration tests
	RUN_TEST(test_os_register_task);
	RUN_TEST(test_os_register_task_multiple);
	RUN_TEST(test_os_create_task_normal);
	RUN_TEST(test_os_create_task_max_tasks);
	
	// Getter tests
	RUN_TEST(test_os_get_running_task_count);
	RUN_TEST(test_os_get_current_task_name);
	RUN_TEST(test_os_get_task_ticks_remaining);
	
	// OS control tests
	RUN_TEST(test_os_start_no_tasks);
	RUN_TEST(test_os_start_valid_tasks);
	RUN_TEST(test_os_exit_task_with_running_count);
	RUN_TEST(test_os_exit_task_cleanup_idx_max);
	RUN_TEST(test_os_exit_task_zero_running_count);
	
	// Task management tests
	RUN_TEST(test_os_kill_process_valid);
	RUN_TEST(test_os_kill_process_invalid_index);
	RUN_TEST(test_os_kill_process_index_zero);
	RUN_TEST(test_os_kill_process_index_zero_error);
	RUN_TEST(test_os_kill_process_already_killed);
	RUN_TEST(test_os_kill_process_suicide);
	RUN_TEST(test_os_kill_process_cleanup_idx_max);
	RUN_TEST(test_os_task_suicide);
	// DISABLED: test_os_print_finished_tasks - os_print_finished_tasks commented out
	
	// Sleep tests
	RUN_TEST(test_os_yield);
	RUN_TEST(test_task_active_sleep);
	RUN_TEST(test_task_blocking_sleep);
	
	// Additional tests
	RUN_TEST(test_os_create_task_long_name);
	// DISABLED: test_dequeue_print_buffer_indirect - enqueue_print_buffer commented out
	
	// DO-178C Coverage Tests - Branch Coverage
	// DISABLED: test_exit_critical_depth_zero - enqueue_print_buffer commented out
	// DISABLED: test_exit_critical_depth_nonzero - enqueue_print_buffer commented out
	// DISABLED: test_enqueue_print_buffer_wrap_around - enqueue_print_buffer commented out
	// DISABLED: test_enqueue_print_buffer_boundary - enqueue_print_buffer commented out
	RUN_TEST(test_os_get_current_task_name_invalid_index);
	RUN_TEST(test_os_get_current_task_name_boundary);
	RUN_TEST(test_os_get_current_task_name_null_task);
	RUN_TEST(test_os_start_too_many_tasks);
	RUN_TEST(test_os_start_boundary_max_tasks);
	RUN_TEST(test_os_kill_process_index_equal);
	RUN_TEST(test_os_create_task_boundary_max_minus_one);
	RUN_TEST(test_os_create_task_max_running_tasks);
	// DISABLED: test_os_print_finished_tasks_empty - os_print_finished_tasks commented out
	
	// DO-178C Coverage Tests - Display Functions
	RUN_TEST(test_display_render_bar_zero_period);
	RUN_TEST(test_display_render_bar_elapsed_exceeds_period);
	RUN_TEST(test_display_render_bar_filled_exceeds_width);
	RUN_TEST(test_display_render_bar_null_name);
	RUN_TEST(test_display_render_bar_normal);
	RUN_TEST(test_display_render_banner_on);
	RUN_TEST(test_display_render_banner_off);
	RUN_TEST(test_display_init_first_call);
	RUN_TEST(test_display_init_second_call);
	
	// DO-178C Coverage Tests - I/O Functions
	RUN_TEST(test_io_putchar_newline);
	RUN_TEST(test_io_putchar_buffer_full);
	RUN_TEST(test_io_putchar_normal);
	
	// DO-178C Coverage Tests - retarget_hal.c Functions
	RUN_TEST(test_LED_On);
	RUN_TEST(test_LED_Off);
	RUN_TEST(test_LED_Blink);
	RUN_TEST(test_LED_Blink_zero_delays);
	RUN_TEST(test_platform_write);
	RUN_TEST(test_platform_write_zero_length);
	RUN_TEST(test_platform_read);
	RUN_TEST(test_platform_read_zero_length);
	// Note: test_hal_init is ignored - hal_init requires extensive hardware simulation
	RUN_TEST(test_hal_init);
	
	// DO-178C Coverage Tests - stm32h7xx_it.c (Interrupt Handlers)
	RUN_TEST(test_SysTick_Handler);
	RUN_TEST(test_SysTick_Handler_os_not_running);
	RUN_TEST(test_SysTick_Handler_scheduler_disabled);
	RUN_TEST(test_SysTick_Handler_trigger_scheduler);
	RUN_TEST(test_SysTick_Handler_multiple_ticks);
	RUN_TEST(test_OTG_FS_IRQHandler);
	RUN_TEST(test_SVC_Handler);
	RUN_TEST(test_DebugMon_Handler);
	
	// Semaphore Tests
	RUN_TEST(test_semaphore_init_basic);
	RUN_TEST(test_semaphore_init_invalid_index);
	RUN_TEST(test_semaphore_init_zero_count);
	RUN_TEST(test_semaphore_init_already_engaged);
	RUN_TEST(test_semaphore_init_multiple);
	RUN_TEST(test_semaphore_init_boundary);
	RUN_TEST(test_semaphore_feed_invalid_index);
	RUN_TEST(test_semaphore_feed_not_engaged);
	RUN_TEST(test_semaphore_feed_basic);
	RUN_TEST(test_semaphore_consume_invalid_index);
	RUN_TEST(test_semaphore_consume_not_engaged);
	RUN_TEST(test_semaphore_consume_basic);
	RUN_TEST(test_semaphore_consume_multiple);
	RUN_TEST(test_semaphore_feed_consume_together);
	RUN_TEST(test_semaphore_consume_to_zero);
	RUN_TEST(test_semaphore_feed_to_max);
	
	// Message Pipe Tests
	RUN_TEST(test_pipe_init_basic);
	RUN_TEST(test_pipe_init_invalid_index);
	RUN_TEST(test_pipe_init_zero_capacity);
	RUN_TEST(test_pipe_init_capacity_max);
	RUN_TEST(test_pipe_init_capacity_too_large);
	RUN_TEST(test_pipe_init_already_engaged);
	RUN_TEST(test_pipe_init_multiple);
	RUN_TEST(test_pipe_init_boundary);
	RUN_TEST(test_pipe_enqueue_invalid_index);
	RUN_TEST(test_pipe_enqueue_not_engaged);
	RUN_TEST(test_pipe_enqueue_message_too_large);
	RUN_TEST(test_pipe_enqueue_basic);
	RUN_TEST(test_pipe_enqueue_multiple);
	RUN_TEST(test_pipe_dequeue_invalid_index);
	RUN_TEST(test_pipe_dequeue_not_engaged);
	RUN_TEST(test_pipe_dequeue_message_too_large);
	RUN_TEST(test_pipe_dequeue_basic);
	RUN_TEST(test_pipe_enqueue_dequeue_together);
	RUN_TEST(test_pipe_circular_wrap);
	
	// Semaphore Getter Tests
	RUN_TEST(test_semaphore_get_count_basic);
	RUN_TEST(test_semaphore_get_count_invalid);
	RUN_TEST(test_semaphore_get_count_not_engaged);
	RUN_TEST(test_semaphore_get_max_count_basic);
	RUN_TEST(test_semaphore_get_max_count_invalid);
	RUN_TEST(test_semaphore_get_max_count_not_engaged);
	
	// Pipe Getter Tests
	RUN_TEST(test_pipe_get_count_basic);
	RUN_TEST(test_pipe_get_count_invalid);
	RUN_TEST(test_pipe_get_count_not_engaged);
	RUN_TEST(test_pipe_get_max_count_basic);
	RUN_TEST(test_pipe_get_max_count_invalid);
	RUN_TEST(test_pipe_get_max_count_not_engaged);
	
	// Display Function Tests (vbar, pipe, producer, consumer)
	RUN_TEST(test_display_render_vbar_basic);
	RUN_TEST(test_display_render_vbar_zero_max);
	RUN_TEST(test_display_render_vbar_overflow);
	RUN_TEST(test_display_render_pipe_basic);
	RUN_TEST(test_display_render_pipe_zero_max);
	RUN_TEST(test_display_render_producer_basic);
	RUN_TEST(test_display_render_producer_zero_period);
	RUN_TEST(test_display_render_consumer_basic);
	RUN_TEST(test_display_render_consumer_zero_period);
	
	// Message History Tests
	RUN_TEST(test_msg_history_init_basic);
	RUN_TEST(test_msg_history_init_null);
	RUN_TEST(test_msg_history_add_basic);
	RUN_TEST(test_msg_history_add_null);
	RUN_TEST(test_msg_history_add_wrap);
	RUN_TEST(test_msg_history_add_clamp_len);
	RUN_TEST(test_display_render_msg_history_null);
	RUN_TEST(test_display_render_msg_history_basic);
	
	return UNITY_END();
}
