/**
 * @file    svc.h
 * @brief   ICARUS Supervisor Call (SVC) Definitions
 * @version 0.1.0
 *
 * @details Defines SVC numbers for MPU-protected kernel functions.
 *          These functions execute in privileged mode with controlled
 *          access to protected kernel data structures.
 *
 *          Organization:
 *          - This header defines SVC numbers
 *          - svc.c contains all SVC wrapper implementations
 *          - Implementation files contain __prefixed privileged functions
 *
 * @author  Souham Biswas
 * @date    2025
 *
 * @copyright Copyright 2025 Souham Biswas
 *            https://github.com/ironhide23586/icarus-os-core
 *            Licensed under the Apache License, Version 2.0
 */

#ifndef ICARUS_SVC_H
#define ICARUS_SVC_H


#ifdef __cplusplus
extern "C" {
#endif



#define SVC_TASK_ACTIVE_SLEEP           0
#define SVC_TASK_BLOCKING_SLEEP         1

#define SVC_ENTER_CRITICAL              2
#define SVC_EXIT_CRITICAL               3

#define SVC_PIPE_INIT                   4
#define SVC_PIPE_ENQUEUE                5
#define SVC_PIPE_DEQUEUE                6

#define SVC_OS_INIT                     7
#define SVC_OS_START                    8
#define SVC_OS_YIELD                    9
#define SVC_OS_REGISTER_TASK            10
#define SVC_OS_GET_CURRENT_TASK_NAME    11
#define SVC_OS_GET_TICK_COUNT           12
#define SVC_OS_EXIT_TASK                13
#define SVC_OS_KILL_PROCESS             14
#define SVC_OS_TASK_SUICIDE             15

#define SVC_SEMAPHORE_INIT              16
#define SVC_SEMAPHORE_CONSUME           17
#define SVC_SEMAPHORE_FEED              18
#define SVC_SEMAPHORE_GET_COUNT         19
#define SVC_SEMAPHORE_GET_MAX_COUNT     20

#define SVC_PIPE_GET_COUNT              21
#define SVC_PIPE_GET_MAX_COUNT          22

#define SVC_TASK_BUSY_WAIT              23
#define SVC_OS_GET_RUNNING_TASK_COUNT   24
#define SVC_OS_GET_TASK_TICKS_REMAINING 25

#define SVC_KERNEL_GET_STACK            26
#define SVC_KERNEL_GET_DATA             27
#define SVC_KERNEL_PROTECTED_DATA       28




#ifdef __cplusplus
}
#endif

#endif /* ICARUS_SVC_H */