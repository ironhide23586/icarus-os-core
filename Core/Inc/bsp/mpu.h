
#ifndef BSP_MPU_H
#define BSP_MPU_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * MEMORY MAP (STM32H750)
 * ========================================================================= */

/**
 * @defgroup MEMORY_MAP Memory Regions
 * @brief    STM32H750 memory region definitions
 * @see      STM32H750 Reference Manual RM0433, Section 2.3
 * @{
 */

/** @brief ITCM RAM base address (64KB, zero wait-state code) */
#define BSP_ITCM_BASE           0x00000000UL
#define BSP_ITCM_SIZE           (64 * 1024)

/** @brief DTCM RAM base address (128KB, zero wait-state data) */
#define BSP_DTCM_BASE           0x20000000UL
#define BSP_DTCM_SIZE           (128 * 1024)

/** @brief AXI SRAM (RAM_D1) base address (512KB) */
#define BSP_RAM_D1_BASE         0x24000000UL
#define BSP_RAM_D1_SIZE         (512 * 1024)

/** @brief AHB SRAM (RAM_D2) base address (288KB) */
#define BSP_RAM_D2_BASE         0x30000000UL
#define BSP_RAM_D2_SIZE         (288 * 1024)

/** @brief AHB SRAM (RAM_D3) base address (64KB) */
#define BSP_RAM_D3_BASE         0x38000000UL
#define BSP_RAM_D3_SIZE         (64 * 1024)

/** @brief Internal Flash base address (128KB) */
#define BSP_FLASH_BASE          0x08000000UL
#define BSP_FLASH_SIZE          (128 * 1024)

/** @brief QSPI Flash base address (8MB external) */
#define BSP_QSPI_BASE           0x90000000UL
#define BSP_QSPI_SIZE           (8 * 1024 * 1024)

/** @} */


#define PERIPH_MPU_REGION_BASE_ADDR  0x40000000
#define PERIPH_MPU_REGION_SIZE       (512 * 1024)  // 512MB, but use size encoding


#define MPU_REGION_FLASH      0
#define MPU_REGION_DTCMRAM    1
#define MPU_REGION_RAMD1      2
#define MPU_REGION_RAMD2      3
#define MPU_REGION_RAMD3      4
#define MPU_REGION_ITCMRAM    5
#define MPU_REGION_PERIPH     6


void MPU_Config(void);


#ifdef __cplusplus
}
#endif

#endif