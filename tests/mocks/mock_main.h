/*
 * mock_main.h
 * Minimal mock for main.h
 */

#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>
#include "mock_gpio.h"

// Minimal definitions from main.h needed for testing
#define TICKS_PER_TASK 50

// GPIO pin definitions
#define E3_Pin GPIO_PIN_3
extern GPIO_TypeDef GPIOE;
#define E3_GPIO_Port (&GPIOE)

// Error handler (mock)
void Error_Handler(void);

#endif /* MAIN_H */
