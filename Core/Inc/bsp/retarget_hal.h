/**
 * @file    retarget_hal.h
 * @brief   ICARUS OS Hardware Abstraction Layer Interface
 * @version 0.1.0
 *
 * @details Provides hardware initialization and low-level platform services
 *          for the STM32H750VBT6 target.
 *
 * @par Target Hardware:
 *      - MCU: STM32H750VBT6 (ARM Cortex-M7 @ 480MHz)
 *      - Board: WeAct Studio STM32H750VBT6 Core Board
 *      - LED: PE3 (active high)
 *      - IMU: LSM9DS1 on I2C2 (address 0x6B)
 *
 * @see     STM32H750 Reference Manual RM0433
 *
 * @author  Souham Biswas
 * @date    2025
 *
 * @copyright Copyright 2025 Souham Biswas
 *            https://github.com/ironhide23586/icarus-os-core
 *            Licensed under the Apache License, Version 2.0
 */

#ifndef INC_BSP_RETARGET_HAL_H_
#define INC_BSP_RETARGET_HAL_H_

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * INCLUDES
 * ========================================================================= */

// #include "main.h"
#include "bsp/i2c.h"
#include "bsp/rtc.h"
#include "bsp/spi.h"
#include "bsp/timer.h"
#include "usb_device.h"
#include "bsp/gpio.h"
#include "lcd.h"
#include "bsp/lsm9ds1_reg.h"

#include "bsp/mpu.h"

/* ============================================================================
 * CONFIGURATION
 * ========================================================================= */

/** @brief I2C address for LSM9DS1 IMU (accelerometer/gyroscope) */
#define IMU_ADDRESS 0x6B

/* ============================================================================
 * HAL INITIALIZATION API
 * ========================================================================= */

/**
 * @brief   Initialize all hardware abstraction layer components
 *
 * @details Configures MPU, caches, clocks, and all peripherals.
 *          Must be called before os_init().
 *
 * @pre     None (called at system startup)
 * @post    All hardware ready for kernel initialization
 */
void hal_init(void);

/* ============================================================================
 * LED CONTROL API
 * ========================================================================= */

/**
 * @brief   Blink LED with specified on/off timing
 *
 * @param[in] Hdelay  LED on duration in OS ticks
 * @param[in] Ldelay  LED off duration in OS ticks
 *
 * @note    Uses task_active_sleep() - must be called from task context
 */
void LED_Blink(uint32_t Hdelay, uint32_t Ldelay);

/**
 * @brief   Turn LED on (active high)
 */
void LED_On(void);

/**
 * @brief   Turn LED off
 */
void LED_Off(void);

#ifdef __cplusplus
}
#endif

#endif /* INC_BSP_RETARGET_HAL_H_ */
