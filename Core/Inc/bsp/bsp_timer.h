/**
 * @file    bsp_timer.h
 * @brief   Board Support Package - Timer/PWM Driver
 * @version 0.1.0
 *
 * @details TIM1 driver for PWM output (LCD backlight).
 *
 * @par TIM1 Configuration:
 *      - Mode: PWM
 *      - Channel: CH2N (PE10)
 *      - Frequency: 10kHz
 *      - Resolution: 1000 steps (0-999)
 *
 * @see     STM32H750 Reference Manual RM0433, Section 43 (TIM1)
 *
 * @author  Souham Biswas
 * @date    2026
 *
 * @copyright Copyright (c) 2026 ICARUS Project
 */

#ifndef BSP_TIMER_H
#define BSP_TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bsp_config.h"

/** @brief TIM1 handle (PWM output) */
extern TIM_HandleTypeDef htim1;

/**
 * @brief Initialize TIM1 for PWM output
 *
 * @details Configures TIM1 Channel 2N for PWM:
 *          - 10kHz PWM frequency
 *          - 0-999 duty cycle range
 */
void MX_TIM1_Init(void);

/**
 * @brief Post-initialization for TIM1 GPIO
 *
 * @param[in] htim  Timer handle
 */
void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/**
 * @brief Set PWM duty cycle
 *
 * @param[in] duty  Duty cycle (0-999)
 */
static inline void bsp_pwm_set_duty(uint16_t duty)
{
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, duty);
}

#ifdef __cplusplus
}
#endif

#endif /* BSP_TIMER_H */
