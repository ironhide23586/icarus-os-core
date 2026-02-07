/*
 * mock_hal.h
 * Mock for HAL dependencies
 */

#ifndef MOCK_HAL_H
#define MOCK_HAL_H

#include <stdint.h>
#include <stdbool.h>

// Include GPIO mocks
#include "mock_gpio.h"

// Mock HAL functions and types
typedef struct {
	uint32_t Instance;
} TIM_HandleTypeDef;

typedef struct {
	uint32_t Instance;
} I2C_HandleTypeDef;

typedef struct {
	uint32_t Instance;
} RTC_HandleTypeDef;

typedef struct {
	uint32_t Instance;
} SPI_HandleTypeDef;

// Mock HAL status
typedef enum {
	HAL_OK = 0,
	HAL_ERROR = 1,
	HAL_BUSY = 2
} HAL_StatusTypeDef;

#define HAL_MAX_DELAY 0xFFFFFFFFU

// Mock delay
// Note: GPIO functions are in mock_gpio.h to avoid duplicates
void HAL_Delay(uint32_t delay);

// Mock I2C functions
HAL_StatusTypeDef HAL_I2C_Mem_Write(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout);
HAL_StatusTypeDef HAL_I2C_Mem_Read(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout);

// Mock MPU functions
typedef struct {
	uint32_t Enable;
	uint32_t Number;
	uint32_t BaseAddress;
	uint32_t Size;
	uint32_t AccessPermission;
	uint32_t IsBufferable;
	uint32_t IsCacheable;
	uint32_t IsShareable;
	uint32_t DisableExec;
	uint32_t TypeExtField;
	uint32_t SubRegionDisable;
} MPU_Region_InitTypeDef;

#define MPU_REGION_ENABLE 1U
#define MPU_REGION_NUMBER0 0U
#define MPU_REGION_NUMBER1 1U
#define MPU_REGION_SIZE_256MB 0x1DU
#define MPU_REGION_SIZE_8MB 0x1AU
#define MPU_REGION_NO_ACCESS 0x00U
#define MPU_REGION_PRIV_RO 0x05U
#define MPU_ACCESS_NOT_BUFFERABLE 0U
#define MPU_ACCESS_BUFFERABLE 1U
#define MPU_ACCESS_NOT_CACHEABLE 0U
#define MPU_ACCESS_CACHEABLE 1U
#define MPU_ACCESS_NOT_SHAREABLE 0U
#define MPU_INSTRUCTION_ACCESS_DISABLE 1U
#define MPU_INSTRUCTION_ACCESS_ENABLE 0U
#define MPU_TEX_LEVEL1 1U
#define MPU_PRIVILEGED_DEFAULT 0x05U

void HAL_MPU_Disable(void);
void HAL_MPU_ConfigRegion(MPU_Region_InitTypeDef *MPU_Init);
void HAL_MPU_Enable(uint32_t MPU_Control);

// Mock MPU configuration function
void __mpu_config(void);

// Mock RCC/PWR functions
typedef struct {
	uint32_t OscillatorType;
	uint32_t HSEState;
	uint32_t LSEState;
	uint32_t HSI48State;
	struct {
		uint32_t PLLState;
		uint32_t PLLSource;
		uint32_t PLLM;
		uint32_t PLLN;
		uint32_t PLLP;
		uint32_t PLLQ;
		uint32_t PLLR;
		uint32_t PLLRGE;
		uint32_t PLLVCOSEL;
		uint32_t PLLFRACN;
	} PLL;
} RCC_OscInitTypeDef;

typedef struct {
	uint32_t ClockType;
	uint32_t SYSCLKSource;
	uint32_t SYSCLKDivider;
	uint32_t AHBCLKDivider;
	uint32_t APB3CLKDivider;
	uint32_t APB1CLKDivider;
	uint32_t APB2CLKDivider;
	uint32_t APB4CLKDivider;
} RCC_ClkInitTypeDef;

#define RCC_OSCILLATORTYPE_HSI48 0
#define RCC_OSCILLATORTYPE_HSE 0
#define RCC_OSCILLATORTYPE_LSE 0
#define RCC_HSE_ON 0
#define RCC_LSE_ON 0
#define RCC_HSI48_ON 0
#define RCC_PLL_ON 0
#define RCC_PLLSOURCE_HSE 0
#define RCC_PLL1VCIRANGE_2 0
#define RCC_PLL1VCOWIDE 0
#define RCC_CLOCKTYPE_HCLK 0
#define RCC_CLOCKTYPE_SYSCLK 0
#define RCC_CLOCKTYPE_PCLK1 0
#define RCC_CLOCKTYPE_PCLK2 0
#define RCC_CLOCKTYPE_D3PCLK1 0
#define RCC_CLOCKTYPE_D1PCLK1 0
#define RCC_SYSCLKSOURCE_PLLCLK 0
#define RCC_SYSCLK_DIV1 0
#define RCC_HCLK_DIV2 0
#define RCC_APB3_DIV1 0
#define RCC_APB1_DIV1 0
#define RCC_APB2_DIV1 0
#define RCC_APB4_DIV1 0
#define FLASH_LATENCY_1 0
#define PWR_LDO_SUPPLY 0
#define PWR_REGULATOR_VOLTAGE_SCALE0 0
#define PWR_FLAG_VOSRDY 0
#define RCC_LSEDRIVE_MEDIUMHIGH 0

HAL_StatusTypeDef HAL_RCC_OscConfig(RCC_OscInitTypeDef *RCC_OscInitStruct);
HAL_StatusTypeDef HAL_RCC_ClockConfig(RCC_ClkInitTypeDef *RCC_ClkInitStruct, uint32_t FLatency);
HAL_StatusTypeDef HAL_PWREx_ConfigSupply(uint32_t SupplySource);
void __HAL_PWR_VOLTAGESCALING_CONFIG(uint32_t VoltageScaling);
uint32_t __HAL_PWR_GET_FLAG(uint32_t Flag);
void HAL_PWR_EnableBkUpAccess(void);
void __HAL_RCC_LSEDRIVE_CONFIG(uint32_t LSEDrive);
HAL_StatusTypeDef HAL_Init(void);

// Mock I2C constants
#define I2C_MEMADD_SIZE_8BIT 0

// Mock TIM constants and functions
#define TIM_CHANNEL_2 0x04U
#define __HAL_TIM_SET_COMPARE(htim, channel, compare) do { (void)(htim); (void)(channel); (void)(compare); } while(0)

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

// Mock SCB cache functions
void SCB_EnableICache(void);
void SCB_EnableDCache(void);

// Mock QSPI base (for VTOR)
#define QSPI_BASE 0x90000000UL

// Mock MX init functions (declared in headers, implemented in mocks)
void MX_GPIO_Init(void);
void MX_RTC_Init(void);
void MX_SPI4_Init(void);
void MX_TIM1_Init(void);
void MX_I2C2_Init(void);
void MX_USB_DEVICE_Init(void);

// Mock HAL tick function
void HAL_IncTick(void);

// Mock USB PCD types and functions
typedef struct {
	uint32_t Instance;
} PCD_HandleTypeDef;

void HAL_PCD_IRQHandler(PCD_HandleTypeDef *hpcd);

#endif // MOCK_HAL_H
