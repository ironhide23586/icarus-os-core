/**
 * @file    bsp_i2c.h
 * @brief   Board Support Package - I2C Driver
 * @version 0.1.0
 *
 * @details I2C2 driver for IMU (LSM9DS1) interface.
 *
 * @par I2C2 Configuration:
 *      - Speed: 400kHz (Fast mode)
 *      - Address mode: 7-bit
 *      - Pins: PB10 (SCL), PB11 (SDA)
 *
 * @see     STM32H750 Reference Manual RM0433, Section 52 (I2C)
 * @see     LSM9DS1 Datasheet for register map
 *
 * @author  Souham Biswas
 * @date    2026
 *
 * @copyright Copyright (c) 2026 ICARUS Project
 */

#ifndef BSP_I2C_H
#define BSP_I2C_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bsp_config.h"

/** @brief I2C2 handle (IMU interface) */
extern I2C_HandleTypeDef hi2c2;

/**
 * @brief Initialize I2C2 peripheral
 *
 * @details Configures I2C2 for IMU communication:
 *          - 400kHz Fast mode
 *          - 7-bit addressing
 *          - Analog filter enabled
 */
void MX_I2C2_Init(void);

/**
 * @brief Platform write function for IMU driver
 *
 * @param[in] handle  I2C handle pointer
 * @param[in] reg     Register address
 * @param[in] bufp    Data buffer to write
 * @param[in] len     Number of bytes
 *
 * @return 0 on success
 */
int32_t platform_write(void* handle, uint8_t reg,
                       const uint8_t *bufp, uint16_t len);

/**
 * @brief Platform read function for IMU driver
 *
 * @param[in]  handle  I2C handle pointer
 * @param[in]  reg     Register address
 * @param[out] bufp    Buffer to receive data
 * @param[in]  len     Number of bytes
 *
 * @return 0 on success
 */
int32_t platform_read(void* handle, uint8_t reg,
                      uint8_t *bufp, uint16_t len);

/**
 * @brief Platform delay function
 *
 * @param[in] ms  Milliseconds to delay
 */
void platform_delay(uint32_t ms);

#ifdef __cplusplus
}
#endif

#endif /* BSP_I2C_H */
