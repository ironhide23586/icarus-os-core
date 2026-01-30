/**
 * @file    bsp_spi.h
 * @brief   Board Support Package - SPI Driver
 * @version 0.1.0
 *
 * @details SPI4 driver for LCD display interface.
 *
 * @par SPI4 Configuration:
 *      - Mode: Master, transmit only
 *      - Data size: 8-bit
 *      - Clock polarity: Low
 *      - Clock phase: 1st edge
 *      - Baud rate: ~15MHz (APB2/8)
 *
 * @see     STM32H750 Reference Manual RM0433, Section 51 (SPI)
 *
 * @author  Souham Biswas
 * @date    2026
 *
 * @copyright Copyright (c) 2026 ICARUS Project
 */

#ifndef BSP_SPI_H
#define BSP_SPI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bsp_config.h"

/** @brief SPI4 handle (LCD interface) */
extern SPI_HandleTypeDef hspi4;

/**
 * @brief Initialize SPI4 peripheral
 *
 * @details Configures SPI4 for LCD communication:
 *          - Master mode, transmit only
 *          - 8-bit data, MSB first
 *          - Software NSS management
 */
void MX_SPI4_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_SPI_H */
