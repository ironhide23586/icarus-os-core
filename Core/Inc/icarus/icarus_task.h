/**
 * @file    icarus_task.h
 * @brief   ICARUS OS API - Compatibility Header
 * @version 0.1.0
 *
 * @details This header provides backward compatibility by including all
 *          ICARUS kernel modules. New code should include specific headers.
 *
 * @deprecated Use specific headers instead:
 *             - icarus/kernel.h    - Kernel init and state
 *             - icarus/scheduler.h - Scheduling and sleep
 *             - icarus/task.h      - Task creation/lifecycle
 *             - icarus/semaphore.h - Semaphores
 *             - icarus/pipe.h      - Message pipes
 *
 * @author  Souham Biswas
 * @date    2025
 *
 * @copyright Copyright 2025 Souham Biswas
 *            https://github.com/ironhide23586/icarus-os-core
 *            Licensed under the Apache License, Version 2.0
 */

#ifndef ICARUS_TASK_H_COMPAT
#define ICARUS_TASK_H_COMPAT

/* Include all ICARUS modules for backward compatibility */
#include "icarus/kernel.h"
#include "icarus/scheduler.h"
#include "icarus/task.h"
#include "icarus/semaphore.h"
#include "icarus/pipe.h"

#endif /* ICARUS_TASK_H_COMPAT */
