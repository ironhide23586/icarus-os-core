/**
 * @file    bsp_rtc.h
 * @brief   Board Support Package - Real-Time Clock Driver
 * @version 0.1.0
 *
 * @details RTC driver for timekeeping and calendar functions.
 *
 * @par RTC Configuration:
 *      - Clock source: LSE (32.768kHz)
 *      - Hour format: 24-hour
 *      - Async prescaler: 127
 *      - Sync prescaler: 255
 *
 * @see     STM32H750 Reference Manual RM0433, Section 50 (RTC)
 *
 * @author  Souham Biswas
 * @date    2026
 *
 * @copyright Copyright (c) 2026 ICARUS Project
 */

#ifndef BSP_RTC_H
#define BSP_RTC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bsp_config.h"

/** @brief RTC handle */
extern RTC_HandleTypeDef hrtc;

/**
 * @brief Initialize RTC peripheral
 *
 * @details Configures RTC with:
 *          - LSE clock source
 *          - 24-hour format
 *          - Default time: 12:00:00
 *          - Default date: Monday, June 1, 2020
 */
void MX_RTC_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_RTC_H */
