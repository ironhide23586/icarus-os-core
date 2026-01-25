/*
 * mock_asm.h
 * Mock for assembly functions that are ARM-specific
 */

#ifndef MOCK_ASM_H
#define MOCK_ASM_H

#include "kernel/task.h"

// Mock assembly functions for host testing
// These are ARM assembly functions in context_switch.s
// For host testing, we provide C implementations

void start_cold_task(task_t *task);
void os_yield_trampoline(void);
void os_yield_pendsv(void);

#endif /* MOCK_ASM_H */
