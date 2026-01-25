/*
 * mock_gpio.h
 * Minimal mock for GPIO
 */

#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>

// GPIO types (minimal)
typedef struct {
	uint32_t Instance;
} GPIO_TypeDef;

// GPIO pins
#define GPIO_PIN_0  0x0001U
#define GPIO_PIN_1  0x0002U
#define GPIO_PIN_2  0x0004U
#define GPIO_PIN_3  0x0008U
#define GPIO_PIN_4  0x0010U
#define GPIO_PIN_5  0x0020U
#define GPIO_PIN_6  0x0040U
#define GPIO_PIN_7  0x0080U
#define GPIO_PIN_8  0x0100U
#define GPIO_PIN_9  0x0200U
#define GPIO_PIN_10 0x0400U

// GPIO states
#define GPIO_PIN_RESET 0U
#define GPIO_PIN_SET   1U

// Functions
void HAL_GPIO_WritePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint32_t PinState);
void HAL_GPIO_TogglePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);

#endif /* GPIO_H */
