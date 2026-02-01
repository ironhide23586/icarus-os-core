/**
 * @file    bsp_error.h
 * @brief   Board Support Package - Error Handling
 * @version 0.1.0
 *
 * @details System error handling and fault management.
 *
 * @author  Souham Biswas
 * @date    2026
 *
 * @copyright Copyright (c) 2026 ICARUS Project
 */

#ifndef BSP_ERROR_H
#define BSP_ERROR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* ============================================================================
 * ERROR CODES
 * ========================================================================= */

/**
 * @brief System error codes
 */
typedef enum {
    BSP_OK              = 0,    /**< No error */
    BSP_ERROR           = 1,    /**< Generic error */
    BSP_ERROR_TIMEOUT   = 2,    /**< Operation timed out */
    BSP_ERROR_BUSY      = 3,    /**< Resource busy */
    BSP_ERROR_PARAM     = 4,    /**< Invalid parameter */
    BSP_ERROR_MEMORY    = 5,    /**< Memory allocation failed */
    BSP_ERROR_HARDWARE  = 6     /**< Hardware failure */
} bsp_status_t;

/* ============================================================================
 * ERROR HANDLING API
 * ========================================================================= */

/**
 * @brief System error handler
 *
 * @details Called when an unrecoverable error occurs.
 *          Disables interrupts and enters infinite loop.
 *
 * @warning This function never returns!
 */
void Error_Handler(void);

/**
 * @brief Assert handler with location info
 *
 * @param[in] file  Source file name
 * @param[in] line  Line number
 *
 * @note Called by assert_param() macro when assertion fails
 */
void assert_failed(uint8_t *file, uint32_t line);

#ifdef __cplusplus
}
#endif

#endif /* BSP_ERROR_H */
