/*
 * mock_stm32h7xx_it.h
 * Mock header for stm32h7xx_it.h
 */

#ifndef STM32H7XX_IT_H
#define STM32H7XX_IT_H

#include <stdint.h>
#include "mock_hal.h"
#include "cmsis_gcc.h"

// Forward declarations for interrupt handlers
void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void SVC_Handler(void);
void DebugMon_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);
void OTG_FS_IRQHandler(void);

// External USB handle (will be provided by mock)
extern PCD_HandleTypeDef hpcd_USB_OTG_FS;

#endif /* STM32H7XX_IT_H */
