/*
 * stm32h7xx_hal.h - Mock for host testing
 * Provides minimal HAL definitions needed for kernel testing
 */

#ifndef STM32H7XX_HAL_H
#define STM32H7XX_HAL_H

#include <stdint.h>
#include "cmsis_gcc.h"

/* HAL tick functions - mocked */
uint32_t HAL_GetTick(void);
void HAL_IncTick(void);

#endif /* STM32H7XX_HAL_H */
