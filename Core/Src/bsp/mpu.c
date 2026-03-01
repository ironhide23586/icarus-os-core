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
 *          | 0 | ITCM_PRIV     | 0x00000000 | 256B  | Priv RO+exec only   |
 *          | 1 | QSPI flash    | 0x90000000 | 8MB   | Priv+User RO+exec   |
 *          | 2 | Internal Flash| 0x08000000 | 128KB | Priv+User RO+exec   |
 *          | 3 | ITCM_USER     | 0x00000400 | 1KB   | Priv+User RO+exec   |
 *          | 4 | Task data     | dynamic    | 2KB   | Priv+User RW        |
 *          | 5 | DTCM          | 0x20000000 | 128KB | Priv+User Full      |
 *          | 6 | RAM_D1        | 0x24000000 | 512KB | Priv+User Full      |
 *          | 7 | Peripherals   | 0x40000000 | 512MB | Priv+User Device    |
 *
 *          DTCM is currently full access because spinning functions
 *          (__semaphore_feed/consume, __pipe_enqueue/dequeue, __task_busy_wait)
 *          read kernel data (os_tick_count, semaphore_list, message_pipe_list)
 *          directly from unprivileged thread mode. Step 7 will redesign these
 *          to go through SVC, at which point DTCM can be tightened to priv-only.
 *
 * @author  Souham Biswas
 * @date    2025
 *
 * @copyright Copyright 2025 Souham Biswas
 *            https://github.com/ironhide23586/icarus-os-core
 *            Licensed under the Apache License, Version 2.0
 */

#include "bsp/mpu.h"
#include "stm32h7xx.h"

/* ============================================================================
 * LINKER SYMBOLS — ITCM SECTION BOUNDARIES
 * ========================================================================= */

/* Provided by STM32H750VBTX_FLASH.ld */
extern uint32_t _sitcm_priv;   /* start of .itcm_priv in ITCM */
extern uint32_t _eitcm_priv;   /* end   of .itcm_priv in ITCM */
extern uint32_t _sitcm_user;   /* start of .itcm_user in ITCM */
extern uint32_t _eitcm_user;   /* end   of .itcm_user in ITCM */

/* ============================================================================
 * MPU CONFIGURATION
 * ========================================================================= */

void MPU_Config(void) {
    MPU_Region_InitTypeDef r = {0};

    HAL_MPU_Disable();

    /* ---- Region 0: ITCM_PRIV 256B — Priv-only RO+exec ---- */
    /* SVC_Handler, PendSV_Handler, __os_yield, __task_active_sleep.        */
    /* Unprivileged code cannot read or execute this region.                */
    /* Size fixed at 256B (next power-of-2 above actual 160B content).      */
    r.Enable           = MPU_REGION_ENABLE;
    r.Number           = MPU_REGION_ITCM_PRIV;
    r.BaseAddress      = (uint32_t)&_sitcm_priv;
    r.Size             = MPU_REGION_SIZE_256B;
    r.AccessPermission = MPU_REGION_PRIV_RO;
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

    /* ---- Region 3: ITCM_USER 1KB — Priv+User RO+exec ---- */
    /* Spinning wrappers, context switch (os_yield_pendsv).                 */
    /* Linker script pads .itcm_priv to 1KB so _sitcm_user = 0x00000400.   */
    /* MPU requires base address to be a multiple of region size (1KB).     */
    r.Enable           = MPU_REGION_ENABLE;
    r.Number           = MPU_REGION_ITCM_USER;
    r.BaseAddress      = (uint32_t)&_sitcm_user;
    r.Size             = MPU_REGION_SIZE_1KB;
    r.AccessPermission = MPU_REGION_PRIV_RO_URO;
    r.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
    r.IsCacheable      = MPU_ACCESS_NOT_CACHEABLE;
    r.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
    r.DisableExec      = MPU_INSTRUCTION_ACCESS_ENABLE;
    r.TypeExtField     = MPU_TEX_LEVEL1;
    r.SubRegionDisable = 0x00;
    HAL_MPU_ConfigRegion(&r);

    /* ---- Region 4: Task data (dynamic, configured per context switch) ---- */
    /* Left unconfigured here — set by MPU_ConfigureTaskData()              */

    /* ---- Region 5: DTCM 128K — Priv+User Full Access ---- */
    /* Kernel data (.dtcm_priv) lives here. Full access required until      */
    /* Step 7 redesigns spinning functions to access kernel data via SVC.   */
    r.Enable           = MPU_REGION_ENABLE;
    r.Number           = MPU_REGION_DTCM;
    r.BaseAddress      = BSP_DTCM_BASE;
    r.Size             = MPU_REGION_SIZE_128KB;
    r.AccessPermission = MPU_REGION_FULL_ACCESS;
    r.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
    r.IsCacheable      = MPU_ACCESS_NOT_CACHEABLE;
    r.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
    r.DisableExec      = MPU_INSTRUCTION_ACCESS_DISABLE;
    r.TypeExtField     = MPU_TEX_LEVEL1;
    r.SubRegionDisable = 0x00;
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
}

/* ============================================================================
 * DYNAMIC TASK DATA REGION
 * ========================================================================= */

/**
 * @brief Configure MPU region 4 for the current task's data section
 * @param task_data_base Base address of the task's data pool
 * @note  Called from assembly context switch handler (os_yield_pendsv)
 */
void MPU_ConfigureTaskData(uint32_t task_data_base) {
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
}
