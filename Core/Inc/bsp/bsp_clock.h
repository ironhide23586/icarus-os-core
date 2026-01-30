/**
 * @file    bsp_clock.h
 * @brief   Board Support Package - Clock Configuration
 * @version 0.1.0
 *
 * @details System clock, PLL, and power management configuration.
 *
 * @par Clock Tree (STM32H750 @ 480MHz):
 *      HSE (25MHz) -> PLL1 -> SYSCLK (480MHz)
 *                          -> HCLK (240MHz)
 *                          -> APB1 (120MHz)
 *                          -> APB2 (120MHz)
 *
 * @see     STM32H750 Reference Manual RM0433, Section 8 (RCC)
 *
 * @author  Souham Biswas
 * @date    2026
 *
 * @copyright Copyright (c) 2026 ICARUS Project
 */

#ifndef BSP_CLOCK_H
#define BSP_CLOCK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bsp_config.h"

/**
 * @brief Configure system clocks
 *
 * @details Configures:
 *          - HSE oscillator (25MHz external crystal)
 *          - PLL1 for 480MHz SYSCLK
 *          - AHB/APB prescalers
 *          - Flash latency
 *
 * @note Called by bsp_init()
 */
void bsp_clock_init(void);

/**
 * @brief Configure MPU regions
 *
 * @details Sets up memory protection for:
 *          - QSPI flash region (no access by default)
 *          - QSPI flash region (read-only, cacheable)
 *
 * @note Called before cache enable
 */
void bsp_mpu_config(void);

/**
 * @brief Enable CPU caches
 *
 * @details Enables:
 *          - Instruction cache (I-Cache)
 *          - Data cache (D-Cache)
 *
 * @note Called after MPU configuration
 */
void bsp_cache_enable(void);

/**
 * @brief Enter Run0 mode exit sequence
 *
 * @details Required power supply configuration for high-speed operation.
 *
 * @note Called at very start of initialization
 */
void ExitRun0Mode(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_CLOCK_H */
