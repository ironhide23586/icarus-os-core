/**
 * @file    demo_tasks_posix.h
 * @brief   POSIX API Demo Tasks Header
 * @version 0.1.0
 *
 * @details Demonstration tasks using POSIX-compatible APIs.
 *          Shows usage of pthread, sem_*, and mq_* functions.
 *
 * @see     Core/Src/demo_tasks_posix.c for implementation
 * @see     Core/Inc/icarus/posix/posix.h for POSIX API
 *
 * @author  Souham Biswas
 * @date    2026
 *
 * @copyright Copyright (c) 2026 ICARUS Project
 */

#ifndef DEMO_TASKS_POSIX_H_
#define DEMO_TASKS_POSIX_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize and start all POSIX demo tasks
 *
 * @details Initializes POSIX subsystem and creates demo threads:
 *          - Semaphore producer/consumer demo (sem_wait/sem_post)
 *          - Reference task
 *          - Message queue demos using mq_send/mq_receive
 *
 * @note Must be called after os_init()
 */
void demo_tasks_posix_init(void);

#ifdef __cplusplus
}
#endif

#endif /* DEMO_TASKS_POSIX_H_ */
