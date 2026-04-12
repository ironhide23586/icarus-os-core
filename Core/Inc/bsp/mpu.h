
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

/** @brief Upper DTCM for OBC application hot data (64KB, unprivileged RW) */
#define BSP_DTCM_OBC_BASE      0x20010000UL
#define BSP_DTCM_OBC_SIZE      (64 * 1024)
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

#define MPU_REGION_ITCM_BASE         MPU_REGION_NUMBER0
#define MPU_REGION_QSPI_FLASH       MPU_REGION_NUMBER1
#define MPU_REGION_FLASH             MPU_REGION_NUMBER2
#define MPU_REGION_DTCM_OBC         MPU_REGION_NUMBER3
#define MPU_REGION_TASK_DATA         MPU_REGION_NUMBER4
#define MPU_REGION_DTCM              MPU_REGION_NUMBER5
#define MPU_REGION_RAM_D1            MPU_REGION_NUMBER6
#define MPU_REGION_PERIPH            MPU_REGION_NUMBER7


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
#define TASK_DATA_SIZE_BYTES ((ICARUS_DATA_WORDS) * 4u)
#define TASK_DATA_SIZE_MPU   MPU_REGION_SIZE_2KB

#ifndef SKIP_STATIC_ASSERTS
_Static_assert(TASK_DATA_SIZE_BYTES == 2048u,
               "TASK_DATA_SIZE_BYTES changed — update TASK_DATA_SIZE_MPU");
#endif

/**
 * @brief   Configure Memory Protection Unit regions
 *
 * @details Sets up 8 MPU regions for memory protection:
 *          - Region 0: ITCM (read-only for all, prevents code modification)
 *          - Region 1: QSPI Flash (read-only, cacheable)
 *          - Region 2: Internal Flash (read-only, cacheable)
 *          - Region 3: DISABLED (consolidated with Region 0)
 *          - Region 4: Task Data (dynamic, configured per context switch)
 *          - Region 5: DTCM (privileged-only, protects kernel data)
 *          - Region 6: RAM_D1 (shared buffers, full access)
 *          - Region 7: Peripherals (device memory, full access)
 *
 * @par Region Configuration:
 *      | Region | Base       | Size  | Access         | Purpose                |
 *      |--------|------------|-------|----------------|------------------------|
 *      | 0      | 0x00000000 | 64KB  | Priv+User RO   | ITCM code protection   |
 *      | 1      | 0x90000000 | 8MB   | Priv+User RO   | QSPI Flash             |
 *      | 2      | 0x08000000 | 128KB | Priv+User RO   | Internal Flash         |
 *      | 3      | 0x20010000 | 64KB  | Full Access    | DTCM OBC hot data      |
 *      | 4      | Dynamic    | 2KB   | Priv+User RW   | Task data isolation    |
 *      | 5      | 0x20000000 | 128KB | Priv RW        | DTCM kernel data       |
 *      | 6      | 0x24000000 | 512KB | Full Access    | RAM_D1 shared buffers  |
 *      | 7      | 0x40000000 | 512MB | Full Access    | Peripherals            |
 *
 * @note    Called once during system initialization before tasks start
 * @note    Enables MPU with privileged default background map
 * @note    Memory barriers (DSB/ISB) ensure MPU changes take effect
 *
 * @see     ARMv7-M Architecture Reference Manual, Section B3.5 (MPU)
 * @see     STM32H750 Reference Manual RM0433, Section 2.3.4
 */
void MPU_Config(void);

/**
 * @brief   Configure MPU Region 4 for current task's data region
 *
 * @param   task_data_base Base address of task's 2KB data region (must be 2KB-aligned)
 *
 * @details Reconfigures MPU Region 4 to grant the current task exclusive access
 *          to its data region. This provides task data isolation - each task can
 *          only access its own 2KB data region, preventing cross-task corruption.
 *
 * @note    Called from assembly context switch handler (os_yield_pendsv)
 * @note    Memory barriers (DSB/ISB) ensure MPU changes take effect before task runs
 * @note    Task data regions are allocated from data_pool in RAM_D2 (0x30000000)
 *
 * @see     MPU_Config() for initial MPU setup
 */
void MPU_ConfigureTaskData(uint32_t task_data_base);

#ifdef __cplusplus
}
#endif

#endif