/**
 * @file    error.c
 * @brief   ICARUS OS Error Handling Implementation
 * @version 0.1.0
 *
 * @details Provides system-wide error handling for unrecoverable faults.
 *          The Error_Handler enters an infinite loop with interrupts disabled
 *          to prevent further system corruption.
 *
 * @par Error Recovery:
 *      The Error_Handler does not return. System recovery requires:
 *      - Hardware reset (NRST pin)
 *      - Power cycle
 *      - Watchdog timeout (if enabled)
 *
 * @see     docs/do178c/design/SDD.md Section 7 - Error Handling
 *
 * @author  Souham Biswas
 * @date    2025
 *
 * @copyright Copyright 2025 Souham Biswas
 *            https://github.com/ironhide23586/icarus-os-core
 *            Licensed under the Apache License, Version 2.0
 */

#ifndef SRC_BSP_ERROR_C_
#define SRC_BSP_ERROR_C_

#include "bsp/error.h"

/**
 * @brief   System error handler for unrecoverable faults
 *
 * @details Called when a fatal error is detected that cannot be recovered.
 *          Disables all interrupts and enters an infinite loop to prevent
 *          further system corruption.
 *
 * @par Typical Callers:
 *      - HAL initialization failures
 *      - Clock configuration errors
 *      - Peripheral initialization failures
 *      - Stack overflow detection (if enabled)
 *
 * @warning This function never returns!
 * @warning All interrupts are disabled - watchdog will not be serviced
 *
 * @note    For debugging, set a breakpoint on this function to catch errors
 * @note    Consider adding LED blink pattern for field diagnosis
 */
void Error_Handler(void) {
	__disable_irq();
	while (1) {
		__NOP();
	}
}




#endif /* SRC_BSP_ERROR_C_ */
