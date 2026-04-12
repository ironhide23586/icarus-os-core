/**
 * @file    svc.c
 * @brief   ICARUS Supervisor Call (SVC) Implementation
 * @version 0.1.0
 *
 * @details SVC handler and public wrapper functions for MPU-protected kernel
 *          calls. Non-spinning wrappers issue SVC instructions to execute in
 *          privileged handler mode. Spinning wrappers (semaphore_feed/consume,
 *          pipe_enqueue/dequeue, task_blocking_sleep/busy_wait) call __func
 *          directly in thread mode — they cannot run inside the SVC handler
 *          because PendSV is lower priority and cannot preempt it.
 *
 * @author  Souham Biswas
 * @date    2025
 *
 * @copyright Copyright 2025-2026 Souham Biswas
 *            https://github.com/ironhide23586/icarus-os-core
 *            Licensed under the Apache License, Version 2.0
 */

#include "icarus/svc.h"
#include "icarus/kernel.h"
#include "icarus/task.h"
#include "icarus/scheduler.h"
#include "icarus/pipe.h"
#include "icarus/semaphore.h"
#include "icarus/cdc_rx.h"
#include "icarus/event.h"
#include "icarus/tables.h"
#include "icarus/cs.h"
#include "icarus/sb.h"
#include "icarus/fs.h"
#include "bsp/mpu.h"
#include <stddef.h>
#include <string.h>

/* ============================================================================
 * SVC HANDLER (target only)
 * ========================================================================= */

#ifndef HOST_TEST

/**
 * @brief C-level SVC dispatcher called from SVC_Handler assembly trampoline
 * @param stack_frame Pointer to exception stack frame (R0-R3, R12, LR, PC, xPSR)
 *
 * @note SVC number is extracted from the SVC instruction encoding:
 *       The SVC instruction is at [PC - 2], and the immediate is the low byte.
 */
void SVC_Handler_C(uint32_t *stack_frame) {
    uint8_t svc_number = ((uint8_t *)(uintptr_t)stack_frame[6])[-2];
    uint32_t arg0 = stack_frame[0];
    uint32_t arg1 = stack_frame[1];

    switch (svc_number) {

        /* Critical section */
        case SVC_ENTER_CRITICAL:
            __enter_critical();
            break;
        case SVC_EXIT_CRITICAL:
            __exit_critical();
            break;

        /* Scheduler */
        case SVC_OS_YIELD:
            __os_yield();
            break;
        case SVC_TASK_ACTIVE_SLEEP:
            stack_frame[0] = __task_active_sleep(arg0);
            break;
        case SVC_OS_GET_TICK_COUNT:
            stack_frame[0] = __os_get_tick_count();
            break;
        case SVC_OS_GET_CURRENT_TASK_NAME: {
            const char *name = __os_get_current_task_name();
            stack_frame[0] = (uint32_t)(uintptr_t)name;
            break;
        }
        case SVC_OS_GET_RUNNING_TASK_COUNT:
            stack_frame[0] = __os_get_running_task_count();
            break;
        case SVC_OS_GET_TASK_TICKS_REMAINING:
            stack_frame[0] = __os_get_task_ticks_remaining();
            break;

        /* Task lifecycle */
        case SVC_OS_REGISTER_TASK:
            __os_register_task((void (*)(void))(uintptr_t)arg0,
                               (const char *)(uintptr_t)arg1);
            break;
        case SVC_OS_EXIT_TASK:
            __os_exit_task();
            break;
        case SVC_OS_KILL_PROCESS:
            __os_kill_process((uint8_t)arg0);
            break;
        case SVC_OS_TASK_SUICIDE:
            __os_task_suicide();
            break;

        /* Kernel data */
        case SVC_KERNEL_GET_STACK: {
            uint32_t *ptr = __kernel_get_stack((uint8_t)arg0);
            stack_frame[0] = (uint32_t)(uintptr_t)ptr;
            break;
        }
        case SVC_KERNEL_GET_DATA: {
            uint32_t *ptr = __kernel_get_data((uint8_t)arg0);
            stack_frame[0] = (uint32_t)(uintptr_t)ptr;
            break;
        }
        case SVC_KERNEL_PROTECTED_DATA: {
            void *ptr = __kernel_protected_data((uint16_t)arg0);
            stack_frame[0] = (uint32_t)(uintptr_t)ptr;
            break;
        }

        /* Semaphore (non-spinning) */
        case SVC_SEMAPHORE_INIT: {
            bool ret = __semaphore_init((uint8_t)arg0, arg1);
            stack_frame[0] = (uint32_t)ret;
            break;
        }
        case SVC_SEMAPHORE_GET_COUNT:
            stack_frame[0] = __semaphore_get_count((uint8_t)arg0);
            break;
        case SVC_SEMAPHORE_GET_MAX_COUNT:
            stack_frame[0] = __semaphore_get_max_count((uint8_t)arg0);
            break;

        /* Pipe (non-spinning) */
        case SVC_PIPE_INIT: {
            bool ret = __pipe_init((uint8_t)arg0, (uint8_t)arg1);
            stack_frame[0] = (uint32_t)ret;
            break;
        }
        case SVC_PIPE_GET_COUNT:
            stack_frame[0] = (uint32_t)__pipe_get_count((uint8_t)arg0);
            break;
        case SVC_PIPE_GET_MAX_COUNT:
            stack_frame[0] = (uint32_t)__pipe_get_max_count((uint8_t)arg0);
            break;

        /* Spinning functions — not dispatched via SVC, run in thread mode */
        case SVC_SEMAPHORE_FEED:
        case SVC_SEMAPHORE_CONSUME:
        case SVC_PIPE_ENQUEUE:
        case SVC_PIPE_DEQUEUE:
        case SVC_TASK_BUSY_WAIT:
        case SVC_TASK_BLOCKING_SLEEP:
            break;

        /* Call gates for spinning functions — read kernel state atomically */
        case SVC_SEM_CAN_FEED: {
            bool r = __sem_can_feed((uint8_t)arg0);
            stack_frame[0] = r ? 1u : 0u;
            break;
        }
        case SVC_SEM_CAN_CONSUME: {
            bool r = __sem_can_consume((uint8_t)arg0);
            stack_frame[0] = r ? 1u : 0u;
            break;
        }
        case SVC_PIPE_CAN_ENQUEUE: {
            bool r = __pipe_can_enqueue((uint8_t)arg0, (uint8_t)arg1);
            stack_frame[0] = r ? 1u : 0u;
            break;
        }
        case SVC_PIPE_CAN_DEQUEUE: {
            bool r = __pipe_can_dequeue((uint8_t)arg0, (uint8_t)arg1);
            stack_frame[0] = r ? 1u : 0u;
            break;
        }

        /* Write gates for spinning functions — modify kernel state atomically */
        case SVC_SEM_INCREMENT:
            __sem_increment((uint8_t)arg0);
            break;
        case SVC_SEM_DECREMENT:
            __sem_decrement((uint8_t)arg0);
            break;
        case SVC_PIPE_WRITE_BYTES:
            __pipe_write_bytes((uint8_t)arg0, (uint8_t *)(uintptr_t)arg1,
                               (uint8_t)stack_frame[2]);
            break;
        case SVC_PIPE_READ_BYTES:
            __pipe_read_bytes((uint8_t)arg0, (uint8_t *)(uintptr_t)arg1,
                              (uint8_t)stack_frame[2]);
            break;

        /* Task metadata read gates — for display/diagnostics from unprivileged mode */
        case SVC_GET_TASK_NAME: {
            const char *name = __os_get_task_name((uint8_t)arg0);
            stack_frame[0] = (uint32_t)(uintptr_t)name;
            break;
        }
        case SVC_GET_NUM_TASKS:
            stack_frame[0] = (uint32_t)__os_get_num_created_tasks();
            break;
        case SVC_OS_IS_RUNNING:
            stack_frame[0] = (uint32_t)__os_is_running();
            break;

        /* CDC RX ring buffer */
        case SVC_CDC_RX_INIT:
            __cdc_rx_init();
            break;
        case SVC_CDC_RX_READ_BYTE: {
            bool ok = __cdc_rx_read_byte((uint8_t *)(uintptr_t)arg0);
            stack_frame[0] = (uint32_t)ok;
            break;
        }
        case SVC_CDC_RX_AVAILABLE:
            stack_frame[0] = __cdc_rx_available();
            break;

        /* Event ring + squelch */
        case SVC_EVENT_INIT:
            __event_init();
            break;
        case SVC_OS_EVENT: {
            /* arg0 packs (event_id << 16) | (severity << 8) | module_id
             * arg1 = payload pointer
             * stack_frame[2] = payload_len
             * Packing keeps the SVC wrapper inside the AAPCS R0-R3 budget. */
            uint8_t  module_id   = (uint8_t)(arg0 & 0xFF);
            uint8_t  severity    = (uint8_t)((arg0 >> 8) & 0xFF);
            uint16_t event_id    = (uint16_t)((arg0 >> 16) & 0xFFFF);
            const void *payload  = (const void *)(uintptr_t)arg1;
            uint8_t payload_len  = (uint8_t)stack_frame[2];
            __os_event(module_id, (event_severity_t)severity, event_id,
                       payload, payload_len);
            break;
        }
        case SVC_EVENT_SET_SQUELCH:
            __event_set_squelch((uint8_t)arg0, (event_severity_t)arg1);
            break;
        case SVC_EVENT_GET_SQUELCH: {
            event_severity_t sq = __event_get_squelch((uint8_t)arg0);
            stack_frame[0] = (uint32_t)sq;
            break;
        }
        case SVC_EVENT_DRAIN: {
            bool ok = __event_drain(
                (event_entry_t *)(uintptr_t)arg0,
                (uint8_t)arg1,
                (uint8_t *)(uintptr_t)stack_frame[2]);
            stack_frame[0] = (uint32_t)ok;
            break;
        }
        case SVC_EVENT_GET_COUNT:
            stack_frame[0] = __event_get_count();
            break;

        /* Ground-loadable table engine */
        case SVC_TBL_INIT:
            __tbl_init();
            break;
        case SVC_TBL_REGISTER: {
            bool ok = __tbl_register(
                (const tbl_descriptor_t *)(uintptr_t)arg0);
            stack_frame[0] = (uint32_t)ok;
            break;
        }
        case SVC_TBL_LOAD: {
            bool ok = __tbl_load(
                (tbl_id_t)arg0,
                (const uint8_t *)(uintptr_t)arg1,
                (uint16_t)stack_frame[2],
                (uint16_t)stack_frame[3]);
            stack_frame[0] = (uint32_t)ok;
            break;
        }
        case SVC_TBL_ACTIVATE_PREPARE: {
            bool ok = __tbl_activate_prepare(
                (tbl_id_t)arg0,
                (uint8_t *)(uintptr_t)arg1,
                (uint16_t *)(uintptr_t)stack_frame[2],
                (tbl_activate_fn *)(uintptr_t)stack_frame[3]);
            stack_frame[0] = (uint32_t)ok;
            break;
        }
        case SVC_TBL_ACTIVATE_COMMIT: {
            bool ok = __tbl_activate_commit(
                (tbl_id_t)arg0,
                (const uint8_t *)(uintptr_t)arg1,
                (uint16_t)stack_frame[2]);
            stack_frame[0] = (uint32_t)ok;
            break;
        }
        case SVC_TBL_DUMP: {
            int16_t n = __tbl_dump(
                (tbl_id_t)arg0,
                (uint8_t *)(uintptr_t)arg1,
                (uint16_t)stack_frame[2]);
            stack_frame[0] = (uint32_t)(int32_t)n;
            break;
        }
        case SVC_TBL_GET_DESCRIPTOR: {
            const tbl_descriptor_t *d = __tbl_get_descriptor((tbl_id_t)arg0);
            stack_frame[0] = (uint32_t)(uintptr_t)d;
            break;
        }
        case SVC_TBL_COUNT:
            stack_frame[0] = (uint32_t)__tbl_count();
            break;

        /* ---- Task lifecycle extensions ---- */
        case SVC_OS_RESTART_TASK:
            __os_restart_task((uint8_t)arg0);
            break;

        /* ---- Timed semaphore ---- */
        case SVC_SEMAPHORE_CONSUME_TIMEOUT: {
            bool ok = __semaphore_consume_timeout((uint8_t)arg0, arg1);
            stack_frame[0] = ok ? 1u : 0u;
            break;
        }

        /* ---- Task diagnostics ---- */
        case SVC_GET_TASK_STATE: {
            icarus_task_state_t st = __os_get_task_state((uint8_t)arg0);
            stack_frame[0] = (uint32_t)st;
            break;
        }
        case SVC_GET_TASK_DISPATCH_COUNT:
            stack_frame[0] = __os_get_task_dispatch_count((uint8_t)arg0);
            break;
        case SVC_GET_STACK_WATERMARK:
            stack_frame[0] = __os_get_stack_watermark((uint8_t)arg0);
            break;
        case SVC_UPDATE_STACK_WATERMARK:
            __os_update_stack_watermark((uint8_t)arg0);
            break;

        /* ---- Checksum integrity monitor ---- */
        case SVC_CS_INIT:
            __cs_init();
            break;
        case SVC_CS_SET_CALLBACK:
            __cs_set_callback((cs_mismatch_fn)(uintptr_t)arg0);
            break;
        case SVC_CS_ADD_REGION: {
            bool ok = __cs_add_region((uint8_t)arg0,
                                       (const uint8_t *)(uintptr_t)arg1,
                                       stack_frame[2]);
            stack_frame[0] = (uint32_t)ok;
            break;
        }
        case SVC_CS_ENABLE: {
            bool ok = __cs_enable((uint8_t)arg0, (bool)arg1);
            stack_frame[0] = (uint32_t)ok;
            break;
        }
        case SVC_CS_REBASELINE: {
            bool ok = __cs_rebaseline((uint8_t)arg0);
            stack_frame[0] = (uint32_t)ok;
            break;
        }
        case SVC_CS_CHECK_ALL:
            stack_frame[0] = (uint32_t)__cs_check_all();
            break;
        case SVC_CS_GET_REGION: {
            bool ok = __cs_get_region((uint8_t)arg0,
                                       (cs_region_t *)(uintptr_t)arg1);
            stack_frame[0] = (uint32_t)ok;
            break;
        }
        case SVC_CS_REGION_COUNT:
            stack_frame[0] = (uint32_t)__cs_region_count();
            break;

        /* ---- BKPRAM write gate ---- */
        case SVC_BKPRAM_WRITE: {
            const void *src = (const void *)(uintptr_t)arg0;
            uint32_t offset = arg1;
            uint32_t len    = stack_frame[2];
            bool ok = false;
            if (len > 0 && (offset + len) <= BSP_RAM_D3_SIZE &&
                (offset + len) >= offset) {
                memcpy((void *)(BSP_RAM_D3_BASE + offset), src, len);
                ok = true;
            }
            stack_frame[0] = (uint32_t)ok;
            break;
        }

        /* ---- Software Bus ---- */
        case SVC_SB_INIT:
            __sb_init();
            break;
        case SVC_SB_SUBSCRIBE: {
            bool ok = __sb_subscribe((sb_msg_id_t)arg0, (uint8_t)arg1);
            stack_frame[0] = (uint32_t)ok;
            break;
        }
        case SVC_SB_UNSUBSCRIBE: {
            bool ok = __sb_unsubscribe((sb_msg_id_t)arg0, (uint8_t)arg1);
            stack_frame[0] = (uint32_t)ok;
            break;
        }
        case SVC_SB_PUBLISH: {
            uint8_t n = __sb_publish((sb_msg_id_t)arg0,
                                     (const uint8_t *)(uintptr_t)arg1,
                                     (uint8_t)stack_frame[2]);
            stack_frame[0] = (uint32_t)n;
            break;
        }
        case SVC_SB_SUBSCRIBER_COUNT:
            stack_frame[0] = (uint32_t)__sb_subscriber_count(
                                 (sb_msg_id_t)arg0);
            break;
        case SVC_SB_ROUTE_COUNT:
            stack_frame[0] = (uint32_t)__sb_route_count();
            break;

        /* ---- Filesystem ---- */
        case SVC_FS_INIT:
            __fs_init();
            break;
        case SVC_FS_CREATE: {
            bool ok = __fs_create((const char *)(uintptr_t)arg0,
                                  (fs_file_t *)(uintptr_t)arg1);
            stack_frame[0] = (uint32_t)ok;
            break;
        }
        case SVC_FS_OPEN: {
            bool ok = __fs_open((const char *)(uintptr_t)arg0,
                                (fs_file_t *)(uintptr_t)arg1);
            stack_frame[0] = (uint32_t)ok;
            break;
        }
        case SVC_FS_WRITE: {
            bool ok = __fs_write((fs_file_t *)(uintptr_t)arg0,
                                 (const uint8_t *)(uintptr_t)arg1,
                                 (uint16_t)stack_frame[2]);
            stack_frame[0] = (uint32_t)ok;
            break;
        }
        case SVC_FS_READ: {
            uint16_t n = __fs_read((fs_file_t *)(uintptr_t)arg0,
                                   (uint8_t *)(uintptr_t)arg1,
                                   (uint16_t)stack_frame[2],
                                   (uint16_t)stack_frame[3]);
            stack_frame[0] = (uint32_t)n;
            break;
        }
        case SVC_FS_DELETE: {
            bool ok = __fs_delete((const char *)(uintptr_t)arg0);
            stack_frame[0] = (uint32_t)ok;
            break;
        }
        case SVC_FS_LIST: {
            uint8_t n = __fs_list((fs_file_info_t *)(uintptr_t)arg0,
                                  (uint8_t)arg1);
            stack_frame[0] = (uint32_t)n;
            break;
        }
        case SVC_FS_STATS:
            __fs_stats((fs_stats_t *)(uintptr_t)arg0);
            break;

        default:
            break;
    }
}

#endif /* HOST_TEST */

/* ============================================================================
 * CRITICAL SECTION WRAPPERS
 * ========================================================================= */

/**
 * @brief Enter critical section (disable scheduler)
 */
void enter_critical(void) {
#ifndef HOST_TEST
    __asm__ volatile ("svc %0\n" : : "I" (SVC_ENTER_CRITICAL));
#else
    __enter_critical();
#endif
}

/**
 * @brief Exit critical section (re-enable scheduler if outermost)
 */
void exit_critical(void) {
#ifndef HOST_TEST
    __asm__ volatile ("svc %0\n" : : "I" (SVC_EXIT_CRITICAL));
#else
    __exit_critical();
#endif
}

/* ============================================================================
 * KERNEL INIT / START WRAPPERS
 * ========================================================================= */

void os_init(void)  { __os_init(); }
void os_start(void) { __os_start(); }

/* ============================================================================
 * SCHEDULER WRAPPERS
 * ========================================================================= */

/**
 * @brief Voluntarily yield CPU to scheduler
 */
void os_yield(void) {
#ifndef HOST_TEST
    __asm__ volatile ("svc %0\n" : : "I" (SVC_OS_YIELD));
#else
    __os_yield();
#endif
}

/**
 * @brief Sleep for specified ticks (cooperative, via SVC)
 */
uint32_t task_active_sleep(uint32_t ticks) {
#ifndef HOST_TEST
    uint32_t result;
    __asm__ volatile (
        "mov r0, %1\n"
        "svc %2\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "r" (ticks), "I" (SVC_TASK_ACTIVE_SLEEP)
        : "r0"
    );
    return result;
#else
    return __task_active_sleep(ticks);
#endif
}

/**
 * @brief Blocking sleep — spin-wait in thread mode (cannot run in SVC handler)
 */
uint32_t task_blocking_sleep(uint32_t ticks) {
    return __task_blocking_sleep(ticks);
}

/**
 * @brief Busy-wait for specified ticks — spin in thread mode
 */
uint32_t task_busy_wait(uint32_t ticks) {
    return __task_busy_wait(ticks);
}

/**
 * @brief Get current system tick count
 */
uint32_t os_get_tick_count(void) {
#ifndef HOST_TEST
    uint32_t result;
    __asm__ volatile (
        "svc %1\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "I" (SVC_OS_GET_TICK_COUNT)
        : "r0"
    );
    return result;
#else
    return __os_get_tick_count();
#endif
}

/**
 * @brief Get name of currently executing task
 */
const char *os_get_current_task_name(void) {
#ifndef HOST_TEST
    const char *result;
    __asm__ volatile (
        "svc %1\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "I" (SVC_OS_GET_CURRENT_TASK_NAME)
        : "r0"
    );
    return result;
#else
    return __os_get_current_task_name();
#endif
}

/**
 * @brief Get number of active tasks
 */
uint8_t os_get_running_task_count(void) {
#ifndef HOST_TEST
    uint8_t result;
    __asm__ volatile (
        "svc %1\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "I" (SVC_OS_GET_RUNNING_TASK_COUNT)
        : "r0"
    );
    return result;
#else
    return __os_get_running_task_count();
#endif
}

/**
 * @brief Get remaining ticks in current time slice
 */
uint32_t os_get_task_ticks_remaining(void) {
#ifndef HOST_TEST
    uint32_t result;
    __asm__ volatile (
        "svc %1\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "I" (SVC_OS_GET_TASK_TICKS_REMAINING)
        : "r0"
    );
    return result;
#else
    return __os_get_task_ticks_remaining();
#endif
}

/* ============================================================================
 * TASK LIFECYCLE WRAPPERS
 * ========================================================================= */

/**
 * @brief Exit current task
 */
void os_exit_task(void) {
#ifndef HOST_TEST
    __asm__ volatile ("svc %0\n" : : "I" (SVC_OS_EXIT_TASK));
#else
    __os_exit_task();
#endif
}

/**
 * @brief Terminate current task (suicide)
 */
void os_task_suicide(void) {
#ifndef HOST_TEST
    __asm__ volatile ("svc %0\n" : : "I" (SVC_OS_TASK_SUICIDE));
#else
    __os_task_suicide();
#endif
}

/**
 * @brief Register a task with the scheduler
 */
void os_register_task(void (*function)(void), const char *name) {
#ifndef HOST_TEST
    __asm__ volatile (
        "mov r0, %0\n"
        "mov r1, %1\n"
        "svc %2\n"
        :
        : "r" (function), "r" (name), "I" (SVC_OS_REGISTER_TASK)
        : "r0", "r1"
    );
#else
    __os_register_task(function, name);
#endif
}

/**
 * @brief Kill a task by index
 */
void os_kill_process(uint8_t task_index) {
#ifndef HOST_TEST
    __asm__ volatile (
        "mov r0, %0\n"
        "svc %1\n"
        :
        : "r" ((uint32_t)task_index), "I" (SVC_OS_KILL_PROCESS)
        : "r0"
    );
#else
    __os_kill_process(task_index);
#endif
}

/**
 * @brief Restart a killed/finished task in-place (cold restart)
 */
void os_restart_task(uint8_t task_index) {
#ifndef HOST_TEST
    __asm__ volatile (
        "mov r0, %0\n"
        "svc %1\n"
        :
        : "r" ((uint32_t)task_index), "I" (SVC_OS_RESTART_TASK)
        : "r0"
    );
#else
    __os_restart_task(task_index);
#endif
}

/* ============================================================================
 * KERNEL DATA WRAPPERS
 * ========================================================================= */

/**
 * @brief Get stack pointer for task index
 */
uint32_t *kernel_get_stack(uint8_t task_idx) {
#ifndef HOST_TEST
    uint32_t *result;
    __asm__ volatile (
        "mov r0, %1\n"
        "svc %2\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "r" ((uint32_t)task_idx), "I" (SVC_KERNEL_GET_STACK)
        : "r0"
    );
    return result;
#else
    return __kernel_get_stack(task_idx);
#endif
}

/**
 * @brief Get data pointer for task index
 */
uint32_t *kernel_get_data(uint8_t task_idx) {
#ifndef HOST_TEST
    uint32_t *result;
    __asm__ volatile (
        "mov r0, %1\n"
        "svc %2\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "r" ((uint32_t)task_idx), "I" (SVC_KERNEL_GET_DATA)
        : "r0"
    );
    return result;
#else
    return __kernel_get_data(task_idx);
#endif
}

/**
 * @brief Allocate protected data words for current task
 */
void *kernel_protected_data(uint16_t num_words) {
#ifndef HOST_TEST
    void *result;
    __asm__ volatile (
        "mov r0, %1\n"
        "svc %2\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "r" ((uint32_t)num_words), "I" (SVC_KERNEL_PROTECTED_DATA)
        : "r0"
    );
    return result;
#else
    return __kernel_protected_data(num_words);
#endif
}

/* ============================================================================
 * SEMAPHORE WRAPPERS
 * ========================================================================= */

/**
 * @brief Initialize a counting semaphore
 */
bool semaphore_init(uint8_t semaphore_idx, uint32_t semaphore_count) {
#ifndef HOST_TEST
    uint32_t result;
    __asm__ volatile (
        "mov r0, %1\n"
        "mov r1, %2\n"
        "svc %3\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "r" ((uint32_t)semaphore_idx), "r" (semaphore_count),
          "I" (SVC_SEMAPHORE_INIT)
        : "r0", "r1"
    );
    return (bool)result;
#else
    return __semaphore_init(semaphore_idx, semaphore_count);
#endif
}

/**
 * @brief Increment semaphore count — spin-wait in thread mode
 */
bool semaphore_feed(uint8_t semaphore_idx) {
    return __semaphore_feed(semaphore_idx);
}

/**
 * @brief Decrement semaphore count — spin-wait in thread mode
 */
bool semaphore_consume(uint8_t semaphore_idx) {
    return __semaphore_consume(semaphore_idx);
}

/**
 * @brief Decrement semaphore count with timeout
 */
bool semaphore_consume_timeout(uint8_t semaphore_idx, uint32_t max_ticks) {
    return __semaphore_consume_timeout(semaphore_idx, max_ticks);
}

/**
 * @brief Get current semaphore count
 */
uint32_t semaphore_get_count(uint8_t semaphore_idx) {
#ifndef HOST_TEST
    uint32_t result;
    __asm__ volatile (
        "mov r0, %1\n"
        "svc %2\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "r" ((uint32_t)semaphore_idx), "I" (SVC_SEMAPHORE_GET_COUNT)
        : "r0"
    );
    return result;
#else
    return __semaphore_get_count(semaphore_idx);
#endif
}

/**
 * @brief Get semaphore maximum count
 */
uint32_t semaphore_get_max_count(uint8_t semaphore_idx) {
#ifndef HOST_TEST
    uint32_t result;
    __asm__ volatile (
        "mov r0, %1\n"
        "svc %2\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "r" ((uint32_t)semaphore_idx), "I" (SVC_SEMAPHORE_GET_MAX_COUNT)
        : "r0"
    );
    return result;
#else
    return __semaphore_get_max_count(semaphore_idx);
#endif
}

/* ============================================================================
 * PIPE WRAPPERS
 * ========================================================================= */

/**
 * @brief Initialize a message pipe
 */
bool pipe_init(uint8_t pipe_idx, uint16_t pipe_capacity_bytes) {
#ifndef HOST_TEST
    uint32_t result;
    __asm__ volatile (
        "mov r0, %1\n"
        "mov r1, %2\n"
        "svc %3\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "r" ((uint32_t)pipe_idx), "r" ((uint32_t)pipe_capacity_bytes),
          "I" (SVC_PIPE_INIT)
        : "r0", "r1"
    );
    return (bool)result;
#else
    return __pipe_init(pipe_idx, pipe_capacity_bytes);
#endif
}

/**
 * @brief Enqueue bytes to pipe — spin-wait in thread mode
 */
bool pipe_enqueue(uint8_t pipe_idx, uint8_t *message, uint8_t message_bytes) {
    return __pipe_enqueue(pipe_idx, message, message_bytes);
}

/**
 * @brief Dequeue bytes from pipe — spin-wait in thread mode
 */
bool pipe_dequeue(uint8_t pipe_idx, uint8_t *message, uint8_t message_bytes) {
    return __pipe_dequeue(pipe_idx, message, message_bytes);
}

/**
 * @brief Get current byte count in pipe
 */
uint16_t pipe_get_count(uint8_t pipe_idx) {
#ifndef HOST_TEST
    uint16_t result;
    __asm__ volatile (
        "mov r0, %1\n"
        "svc %2\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "r" ((uint32_t)pipe_idx), "I" (SVC_PIPE_GET_COUNT)
        : "r0"
    );
    return result;
#else
    return __pipe_get_count(pipe_idx);
#endif
}

/**
 * @brief Get pipe capacity
 */
uint16_t pipe_get_max_count(uint8_t pipe_idx) {
#ifndef HOST_TEST
    uint16_t result;
    __asm__ volatile (
        "mov r0, %1\n"
        "svc %2\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "r" ((uint32_t)pipe_idx), "I" (SVC_PIPE_GET_MAX_COUNT)
        : "r0"
    );
    return result;
#else
    return __pipe_get_max_count(pipe_idx);
#endif
}

/* ============================================================================
 * CALL GATE WRAPPERS (for spinning functions — read kernel state via SVC)
 * ========================================================================= */

/**
 * @brief Check if semaphore can accept a feed (count < max && engaged)
 * @note  Runs in privileged mode — safe once DTCM is priv-only
 */
bool sem_can_feed(uint8_t semaphore_idx) {
#ifndef HOST_TEST
    uint32_t result;
    __asm__ volatile (
        "mov r0, %1\n"
        "svc %2\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "r" ((uint32_t)semaphore_idx), "I" (SVC_SEM_CAN_FEED)
        : "r0"
    );
    return (bool)result;
#else
    return __sem_can_feed(semaphore_idx);
#endif
}

/**
 * @brief Check if semaphore can be consumed (count > 0 && engaged)
 * @note  Runs in privileged mode — safe once DTCM is priv-only
 */
bool sem_can_consume(uint8_t semaphore_idx) {
#ifndef HOST_TEST
    uint32_t result;
    __asm__ volatile (
        "mov r0, %1\n"
        "svc %2\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "r" ((uint32_t)semaphore_idx), "I" (SVC_SEM_CAN_CONSUME)
        : "r0"
    );
    return (bool)result;
#else
    return __sem_can_consume(semaphore_idx);
#endif
}

/**
 * @brief Check if pipe has room for message_bytes (free >= bytes && engaged)
 * @note  Runs in privileged mode — safe once DTCM is priv-only
 */
bool pipe_can_enqueue(uint8_t pipe_idx, uint8_t message_bytes) {
#ifndef HOST_TEST
    uint32_t result;
    __asm__ volatile (
        "mov r0, %1\n"
        "mov r1, %2\n"
        "svc %3\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "r" ((uint32_t)pipe_idx), "r" ((uint32_t)message_bytes),
          "I" (SVC_PIPE_CAN_ENQUEUE)
        : "r0", "r1"
    );
    return (bool)result;
#else
    return __pipe_can_enqueue(pipe_idx, message_bytes);
#endif
}

/**
 * @brief Check if pipe has message_bytes available (count >= bytes && engaged)
 * @note  Runs in privileged mode — safe once DTCM is priv-only
 */
bool pipe_can_dequeue(uint8_t pipe_idx, uint8_t message_bytes) {
#ifndef HOST_TEST
    uint32_t result;
    __asm__ volatile (
        "mov r0, %1\n"
        "mov r1, %2\n"
        "svc %3\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "r" ((uint32_t)pipe_idx), "r" ((uint32_t)message_bytes),
          "I" (SVC_PIPE_CAN_DEQUEUE)
        : "r0", "r1"
    );
    return (bool)result;
#else
    return __pipe_can_dequeue(pipe_idx, message_bytes);
#endif
}

/* ============================================================================
 * WRITE GATE WRAPPERS (for spinning functions — modify kernel state via SVC)
 * ========================================================================= */

/**
 * @brief Increment semaphore count atomically in privileged mode
 * @note  Called after sem_can_feed() spin loop exits
 */
void sem_increment(uint8_t semaphore_idx) {
#ifndef HOST_TEST
    __asm__ volatile (
        "mov r0, %0\n"
        "svc %1\n"
        :
        : "r" ((uint32_t)semaphore_idx), "I" (SVC_SEM_INCREMENT)
        : "r0"
    );
#else
    __sem_increment(semaphore_idx);
#endif
}

/**
 * @brief Decrement semaphore count atomically in privileged mode
 * @note  Called after sem_can_consume() spin loop exits
 */
void sem_decrement(uint8_t semaphore_idx) {
#ifndef HOST_TEST
    __asm__ volatile (
        "mov r0, %0\n"
        "svc %1\n"
        :
        : "r" ((uint32_t)semaphore_idx), "I" (SVC_SEM_DECREMENT)
        : "r0"
    );
#else
    __sem_decrement(semaphore_idx);
#endif
}

/**
 * @brief Write bytes to pipe buffer atomically in privileged mode
 * @note  Called after pipe_can_enqueue() spin loop exits
 */
void pipe_write_bytes(uint8_t pipe_idx, uint8_t *message, uint8_t message_bytes) {
#ifndef HOST_TEST
    __asm__ volatile (
        "mov r0, %0\n"
        "mov r1, %1\n"
        "mov r2, %2\n"
        "svc %3\n"
        :
        : "r" ((uint32_t)pipe_idx), "r" (message), "r" ((uint32_t)message_bytes),
          "I" (SVC_PIPE_WRITE_BYTES)
        : "r0", "r1", "r2"
    );
#else
    __pipe_write_bytes(pipe_idx, message, message_bytes);
#endif
}

/**
 * @brief Read bytes from pipe buffer atomically in privileged mode
 * @note  Called after pipe_can_dequeue() spin loop exits
 */
void pipe_read_bytes(uint8_t pipe_idx, uint8_t *message, uint8_t message_bytes) {
#ifndef HOST_TEST
    __asm__ volatile (
        "mov r0, %0\n"
        "mov r1, %1\n"
        "mov r2, %2\n"
        "svc %3\n"
        :
        : "r" ((uint32_t)pipe_idx), "r" (message), "r" ((uint32_t)message_bytes),
          "I" (SVC_PIPE_READ_BYTES)
        : "r0", "r1", "r2"
    );
#else
    __pipe_read_bytes(pipe_idx, message, message_bytes);
#endif
}


/* ============================================================================
 * TASK METADATA READ GATES (for display/diagnostics from unprivileged mode)
 * ========================================================================= */

/**
 * @brief Get task name by index (read from task_list in privileged mode)
 * @param task_idx Task index (0 to num_created_tasks-1)
 * @return Pointer to task name string, or NULL if invalid index
 * @note  Safe to call from unprivileged mode once DTCM is priv-only
 */
const char *os_get_task_name(uint8_t task_idx) {
#ifndef HOST_TEST
    const char *result;
    __asm__ volatile (
        "mov r0, %1\n"
        "svc %2\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "r" ((uint32_t)task_idx), "I" (SVC_GET_TASK_NAME)
        : "r0"
    );
    return result;
#else
    return __os_get_task_name(task_idx);
#endif
}

/**
 * @brief Get number of created tasks (read num_created_tasks in privileged mode)
 * @return Number of tasks registered with os_register_task()
 * @note  Safe to call from unprivileged mode once DTCM is priv-only
 */
uint8_t os_get_num_created_tasks(void) {
#ifndef HOST_TEST
    uint8_t result;
    __asm__ volatile (
        "svc %1\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "I" (SVC_GET_NUM_TASKS)
        : "r0"
    );
    return result;
#else
    return __os_get_num_created_tasks();
#endif
}

/**
 * @brief Check if OS is running (safe from unprivileged mode)
 * @return 1 if OS is running, 0 otherwise
 * @note  Safe to call from unprivileged mode once DTCM is priv-only
 */
uint8_t os_is_running(void) {
#ifndef HOST_TEST
    uint8_t result;
    __asm__ volatile (
        "svc %1\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "I" (SVC_OS_IS_RUNNING)
        : "r0"
    );
    return result;
#else
    return __os_is_running();
#endif
}

/**
 * @brief Get a task's current state
 */
icarus_task_state_t os_get_task_state(uint8_t task_idx) {
#ifndef HOST_TEST
    uint32_t result;
    __asm__ volatile (
        "mov r0, %1\n"
        "svc %2\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "r" ((uint32_t)task_idx), "I" (SVC_GET_TASK_STATE)
        : "r0"
    );
    return (icarus_task_state_t)result;
#else
    return __os_get_task_state(task_idx);
#endif
}

/**
 * @brief Get a task's dispatch count
 */
uint32_t os_get_task_dispatch_count(uint8_t task_idx) {
#ifndef HOST_TEST
    uint32_t result;
    __asm__ volatile (
        "mov r0, %1\n"
        "svc %2\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "r" ((uint32_t)task_idx), "I" (SVC_GET_TASK_DISPATCH_COUNT)
        : "r0"
    );
    return result;
#else
    return __os_get_task_dispatch_count(task_idx);
#endif
}

/**
 * @brief Get a task's stack high-water mark
 */
uint32_t os_get_stack_watermark(uint8_t task_idx) {
#ifndef HOST_TEST
    uint32_t result;
    __asm__ volatile (
        "mov r0, %1\n"
        "svc %2\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "r" ((uint32_t)task_idx), "I" (SVC_GET_STACK_WATERMARK)
        : "r0"
    );
    return result;
#else
    return __os_get_stack_watermark(task_idx);
#endif
}

/**
 * @brief Scan and update a task's stack watermark
 */
void os_update_stack_watermark(uint8_t task_idx) {
#ifndef HOST_TEST
    __asm__ volatile (
        "mov r0, %0\n"
        "svc %1\n"
        :
        : "r" ((uint32_t)task_idx), "I" (SVC_UPDATE_STACK_WATERMARK)
        : "r0"
    );
#else
    __os_update_stack_watermark(task_idx);
#endif
}

/* ============================================================================
 * CDC RX RING BUFFER WRAPPERS
 * ========================================================================= */

/* Init: void → void. */
void cdc_rx_init(void) {
#ifndef HOST_TEST
    __asm__ volatile ("svc %0\n" : : "I" (SVC_CDC_RX_INIT));
#else
    __cdc_rx_init();
#endif
}

/* Producer: called from the privileged USB CDC ISR. No SVC — the caller is
 * already in privileged handler context, and issuing an SVC from a high-
 * priority interrupt would cause a usage fault. */
void cdc_rx_push(const uint8_t *data, uint32_t len) {
    __cdc_rx_push(data, len);
}

bool cdc_rx_read_byte(uint8_t *out) {
#ifndef HOST_TEST
    uint32_t result;
    __asm__ volatile (
        "mov r0, %1\n"
        "svc %2\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "r" ((uint32_t)(uintptr_t)out), "I" (SVC_CDC_RX_READ_BYTE)
        : "r0"
    );
    return (bool)result;
#else
    return __cdc_rx_read_byte(out);
#endif
}

uint32_t cdc_rx_available(void) {
#ifndef HOST_TEST
    uint32_t result;
    __asm__ volatile (
        "svc %1\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "I" (SVC_CDC_RX_AVAILABLE)
        : "r0"
    );
    return result;
#else
    return __cdc_rx_available();
#endif
}

/* ============================================================================
 * EVENT RING BUFFER WRAPPERS
 * ========================================================================= */

void event_init(void) {
#ifndef HOST_TEST
    __asm__ volatile ("svc %0\n" : : "I" (SVC_EVENT_INIT));
#else
    __event_init();
#endif
}

/* os_event packs five user-level args into three SVC registers (R0/R1/R2)
 * to stay inside the AAPCS budget without spilling to the caller's stack:
 *
 *   R0  = (event_id << 16) | (severity << 8) | module_id
 *   R1  = payload pointer
 *   R2  = payload_len
 */
void os_event(uint8_t module_id, event_severity_t severity, uint16_t event_id,
              const void *payload, uint8_t payload_len) {
#ifndef HOST_TEST
    uint32_t packed = ((uint32_t)event_id << 16)
                    | ((uint32_t)((uint8_t)severity) << 8)
                    | (uint32_t)module_id;
    __asm__ volatile (
        "mov r0, %0\n"
        "mov r1, %1\n"
        "mov r2, %2\n"
        "svc %3\n"
        :
        : "r" (packed), "r" ((uint32_t)(uintptr_t)payload),
          "r" ((uint32_t)payload_len), "I" (SVC_OS_EVENT)
        : "r0", "r1", "r2"
    );
#else
    __os_event(module_id, severity, event_id, payload, payload_len);
#endif
}

void event_set_squelch(uint8_t module_id, event_severity_t min_severity) {
#ifndef HOST_TEST
    __asm__ volatile (
        "mov r0, %0\n"
        "mov r1, %1\n"
        "svc %2\n"
        :
        : "r" ((uint32_t)module_id), "r" ((uint32_t)min_severity),
          "I" (SVC_EVENT_SET_SQUELCH)
        : "r0", "r1"
    );
#else
    __event_set_squelch(module_id, min_severity);
#endif
}

event_severity_t event_get_squelch(uint8_t module_id) {
#ifndef HOST_TEST
    uint32_t result;
    __asm__ volatile (
        "mov r0, %1\n"
        "svc %2\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "r" ((uint32_t)module_id), "I" (SVC_EVENT_GET_SQUELCH)
        : "r0"
    );
    return (event_severity_t)result;
#else
    return __event_get_squelch(module_id);
#endif
}

bool event_drain(event_entry_t *out_buf, uint8_t max_entries,
                 uint8_t *num_drained) {
#ifndef HOST_TEST
    uint32_t result;
    __asm__ volatile (
        "mov r0, %1\n"
        "mov r1, %2\n"
        "mov r2, %3\n"
        "svc %4\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "r" ((uint32_t)(uintptr_t)out_buf),
          "r" ((uint32_t)max_entries),
          "r" ((uint32_t)(uintptr_t)num_drained),
          "I" (SVC_EVENT_DRAIN)
        : "r0", "r1", "r2"
    );
    return (bool)result;
#else
    return __event_drain(out_buf, max_entries, num_drained);
#endif
}

uint32_t event_get_count(void) {
#ifndef HOST_TEST
    uint32_t result;
    __asm__ volatile (
        "svc %1\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "I" (SVC_EVENT_GET_COUNT)
        : "r0"
    );
    return result;
#else
    return __event_get_count();
#endif
}

/* ============================================================================
 * GROUND-LOADABLE TABLE ENGINE WRAPPERS
 * ========================================================================= */

void tbl_init(void) {
#ifndef HOST_TEST
    __asm__ volatile ("svc %0\n" : : "I" (SVC_TBL_INIT));
#else
    __tbl_init();
#endif
}

bool tbl_register(const tbl_descriptor_t *desc) {
#ifndef HOST_TEST
    uint32_t result;
    __asm__ volatile (
        "mov r0, %1\n"
        "svc %2\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "r" ((uint32_t)(uintptr_t)desc), "I" (SVC_TBL_REGISTER)
        : "r0"
    );
    return (bool)result;
#else
    return __tbl_register(desc);
#endif
}

bool tbl_load(tbl_id_t id, const uint8_t *data, uint16_t len,
              uint16_t schema_crc) {
#ifndef HOST_TEST
    uint32_t result;
    __asm__ volatile (
        "mov r0, %1\n"
        "mov r1, %2\n"
        "mov r2, %3\n"
        "mov r3, %4\n"
        "svc %5\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "r" ((uint32_t)id), "r" ((uint32_t)(uintptr_t)data),
          "r" ((uint32_t)len), "r" ((uint32_t)schema_crc),
          "I" (SVC_TBL_LOAD)
        : "r0", "r1", "r2", "r3"
    );
    return (bool)result;
#else
    return __tbl_load(id, data, len, schema_crc);
#endif
}

/* tbl_activate is the only call that must straddle priv ↔ thread mode: the
 * registered activate callback runs in unprivileged thread mode between
 * the prepare (validate + copy staging into a thread-mode scratch buffer)
 * and the commit (copy scratch into the active buffer). This mirrors the
 * pattern used by semaphore/pipe spin loops. */
bool tbl_activate(tbl_id_t id) {
    uint8_t                 scratch[TBL_MAX_SIZE];
    uint16_t                scratch_len = 0;
    tbl_activate_fn         activate_cb = NULL;
    bool                    ok;

#ifndef HOST_TEST
    {
        uint32_t result;
        __asm__ volatile (
            "mov r0, %1\n"
            "mov r1, %2\n"
            "mov r2, %3\n"
            "mov r3, %4\n"
            "svc %5\n"
            "mov %0, r0\n"
            : "=r" (result)
            : "r" ((uint32_t)id),
              "r" ((uint32_t)(uintptr_t)scratch),
              "r" ((uint32_t)(uintptr_t)&scratch_len),
              "r" ((uint32_t)(uintptr_t)&activate_cb),
              "I" (SVC_TBL_ACTIVATE_PREPARE)
            : "r0", "r1", "r2", "r3"
        );
        ok = (bool)result;
    }
#else
    ok = __tbl_activate_prepare(id, scratch, &scratch_len, &activate_cb);
#endif

    if (!ok) {
        return false;
    }

    /* Run the user activate callback in thread mode against the scratch
     * copy. The active buffer is unchanged at this point. */
    if (activate_cb) {
        if (!activate_cb(scratch, scratch_len)) {
            return false;
        }
    }

#ifndef HOST_TEST
    {
        uint32_t result;
        __asm__ volatile (
            "mov r0, %1\n"
            "mov r1, %2\n"
            "mov r2, %3\n"
            "svc %4\n"
            "mov %0, r0\n"
            : "=r" (result)
            : "r" ((uint32_t)id),
              "r" ((uint32_t)(uintptr_t)scratch),
              "r" ((uint32_t)scratch_len),
              "I" (SVC_TBL_ACTIVATE_COMMIT)
            : "r0", "r1", "r2"
        );
        return (bool)result;
    }
#else
    return __tbl_activate_commit(id, scratch, scratch_len);
#endif
}

int16_t tbl_dump(tbl_id_t id, uint8_t *out, uint16_t max) {
#ifndef HOST_TEST
    int32_t result;
    __asm__ volatile (
        "mov r0, %1\n"
        "mov r1, %2\n"
        "mov r2, %3\n"
        "svc %4\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "r" ((uint32_t)id), "r" ((uint32_t)(uintptr_t)out),
          "r" ((uint32_t)max), "I" (SVC_TBL_DUMP)
        : "r0", "r1", "r2"
    );
    return (int16_t)result;
#else
    return __tbl_dump(id, out, max);
#endif
}

const tbl_descriptor_t *tbl_get_descriptor(tbl_id_t id) {
#ifndef HOST_TEST
    uint32_t result;
    __asm__ volatile (
        "mov r0, %1\n"
        "svc %2\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "r" ((uint32_t)id), "I" (SVC_TBL_GET_DESCRIPTOR)
        : "r0"
    );
    return (const tbl_descriptor_t *)(uintptr_t)result;
#else
    return __tbl_get_descriptor(id);
#endif
}

uint8_t tbl_count(void) {
#ifndef HOST_TEST
    uint32_t result;
    __asm__ volatile (
        "svc %1\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "I" (SVC_TBL_COUNT)
        : "r0"
    );
    return (uint8_t)result;
#else
    return __tbl_count();
#endif
}

/* ============================================================================
 * CHECKSUM INTEGRITY MONITOR WRAPPERS
 * ========================================================================= */

void cs_init(void) {
#ifndef HOST_TEST
    __asm__ volatile ("svc %0\n" : : "I" (SVC_CS_INIT));
#else
    __cs_init();
#endif
}

void cs_set_callback(cs_mismatch_fn fn) {
#ifndef HOST_TEST
    __asm__ volatile (
        "mov r0, %0\n"
        "svc %1\n"
        :
        : "r" ((uint32_t)(uintptr_t)fn), "I" (SVC_CS_SET_CALLBACK)
        : "r0"
    );
#else
    __cs_set_callback(fn);
#endif
}

bool cs_add_region(uint8_t idx, const uint8_t *addr, uint32_t size) {
#ifndef HOST_TEST
    uint32_t result;
    __asm__ volatile (
        "mov r0, %1\n"
        "mov r1, %2\n"
        "mov r2, %3\n"
        "svc %4\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "r" ((uint32_t)idx), "r" ((uint32_t)(uintptr_t)addr),
          "r" (size), "I" (SVC_CS_ADD_REGION)
        : "r0", "r1", "r2"
    );
    return (bool)result;
#else
    return __cs_add_region(idx, addr, size);
#endif
}

bool cs_enable(uint8_t idx, bool enabled) {
#ifndef HOST_TEST
    uint32_t result;
    __asm__ volatile (
        "mov r0, %1\n"
        "mov r1, %2\n"
        "svc %3\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "r" ((uint32_t)idx), "r" ((uint32_t)enabled),
          "I" (SVC_CS_ENABLE)
        : "r0", "r1"
    );
    return (bool)result;
#else
    return __cs_enable(idx, enabled);
#endif
}

bool cs_rebaseline(uint8_t idx) {
#ifndef HOST_TEST
    uint32_t result;
    __asm__ volatile (
        "mov r0, %1\n"
        "svc %2\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "r" ((uint32_t)idx), "I" (SVC_CS_REBASELINE)
        : "r0"
    );
    return (bool)result;
#else
    return __cs_rebaseline(idx);
#endif
}

uint8_t cs_check_all(void) {
#ifndef HOST_TEST
    uint32_t result;
    __asm__ volatile (
        "svc %1\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "I" (SVC_CS_CHECK_ALL)
        : "r0"
    );
    return (uint8_t)result;
#else
    return __cs_check_all();
#endif
}

bool cs_get_region(uint8_t idx, cs_region_t *out) {
#ifndef HOST_TEST
    uint32_t result;
    __asm__ volatile (
        "mov r0, %1\n"
        "mov r1, %2\n"
        "svc %3\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "r" ((uint32_t)idx), "r" ((uint32_t)(uintptr_t)out),
          "I" (SVC_CS_GET_REGION)
        : "r0", "r1"
    );
    return (bool)result;
#else
    return __cs_get_region(idx, out);
#endif
}

uint8_t cs_region_count(void) {
#ifndef HOST_TEST
    uint32_t result;
    __asm__ volatile (
        "svc %1\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "I" (SVC_CS_REGION_COUNT)
        : "r0"
    );
    return (uint8_t)result;
#else
    return __cs_region_count();
#endif
}

/* ============================================================================
 * BKPRAM WRITE GATE
 * ========================================================================= */

/** @copydoc bkpram_write */
bool bkpram_write(const void *src, uint32_t offset, uint32_t len) {
#ifndef HOST_TEST
    uint32_t result;
    __asm__ volatile (
        "mov r0, %1\n"
        "mov r1, %2\n"
        "mov r2, %3\n"
        "svc %4\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "r" ((uint32_t)(uintptr_t)src), "r" (offset),
          "r" (len), "I" (SVC_BKPRAM_WRITE)
        : "r0", "r1", "r2"
    );
    return (bool)result;
#else
    (void)offset;
    if (len == 0u) {
        return false;
    }
    /* HOST_TEST: no BKPRAM hardware — treat as no-op success */
    (void)src;
    return true;
#endif
}
