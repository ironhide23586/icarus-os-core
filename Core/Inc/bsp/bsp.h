/**
 * @file    bsp.h
 * @brief   Board Support Package - Master Include Header
 * @version 0.1.0
 *
 * @details Single include point for all BSP functionality.
 *          Include this header to access all board-level drivers.
 *
 * @par Target Hardware:
 *      - MCU: STM32H750VBT6 (ARM Cortex-M7 @ 480MHz)
 *      - Board: WeAct Studio STM32H750VBT6 Core Board
 *
 * @see     STM32H750 Reference Manual RM0433
 * @see     STM32H750 Datasheet DS12556
 *
 * @author  Souham Biswas
 * @date    2025
 *
 * @copyright Copyright 2025-2026 Souham Biswas
 *            https://github.com/ironhide23586/icarus-os-core
 *            Licensed under the Apache License, Version 2.0
 */

#ifndef BSP_H
#define BSP_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * BSP MODULE INCLUDES
 * ========================================================================= */

#include "bsp/config.h"     /* Board configuration and pin definitions */
#include "bsp/clock.h"      /* Clock and power management */
#include "bsp/gpio.h"       /* GPIO driver */
#include "bsp/uart.h"       /* UART/USART driver (if used) */
#include "bsp/spi.h"        /* SPI driver */
#include "bsp/i2c.h"        /* I2C driver */
#include "bsp/timer.h"      /* Timer/PWM driver */
#include "bsp/rtc.h"        /* Real-Time Clock driver */
#include "bsp/usb.h"        /* USB CDC driver */
#include "bsp/display.h"    /* Terminal display driver */
#include "bsp/led.h"        /* LED control */
#include "bsp/error.h"      /* Error handling */

/* ============================================================================
 * BSP INITIALIZATION
 * ========================================================================= */

/**
 * @brief Initialize all board support package components
 *
 * @details Initializes in order:
 *          1. MPU configuration
 *          2. CPU caches (I-Cache, D-Cache)
 *          3. HAL layer
 *          4. System clocks (480MHz core)
 *          5. GPIO pins
 *          6. Peripheral drivers (SPI, I2C, Timer, RTC)
 *          7. USB CDC interface
 *
 * @pre     None (called before os_init)
 * @post    All hardware ready for kernel startup
 *
 * @note    Must be called before os_init()
 */
void bsp_init(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_H */
