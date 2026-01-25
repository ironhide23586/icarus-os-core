/*
 * mock_hal.h
 * Mock for HAL dependencies
 */

#ifndef MOCK_HAL_H
#define MOCK_HAL_H

#include <stdint.h>
#include <stdbool.h>

// Mock HAL functions and types
typedef struct {
	uint32_t Instance;
} TIM_HandleTypeDef;

typedef struct {
	uint32_t Instance;
} I2C_HandleTypeDef;

typedef struct {
	uint32_t Instance;
} SPI_HandleTypeDef;

typedef struct {
	uint32_t Instance;
} RTC_HandleTypeDef;

// Mock HAL status
typedef enum {
	HAL_OK = 0,
	HAL_ERROR = 1,
	HAL_BUSY = 2
} HAL_StatusTypeDef;

// Mock delay
// Note: GPIO functions are in mock_gpio.h to avoid duplicates
void HAL_Delay(uint32_t delay);

#endif // MOCK_HAL_H
