/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32h7xx_it.c
  * @brief   Interrupt Service Routines.
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

/* Includes ------------------------------------------------------------------*/
#include "bsp/config.h"

#include "stm32h7xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdbool.h>
#include "icarus/config.h"
#include "icarus/svc.h"
#ifdef HOST_TEST
// For host testing, include mock header for os_yield_pendsv
#include "mock_asm.h"
#endif
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* ITCM_FUNC and ITCM_FUNC are defined in icarus/config.h */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern PCD_HandleTypeDef hpcd_USB_OTG_FS;
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
   while (1)
  {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
#ifndef HOST_TEST
void HardFault_Handler(void)
{
  /* Capture fault info from SCB */
  volatile uint32_t cfsr  = SCB->CFSR;
  volatile uint32_t hfsr  = SCB->HFSR;
  volatile uint32_t mmfar = SCB->MMFAR;
  volatile uint32_t bfar  = SCB->BFAR;
  (void)cfsr; (void)hfsr; (void)mmfar; (void)bfar;

  /* Extract stacked PC via PSP or MSP */
  uint32_t *sp;
  __asm__ volatile ("tst lr, #4\n"
                    "ite eq\n"
                    "mrseq %0, msp\n"
                    "mrsne %0, psp\n"
                    : "=r" (sp));
  volatile uint32_t stacked_r0  = sp[0];
  volatile uint32_t stacked_r1  = sp[1];
  volatile uint32_t stacked_r2  = sp[2];
  volatile uint32_t stacked_r3  = sp[3];
  volatile uint32_t stacked_r12 = sp[4];
  volatile uint32_t stacked_lr  = sp[5];
  volatile uint32_t stacked_pc  = sp[6];
  volatile uint32_t stacked_psr = sp[7];
  (void)stacked_r0; (void)stacked_r1; (void)stacked_r2; (void)stacked_r3;
  (void)stacked_r12; (void)stacked_lr; (void)stacked_pc; (void)stacked_psr;

  /* Fast LED blink = HardFault (3 fast blinks, pause, repeat) */
  while (1)
  {
    for (int i = 0; i < 3; i++) {
      HAL_GPIO_WritePin(E3_GPIO_Port, E3_Pin, GPIO_PIN_SET);
      for (volatile int d = 0; d < 16000000; d++) {}
      HAL_GPIO_WritePin(E3_GPIO_Port, E3_Pin, GPIO_PIN_RESET);
      for (volatile int d = 0; d < 16000000; d++) {}
    }
    for (volatile int d = 0; d < 160000000; d++) {}
  }
}
#else
void HardFault_Handler(void) { while (1) {} }
#endif

/* Incremented by recoverable MemManage faults */
volatile uint32_t g_memmanage_fault_count = 0;

/* Debug: capture last fault address */
volatile uint32_t g_last_fault_addr = 0;
volatile uint32_t g_last_fault_pc = 0;

/**
  * @brief This function handles Memory management fault.
  *
  * @details If the fault is a recoverable data access violation (DACCVIOL)
  *          from unprivileged thread mode (PSP), we advance the stacked PC
  *          by 2 bytes (Thumb instruction) and return so the task continues.
  *          The task can then check g_memmanage_fault_count to detect it.
  *
  *          If the fault is not recoverable (instruction fetch, or from MSP),
  *          fall through to the 4-blink halt.
  */
#ifndef HOST_TEST
void MemManage_Handler(void)
{
  uint32_t cfsr  = SCB->CFSR;

  /* MemManage fault types:
   * DACCVIOL (bit 1) = data access violation (read or write)
   * IACCVIOL (bit 0) = instruction fetch violation — not recoverable here
   * MMARVALID (bit 7) = MMFAR holds valid fault address */
  bool is_daccviol = (cfsr & SCB_CFSR_DACCVIOL_Msk) != 0;
  bool is_iaccviol = (cfsr & SCB_CFSR_IACCVIOL_Msk) != 0;

  /* Check if fault came from thread mode (PSP) vs kernel (MSP) */
  uint32_t lr_val;
  __asm__ volatile ("mov %0, lr" : "=r" (lr_val));
  bool from_psp = (lr_val & 0x4) != 0;

  /* Recover from data access violations (read or write) from unprivileged tasks */
  if (is_daccviol && !is_iaccviol && from_psp) {
    /* Recoverable: advance stacked PC past the faulting Thumb instruction */
    uint32_t *sp;
    __asm__ volatile ("mrs %0, psp" : "=r" (sp));
    
    /* Capture fault address for debugging */
    if (cfsr & SCB_CFSR_MMARVALID_Msk) {
      g_last_fault_addr = SCB->MMFAR;
    } else {
      g_last_fault_addr = 0xFFFFFFFF;  /* Invalid */
    }
    g_last_fault_pc = sp[6];
    
    /* Determine instruction length: Thumb-2 (32-bit) vs Thumb (16-bit)
     * Thumb-2 instructions have bits [15:11] >= 0b11101 (0x1D)
     * See ARMv7-M Architecture Reference Manual A6.3 */
    uint16_t *pc_ptr = (uint16_t *)sp[6];
    uint16_t instr = *pc_ptr;
    uint8_t instr_len = ((instr & 0xF800) >= 0xE800) ? 4 : 2;
    
    sp[6] += instr_len;  /* stacked PC is at offset 6 in the exception frame */

    /* Clear the fault status bits so we can return cleanly */
    SCB->CFSR = cfsr;

    g_memmanage_fault_count++;
    
    /* Limit fault recovery to prevent infinite loops */
    if (g_memmanage_fault_count > 100) {
      /* Halt with rapid 4 blinks */
      while (1) {
        for (int i = 0; i < 4; i++) {
          HAL_GPIO_WritePin(E3_GPIO_Port, E3_Pin, GPIO_PIN_SET);
          for (volatile int d = 0; d < 8000000; d++) {}
          HAL_GPIO_WritePin(E3_GPIO_Port, E3_Pin, GPIO_PIN_RESET);
          for (volatile int d = 0; d < 8000000; d++) {}
        }
        for (volatile int d = 0; d < 80000000; d++) {}
      }
    }
    
    return;
  }

  /* Non-recoverable: 4 fast blinks = MemManage */
  while (1)
  {
    for (int i = 0; i < 4; i++) {
      HAL_GPIO_WritePin(E3_GPIO_Port, E3_Pin, GPIO_PIN_SET);
      for (volatile int d = 0; d < 16000000; d++) {}
      HAL_GPIO_WritePin(E3_GPIO_Port, E3_Pin, GPIO_PIN_RESET);
      for (volatile int d = 0; d < 16000000; d++) {}
    }
    for (volatile int d = 0; d < 160000000; d++) {}
  }
}
#else
void MemManage_Handler(void) { while (1) {} }
#endif

/**
  * @brief This function handles Pre-fetch fault, memory access fault.
  */
#ifndef HOST_TEST
void BusFault_Handler(void)
{
  volatile uint32_t cfsr = SCB->CFSR;
  volatile uint32_t bfar = SCB->BFAR;
  (void)cfsr; (void)bfar;

  /* 5 fast blinks = BusFault */
  while (1)
  {
    for (int i = 0; i < 5; i++) {
      HAL_GPIO_WritePin(E3_GPIO_Port, E3_Pin, GPIO_PIN_SET);
      for (volatile int d = 0; d < 16000000; d++) {}
      HAL_GPIO_WritePin(E3_GPIO_Port, E3_Pin, GPIO_PIN_RESET);
      for (volatile int d = 0; d < 16000000; d++) {}
    }
    for (volatile int d = 0; d < 160000000; d++) {}
  }
}
#else
void BusFault_Handler(void) { while (1) {} }
#endif

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
#ifndef HOST_TEST
void UsageFault_Handler(void)
{
  volatile uint32_t cfsr = SCB->CFSR;
  (void)cfsr;

  /* 6 fast blinks = UsageFault */
  while (1)
  {
    for (int i = 0; i < 6; i++) {
      HAL_GPIO_WritePin(E3_GPIO_Port, E3_Pin, GPIO_PIN_SET);
      for (volatile int d = 0; d < 16000000; d++) {}
      HAL_GPIO_WritePin(E3_GPIO_Port, E3_Pin, GPIO_PIN_RESET);
      for (volatile int d = 0; d < 16000000; d++) {}
    }
    for (volatile int d = 0; d < 160000000; d++) {}
  }
}
#else
void UsageFault_Handler(void) { while (1) {} }
#endif

/**
  * @brief This function handles System service call via SWI instruction.
  */
#ifdef HOST_TEST
void SVC_Handler(void)
{
  /* Empty for host testing */
}
#else
ITCM_FUNC __attribute__((naked)) void SVC_Handler(void)
{
  __asm__ volatile (
    "tst lr, #4\n"
    "ite eq\n"
    "mrseq r0, msp\n"
    "mrsne r0, psp\n"
    "b SVC_Handler_C\n"
  );
}
#endif

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/**
  * @brief This function handles Pendable request for system service.
  */
#ifdef HOST_TEST
// For host testing, use regular function (not naked) to call mock
void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */
  os_yield_pendsv();
  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}
#else
ITCM_FUNC __attribute__ ((naked)) void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */
  __asm__ volatile (
    "b os_yield_pendsv"
  );
  
  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}
#endif

/**
  * @brief This function handles System tick timer.
  * @note  Must run in privileged mode to write to os_tick_count in DTCM
  */
ITCM_FUNC void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */
  extern volatile uint32_t os_tick_count;
  extern volatile uint8_t os_running;
  extern volatile uint32_t current_task_ticks_remaining;
  extern volatile bool scheduler_enabled;
  
  os_tick_count++;

  if (os_running && --current_task_ticks_remaining == 0 && scheduler_enabled) {
    current_task_ticks_remaining = ICARUS_TICKS_PER_TASK;  // Reset for next task
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
  }
  
  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32H7xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32h7xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles USB On The Go FS global interrupt.
  */
void OTG_FS_IRQHandler(void)
{
  /* USER CODE BEGIN OTG_FS_IRQn 0 */

  /* USER CODE END OTG_FS_IRQn 0 */
  HAL_PCD_IRQHandler(&hpcd_USB_OTG_FS);
  /* USER CODE BEGIN OTG_FS_IRQn 1 */

  /* USER CODE END OTG_FS_IRQn 1 */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
