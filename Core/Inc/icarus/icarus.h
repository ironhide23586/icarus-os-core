/**
 * @file    icarus.h
 * @brief   ICARUS OS - Main API Header
 * @version 0.1.0
 *
 * @details Single umbrella header for user task code. Include this in your
 *          application tasks to access all ICARUS kernel functionality.
 *
 * @author  Souham Biswas
 * @date    2025
 *
 * @copyright Copyright 2025-2026 Souham Biswas
 *            https://github.com/ironhide23586/icarus-os-core
 *            Licensed under the Apache License, Version 2.0
 */

#ifndef ICARUS_H
#define ICARUS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Core kernel modules */
#include "icarus/kernel.h"
#include "icarus/scheduler.h"
#include "icarus/task.h"
#include "icarus/semaphore.h"
#include "icarus/pipe.h"
#include "icarus/cdc_rx.h"

#ifdef __cplusplus
}
#endif

#endif /* ICARUS_H */
