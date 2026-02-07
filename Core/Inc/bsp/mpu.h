
#ifndef BSP_MPU_H
#define BSP_MPU_H

#ifdef __cplusplus
extern "C" {
#endif

/* Include HAL only when not testing */
#ifndef HOST_TEST
#include "stm32h7xx_hal.h"
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
#include "icarus/config.h"

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

/** @brief Peripheral base address (512MB) */
#define BSP_PERIPH_BASE         0x40000000
#define BSP_PERIPH_SIZE         (512 * 1024 * 1024)


/** @} */

#define MPU_REGION_QSPI_BACKGROUND  MPU_REGION_NUMBER0
#define MPU_REGION_QSPI_FLASH       MPU_REGION_NUMBER1
#define MPU_REGION_ITCM_PRIVILEGED  MPU_REGION_NUMBER2
#define MPU_REGION_DTCM             MPU_REGION_NUMBER3
#define MPU_REGION_TASK_DATA        MPU_REGION_NUMBER4


/**
 * @brief Convert size in bytes to MPU region size constant
 * @note Only works for power-of-2 sizes from 32 bytes to 4GB
 */
#define BYTES_TO_MPU_SIZE(bytes) ( \
    (bytes) == 32        ? MPU_REGION_SIZE_32B   : \
    (bytes) == 64        ? MPU_REGION_SIZE_64B   : \
    (bytes) == 128       ? MPU_REGION_SIZE_128B  : \
    (bytes) == 256       ? MPU_REGION_SIZE_256B  : \
    (bytes) == 512       ? MPU_REGION_SIZE_512B  : \
    (bytes) == 1024      ? MPU_REGION_SIZE_1KB   : \
    (bytes) == 2048      ? MPU_REGION_SIZE_2KB   : \
    (bytes) == 4096      ? MPU_REGION_SIZE_4KB   : \
    (bytes) == 8192      ? MPU_REGION_SIZE_8KB   : \
    (bytes) == 16384     ? MPU_REGION_SIZE_16KB  : \
    (bytes) == 32768     ? MPU_REGION_SIZE_32KB  : \
    (bytes) == 65536     ? MPU_REGION_SIZE_64KB  : \
    (bytes) == 131072    ? MPU_REGION_SIZE_128KB : \
    (bytes) == 262144    ? MPU_REGION_SIZE_256KB : \
    (bytes) == 524288    ? MPU_REGION_SIZE_512KB : \
    MPU_REGION_SIZE_1MB)

/**
 * @brief Task data region size for MPU configuration
 * @note Derived from ICARUS_DATA_WORDS in config.h
 */
#define TASK_DATA_SIZE_BYTES (ICARUS_DATA_WORDS * 4)
#define TASK_DATA_SIZE_MPU   BYTES_TO_MPU_SIZE(TASK_DATA_SIZE_BYTES)

/* ============================================================================
 * SECTION PLACEMENT MACROS
 * ========================================================================= */

#ifndef HOST_TEST
#define ITCM_FUNC_PRIV __attribute__((section(".itcm.privileged")))
#else
#define ITCM_FUNC_PRIV
#endif

/**
 * @brief   Configure Memory Protection Unit for QSPI flash access
 *
 * @details Sets up MPU regions for system protection:
 *          - Region 0: 256MB QSPI background region with no access (catches stray accesses)
 *          - Region 1: 8MB QSPI overlay for actual flash with read-only, cacheable access
 *          - Region 2: Privileged ITCM kernel code (no access for unprivileged)
 *          - Region 3: Privileged DTCM kernel data (no access for unprivileged)
 *          - Region 4: Task-specific data (configured dynamically per task)
 *
 * @par Region Configuration:
 *      | Region | Base       | Size     | Access         | Cache     | Execute |
 *      |--------|------------|----------|----------------|-----------|---------|
 *      | 0      | 0x90000000 | 256MB    | No Access      | None      | No      |
 *      | 1      | 0x90000000 | 8MB      | Priv RO        | WT Cache  | Yes     |
 *      | 2      | 0x00000000 | Dynamic  | Priv RO/Exec   | None      | Yes     |
 *      | 3      | 0x20000000 | Dynamic  | Priv RW        | None      | No      |
 *      | 4      | Dynamic    | Dynamic  | Full Access    | Cache     | No      |
 *
 * @note    Region 2 and 3 sizes are calculated dynamically from linker symbols
 * @note    User code can still use remaining ITCM/DTCM space not covered by MPU regions
 * @note    Must be called before enabling caches
 * @note    Uses privileged default memory map for non-configured regions
 *
 * @see     ARMv7-M Architecture Reference Manual, Section B3.5 (MPU)
 * @see     STM32H750 Reference Manual RM0433, Section 2.3.4
 */
ITCM_FUNC_PRIV void __mpu_config(void);

/**
 * @brief Configure MPU for current task's data region
 * @param task_id Task identifier
 * @note Called from assembly context switch handler
 */
ITCM_FUNC_PRIV void __mpu_configure_task_data(uint32_t task_data_base);

#ifdef __cplusplus
}
#endif

#endif