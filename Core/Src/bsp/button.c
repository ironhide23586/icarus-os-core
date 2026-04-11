/**
 * @file    button.c
 * @brief   User button (K1) implementation
 *
 * @details Reads the K1 GPIO via `HAL_GPIO_ReadPin` and inverts the
 *          active-low electrical convention so callers see `true`
 *          when the button is held down.
 *
 *          Under HOST_TEST the function is backed by an in-memory
 *          state variable that the unit tests can drive via
 *          `__button_host_set_pressed()`.
 *
 * @author  Souham Biswas
 * @date    2026
 *
 * @copyright Copyright 2025-2026 Souham Biswas
 *            https://github.com/ironhide23586/icarus-os-core
 *            Licensed under the Apache License, Version 2.0
 */

#include "bsp/button.h"

#ifndef HOST_TEST

#include "bsp/config.h"
#include "stm32h7xx_hal.h"

bool Button_IsPressed(void) {
    return HAL_GPIO_ReadPin(BSP_KEY_PORT, BSP_KEY_PIN) == GPIO_PIN_RESET;
}

#else /* HOST_TEST — software model */

static bool g_button_pressed;

bool Button_IsPressed(void) {
    return g_button_pressed;
}

/* ---- Test-only host hooks ---------------------------------------------- */

void __button_host_set_pressed(bool pressed) {
    g_button_pressed = pressed;
}

void __button_host_reset_state(void) {
    g_button_pressed = false;
}

#endif /* HOST_TEST */
