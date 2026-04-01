/**
 * @file    error.h
 * @brief   ICARUS OS Error Handling Interface
 * @version 0.1.0
 *
 * @details Provides system-wide error handling declarations.
 *          The Error_Handler is called for unrecoverable faults.
 *
 * @author  Souham Biswas
 * @date    2025
 *
 * @copyright Copyright 2025-2026 Souham Biswas
 *            https://github.com/ironhide23586/icarus-os-core
 *            Licensed under the Apache License, Version 2.0
 */

#ifndef INC_BSP_ERROR_H_
#define INC_BSP_ERROR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "cmsis_gcc.h"

/* ============================================================================
 * ERROR HANDLING API
 * ========================================================================= */

/**
 * @brief   System error handler for unrecoverable faults
 *
 * @details Called when a fatal error is detected. Disables interrupts
 *          and enters an infinite loop. Does not return.
 *
 * @warning This function never returns!
 */
void Error_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* INC_BSP_ERROR_H_ */
