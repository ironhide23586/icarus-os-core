
#include "bsp/mpu.h"
#include "stm32h7xx.h"

/* ============================================================================
 * LINKER SYMBOLS
 * ========================================================================= */

/* Privileged ITCM section boundaries (from linker script) */
extern uint32_t _sitcm_priv;
extern uint32_t _eitcm_priv;

/* Privileged DTCM section boundaries (from linker script) */
extern uint32_t _sdtcm_priv;
extern uint32_t _edtcm_priv;

/* ============================================================================
 * HELPER FUNCTIONS
 * ========================================================================= */

/**
 * @brief Calculate MPU region size for given byte count
 * @param size_bytes Number of bytes to protect
 * @return MPU region size code (5-31 for 32B to 4GB)
 * @note Rounds up to next power of 2
 */
static uint8_t calculate_mpu_region_size(uint32_t size_bytes)
{
    /* MPU regions must be power of 2, minimum 32 bytes */
    uint32_t mpu_size = 32;
    uint8_t size_bits = 5;
    
    while (mpu_size < size_bytes && size_bits < 32) {
        mpu_size <<= 1;
        size_bits++;
    }
    
    return size_bits - 1;  /* MPU uses (size_bits - 1) encoding */
}

/**
 * @brief Align address down to region size boundary
 * @param addr Address to align
 * @param region_size Region size in bytes (must be power of 2)
 * @return Aligned address
 */
static uint32_t align_to_region(uint32_t addr, uint32_t region_size)
{
    return addr & ~(region_size - 1);
}

/* ============================================================================
 * MPU CONFIGURATION
 * ========================================================================= */


void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

  /* ========================================================================
   * REGION 0: QSPI Background (256MB) - No Access
   * ======================================================================== */
  MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
  MPU_InitStruct.Number           = MPU_REGION_QSPI_BACKGROUND;
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

  /* ========================================================================
   * REGION 1: QSPI Flash (8MB) - Privileged Read-Only, Cacheable
   * ======================================================================== */
  MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
  MPU_InitStruct.Number           = MPU_REGION_QSPI_FLASH;
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

  /* ========================================================================
   * REGION 2: Privileged ITCM Kernel Code - Privileged Execute Only
   * ======================================================================== */
  uint32_t itcm_priv_start = (uint32_t)&_sitcm_priv;
  uint32_t itcm_priv_end = (uint32_t)&_eitcm_priv;
  uint32_t itcm_priv_size = itcm_priv_end - itcm_priv_start;
  
  if (itcm_priv_size > 0) {
    /* Calculate power-of-2 region size */
    uint8_t region_size_code = calculate_mpu_region_size(itcm_priv_size);
    uint32_t region_size_bytes = 1UL << (region_size_code + 1);
    
    /* Align base address to region size */
    uint32_t aligned_base = align_to_region(itcm_priv_start, region_size_bytes);
    
    MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
    MPU_InitStruct.Number           = MPU_REGION_ITCM_PRIVILEGED;
    MPU_InitStruct.BaseAddress      = aligned_base;
    MPU_InitStruct.Size             = region_size_code << 1;  /* HAL expects size << 1 */
    MPU_InitStruct.AccessPermission = MPU_REGION_PRIV_RO;     /* Privileged read-only, unprivileged no access */
    MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
    MPU_InitStruct.IsCacheable      = MPU_ACCESS_NOT_CACHEABLE;
    MPU_InitStruct.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
    MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_ENABLE;  /* Executable */
    MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL0;
    MPU_InitStruct.SubRegionDisable = 0x00;
    HAL_MPU_ConfigRegion(&MPU_InitStruct);
  }

  /* ========================================================================
   * REGION 3: Privileged DTCM Kernel Data - Privileged Read/Write Only
   * ======================================================================== */
  uint32_t dtcm_priv_start = (uint32_t)&_sdtcm_priv;
  uint32_t dtcm_priv_end = (uint32_t)&_edtcm_priv;
  uint32_t dtcm_priv_size = dtcm_priv_end - dtcm_priv_start;
  
  if (dtcm_priv_size > 0) {
    /* Calculate power-of-2 region size */
    uint8_t region_size_code = calculate_mpu_region_size(dtcm_priv_size);
    uint32_t region_size_bytes = 1UL << (region_size_code + 1);
    
    /* Align base address to region size */
    uint32_t aligned_base = align_to_region(dtcm_priv_start, region_size_bytes);
    
    MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
    MPU_InitStruct.Number           = MPU_REGION_DTCM;
    MPU_InitStruct.BaseAddress      = aligned_base;
    MPU_InitStruct.Size             = region_size_code << 1;  /* HAL expects size << 1 */
    MPU_InitStruct.AccessPermission = MPU_REGION_PRIV_RW;     /* Privileged RW, unprivileged no access */
    MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
    MPU_InitStruct.IsCacheable      = MPU_ACCESS_NOT_CACHEABLE;
    MPU_InitStruct.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
    MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_DISABLE;
    MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL0;
    MPU_InitStruct.SubRegionDisable = 0x00;
    HAL_MPU_ConfigRegion(&MPU_InitStruct);
  }

  /* Enables the MPU with privileged default memory map */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

void MPU_ConfigureTaskData(uint32_t task_data_base) {
    MPU_Region_InitTypeDef MPU_InitStruct = {0};
    
    MPU_InitStruct.Enable = MPU_REGION_ENABLE;
    MPU_InitStruct.Number = MPU_REGION_TASK_DATA;  /* Region 4 - dynamic task data */
    MPU_InitStruct.BaseAddress = task_data_base;
    MPU_InitStruct.Size = TASK_DATA_SIZE_MPU;
    MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
    MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
    MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;
    MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
    MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
    MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
    MPU_InitStruct.SubRegionDisable = 0x00;
    
    HAL_MPU_ConfigRegion(&MPU_InitStruct);
}

