/*
 * mock_asm.h
 * Mock for assembly functions that are ARM-specific
 */

#ifndef MOCK_ASM_H
#define MOCK_ASM_H

#include "icarus/types.h"

// Mock assembly functions for host testing
// These are ARM assembly functions in context_switch.s
// For host testing, we provide C implementations

void __start_cold_task(icarus_task_t *task);
void os_yield_trampoline(void);
void __os_yield_pendsv(void);

// Test helper: simulate tick advancement
// Call this to advance os_tick_count, allowing sleep functions to complete
void test_advance_ticks(uint32_t ticks);

// Test helper: get number of times os_yield_pendsv was called
uint32_t test_get_yield_count(void);

// Test helper: reset yield count
void test_reset_yield_count(void);

#endif /* MOCK_ASM_H */
