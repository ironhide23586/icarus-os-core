/**
 * @file    mpu.c
 * @brief   Memory Protection Unit Configuration
 * @version 0.1.0
 *
 * @details Configures MPU regions for unprivileged task execution (CONTROL=0x03).
 *
 *          Region map:
 *          | # | Name          | Base       | Size  | Access              |
 *          |---|---------------|------------|-------|---------------------|
 *          | 0 | ITCM base     | 0x00000000 | 64KB  | Priv+User RO+exec   |
 *          | 1 | QSPI flash    | 0x90000000 | 8MB   | Priv+User RO+exec   |
 *          | 2 | Internal Flash| 0x08000000 | 128KB | Priv+User RO+exec   |
 *          | 3 | DTCM_OBC      | 0x20010000 | 64KB  | Priv+User RW         |
 *          | 4 | Task data     | dynamic    | 2KB   | Priv+User RW        |
 *          | 5 | DTCM          | 0x20000000 | 128KB | Priv RW (lower 64K) |
 *          | 6 | RAM_D1        | 0x24000000 | 512KB | Priv+User Full      |
 *          | 7 | Peripherals   | 0x40000000 | 512MB | Priv+User Device    |
 *
 *          * Region 3 uses subregion disable (0xFC) to only protect the first
 *            256 bytes (subregions 0-1: 0x000–0x1FF) where kernel handlers
 *            live. Subregions 2-7 (0x200–0x3FF) fall back to region 0
 *            (PRIV_RO_URO) because C library code may read from the linker
 *            padding area in unprivileged mode.
 *
 *          DTCM protection (Step 7b complete): All kernel data reads/writes
 *          use SVC call gates, so unprivileged tasks cannot corrupt kernel
 *          state (task_list, semaphore_list, message_pipe_list, os_tick_count).
 *
 * @author  Souham Biswas
 * @date    2025
 *
 * @copyright Copyright 2025-2026 Souham Biswas
 *            https://github.com/ironhide23586/icarus-os-core
 *            Licensed under the Apache License, Version 2.0
 */

#include "bsp/mpu.h"
#include "stm32h7xx.h"

/* ============================================================================
 * MPU CONFIGURATION
 * ========================================================================= */

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
 *          Enables MPU with privileged default background map to block
 *          unprivileged access to system control space (NVIC, SCB, etc.)
 *
 * @note    Called once during system initialization before tasks start
 * @note    Memory barriers (DSB/ISB) ensure MPU changes take effect
 */
void MPU_Config(void)
{
    MPU_Region_InitTypeDef r = {0};

    HAL_MPU_Disable();

    /* ---- Region 0: ITCM 64KB base — RO+exec for all ---- */
    /* Kernel functions in ITCM - read-only prevents code modification */
    /* Note: ARM MPU cannot prevent unprivileged EXECUTION of privileged code */
    /* Protection relies on exception mechanism (SVC/PendSV only via instructions) */
    r.Enable           = MPU_REGION_ENABLE;
    r.Number           = MPU_REGION_ITCM_BASE;
    r.BaseAddress      = BSP_ITCM_BASE;
    r.Size             = MPU_REGION_SIZE_64KB;
    r.AccessPermission = MPU_REGION_PRIV_RO_URO;  /* Read-only for all */
    r.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
    r.IsCacheable      = MPU_ACCESS_NOT_CACHEABLE;
    r.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
    r.DisableExec      = MPU_INSTRUCTION_ACCESS_ENABLE;
    r.TypeExtField     = MPU_TEX_LEVEL1;
    r.SubRegionDisable = 0x00;
    HAL_MPU_ConfigRegion(&r);

    /* ---- Region 1: QSPI 8MB flash — Priv+User RO+exec, cacheable ---- */
    r.Enable           = MPU_REGION_ENABLE;
    r.Number           = MPU_REGION_QSPI_FLASH;
    r.BaseAddress      = QSPI_BASE;
    r.Size             = MPU_REGION_SIZE_8MB;
    r.AccessPermission = MPU_REGION_PRIV_RO_URO;
    r.IsBufferable     = MPU_ACCESS_BUFFERABLE;
    r.IsCacheable      = MPU_ACCESS_CACHEABLE;
    r.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
    r.DisableExec      = MPU_INSTRUCTION_ACCESS_ENABLE;
    r.TypeExtField     = MPU_TEX_LEVEL1;
    r.SubRegionDisable = 0x00;
    HAL_MPU_ConfigRegion(&r);

    /* ---- Region 2: Internal Flash 128K — Priv+User RO+exec ---- */
    r.Enable           = MPU_REGION_ENABLE;
    r.Number           = MPU_REGION_FLASH;
    r.BaseAddress      = BSP_FLASH_BASE;
    r.Size             = MPU_REGION_SIZE_128KB;
    r.AccessPermission = MPU_REGION_PRIV_RO_URO;
    r.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
    r.IsCacheable      = MPU_ACCESS_CACHEABLE;
    r.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
    r.DisableExec      = MPU_INSTRUCTION_ACCESS_ENABLE;
    r.TypeExtField     = MPU_TEX_LEVEL1;
    r.SubRegionDisable = 0x00;
    HAL_MPU_ConfigRegion(&r);

    /* ---- Region 3: Upper DTCM 64KB — Priv+User RW (OBC hot data) ---- */
    /* Region 5 (DTCM 128KB PRIV_RW) disables subregions 4-7 so this       */
    /* lower-numbered region takes effect for the upper half.               */
    /* OBC application data tagged DTCM_DATA_OBC lands here for zero        */
    /* wait-state access from unprivileged task code.                       */
    r.Enable           = MPU_REGION_ENABLE;
    r.Number           = MPU_REGION_DTCM_OBC;
    r.BaseAddress      = BSP_DTCM_OBC_BASE;
    r.Size             = MPU_REGION_SIZE_64KB;
    r.AccessPermission = MPU_REGION_FULL_ACCESS;
    r.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
    r.IsCacheable      = MPU_ACCESS_NOT_CACHEABLE;
    r.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
    r.DisableExec      = MPU_INSTRUCTION_ACCESS_DISABLE;
    r.TypeExtField     = MPU_TEX_LEVEL1;
    r.SubRegionDisable = 0x00;
    HAL_MPU_ConfigRegion(&r);

    /* ---- Region 4: Task data (dynamic, configured per context switch) ---- */
    /* Left unconfigured here — set by MPU_ConfigureTaskData()              */

    /* ---- Region 5: DTCM 128K — PRIV_RW (kernel data, lower 64K only) ---- */
    /* Kernel data (.dtcm_priv) lives in the lower 64KB (subregions 0-3).   */
    /* Subregions 4-7 disabled so Region 3 (FULL_ACCESS) governs the upper  */
    /* 64KB where OBC application hot data (.dtcm_obc) is placed.           */
    r.Enable           = MPU_REGION_ENABLE;
    r.Number           = MPU_REGION_DTCM;
    r.BaseAddress      = BSP_DTCM_BASE;
    r.Size             = MPU_REGION_SIZE_128KB;
    r.AccessPermission = MPU_REGION_PRIV_RW;
    r.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
    r.IsCacheable      = MPU_ACCESS_NOT_CACHEABLE;
    r.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
    r.DisableExec      = MPU_INSTRUCTION_ACCESS_DISABLE;
    r.TypeExtField     = MPU_TEX_LEVEL1;
    r.SubRegionDisable = 0xF0;
    HAL_MPU_ConfigRegion(&r);

    /* ---- Region 6: RAM_D1 512K — Priv+User Full Access ---- */
    /* Stacks, .data, .bss, heap, task stacks/data pools.                   */
    r.Enable           = MPU_REGION_ENABLE;
    r.Number           = MPU_REGION_RAM_D1;
    r.BaseAddress      = BSP_RAM_D1_BASE;
    r.Size             = MPU_REGION_SIZE_512KB;
    r.AccessPermission = MPU_REGION_FULL_ACCESS;
    r.IsBufferable     = MPU_ACCESS_BUFFERABLE;
    r.IsCacheable      = MPU_ACCESS_CACHEABLE;
    r.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
    r.DisableExec      = MPU_INSTRUCTION_ACCESS_DISABLE;
    r.TypeExtField     = MPU_TEX_LEVEL1;
    r.SubRegionDisable = 0x00;
    HAL_MPU_ConfigRegion(&r);

    /* ---- Region 7: Peripherals 512MB — Priv+User Full Access, Device ---- */
    /* GPIO, USB, SPI, I2C, UART, etc.                                      */
    r.Enable           = MPU_REGION_ENABLE;
    r.Number           = MPU_REGION_PERIPH;
    r.BaseAddress      = BSP_PERIPH_BASE;
    r.Size             = MPU_REGION_SIZE_512MB;
    r.AccessPermission = MPU_REGION_FULL_ACCESS;
    r.IsBufferable     = MPU_ACCESS_BUFFERABLE;
    r.IsCacheable      = MPU_ACCESS_NOT_CACHEABLE;
    r.IsShareable      = MPU_ACCESS_SHAREABLE;
    r.DisableExec      = MPU_INSTRUCTION_ACCESS_DISABLE;
    r.TypeExtField     = MPU_TEX_LEVEL0;
    r.SubRegionDisable = 0x00;
    HAL_MPU_ConfigRegion(&r);

    /* Enable MPU with privileged default map as background fallback.       */
    /* MPU_PRIVILEGED_DEFAULT blocks unprivileged access to SCS (0xE000E000)*/
    /* — NVIC, SCB, SysTick, MPU registers — forcing SVC for those writes.  */
    HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
    
    /* Memory barriers required after MPU configuration (ARM Cortex-M7)
     * DSB: Ensure all memory accesses complete before MPU change takes effect
     * ISB: Flush instruction pipeline to ensure new MPU config is used */
    __DSB();
    __ISB();
}

/* ============================================================================
 * DYNAMIC TASK DATA REGION
 * ========================================================================= */

/**
 * @brief Configure MPU region 4 for the current task's data section
 * @param task_data_base Base address of the task's data pool
 * @note  Called from assembly context switch handler (os_yield_pendsv)
 */
void MPU_ConfigureTaskData(uint32_t task_data_base)
{
    MPU_Region_InitTypeDef r = {0};

    r.Enable           = MPU_REGION_ENABLE;
    r.Number           = MPU_REGION_TASK_DATA;
    r.BaseAddress      = task_data_base;
    r.Size             = TASK_DATA_SIZE_MPU;
    r.AccessPermission = MPU_REGION_FULL_ACCESS;
    r.IsCacheable      = MPU_ACCESS_CACHEABLE;
    r.IsBufferable     = MPU_ACCESS_BUFFERABLE;
    r.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
    r.DisableExec      = MPU_INSTRUCTION_ACCESS_DISABLE;
    r.TypeExtField     = MPU_TEX_LEVEL0;
    r.SubRegionDisable = 0x00;

    HAL_MPU_ConfigRegion(&r);
    
    /* Memory barriers required after MPU reconfiguration (ARM Cortex-M7)
     * DSB: Ensure all memory accesses complete before MPU change takes effect
     * ISB: Flush instruction pipeline to ensure new MPU config is used */
    __DSB();
    __ISB();
}
