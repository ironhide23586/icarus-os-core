

#include "bsp/mpu.h"
#include "stm32h7xx.h"


/* ============================================================================
 * MPU CONFIGURATION
 * ========================================================================= */

/**
 * @brief   Configure Memory Protection Unit for QSPI flash access
 *
 * @details Sets up two MPU regions for QSPI memory-mapped flash:
 *          - Region 0: 256MB background region with no access (catches stray accesses)
 *          - Region 1: 8MB overlay for actual flash with read-only, cacheable access
 *
 * @par Region Configuration:
 *      | Region | Base       | Size  | Access    | Cache     | Execute |
 *      |--------|------------|-------|-----------|-----------|---------|
 *      | 0      | 0x90000000 | 256MB | No Access | None      | No      |
 *      | 1      | 0x90000000 | 8MB   | Priv RO   | WT Cache  | Yes     |
 *
 * @note    Must be called before enabling caches
 * @note    Uses privileged default memory map for non-configured regions
 *
 * @see     ARMv7-M Architecture Reference Manual, Section B3.5 (MPU)
 * @see     STM32H750 Reference Manual RM0433, Section 2.3.4
 */
void MPU_Config(void)
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
