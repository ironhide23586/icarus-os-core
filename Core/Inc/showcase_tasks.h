/**
 * @file    showcase_tasks.h
 * @brief   Demo tasks exercising the shared kernel modules end-to-end
 *
 * @details Drives crc / cdc_rx / event / fs / tables together in a single
 *          self-contained scenario. Intended to give a smoke-test of the
 *          new kernel infrastructure on real hardware after the
 *          ITCM / DTCM_PRIV / SVC-gate hardening work.
 *
 * @author  Souham Biswas
 * @date    2026
 *
 * @copyright Copyright 2025-2026 Souham Biswas
 *            https://github.com/ironhide23586/icarus-os-core
 *            Licensed under the Apache License, Version 2.0
 */

#ifndef ICARUS_SHOWCASE_TASKS_H
#define ICARUS_SHOWCASE_TASKS_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Register the showcase tasks with the scheduler.
 * @note   Call after os_init() and before os_start().
 */
void showcase_tasks_init(void);

#ifdef __cplusplus
}
#endif

#endif /* ICARUS_SHOWCASE_TASKS_H */
