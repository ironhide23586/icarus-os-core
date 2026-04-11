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
 * @copyright Copyright 2025-2026 Souham Biswas
 *            https://github.com/ironhide23586/icarus-os-core
 *            Licensed under the Apache License, Version 2.0
 */

#ifndef ICARUS_SVC_H
#define ICARUS_SVC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

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

/* Call gates for spinning functions — read kernel state from priv mode      */
/* so DTCM can be made priv-only without faulting the spin loops             */
#define SVC_SEM_CAN_FEED                29  /* bool: count < max && engaged  */
#define SVC_SEM_CAN_CONSUME             30  /* bool: count > 0  && engaged   */
#define SVC_PIPE_CAN_ENQUEUE            31  /* bool: free >= bytes && engaged */
#define SVC_PIPE_CAN_DEQUEUE            32  /* bool: count >= bytes && engaged*/

/* Write gates for spinning functions — modify kernel state from priv mode   */
#define SVC_SEM_INCREMENT               33  /* ++count, update tick          */
#define SVC_SEM_DECREMENT               34  /* --count, update tick          */
#define SVC_PIPE_WRITE_BYTES            35  /* write to buffer, update tick  */
#define SVC_PIPE_READ_BYTES             36  /* read from buffer, update tick */

/* Read gates for display/diagnostics — read task metadata from priv mode    */
#define SVC_GET_TASK_NAME               37  /* const char*: task_list[i]->name */
#define SVC_GET_NUM_TASKS               38  /* uint8_t: num_created_tasks    */
#define SVC_OS_IS_RUNNING               39  /* uint8_t: os_running flag      */

/* CDC RX ring buffer (data in DTCM_PRIV)                                    */
#define SVC_CDC_RX_INIT                 40  /* clear head/tail               */
#define SVC_CDC_RX_READ_BYTE            41  /* bool: pop one byte if any     */
#define SVC_CDC_RX_AVAILABLE            42  /* uint32_t: bytes available     */

/* Event ring buffer + per-module squelch (data in DTCM_PRIV)                */
#define SVC_EVENT_INIT                  43  /* clear ring + squelch          */
#define SVC_OS_EVENT                    44  /* emit one entry                */
#define SVC_EVENT_SET_SQUELCH           45  /* set per-module threshold      */
#define SVC_EVENT_GET_SQUELCH           46  /* read per-module threshold     */
#define SVC_EVENT_DRAIN                 47  /* copy entries into caller buf  */
#define SVC_EVENT_GET_COUNT             48  /* uint32_t: ring fill           */

/* Ground-loadable table engine (data in DTCM_PRIV)                          */
#define SVC_TBL_INIT                    49  /* clear registry                */
#define SVC_TBL_REGISTER                50  /* add a descriptor              */
#define SVC_TBL_LOAD                    51  /* append bytes to staging       */
#define SVC_TBL_ACTIVATE_PREPARE        52  /* validate + copy staging→tmp   */
#define SVC_TBL_ACTIVATE_COMMIT         53  /* copy tmp→active               */
#define SVC_TBL_DUMP                    54  /* copy active→caller buf        */
#define SVC_TBL_GET_DESCRIPTOR          55  /* return descriptor pointer     */
#define SVC_TBL_COUNT                   56  /* uint8_t: registered count     */

/* Task lifecycle extensions                                                 */
#define SVC_OS_RESTART_TASK             57  /* restart killed/finished task   */

/* Timed semaphore wait                                                      */
#define SVC_SEMAPHORE_CONSUME_TIMEOUT   58  /* bool: acquire with timeout    */

/* Task diagnostics                                                          */
#define SVC_GET_TASK_STATE              59  /* icarus_task_state_t           */
#define SVC_GET_TASK_DISPATCH_COUNT     60  /* uint32_t: dispatch counter    */
#define SVC_GET_STACK_WATERMARK         61  /* uint32_t: min free words      */
#define SVC_UPDATE_STACK_WATERMARK      62  /* void: scan + update           */

/* ============================================================================
 * COMPILE-TIME SVC VALIDATION
 * ========================================================================= */

/* SVC instruction encodes number in 1 byte (0-255) */
_Static_assert(SVC_UPDATE_STACK_WATERMARK <= 255,
               "Highest SVC number must fit in 8-bit immediate");

_Static_assert(SVC_UPDATE_STACK_WATERMARK >= SVC_KERNEL_PROTECTED_DATA,
               "SVC_UPDATE_STACK_WATERMARK must be >= all other SVC numbers");

/* ============================================================================
 * SVC HANDLER (called from assembly - target only)
 * ========================================================================= */

#ifndef HOST_TEST
/**
 * @brief C-level SVC dispatcher called from SVC_Handler assembly
 * @param stack_frame Pointer to exception stack frame
 */
void SVC_Handler_C(uint32_t *stack_frame);
#endif

#ifdef __cplusplus
}
#endif

#endif /* ICARUS_SVC_H */
