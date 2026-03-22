/**
 * @file    bsp_led.h
 * @brief   Board Support Package - LED Driver
 * @version 0.1.0
 *
 * @details Simple LED control interface for the onboard LED.
 *
 * @par Hardware:
 *      - LED connected to PE3 (active high)
 *
 * @author  Souham Biswas
 * @date    2026
 *
 * @copyright Copyright (c) 2026 ICARUS Project
 */

#ifndef BSP_LED_H
#define BSP_LED_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* ============================================================================
 * LED CONTROL API
 * ========================================================================= */

/**
 * @brief Turn LED on
 */
void LED_On(void);

/**
 * @brief Turn LED off
 */
void LED_Off(void);

/**
 * @brief Toggle LED state
 */
void LED_Toggle(void);

/**
 * @brief Blink LED with specified timing
 *
 * @param[in] on_ticks   Duration LED is on (in OS ticks)
 * @param[in] off_ticks  Duration LED is off (in OS ticks)
 *
 * @note Uses task_active_sleep() - must be called from task context
 */
void LED_Blink(uint32_t on_ticks, uint32_t off_ticks);

#ifdef __cplusplus
}
#endif

#endif /* BSP_LED_H */
