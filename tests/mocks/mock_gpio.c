/*
 * mock_gpio.c
 * Mock GPIO implementation
 */

#include "mock_gpio.h"

// Mock GPIOE port instance
GPIO_TypeDef GPIOE = {0};

void HAL_GPIO_WritePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState) {
	(void)GPIOx;
	(void)GPIO_Pin;
	(void)PinState;
}

void HAL_GPIO_TogglePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin) {
	(void)GPIOx;
	(void)GPIO_Pin;
}
