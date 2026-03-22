/*
 * Mock bsp/config.h for testing
 * Provides minimal BSP configuration definitions
 */

#ifndef BSP_CONFIG_H
#define BSP_CONFIG_H

#include "mock_gpio.h"

/* LED Configuration */
#define BSP_LED_PIN             GPIO_PIN_3
#define BSP_LED_PORT            (&GPIOE)

/* Legacy compatibility defines */
#define E3_Pin                  BSP_LED_PIN
#define E3_GPIO_Port            BSP_LED_PORT

/* Other BSP defines that might be needed */
#define BSP_HCLK_FREQ           480000000UL

#endif /* BSP_CONFIG_H */
