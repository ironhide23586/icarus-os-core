
#include "bsp/mpu.h"
#include "bsp/error.h"
#include "stm32h7xx.h"

/* ============================================================================
 * SECTION PLACEMENT MACROS
 * ========================================================================= */

#ifndef HOST_TEST
#define ITCM_FUNC_PRIV __attribute__((section(".itcm.privileged")))
#else
#define ITCM_FUNC_PRIV
#endif

/* ============================================================================
 * COMPILE-TIME MPU VALIDATION
 * ========================================================================= */

/* Task data region must be power of 2 for MPU */
_Static_assert((TASK_DATA_SIZE_BYTES & (TASK_DATA_SIZE_BYTES - 1)) == 0,
               "TASK_DATA_SIZE_BYTES must be power of 2 (MPU region requirement)");

/* Task data region must be at least 32 bytes (MPU minimum) */
_Static_assert(TASK_DATA_SIZE_BYTES >= 32,
               "TASK_DATA_SIZE_BYTES must be >= 32 (MPU minimum region size)");

/* ============================================================================
 * LINKER SYMBOLS
 * ========================================================================= */

/* Privileged ITCM section boundaries (from linker script) */
extern uint32_t __sitcm_priv;
extern uint32_t __eitcm_priv;

/* Privileged DTCM section boundaries (from linker script) */
extern uint32_t __sdtcm_priv;
extern uint32_t __edtcm_priv;

/* ============================================================================
 * HELPER FUNCTIONS
 * ========================================================================= */

/**
 * @brief Calculate MPU region size for given byte count
 * @param size_bytes Number of bytes to protect
 * @return MPU region size code (5-31 for 32B to 4GB)
 * @note Rounds up to next power of 2
 */
static ITCM_FUNC_PRIV uint8_t __calculate_mpu_region_size(uint32_t size_bytes)
{
    /* MPU regions must be power of 2, minimum 32 bytes */
    uint32_t _mpu_size = 32;
    uint8_t _size_bits = 5;
    
    while (_mpu_size < size_bytes && _size_bits < 32) {
        _mpu_size <<= 1;
        _size_bits++;
    }
    
    return _size_bits - 1;  /* MPU uses (size_bits - 1) encoding */
}

/**
 * @brief Align address down to region size boundary
 * @param addr Address to align
 * @param region_size Region size in bytes (must be power of 2)
 * @return Aligned address
 */
static ITCM_FUNC_PRIV uint32_t __align_to_region(uint32_t addr, uint32_t region_size)
{
    return addr & ~(region_size - 1);
}

/* ============================================================================
 * MPU CONFIGURATION
 * ========================================================================= */


ITCM_FUNC_PRIV void __mpu_config(void)
{
  MPU_Region_InitTypeDef _mpu_init = {0};

  /* Runtime validation of linker-provided section sizes */
  uint32_t _itcm_priv_start = (uint32_t)&__sitcm_priv;
  uint32_t _itcm_priv_end = (uint32_t)&__eitcm_priv;
  uint32_t _itcm_priv_size = _itcm_priv_end - _itcm_priv_start;
  uint32_t _dtcm_priv_start = (uint32_t)&__sdtcm_priv;
  uint32_t _dtcm_priv_end = (uint32_t)&__edtcm_priv;
  uint32_t _dtcm_priv_size = _dtcm_priv_end - _dtcm_priv_start;

  /* Privileged sections must fit within their respective TCM regions */
  if (_itcm_priv_size > BSP_ITCM_SIZE) { Error_Handler(); }
  if (_dtcm_priv_size > BSP_DTCM_SIZE) { Error_Handler(); }

  /* Privileged sections must not be empty (kernel code/data must exist) */
  if (_itcm_priv_size == 0) { Error_Handler(); }
  if (_dtcm_priv_size == 0) { Error_Handler(); }

  /* Disables the MPU */
  HAL_MPU_Disable();

  /* ========================================================================
   * REGION 0: QSPI Background (256MB) - No Access
   * ======================================================================== */
  _mpu_init.Enable           = MPU_REGION_ENABLE;
  _mpu_init.Number           = MPU_REGION_QSPI_BACKGROUND;
  _mpu_init.BaseAddress      = QSPI_BASE;
  _mpu_init.Size             = MPU_REGION_SIZE_256MB;
  _mpu_init.AccessPermission = MPU_REGION_NO_ACCESS;
  _mpu_init.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
  _mpu_init.IsCacheable      = MPU_ACCESS_NOT_CACHEABLE;
  _mpu_init.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
  _mpu_init.DisableExec      = MPU_INSTRUCTION_ACCESS_DISABLE;
  _mpu_init.TypeExtField     = MPU_TEX_LEVEL1;
  _mpu_init.SubRegionDisable = 0x00;
  HAL_MPU_ConfigRegion(&_mpu_init);

  /* ========================================================================
   * REGION 1: QSPI Flash (8MB) - Privileged Read-Only, Cacheable
   * ======================================================================== */
  _mpu_init.Enable           = MPU_REGION_ENABLE;
  _mpu_init.Number           = MPU_REGION_QSPI_FLASH;
  _mpu_init.BaseAddress      = QSPI_BASE;
  _mpu_init.Size             = MPU_REGION_SIZE_8MB;
  _mpu_init.AccessPermission = MPU_REGION_PRIV_RO;
  _mpu_init.IsBufferable     = MPU_ACCESS_BUFFERABLE;
  _mpu_init.IsCacheable      = MPU_ACCESS_CACHEABLE;
  _mpu_init.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
  _mpu_init.DisableExec      = MPU_INSTRUCTION_ACCESS_ENABLE;
  _mpu_init.TypeExtField     = MPU_TEX_LEVEL1;
  _mpu_init.SubRegionDisable = 0x00;
  HAL_MPU_ConfigRegion(&_mpu_init);

  /* ========================================================================
   * REGION 2: Privileged ITCM Kernel Code - Privileged Execute Only
   * ======================================================================== */
  if (_itcm_priv_size > 0) {
    uint8_t _region_size_code = __calculate_mpu_region_size(_itcm_priv_size);
    uint32_t _region_size_bytes = 1UL << (_region_size_code + 1);
    uint32_t _aligned_base = __align_to_region(_itcm_priv_start, _region_size_bytes);
    
    _mpu_init.Enable           = MPU_REGION_ENABLE;
    _mpu_init.Number           = MPU_REGION_ITCM_PRIVILEGED;
    _mpu_init.BaseAddress      = _aligned_base;
    _mpu_init.Size             = _region_size_code << 1;
    _mpu_init.AccessPermission = MPU_REGION_PRIV_RO;
    _mpu_init.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
    _mpu_init.IsCacheable      = MPU_ACCESS_NOT_CACHEABLE;
    _mpu_init.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
    _mpu_init.DisableExec      = MPU_INSTRUCTION_ACCESS_ENABLE;
    _mpu_init.TypeExtField     = MPU_TEX_LEVEL0;
    _mpu_init.SubRegionDisable = 0x00;
    HAL_MPU_ConfigRegion(&_mpu_init);
  }

  /* ========================================================================
   * REGION 3: Privileged DTCM Kernel Data - Privileged Read/Write Only
   * ======================================================================== */
  if (_dtcm_priv_size > 0) {
    uint8_t _region_size_code = __calculate_mpu_region_size(_dtcm_priv_size);
    uint32_t _region_size_bytes = 1UL << (_region_size_code + 1);
    uint32_t _aligned_base = __align_to_region(_dtcm_priv_start, _region_size_bytes);
    
    _mpu_init.Enable           = MPU_REGION_ENABLE;
    _mpu_init.Number           = MPU_REGION_DTCM;
    _mpu_init.BaseAddress      = _aligned_base;
    _mpu_init.Size             = _region_size_code << 1;
    _mpu_init.AccessPermission = MPU_REGION_PRIV_RW;
    _mpu_init.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
    _mpu_init.IsCacheable      = MPU_ACCESS_NOT_CACHEABLE;
    _mpu_init.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
    _mpu_init.DisableExec      = MPU_INSTRUCTION_ACCESS_DISABLE;
    _mpu_init.TypeExtField     = MPU_TEX_LEVEL0;
    _mpu_init.SubRegionDisable = 0x00;
    HAL_MPU_ConfigRegion(&_mpu_init);
  }

  /* Enables the MPU with privileged default memory map */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

ITCM_FUNC_PRIV void __mpu_configure_task_data(uint32_t task_data_base) {
    MPU_Region_InitTypeDef _mpu_init = {0};
    
    _mpu_init.Enable = MPU_REGION_ENABLE;
    _mpu_init.Number = MPU_REGION_TASK_DATA;
    _mpu_init.BaseAddress = task_data_base;
    _mpu_init.Size = TASK_DATA_SIZE_MPU;
    _mpu_init.AccessPermission = MPU_REGION_FULL_ACCESS;
    _mpu_init.IsCacheable = MPU_ACCESS_CACHEABLE;
    _mpu_init.IsBufferable = MPU_ACCESS_BUFFERABLE;
    _mpu_init.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
    _mpu_init.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
    _mpu_init.TypeExtField = MPU_TEX_LEVEL0;
    _mpu_init.SubRegionDisable = 0x00;
    
    HAL_MPU_ConfigRegion(&_mpu_init);
}

