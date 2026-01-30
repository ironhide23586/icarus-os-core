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
 * @date    2026
 *
 * @copyright Copyright (c) 2026 ICARUS Project
 *            https://github.com/ironhide23586/icarus-os-core
 *            Licensed under MIT License
 */

#ifndef BSP_H
#define BSP_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * BSP MODULE INCLUDES
 * ========================================================================= */

#include "bsp/bsp_config.h"     /* Board configuration and pin definitions */
#include "bsp/bsp_clock.h"      /* Clock and power management */
#include "bsp/bsp_gpio.h"       /* GPIO driver */
#include "bsp/bsp_uart.h"       /* UART/USART driver (if used) */
#include "bsp/bsp_spi.h"        /* SPI driver */
#include "bsp/bsp_i2c.h"        /* I2C driver */
#include "bsp/bsp_timer.h"      /* Timer/PWM driver */
#include "bsp/bsp_rtc.h"        /* Real-Time Clock driver */
#include "bsp/bsp_usb.h"        /* USB CDC driver */
#include "bsp/bsp_display.h"    /* Terminal display driver */
#include "bsp/bsp_led.h"        /* LED control */
#include "bsp/bsp_error.h"      /* Error handling */

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
