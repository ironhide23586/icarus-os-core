/**
 * @file    retarget_hal.c
 * @brief   ICARUS OS Hardware Abstraction Layer Implementation
 * @version 0.1.0
 *
 * @details Provides hardware initialization and low-level platform services
 *          for the STM32H750VBT6 target. Includes:
 *          - MPU configuration for QSPI flash access
 *          - CPU cache enablement (I-Cache, D-Cache)
 *          - System clock configuration (480MHz)
 *          - Peripheral initialization
 *          - LED control functions
 *          - I2C platform read/write for IMU
 *
 * @par Clock Configuration:
 *      - SYSCLK: 480 MHz (from PLL)
 *      - HCLK: 240 MHz (AHB)
 *      - APB1/APB2/APB3/APB4: 120 MHz
 *      - HSE: 25 MHz external crystal
 *      - LSE: 32.768 kHz for RTC
 *
 * @par Memory Protection:
 *      - Region 0: QSPI 256MB - No access (background)
 *      - Region 1: QSPI 8MB - Read-only, cacheable, executable
 *
 * @see     STM32H750 Reference Manual RM0433
 * @see     docs/do178c/design/SDD.md Section 3 - Hardware Abstraction
 *
 * @author  Souham Biswas
 * @date    2025
 *
 * @copyright Copyright 2025 Souham Biswas
 *            https://github.com/ironhide23586/icarus-os-core
 *            Licensed under the Apache License, Version 2.0
 */

#include "bsp/retarget_hal.h"
#include "bsp/error.h"

/* ============================================================================
 * FORWARD DECLARATIONS
 * ========================================================================= */

static void SystemClock_Config(void);
extern uint32_t task_active_sleep(uint32_t ticks);

/* Function prototypes */
int32_t platform_write(void* handle, uint8_t reg, const uint8_t *bufp, uint16_t len);
int32_t platform_read(void* handle, uint8_t reg, uint8_t *bufp, uint16_t len);

// /* ============================================================================
//  * MPU CONFIGURATION
//  * ========================================================================= */

// /**
//  * @brief   Configure Memory Protection Unit for QSPI flash access
//  *
//  * @details Sets up two MPU regions for QSPI memory-mapped flash:
//  *          - Region 0: 256MB background region with no access (catches stray accesses)
//  *          - Region 1: 8MB overlay for actual flash with read-only, cacheable access
//  *
//  * @par Region Configuration:
//  *      | Region | Base       | Size  | Access    | Cache     | Execute |
//  *      |--------|------------|-------|-----------|-----------|---------|
//  *      | 0      | 0x90000000 | 256MB | No Access | None      | No      |
//  *      | 1      | 0x90000000 | 8MB   | Priv RO   | WT Cache  | Yes     |
//  *
//  * @note    Must be called before enabling caches
//  * @note    Uses privileged default memory map for non-configured regions
//  *
//  * @see     ARMv7-M Architecture Reference Manual, Section B3.5 (MPU)
//  * @see     STM32H750 Reference Manual RM0433, Section 2.3.4
//  */
// static void MPU_Config(void)
// {
//   MPU_Region_InitTypeDef MPU_InitStruct = {0};

//   /* Disables the MPU */
//   HAL_MPU_Disable();

// 	/* Configure the MPU attributes for the QSPI 256MB without instruction access */
//   MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
//   MPU_InitStruct.Number           = MPU_REGION_NUMBER0;
//   MPU_InitStruct.BaseAddress      = QSPI_BASE;
//   MPU_InitStruct.Size             = MPU_REGION_SIZE_256MB;
//   MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
//   MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
//   MPU_InitStruct.IsCacheable      = MPU_ACCESS_NOT_CACHEABLE;
//   MPU_InitStruct.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
//   MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_DISABLE;
//   MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL1;
//   MPU_InitStruct.SubRegionDisable = 0x00;
//   HAL_MPU_ConfigRegion(&MPU_InitStruct);

//   /* Configure the MPU attributes for the QSPI 8MB (QSPI Flash Size) to Cacheable WT */
//   MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
//   MPU_InitStruct.Number           = MPU_REGION_NUMBER1;
//   MPU_InitStruct.BaseAddress      = QSPI_BASE;
//   MPU_InitStruct.Size             = MPU_REGION_SIZE_8MB;
//   MPU_InitStruct.AccessPermission = MPU_REGION_PRIV_RO;
//   MPU_InitStruct.IsBufferable     = MPU_ACCESS_BUFFERABLE;
//   MPU_InitStruct.IsCacheable      = MPU_ACCESS_CACHEABLE;
//   MPU_InitStruct.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
//   MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_ENABLE;
//   MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL1;
//   MPU_InitStruct.SubRegionDisable = 0x00;
//   HAL_MPU_ConfigRegion(&MPU_InitStruct);

//   /* Enables the MPU */
//   HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
// }

/**
 * @brief   Enable CPU instruction and data caches
 *
 * @details Enables both I-Cache and D-Cache for improved performance.
 *          Must be called after MPU configuration to ensure proper
 *          cacheability attributes are in effect.
 *
 * @pre     MPU_Config() must be called first
 * @post    I-Cache and D-Cache enabled
 *
 * @note    Cache coherency must be managed for DMA operations
 *
 * @see     ARMv7-M Architecture Reference Manual, Section B2.2 (Caches)
 */
static void CPU_CACHE_Enable(void)
{
  /* Enable I-Cache */
  SCB_EnableICache();

  /* Enable D-Cache */
  SCB_EnableDCache();
}

/* ============================================================================
 * LED CONTROL FUNCTIONS
 * ========================================================================= */

/**
 * @brief   Blink LED with specified on/off timing
 *
 * @details Turns LED on for specified duration, then off for specified duration.
 *          Uses task_active_sleep() for non-blocking delays.
 *
 * @param[in] Hdelay  LED on duration in OS ticks
 * @param[in] Ldelay  LED off duration in OS ticks
 *
 * @pre     GPIO must be initialized (MX_GPIO_Init called)
 * @pre     Must be called from task context (uses task_active_sleep)
 *
 * @note    Total blink period = Hdelay + Ldelay ticks
 */
void LED_Blink(uint32_t Hdelay, uint32_t Ldelay) {
	HAL_GPIO_WritePin(E3_GPIO_Port,E3_Pin,GPIO_PIN_SET);
	task_active_sleep(Hdelay - 1);
	HAL_GPIO_WritePin(E3_GPIO_Port,E3_Pin,GPIO_PIN_RESET);
	task_active_sleep(Ldelay-1);
}

/**
 * @brief   Turn LED on (active high)
 *
 * @details Sets PE3 GPIO pin high to illuminate the onboard LED.
 *
 * @pre     GPIO must be initialized
 */
void LED_On(void) {
	HAL_GPIO_WritePin(E3_GPIO_Port,E3_Pin,GPIO_PIN_SET);
}

/**
 * @brief   Turn LED off
 *
 * @details Sets PE3 GPIO pin low to turn off the onboard LED.
 *
 * @pre     GPIO must be initialized
 */
void LED_Off(void) {
	HAL_GPIO_WritePin(E3_GPIO_Port,E3_Pin,GPIO_PIN_RESET);
}

/* ============================================================================
 * I2C PLATFORM FUNCTIONS (IMU DRIVER)
 * ========================================================================= */

/**
 * @brief   Platform write function for LSM9DS1 IMU driver
 *
 * @details Writes data to IMU register via I2C. Used by the LSM9DS1
 *          driver library for register configuration.
 *
 * @param[in] handle  I2C handle (cast to I2C_HandleTypeDef*)
 * @param[in] reg     Register address to write
 * @param[in] bufp    Pointer to data buffer
 * @param[in] len     Number of bytes to write
 *
 * @return    0 on success (HAL status ignored for simplicity)
 *
 * @note    Uses blocking I2C transfer with HAL_MAX_DELAY timeout
 */
int32_t platform_write(void* handle, uint8_t reg, const uint8_t *bufp, uint16_t len) {
	/* HAL_I2C_Mem_Write requires non-const pointer, but we receive const
	 * This is safe as HAL will only read from the buffer */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
	HAL_I2C_Mem_Write((I2C_HandleTypeDef*) handle, IMU_ADDRESS, reg, I2C_MEMADD_SIZE_8BIT,
			(uint8_t *)bufp, len, HAL_MAX_DELAY);
#pragma GCC diagnostic pop
	return 0;
}

/**
 * @brief   Platform read function for LSM9DS1 IMU driver
 *
 * @details Reads data from IMU register via I2C. Used by the LSM9DS1
 *          driver library for sensor data acquisition.
 *
 * @param[in]  handle  I2C handle (cast to I2C_HandleTypeDef*)
 * @param[in]  reg     Register address to read
 * @param[out] bufp    Pointer to receive buffer
 * @param[in]  len     Number of bytes to read
 *
 * @return    0 on success (HAL status ignored for simplicity)
 *
 * @note    Uses blocking I2C transfer with HAL_MAX_DELAY timeout
 */
int32_t platform_read(void* handle, uint8_t reg, uint8_t *bufp, uint16_t len) {
	HAL_I2C_Mem_Read((I2C_HandleTypeDef*) handle, IMU_ADDRESS, reg, I2C_MEMADD_SIZE_8BIT,
			bufp, len, HAL_MAX_DELAY);
	return 0;
}

/* ============================================================================
 * HAL INITIALIZATION
 * ========================================================================= */

/**
 * @brief   Initialize all hardware abstraction layer components
 *
 * @details Master initialization function that configures all hardware:
 *          1. Vector table relocation (if running from QSPI)
 *          2. MPU configuration for memory protection
 *          3. CPU cache enablement
 *          4. HAL library initialization
 *          5. System clock configuration (480MHz)
 *          6. GPIO initialization
 *          7. RTC initialization
 *          8. SPI4 initialization (LCD)
 *          9. TIM1 initialization (PWM)
 *          10. I2C2 initialization (IMU)
 *          11. USB CDC initialization
 *
 * @pre     None (called at system startup)
 * @post    All hardware ready for kernel initialization
 *
 * @note    Must be called before os_init()
 * @note    USB may take up to 1 second to enumerate on host
 *
 * @see     SystemClock_Config() for clock tree details
 */
void hal_init(void) {
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

/* ============================================================================
 * SYSTEM CLOCK CONFIGURATION
 * ========================================================================= */

/**
 * @brief   Configure system clocks for 480MHz operation
 *
 * @details Configures the complete clock tree:
 *          - HSE: 25MHz external crystal
 *          - LSE: 32.768kHz for RTC
 *          - HSI48: 48MHz for USB
 *          - PLL: 480MHz SYSCLK from HSE
 *
 * @par PLL Configuration:
 *      - Source: HSE (25MHz)
 *      - PLLM: 5 (VCO input = 5MHz)
 *      - PLLN: 96 (VCO output = 480MHz)
 *      - PLLP: 2 (SYSCLK = 480MHz)
 *
 * @par Bus Clocks:
 *      | Bus   | Divider | Frequency |
 *      |-------|---------|-----------|
 *      | SYSCLK| /1      | 480 MHz   |
 *      | HCLK  | /2      | 240 MHz   |
 *      | APB1  | /1      | 120 MHz   |
 *      | APB2  | /1      | 120 MHz   |
 *      | APB3  | /1      | 120 MHz   |
 *      | APB4  | /1      | 120 MHz   |
 *
 * @note    Calls Error_Handler() on configuration failure
 *
 * @see     STM32H750 Reference Manual RM0433, Section 8 (RCC)
 */
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
