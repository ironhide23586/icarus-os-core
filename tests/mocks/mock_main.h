/*
 * mock_main.h
 * Minimal mock for main.h
 */

#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>
#include "mock_gpio.h"

// Include icarus config for ICARUS_TICKS_PER_TASK
// This also includes bsp/config.h which defines E3_Pin and E3_GPIO_Port
#include "icarus/config.h"

// GPIO definitions are now provided by bsp/config.h
// No need to redefine them here

extern GPIO_TypeDef GPIOE;

// Error handler (mock)
void Error_Handler(void);

#endif /* MAIN_H */
