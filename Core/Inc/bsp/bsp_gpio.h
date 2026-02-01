/**
 * @file    bsp_gpio.h
 * @brief   Board Support Package - GPIO Driver
 * @version 0.1.0
 *
 * @details GPIO initialization and control for all board pins.
 *
 * @see     STM32H750 Reference Manual RM0433, Section 11 (GPIO)
 *
 * @author  Souham Biswas
 * @date    2026
 *
 * @copyright Copyright (c) 2026 ICARUS Project
 */

#ifndef BSP_GPIO_H
#define BSP_GPIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bsp_config.h"

/**
 * @brief Initialize all GPIO pins
 *
 * @details Configures:
 *          - LED output (PE3)
 *          - User button input (PC13)
 *          - LCD control pins (PE11, PE13)
 *          - Oscillator pins (PC14, PC15, PH0, PH1)
 *          - Debug pins (PA13, PA14)
 */
void MX_GPIO_Init(void);

/**
 * @brief Read user button state
 *
 * @retval true   Button pressed
 * @retval false  Button released
 */
static inline bool bsp_button_read(void)
{
    return HAL_GPIO_ReadPin(BSP_KEY_PORT, BSP_KEY_PIN) == GPIO_PIN_SET;
}

#ifdef __cplusplus
}
#endif

#endif /* BSP_GPIO_H */
