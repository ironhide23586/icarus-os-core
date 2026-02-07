/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32h7xx_it.h
  * @brief   This file contains the headers of the interrupt handlers.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32H7xx_IT_H
#define __STM32H7xx_IT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Section placement macros --------------------------------------------------*/
#ifndef HOST_TEST
#define ITCM_FUNC_USER __attribute__((section(".itcm")))
#define ITCM_FUNC_PRIV __attribute__((section(".itcm.privileged")))
#define DTCM_DATA_USER __attribute__((section(".dtcm")))
#define DTCM_DATA_PRIV __attribute__((section(".dtcm.privileged")))
#else
#define ITCM_FUNC_USER
#define ITCM_FUNC_PRIV
#define DTCM_DATA_USER
#define DTCM_DATA_PRIV
#endif

/* Exported functions prototypes ---------------------------------------------*/
void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
ITCM_FUNC_PRIV void SVC_Handler(void);
void DebugMon_Handler(void);
ITCM_FUNC_PRIV void PendSV_Handler(void);
ITCM_FUNC_PRIV void SysTick_Handler(void);
void OTG_FS_IRQHandler(void);
/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /* __STM32H7xx_IT_H */
