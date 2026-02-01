/*
 * mock_main.h
 * Minimal mock for main.h
 */

#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>
#include "mock_gpio.h"

// Include icarus config for ICARUS_TICKS_PER_TASK
#include "icarus/icarus_config.h"

// GPIO pin definitions
#define E3_Pin GPIO_PIN_3
extern GPIO_TypeDef GPIOE;
#define E3_GPIO_Port (&GPIOE)

// Error handler (mock)
void Error_Handler(void);

#endif /* MAIN_H */
