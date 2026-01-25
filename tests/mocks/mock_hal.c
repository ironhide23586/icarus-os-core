/*
 * mock_hal.c
 * Mock HAL implementation for unit testing
 * Note: GPIO functions are in mock_gpio.c to avoid duplicates
 */

#include "mock_hal.h"
#include <stdint.h>

void HAL_Delay(uint32_t delay) {
	(void)delay;
	// Mock: do nothing (tests should run fast)
}

// Mock I2C functions
HAL_StatusTypeDef HAL_I2C_Mem_Write(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout) {
	(void)hi2c;
	(void)DevAddress;
	(void)MemAddress;
	(void)MemAddSize;
	(void)pData;
	(void)Size;
	(void)Timeout;
	return HAL_OK;
}

HAL_StatusTypeDef HAL_I2C_Mem_Read(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout) {
	(void)hi2c;
	(void)DevAddress;
	(void)MemAddress;
	(void)MemAddSize;
	(void)pData;
	(void)Size;
	(void)Timeout;
	return HAL_OK;
}

// Mock MPU functions
void HAL_MPU_Disable(void) {
	// Mock: do nothing
}

void HAL_MPU_ConfigRegion(MPU_Region_InitTypeDef *MPU_Init) {
	(void)MPU_Init;
	// Mock: do nothing
}

void HAL_MPU_Enable(uint32_t MPU_Control) {
	(void)MPU_Control;
	// Mock: do nothing
}

// Mock RCC/PWR functions
HAL_StatusTypeDef HAL_RCC_OscConfig(RCC_OscInitTypeDef *RCC_OscInitStruct) {
	(void)RCC_OscInitStruct;
	return HAL_OK;
}

HAL_StatusTypeDef HAL_RCC_ClockConfig(RCC_ClkInitTypeDef *RCC_ClkInitStruct, uint32_t FLatency) {
	(void)RCC_ClkInitStruct;
	(void)FLatency;
	return HAL_OK;
}

HAL_StatusTypeDef HAL_PWREx_ConfigSupply(uint32_t SupplySource) {
	(void)SupplySource;
	return HAL_OK;
}

void __HAL_PWR_VOLTAGESCALING_CONFIG(uint32_t VoltageScaling) {
	(void)VoltageScaling;
	// Mock: do nothing
}

uint32_t __HAL_PWR_GET_FLAG(uint32_t Flag) {
	(void)Flag;
	return 1; // Return ready flag set
}

void HAL_PWR_EnableBkUpAccess(void) {
	// Mock: do nothing
}

void __HAL_RCC_LSEDRIVE_CONFIG(uint32_t LSEDrive) {
	(void)LSEDrive;
	// Mock: do nothing
}

HAL_StatusTypeDef HAL_Init(void) {
	return HAL_OK;
}

// Mock SCB cache functions
void SCB_EnableICache(void) {
	// Mock: do nothing
}

void SCB_EnableDCache(void) {
	// Mock: do nothing
}

// Mock MX init functions
void MX_GPIO_Init(void) {
	// Mock: do nothing
}

void MX_RTC_Init(void) {
	// Mock: do nothing
}

void MX_SPI4_Init(void) {
	// Mock: do nothing
}

void MX_TIM1_Init(void) {
	// Mock: do nothing
}

void MX_I2C2_Init(void) {
	// Mock: do nothing
}

void MX_USB_DEVICE_Init(void) {
	// Mock: do nothing
}

// Mock HAL tick function
void HAL_IncTick(void) {
	// Mock: do nothing (tick counting is handled by SysTick_Handler)
}

// Mock USB PCD IRQ handler
void HAL_PCD_IRQHandler(PCD_HandleTypeDef *hpcd) {
	(void)hpcd;
	// Mock: do nothing
}

// Mock USB handle instance (for extern in stm32h7xx_it.c)
PCD_HandleTypeDef hpcd_USB_OTG_FS = {0};
