/*
 * test_task_basic.c
 * Basic unit tests for kernel task management functions
 */

#include "unity.h"
#include "kernel/task.h"
#include <string.h>
#include <stdint.h>

// Access to internal kernel variables for testing
// These are declared in task.c but we need them for test setup
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
}

void tearDown(void) {
	// Cleanup after each test
}

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
// NOTE: task_busy_wait has a while(1) loop that waits for os_tick_count to increment.
// In a test environment, ticks don't increment automatically, so this would hang.
// We skip this test for now - it requires interrupt simulation or a different approach.
void test_task_busy_wait_basic(void) {
	// Skip this test - task_busy_wait requires SysTick interrupt to work
	// In host testing, we can't simulate interrupts easily
	// This would need to be tested on target or with interrupt simulation
	TEST_IGNORE_MESSAGE("task_busy_wait requires SysTick interrupt - skip in host tests");
}

// Test: enter_critical and exit_critical
// Note: These are static inline functions, so we test them indirectly
// by checking the scheduler_enabled and critical_stack_depth variables
void test_critical_sections(void) {
	// This test is skipped for now since enter_critical/exit_critical
	// are static inline and not directly accessible
	// We can test them indirectly through functions that use them
	TEST_ASSERT_TRUE(scheduler_enabled);
	TEST_ASSERT_EQUAL(0, critical_stack_depth);
	
	// Test that enqueue_print_buffer uses critical sections
	// (it calls enter_critical/exit_critical internally)
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

// Test runner
int main(void) {
	UNITY_BEGIN();
	
	RUN_TEST(test_enqueue_print_buffer_basic);
	RUN_TEST(test_enqueue_print_buffer_full);
	RUN_TEST(test_task_busy_wait_basic);
	RUN_TEST(test_critical_sections);
	RUN_TEST(test_os_get_tick_count);
	
	return UNITY_END();
}
