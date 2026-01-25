/*
 * mock_hal.c
 * Mock HAL implementation for unit testing
 * Note: GPIO functions are in mock_gpio.c to avoid duplicates
 */

#include "mock_hal.h"

void HAL_Delay(uint32_t delay) {
	(void)delay;
	// Mock: do nothing (tests should run fast)
}
