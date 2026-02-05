/**
 * @file    bsp_config.h
 * @brief   Board Support Package - Hardware Configuration
 * @version 0.1.0
 *
 * @details Pin assignments, peripheral mappings, and hardware constants
 *          for the WeAct STM32H750VBT6 development board.
 *
 * @par Pin Mapping:
 *      | Function    | Pin   | Port  | Notes                    |
 *      |-------------|-------|-------|--------------------------|
 *      | LED         | PE3   | GPIOE | Active high              |
 *      | USER_KEY    | PC13  | GPIOC | Active low, pull-down    |
 *      | LCD_CS      | PE11  | GPIOE | SPI chip select          |
 *      | LCD_DC      | PE13  | GPIOE | Data/Command select      |
 *      | SPI4_SCK    | PE12  | GPIOE | LCD SPI clock            |
 *      | SPI4_MOSI   | PE14  | GPIOE | LCD SPI data             |
 *      | I2C2_SCL    | PB10  | GPIOB | IMU I2C clock            |
 *      | I2C2_SDA    | PB11  | GPIOB | IMU I2C data             |
 *
 * @see     WeAct STM32H750VBT6 Schematic
 * @see     STM32H750 Datasheet DS12556 for alternate function mapping
 *
 * @author  Souham Biswas
 * @date    2026
 *
 * @copyright Copyright (c) 2026 ICARUS Project
 */

#ifndef BSP_CONFIG_H
#define BSP_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef HOST_TEST
#include "stm32h7xx_hal.h"
#else
#include "mock_hal.h"  // Include mock HAL types during testing
#endif

/* ============================================================================
 * SYSTEM CONFIGURATION
 * ========================================================================= */

/**
 * @defgroup SYS_CONFIG System Configuration
 * @brief    Core system parameters
 * @{
 */

/** @brief System clock frequency in Hz (480 MHz) */
#define BSP_SYSCLK_FREQ         480000000UL

/** @brief AHB clock frequency in Hz (240 MHz) */
#define BSP_HCLK_FREQ           240000000UL

/** @brief APB1 clock frequency in Hz (120 MHz) */
#define BSP_APB1_FREQ           120000000UL

/** @brief APB2 clock frequency in Hz (120 MHz) */
#define BSP_APB2_FREQ           120000000UL

/** @brief SysTick frequency in Hz (1000 = 1ms tick) */
#define BSP_SYSTICK_FREQ        1000UL

/** @} */

/* ============================================================================
 * GPIO PIN DEFINITIONS
 * ========================================================================= */

/**
 * @defgroup GPIO_PINS GPIO Pin Definitions
 * @brief    Hardware pin assignments
 * @{
 */

/* LED (Active High) */
#define BSP_LED_PIN             GPIO_PIN_3
#ifndef HOST_TEST
#define BSP_LED_PORT            GPIOE
#else
#define BSP_LED_PORT            (&GPIOE)
#endif
#define BSP_LED_CLK_ENABLE()    __HAL_RCC_GPIOE_CLK_ENABLE()

/* User Button (Active Low with Pull-Down) */
#define BSP_KEY_PIN             GPIO_PIN_13
#define BSP_KEY_PORT            GPIOC
#define BSP_KEY_CLK_ENABLE()    __HAL_RCC_GPIOC_CLK_ENABLE()

/* LCD SPI Chip Select */
#define BSP_LCD_CS_PIN          GPIO_PIN_11
#define BSP_LCD_CS_PORT         GPIOE

/* LCD Data/Command Select */
#define BSP_LCD_DC_PIN          GPIO_PIN_13
#define BSP_LCD_DC_PORT         GPIOE

/** @} */

/* ============================================================================
 * PERIPHERAL CONFIGURATION
 * ========================================================================= */

/**
 * @defgroup PERIPH_CONFIG Peripheral Configuration
 * @brief    Peripheral instance assignments
 * @{
 */

/* SPI4 - LCD Interface */
#define BSP_LCD_SPI             SPI4
#define BSP_LCD_SPI_CLK_ENABLE() __HAL_RCC_SPI4_CLK_ENABLE()

/* I2C2 - IMU Interface */
#define BSP_IMU_I2C             I2C2
#define BSP_IMU_I2C_CLK_ENABLE() __HAL_RCC_I2C2_CLK_ENABLE()
#define BSP_IMU_I2C_ADDRESS     0x6B    /**< LSM9DS1 I2C address */

/* TIM1 - PWM Output */
#define BSP_PWM_TIM             TIM1
#define BSP_PWM_TIM_CLK_ENABLE() __HAL_RCC_TIM1_CLK_ENABLE()

/** @} */

/* ============================================================================
 * LEGACY COMPATIBILITY DEFINES
 * ========================================================================= */

/* Pin names used by STM32CubeMX generated code */
#define E3_Pin                  BSP_LED_PIN
#define E3_GPIO_Port            BSP_LED_PORT
#define KEY_Pin                 BSP_KEY_PIN
#define KEY_GPIO_Port           BSP_KEY_PORT
#define LCD_CS_Pin              BSP_LCD_CS_PIN
#define LCD_CS_GPIO_Port        BSP_LCD_CS_PORT
#define LCD_WR_RS_Pin           BSP_LCD_DC_PIN
#define LCD_WR_RS_GPIO_Port     BSP_LCD_DC_PORT

#ifdef __cplusplus
}
#endif

#endif /* BSP_CONFIG_H */
