/*
 * stm32h7xx_hal_timebase.c
 *
 *  Created on: Jan 21, 2026
 *      Author: Souham Biswas
 *      GitHub: https://github.com/ironhide23586/icarus-os-core
 */



#include "stm32h7xx_hal.h"



HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority) {
	HAL_SYSTICK_Config(SystemCoreClock / 1000);
	HAL_NVIC_SetPriority(SysTick_IRQn, TickPriority, 0);
	return HAL_OK;
}


