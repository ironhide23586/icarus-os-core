/*
 * mock_cmsis_gcc.h
 * Minimal mock for CMSIS GCC intrinsics and SCB
 */

#ifndef CMSIS_GCC_H
#define CMSIS_GCC_H

#include <stdint.h>

// Minimal CMSIS intrinsics needed for testing
// All are no-ops for host testing

#define __disable_irq() do {} while(0)
#define __enable_irq() do {} while(0)
#define __DSB() do {} while(0)
#define __DMB() do {} while(0)
#define __ISB() do {} while(0)

// Additional intrinsics
#define __NOP() do {} while(0)
#define __WFI() do {} while(0)
#define __WFE() do {} while(0)

// System Control Block (SCB) - minimal mock
typedef struct {
	volatile uint32_t ICSR;  // Interrupt Control and State Register
	volatile uint32_t VTOR;  // Vector Table Offset Register
} SCB_Type;

#define SCB_ICSR_PENDSVSET_Msk (1UL << 28U)

// SCB base address (mock - use static memory instead of fixed address)
// This prevents segfaults when writing to SCB->ICSR in host tests
static SCB_Type scb_mock = {0};
#define SCB (&scb_mock)

#endif /* CMSIS_GCC_H */
