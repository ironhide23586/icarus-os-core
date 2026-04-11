/**
 * @file    bsp_button.h
 * @brief   Board Support Package — User button (K1)
 *
 * @details Thin wrapper around the K1 user button on the WeAct
 *          STM32H750VBT6 board (PC13, active low). Hides the
 *          `HAL_GPIO_ReadPin(BSP_KEY_PORT, BSP_KEY_PIN) ==
 *          GPIO_PIN_RESET` idiom behind a clean BSP API so callers
 *          do not have to know the polarity or the HAL pin enums.
 *
 *          The pin assignment lives in `bsp/config.h`
 *          (`BSP_KEY_PORT` / `BSP_KEY_PIN`); this header does not
 *          duplicate it.
 *
 * @par Hardware:
 *      - K1 button on PC13, active low
 *      - Internal pull-up enabled by `MX_GPIO_Init()`
 *      - 50 ms debounce is the responsibility of the caller — this
 *        BSP only reports the raw pin state
 *
 * @author  Souham Biswas
 * @date    2026
 *
 * @copyright Copyright 2025-2026 Souham Biswas
 *            https://github.com/ironhide23586/icarus-os-core
 *            Licensed under the Apache License, Version 2.0
 */

#ifndef BSP_BUTTON_H
#define BSP_BUTTON_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

/**
 * @brief  Read the K1 user button state.
 *
 * @return `true` if the button is currently held down (pin sampled
 *         low after the active-low conversion), `false` otherwise.
 *
 * @note   Pure read — no debounce, no edge detection. Callers that
 *         care about transitions should sample twice with a short
 *         delay between reads or maintain their own previous-state
 *         flag.
 *
 * @note   Safe to call from any task context. Does not enter a
 *         critical section.
 */
bool Button_IsPressed(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_BUTTON_H */
