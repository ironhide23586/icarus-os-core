/*
 * mock_retarget_hal.h
 * Minimal mock for retarget_hal.h
 * This file is used to override the real retarget_hal.h when testing
 * The real retarget_hal.h includes many HAL headers, so we provide a minimal version
 */

#ifndef INC_BSP_RETARGET_HAL_H_
#define INC_BSP_RETARGET_HAL_H_

#include <stdint.h>
#include "mock_main.h"
#include "mock_hal.h"
#include "mock_gpio.h"

// Types are already defined in mock_hal.h, no need to redefine

// Function declarations
void hal_init(void);
void LED_On(void);
void LED_Off(void);
void LED_Blink(uint32_t Hdelay, uint32_t Ldelay);
int32_t platform_write(void* handle, uint8_t reg, const uint8_t *bufp, uint16_t len);
int32_t platform_read(void* handle, uint8_t reg, uint8_t *bufp, uint16_t len);

// Constants
#define IMU_ADDRESS 0x6B

// Forward declarations for MX init functions
void MX_GPIO_Init(void);
void MX_RTC_Init(void);
void MX_SPI4_Init(void);
void MX_TIM1_Init(void);
void MX_I2C2_Init(void);
void MX_USB_DEVICE_Init(void);

#endif /* INC_BSP_RETARGET_HAL_H_ */
