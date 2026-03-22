/**
 * @file    game_tasks.h
 * @brief   LCD Game Demo Tasks
 * @version 0.1.0
 *
 * @details Interactive game demo running on ST7735 LCD with button input.
 *          Demonstrates real-time graphics rendering and user interaction
 *          using ICARUS OS primitives.
 *
 * @par Hardware:
 *      - Display: ST7735 128x160 LCD (SPI4)
 *      - Input: K1 button (PC13, active low)
 *      - Board: WeAct STM32H750VBT6
 *
 * @par Game Architecture:
 *      - Render task: Updates LCD at fixed frame rate
 *      - Input task: Polls button and updates game state
 *      - Game logic: Simple interactive demo (TBD)
 *
 * @author  Souham Biswas
 * @date    2026
 *
 * @copyright Copyright (c) 2026 ICARUS Project
 */

#ifndef GAME_TASKS_H
#define GAME_TASKS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/* ============================================================================
 * CONFIGURATION
 * ========================================================================= */

/**
 * @defgroup GAME_CONFIG Game Configuration
 * @brief    Game parameters and constants
 * @{
 */

/** @brief Target frame rate in Hz */
#define GAME_TARGET_FPS         30

/** @brief Frame period in milliseconds */
#define GAME_FRAME_PERIOD_MS    (1000 / GAME_TARGET_FPS)

/** @brief Button debounce time in milliseconds */
#define GAME_BUTTON_DEBOUNCE_MS 50

/** @} */

/* ============================================================================
 * PUBLIC API
 * ========================================================================= */

/**
 * @brief   Initialize and register game demo tasks
 *
 * @details Registers the following tasks with ICARUS kernel:
 *          - game_render_task: LCD rendering at 30 FPS
 *          - game_input_task: Button polling and debouncing
 *
 * @note    Call this after os_init() and before os_start()
 * @note    Replaces demo_tasks_init() and stress_test_init()
 *
 * @see     os_register_task()
 */
void game_tasks_init(void);

#ifdef __cplusplus
}
#endif

#endif /* GAME_TASKS_H */
