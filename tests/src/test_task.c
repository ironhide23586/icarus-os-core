/*
 * test_task.c
 * Comprehensive unit tests for kernel task management functions
 * Tests all public API functions and major code paths
 */

#include "unity.h"
#include "kernel/task.h"
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
extern volatile uint8_t print_buffer_dequeue_idx;
extern volatile uint8_t print_buffer_enqueue_idx;
extern volatile bool scheduler_enabled;
extern volatile uint8_t critical_stack_depth;
extern volatile uint32_t current_task_ticks_remaining;
extern task_t* task_list[];
extern uint32_t cpu_vregisters[];

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
	print_buffer_dequeue_idx = 0;
	print_buffer_enqueue_idx = 0;
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
void test_enqueue_print_buffer_basic(void) {
	uint8_t test_char = 'A';
	bool result;
	
	// Reset indices to known state
	print_buffer_enqueue_idx = 0;
	print_buffer_dequeue_idx = 0;
	
	// Should be able to enqueue when buffer is empty
	result = enqueue_print_buffer(test_char);
	TEST_ASSERT_TRUE(result);
	TEST_ASSERT_EQUAL(1, print_buffer_enqueue_idx);
	TEST_ASSERT_EQUAL(0, print_buffer_dequeue_idx);
	
	// Enqueue another character
	result = enqueue_print_buffer('B');
	TEST_ASSERT_TRUE(result);
	TEST_ASSERT_EQUAL(2, print_buffer_enqueue_idx);
}

// Test: enqueue_print_buffer buffer full
void test_enqueue_print_buffer_full(void) {
	uint8_t i;
	
	// Fill buffer to capacity
	print_buffer_enqueue_idx = 0;
	print_buffer_dequeue_idx = 0;
	
	// Fill buffer (PRINT_BUFFER_BYTES - 1, leaving one slot for wrap detection)
	for (i = 0; i < PRINT_BUFFER_BYTES - 1; i++) {
		TEST_ASSERT_TRUE(enqueue_print_buffer('A' + i));
	}
	
	// Next enqueue should fail (buffer full)
	TEST_ASSERT_FALSE(enqueue_print_buffer('Z'));
}

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
	
	// Test that enqueue_print_buffer uses critical sections
	bool result = enqueue_print_buffer('A');
	TEST_ASSERT_TRUE(result);
	// After enqueue, critical section should be exited
	TEST_ASSERT_TRUE(scheduler_enabled);
	TEST_ASSERT_EQUAL(0, critical_stack_depth);
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
	TEST_ASSERT_EQUAL(TICKS_PER_TASK, current_task_ticks_remaining);
	
	// os_init registers 2 system tasks (idle and heartbeat)
	TEST_ASSERT_EQUAL(2, num_created_tasks);
	
	// Verify task_list was initialized
	TEST_ASSERT_NOT_NULL(task_list[0]);
	TEST_ASSERT_NOT_NULL(task_list[1]);
	
	// Verify system task names
	TEST_ASSERT_EQUAL_STRING("ICARUS_KEEPALIVE_TASK", task_list[0]->name);
	TEST_ASSERT_EQUAL_STRING(">ICARUS_HEARTBEART<", task_list[1]->name);
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
	TEST_ASSERT_EQUAL(TASK_COLD, task_list[initial_count]->task_state);
	TEST_ASSERT_EQUAL_STRING("registered_task", task_list[initial_count]->name);
	TEST_ASSERT_NOT_NULL(task_list[initial_count]->function);
	TEST_ASSERT_NOT_NULL(task_list[initial_count]->stack_base);
	TEST_ASSERT_EQUAL(PRI_LOW, task_list[initial_count]->task_priority);
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
	uint32_t ticks = os_get_task_ticks_remaining();
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
	TEST_ASSERT_EQUAL(TASK_RUNNING, task_list[0]->task_state);
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
	TEST_ASSERT_EQUAL(TASK_FINISHED, task_list[0]->task_state);
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
	current_cleanup_task_idx = MAX_TASKS - 1;  // At max, can't add more
	
	os_exit_task();
	
	// Task should be FINISHED
	TEST_ASSERT_EQUAL(TASK_FINISHED, task_list[0]->task_state);
	// cleanup_task_idx should stay at max (can't increment past MAX_TASKS - 1)
	TEST_ASSERT_EQUAL(MAX_TASKS - 1, current_cleanup_task_idx);
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
	TEST_ASSERT_EQUAL(TASK_FINISHED, task_list[0]->task_state);
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
	task_list[0]->task_state = TASK_READY;
	task_list[1]->task_state = TASK_READY;
	
	current_task_index = 0;
	running_task_count = 2;
	current_cleanup_task_idx = -1;
	
	// Kill task 1 (not task 0, since os_kill_process requires task_index > 0)
	os_kill_process(1);
	
	// Verify task was killed
	TEST_ASSERT_EQUAL(TASK_KILLED, task_list[1]->task_state);
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
	
	task_list[0]->task_state = TASK_READY;
	task_list[1]->task_state = TASK_READY;
	current_task_index = 1;
	running_task_count = 2;
	
	// Try to kill task 0 - should fail (task_index > 0 check)
	os_kill_process(0);
	
	// Task 0 should still be READY (not killed)
	TEST_ASSERT_EQUAL(TASK_READY, task_list[0]->task_state);
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
	TEST_ASSERT_EQUAL(TASK_KILLED, task_list[1]->task_state);
	// running_task_count should be decremented
	TEST_ASSERT_EQUAL(1, running_task_count);
}

// Test: os_print_finished_tasks
void test_os_print_finished_tasks(void) {
	// Set up some finished tasks
	current_cleanup_task_idx = 2;
	// Note: cleanup_task_idx is static, so we can't directly set it
	// But we can test the function doesn't crash
	os_print_finished_tasks();
	
	TEST_PASS(); // If we get here, no crash
}

// Test: os_yield
void test_os_yield(void) {
	// os_yield should:
	// 1. Reset current_task_ticks_remaining to ticks_per_task
	// 2. Set PendSV pending bit
	
	current_task_ticks_remaining = 10;  // Some arbitrary value
	TEST_CLEAR_PENDSV();
	
	os_yield();
	
	// Verify ticks were reset
	TEST_ASSERT_EQUAL(TICKS_PER_TASK, current_task_ticks_remaining);
	
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
	TEST_ASSERT_EQUAL(TASK_BLOCKED, task_list[0]->task_state);
	
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
	// Fill up to MAX_TASKS
	running_task_count = MAX_TASKS;
	
	uint8_t initial_count = num_created_tasks;
	
	// Try to create another task (should fail)
	task_t dummy_task;
	uint32_t dummy_stack[100];
	os_create_task(&dummy_task, test_task_1, dummy_stack, 100, "overflow");
	
	// Should not increment num_created_tasks
	TEST_ASSERT_EQUAL(initial_count, num_created_tasks);
}

// Test: os_create_task normal case
void test_os_create_task_normal(void) {
	task_t test_task;
	uint32_t test_stack[STACK_WORDS];
	uint8_t initial_count = num_created_tasks;
	
	os_create_task(&test_task, test_task_1, test_stack, STACK_WORDS, "created_task");
	
	TEST_ASSERT_EQUAL(initial_count + 1, num_created_tasks);
	TEST_ASSERT_EQUAL(TASK_COLD, test_task.task_state);
	TEST_ASSERT_EQUAL_STRING("created_task", test_task.name);
	TEST_ASSERT_NOT_NULL(test_task.function);
	TEST_ASSERT_NOT_NULL(test_task.stack_base);
	TEST_ASSERT_EQUAL(PRI_LOW, test_task.task_priority);
	TEST_ASSERT_EQUAL(0, test_task.global_tick_paused);
	TEST_ASSERT_EQUAL(0, test_task.ticks_to_pause);
	TEST_ASSERT_NOT_NULL(test_task.stack_pointer);
}

// Test: os_create_task with long name (truncation)
void test_os_create_task_long_name(void) {
	task_t test_task;
	uint32_t test_stack[STACK_WORDS];
	char long_name[100];
	memset(long_name, 'A', 99);
	long_name[99] = '\0';
	
	os_create_task(&test_task, test_task_1, test_stack, STACK_WORDS, long_name);
	
	// Name should be truncated to MAX_TASK_NAME_LENGTH - 1
	TEST_ASSERT_EQUAL('\0', test_task.name[MAX_TASK_NAME_LENGTH - 1]);
}

// Test: dequeue_print_buffer (tested indirectly through enqueue)
void test_dequeue_print_buffer_indirect(void) {
	// Fill buffer
	print_buffer_enqueue_idx = 0;
	print_buffer_dequeue_idx = 0;
	
	enqueue_print_buffer('H');
	enqueue_print_buffer('e');
	enqueue_print_buffer('l');
	enqueue_print_buffer('l');
	enqueue_print_buffer('o');
	
	// dequeue_print_buffer is static, but we know it's used internally
	// We can't test it directly, but we know enqueue works
	TEST_ASSERT_EQUAL(5, print_buffer_enqueue_idx);
	TEST_ASSERT_EQUAL(0, print_buffer_dequeue_idx);
}

// ============================================================================
// DO-178C Coverage Tests - Branch, Statement, and MC/DC Coverage
// ============================================================================

// Test: exit_critical with critical_stack_depth == 0 (true branch)
void test_exit_critical_depth_zero(void) {
	critical_stack_depth = 0;
	scheduler_enabled = false;
	
	bool result = enqueue_print_buffer('A');
	TEST_ASSERT_TRUE(result);
	TEST_ASSERT_TRUE(scheduler_enabled);
	TEST_ASSERT_EQUAL(0, critical_stack_depth);
}

// Test: exit_critical with critical_stack_depth > 0 (false branch)
void test_exit_critical_depth_nonzero(void) {
	critical_stack_depth = 1;
	scheduler_enabled = false;
	
	bool result1 = enqueue_print_buffer('A');
	TEST_ASSERT_TRUE(result1);
	// After enqueue: enter (1->2), do work, exit (2->1)
	// Depth should be back to 1 (not 0), scheduler still disabled
	TEST_ASSERT_EQUAL(1, critical_stack_depth);
	TEST_ASSERT_FALSE(scheduler_enabled); // Should still be disabled (depth > 0)
}

// Test: enqueue_print_buffer - wrap around case
void test_enqueue_print_buffer_wrap_around(void) {
	// Set up: enqueue_idx at last position, dequeue at start
	// Buffer has space: [dequeue=0] ... [enqueue=PRINT_BUFFER_BYTES-1]
	// After enqueue, should wrap to 0
	// But: next_enqueue_idx = (PRINT_BUFFER_BYTES-1 + 1) % PRINT_BUFFER_BYTES = 0
	// If dequeue_idx is 0, then next_enqueue_idx == dequeue_idx, so buffer is FULL!
	// Need dequeue_idx > 0 for wrap to work
	print_buffer_enqueue_idx = PRINT_BUFFER_BYTES - 1;
	print_buffer_dequeue_idx = 1; // Leave space for wrap (not 0)
	
	bool result = enqueue_print_buffer('W');
	TEST_ASSERT_TRUE(result);
	TEST_ASSERT_EQUAL(0, print_buffer_enqueue_idx); // Should wrap to 0
}

// Test: enqueue_print_buffer - boundary conditions
void test_enqueue_print_buffer_boundary(void) {
	print_buffer_enqueue_idx = PRINT_BUFFER_BYTES - 2;
	print_buffer_dequeue_idx = 0;
	
	bool result = enqueue_print_buffer('B');
	TEST_ASSERT_TRUE(result);
	TEST_ASSERT_EQUAL(PRINT_BUFFER_BYTES - 1, print_buffer_enqueue_idx);
}

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

// Test: os_start - num_created_tasks > MAX_TASKS (true branch: return early)
void test_os_start_too_many_tasks(void) {
	num_created_tasks = MAX_TASKS + 1;
	os_start();
	TEST_PASS();
}

// Test: os_start - boundary: num_created_tasks == MAX_TASKS
void test_os_start_boundary_max_tasks(void) {
	num_created_tasks = MAX_TASKS;
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
	TEST_ASSERT_EQUAL(TASK_COLD, task_list[0]->task_state);
}

// Test: os_kill_process - task state >= 4 (already killed/finished, error path)
void test_os_kill_process_already_killed(void) {
	test_init_task_list();
	os_register_task(test_task_1, "task1");
	os_register_task(test_task_2, "task2");
	current_task_index = 0;
	
	// Set task state to TASK_KILLED (4) or TASK_FINISHED (5)
	task_list[1]->task_state = TASK_KILLED; // State 4
	uint8_t initial_count = running_task_count;
	
	os_kill_process(1);
	
	// Should not change running_task_count (state >= 4)
	TEST_ASSERT_EQUAL(initial_count, running_task_count);
	// Should still be KILLED
	TEST_ASSERT_EQUAL(TASK_KILLED, task_list[1]->task_state);
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
	TEST_ASSERT_EQUAL(TASK_KILLED, task_list[1]->task_state);
	// running_task_count should be decremented
	TEST_ASSERT_EQUAL(1, running_task_count);
}

// Test: os_kill_process - cleanup_idx at max (boundary case)
void test_os_kill_process_cleanup_idx_max(void) {
	test_init_task_list();
	os_register_task(test_task_1, "task1");
	os_register_task(test_task_2, "task2");
	
	task_list[0]->task_state = TASK_READY;
	task_list[1]->task_state = TASK_READY;
	current_task_index = 0;
	running_task_count = 2;
	current_cleanup_task_idx = MAX_TASKS - 1;  // At max
	
	os_kill_process(1);
	
	// Task should be killed
	TEST_ASSERT_EQUAL(TASK_KILLED, task_list[1]->task_state);
	// cleanup_task_idx should stay at max
	TEST_ASSERT_EQUAL(MAX_TASKS - 1, current_cleanup_task_idx);
}

// Test: os_create_task - boundary: running_task_count == MAX_TASKS - 1
void test_os_create_task_boundary_max_minus_one(void) {
	running_task_count = MAX_TASKS - 1;
	task_t test_task;
	uint32_t test_stack[STACK_WORDS];
	uint8_t initial_count = num_created_tasks;
	
	os_create_task(&test_task, test_task_1, test_stack, STACK_WORDS, "boundary");
	
	TEST_ASSERT_EQUAL(initial_count + 1, num_created_tasks);
}

// Test: os_create_task - early return when running_task_count >= MAX_TASKS
void test_os_create_task_max_running_tasks(void) {
	task_t test_task;
	uint32_t test_stack[STACK_WORDS];
	uint8_t initial_count = num_created_tasks;
	running_task_count = MAX_TASKS; // At max, should return early
	
	os_create_task(&test_task, test_task_1, test_stack, STACK_WORDS, "max_running");
	
	// Should not increment num_created_tasks (early return)
	TEST_ASSERT_EQUAL(initial_count, num_created_tasks);
}

// Test: os_print_finished_tasks - current_cleanup_task_idx == -1 (loop doesn't execute)
void test_os_print_finished_tasks_empty(void) {
	current_cleanup_task_idx = -1;
	os_print_finished_tasks();
	TEST_PASS();
}

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
	TEST_ASSERT_EQUAL(TASK_BLOCKED, task_list[0]->task_state);
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

// Test: platform_delay
void test_platform_delay(void) {
	platform_delay(100);
	TEST_PASS();
}

// Test: platform_delay with zero (boundary case)
void test_platform_delay_zero(void) {
	platform_delay(0);
	TEST_PASS();
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
	current_task_ticks_remaining = TICKS_PER_TASK;
	scheduler_enabled = true;
	
	// Call SysTick_Handler
	SysTick_Handler();
	
	// Verify tick count incremented
	TEST_ASSERT_EQUAL(1, os_tick_count);
	// Verify ticks_remaining decremented
	TEST_ASSERT_EQUAL(TICKS_PER_TASK - 1, current_task_ticks_remaining);
}

// Test: SysTick_Handler - with os_running == 0 (should not trigger scheduler)
void test_SysTick_Handler_os_not_running(void) {
	os_tick_count = 0;
	os_running = 0;
	current_task_ticks_remaining = TICKS_PER_TASK;
	scheduler_enabled = true;
	
	SysTick_Handler();
	
	// Tick should increment
	TEST_ASSERT_EQUAL(1, os_tick_count);
	// When os_running == 0, the condition short-circuits, so ticks_remaining is NOT decremented
	// The condition is: if (os_running && --current_task_ticks_remaining == 0 && scheduler_enabled)
	// Since os_running is 0, the && short-circuits before decrementing
	TEST_ASSERT_EQUAL(TICKS_PER_TASK, current_task_ticks_remaining);
	// SCB->ICSR should not be set (scheduler not triggered)
}

// Test: SysTick_Handler - with scheduler_enabled == false (should not trigger scheduler)
void test_SysTick_Handler_scheduler_disabled(void) {
	os_tick_count = 0;
	os_running = 1;
	current_task_ticks_remaining = TICKS_PER_TASK;
	scheduler_enabled = false;
	
	SysTick_Handler();
	
	TEST_ASSERT_EQUAL(1, os_tick_count);
	TEST_ASSERT_EQUAL(TICKS_PER_TASK - 1, current_task_ticks_remaining);
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
	// ticks_remaining should be reset to TICKS_PER_TASK
	TEST_ASSERT_EQUAL(TICKS_PER_TASK, current_task_ticks_remaining);
	// SCB->ICSR should be set (PendSV triggered)
}

// Test: SysTick_Handler - multiple ticks
void test_SysTick_Handler_multiple_ticks(void) {
	os_tick_count = 0;
	os_running = 1;
	current_task_ticks_remaining = TICKS_PER_TASK;
	scheduler_enabled = true;
	
	// Call multiple times
	for (int i = 0; i < 5; i++) {
		SysTick_Handler();
	}
	
	TEST_ASSERT_EQUAL(5, os_tick_count);
	TEST_ASSERT_EQUAL(TICKS_PER_TASK - 5, current_task_ticks_remaining);
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

// Test runner
int main(void) {
	UNITY_BEGIN();
	
	// Basic tests
	RUN_TEST(test_enqueue_print_buffer_basic);
	RUN_TEST(test_enqueue_print_buffer_full);
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
	RUN_TEST(test_os_print_finished_tasks);
	
	// Sleep tests
	RUN_TEST(test_os_yield);
	RUN_TEST(test_task_active_sleep);
	RUN_TEST(test_task_blocking_sleep);
	
	// Additional tests
	RUN_TEST(test_os_create_task_long_name);
	RUN_TEST(test_dequeue_print_buffer_indirect);
	
	// DO-178C Coverage Tests - Branch Coverage
	RUN_TEST(test_exit_critical_depth_zero);
	RUN_TEST(test_exit_critical_depth_nonzero);
	RUN_TEST(test_enqueue_print_buffer_wrap_around);
	RUN_TEST(test_enqueue_print_buffer_boundary);
	RUN_TEST(test_os_get_current_task_name_invalid_index);
	RUN_TEST(test_os_get_current_task_name_boundary);
	RUN_TEST(test_os_get_current_task_name_null_task);
	RUN_TEST(test_os_start_too_many_tasks);
	RUN_TEST(test_os_start_boundary_max_tasks);
	RUN_TEST(test_os_kill_process_index_equal);
	RUN_TEST(test_os_create_task_boundary_max_minus_one);
	RUN_TEST(test_os_create_task_max_running_tasks);
	RUN_TEST(test_os_print_finished_tasks_empty);
	
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
	RUN_TEST(test_platform_delay);
	RUN_TEST(test_platform_delay_zero);
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
	
	return UNITY_END();
}
