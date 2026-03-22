/**
 * @file    interactive_tasks.h
 * @brief   Interactive Input Demo Tasks
 * @version 0.1.0
 *
 * @details Simple demo: LED glows when K1 button is pressed.
 *          
 * @par Hardware:
 *      - Button: K1 (PC13, active low)
 *      - LED: PE3 (active high)
 *      - Board: WeAct STM32H750VBT6
 *
 * @par Demo:
 *      - Press K1 button → LED turns ON
 *      - Release K1 button → LED turns OFF
 *      - Button press count printed to USB serial
 *
 * @author  Souham Biswas
 * @date    2026
 *
 * @copyright Copyright (c) 2026 ICARUS Project
 */

#ifndef INTERACTIVE_TASKS_H
#define INTERACTIVE_TASKS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/* ============================================================================
 * CONFIGURATION
 * ========================================================================= */

/**
 * @defgroup INTERACTIVE_CONFIG Interactive Demo Configuration
 * @brief    Demo parameters and constants
 * @{
 */

/** @brief Button debounce time in milliseconds */
#define INTERACTIVE_BUTTON_DEBOUNCE_MS  50

/** @} */

/* ============================================================================
 * PUBLIC API
 * ========================================================================= */

/**
 * @brief   Initialize and register interactive demo tasks
 *
 * @details Registers button monitoring task that controls LED.
 *
 * @note    Call this after os_init() and before os_start()
 * @note    Replaces demo_tasks_init() and stress_test_init()
 *
 * @see     os_register_task()
 */
void interactive_tasks_init(void);

#ifdef __cplusplus
}
#endif

#endif /* INTERACTIVE_TASKS_H */
