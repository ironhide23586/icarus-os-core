/*
 * retarget_hal.c
 *
 *  Created on: Jan 23, 2026
 *      Author: Souham Biswas
 *      GitHub: https://github.com/ironhide23586/icarus-os-core
 */


#include "retarget_hal.h"


static void SystemClock_Config(void);
extern uint32_t task_active_sleep(uint32_t ticks);

/* Function prototypes */
int32_t platform_write(void* handle, uint8_t reg, const uint8_t *bufp, uint16_t len);
int32_t platform_read(void* handle, uint8_t reg, uint8_t *bufp, uint16_t len);
void platform_delay(uint32_t ms);

static void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

	/* Configure the MPU attributes for the QSPI 256MB without instruction access */
  MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
  MPU_InitStruct.Number           = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress      = QSPI_BASE;
  MPU_InitStruct.Size             = MPU_REGION_SIZE_256MB;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
  MPU_InitStruct.IsCacheable      = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL1;
  MPU_InitStruct.SubRegionDisable = 0x00;
  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /* Configure the MPU attributes for the QSPI 8MB (QSPI Flash Size) to Cacheable WT */
  MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
  MPU_InitStruct.Number           = MPU_REGION_NUMBER1;
  MPU_InitStruct.BaseAddress      = QSPI_BASE;
  MPU_InitStruct.Size             = MPU_REGION_SIZE_8MB;
  MPU_InitStruct.AccessPermission = MPU_REGION_PRIV_RO;
  MPU_InitStruct.IsBufferable     = MPU_ACCESS_BUFFERABLE;
  MPU_InitStruct.IsCacheable      = MPU_ACCESS_CACHEABLE;
  MPU_InitStruct.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_ENABLE;
  MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL1;
  MPU_InitStruct.SubRegionDisable = 0x00;
  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

static void CPU_CACHE_Enable(void)
{
  /* Enable I-Cache */
  SCB_EnableICache();

  /* Enable D-Cache */
  SCB_EnableDCache();
}

void LED_Blink(uint32_t Hdelay, uint32_t Ldelay) {
	HAL_GPIO_WritePin(E3_GPIO_Port,E3_Pin,GPIO_PIN_SET);
	task_active_sleep(Hdelay - 1);
	HAL_GPIO_WritePin(E3_GPIO_Port,E3_Pin,GPIO_PIN_RESET);
	task_active_sleep(Ldelay-1);
}


void LED_On(void) {
	HAL_GPIO_WritePin(E3_GPIO_Port,E3_Pin,GPIO_PIN_SET);
}

void LED_Off(void) {
	HAL_GPIO_WritePin(E3_GPIO_Port,E3_Pin,GPIO_PIN_RESET);
}

int32_t platform_write(void* handle, uint8_t reg, const uint8_t *bufp, uint16_t len) {
	// HAL_I2C_Mem_Write requires non-const pointer, but we receive const
	// This is safe as HAL will only read from the buffer
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
	HAL_I2C_Mem_Write((I2C_HandleTypeDef*) handle, IMU_ADDRESS, reg, I2C_MEMADD_SIZE_8BIT,
			(uint8_t *)bufp, len, HAL_MAX_DELAY);
#pragma GCC diagnostic pop
	return 0;
}


int32_t platform_read(void* handle, uint8_t reg, uint8_t *bufp, uint16_t len) {
	HAL_I2C_Mem_Read((I2C_HandleTypeDef*) handle, IMU_ADDRESS, reg, I2C_MEMADD_SIZE_8BIT,
			bufp, len, HAL_MAX_DELAY);
	return 0;
}


void platform_delay(uint32_t ms) {
    HAL_Delay(ms);
}



void hal_init() {
  /* USER CODE BEGIN 1 */
  #ifdef W25Qxx
    SCB->VTOR = QSPI_BASE;
  #endif
  MPU_Config();
  CPU_CACHE_Enable();

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();

  // Early LED blink to confirm MCU is alive
  // LED_Blink(200, 200);
  // LED_Blink(200, 200);
  // LED_Blink(200, 200);

  MX_RTC_Init();
  MX_SPI4_Init();
  MX_TIM1_Init();
  MX_I2C2_Init();
  MX_USB_DEVICE_Init();

//  HAL_Delay(1000); // Give USB time to initialize

//  printf("TEST\n");
//  printf("USB initialized, starting debug output\n");
//
//  extern volatile uint8_t os_running;
//  printf("OS Running: %d\r\n", os_running);
//  printf("Starting ICARUS OS\r\n");
}


void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_MEDIUMHIGH);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48|RCC_OSCILLATORTYPE_HSE
                              |RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 5;
  RCC_OscInitStruct.PLL.PLLN = 96;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV1;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}
