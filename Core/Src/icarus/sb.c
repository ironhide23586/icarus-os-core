/**
 * @file    sb.c
 * @brief   Software Bus — publish/subscribe message router implementation
 * @version 0.1.0
 *
 * @details Static routing table mapping msg_id → pipe_idx[].  Publishing
 *          iterates subscribers and enqueues into each pipe that has space.
 *
 *          All mutable state lives in DTCM_DATA_PRIV (privileged-only,
 *          zero wait-state on Cortex-M7).  All functions are placed in
 *          ITCM_FUNC for zero wait-state instruction fetch.  Public
 *          access goes through SVC call gates; the __-prefixed functions
 *          are the privileged implementations.
 *
 * @par Thread safety:
 *      Every public function issues an SVC instruction to execute the
 *      privileged implementation in handler mode.  The SVC handler is
 *      non-preemptible, guaranteeing atomic access to the route table.
 *
 * @author  Souham Biswas
 * @date    2026
 *
 * @copyright Copyright 2025-2026 Souham Biswas
 *            https://github.com/ironhide23586/icarus-os-core
 *            Licensed under the Apache License, Version 2.0
 */

#include "icarus/icarus.h"
#include "icarus/svc.h"
#include <string.h>

/* ============================================================================
 * ROUTE TABLE (DTCM)
 * ========================================================================= */

/**
 * @brief  Single route entry: one msg_id → up to SB_MAX_SUBS_PER_MSG pipes.
 */
typedef struct {
    sb_msg_id_t msg_id;                      /**< Subscribed message ID      */
    uint8_t     pipes[SB_MAX_SUBS_PER_MSG];  /**< Subscriber pipe indices    */
    uint8_t     count;                        /**< Active subscriber count    */
    bool        used;                         /**< Slot is occupied           */
} sb_route_t;

/** @brief Route table — privileged DTCM, zero wait-state. */
DTCM_DATA_PRIV static sb_route_t routes[SB_MAX_ROUTES];

/** @brief Number of route slots in use. */
DTCM_DATA_PRIV static uint8_t route_used;

/* ============================================================================
 * INTERNAL HELPERS (ITCM, privileged)
 * ========================================================================= */

/**
 * @brief  Find the route entry for @p msg_id.
 * @return Pointer to route, or NULL if not found.
 * @note   Caller must hold critical section.
 */
ITCM_FUNC static sb_route_t *find_route(sb_msg_id_t msg_id) {
    for (uint8_t i = 0; i < route_used; i++) {
        if (routes[i].used && routes[i].msg_id == msg_id) {
            return &routes[i];
        }
    }
    return NULL;
}

/**
 * @brief  Allocate a fresh route slot.
 * @return Pointer to new slot, or NULL if table is full.
 * @note   Caller must hold critical section.
 */
ITCM_FUNC static sb_route_t *alloc_route(void) {
    if (route_used >= SB_MAX_ROUTES) return NULL;
    sb_route_t *r = &routes[route_used++];
    memset(r, 0, sizeof(*r));
    r->used = true;
    return r;
}

/* ============================================================================
 * PRIVILEGED IMPLEMENTATIONS
 * ========================================================================= */

/**
 * @brief  Privileged implementation of sb_init().
 * @details Zeroes the entire route table and resets the used counter.
 */
ITCM_FUNC void __sb_init(void) {
    memset(routes, 0, sizeof(routes));
    route_used = 0;
}

/**
 * @brief  Privileged implementation of sb_subscribe().
 *
 * @param[in] msg_id    Message identifier.
 * @param[in] pipe_idx  Kernel pipe index to bind.
 * @retval true   Subscription added.
 * @retval false  Table full, subscriber limit hit, or duplicate pair.
 *
 * @details If no route exists for @p msg_id a new slot is allocated.
 *          Duplicate (msg_id, pipe_idx) pairs are rejected to prevent
 *          double-delivery.
 */
ITCM_FUNC bool __sb_subscribe(sb_msg_id_t msg_id, uint8_t pipe_idx) {
    sb_route_t *r = find_route(msg_id);
    if (!r) {
        r = alloc_route();
        if (!r) return false;
        r->msg_id = msg_id;
    }

    /* Subscriber limit */
    if (r->count >= SB_MAX_SUBS_PER_MSG) return false;

    /* Reject duplicate */
    for (uint8_t i = 0; i < r->count; i++) {
        if (r->pipes[i] == pipe_idx) return false;
    }

    r->pipes[r->count++] = pipe_idx;
    return true;
}

/**
 * @brief  Privileged implementation of sb_unsubscribe().
 *
 * @param[in] msg_id    Message identifier.
 * @param[in] pipe_idx  Pipe to remove.
 * @retval true   Subscription found and removed (last-swap compaction).
 * @retval false  No matching subscription.
 */
ITCM_FUNC bool __sb_unsubscribe(sb_msg_id_t msg_id, uint8_t pipe_idx) {
    sb_route_t *r = find_route(msg_id);
    if (!r) return false;

    for (uint8_t i = 0; i < r->count; i++) {
        if (r->pipes[i] == pipe_idx) {
            /* Compact: move last into this slot */
            r->pipes[i] = r->pipes[r->count - 1];
            r->count--;
            return true;
        }
    }
    return false;
}

/**
 * @brief  Privileged implementation of sb_publish().
 *
 * @param[in] msg_id  Message identifier.
 * @param[in] data    Payload bytes.
 * @param[in] len     Payload length.
 * @return Number of subscribers successfully enqueued to.
 *
 * @details Iterates all subscribers for @p msg_id.  For each pipe,
 *          checks capacity via pipe_can_enqueue() before writing.
 *          Pipes that are full are silently skipped (best-effort).
 */
ITCM_FUNC uint8_t __sb_publish(sb_msg_id_t msg_id, const uint8_t *data,
                                uint8_t len) {
    if (!data || len == 0) return 0;

    sb_route_t *r = find_route(msg_id);
    if (!r) return 0;

    uint8_t delivered = 0;
    for (uint8_t i = 0; i < r->count; i++) {
        uint8_t pidx = r->pipes[i];
        if (pipe_can_enqueue(pidx, len)) {
            pipe_enqueue(pidx, (uint8_t *)(uintptr_t)data, len);
            delivered++;
        }
    }
    return delivered;
}

/**
 * @brief  Privileged implementation of sb_subscriber_count().
 * @param[in] msg_id  Message identifier.
 * @return Active subscriber count, or 0 if no route exists.
 */
ITCM_FUNC uint8_t __sb_subscriber_count(sb_msg_id_t msg_id) {
    sb_route_t *r = find_route(msg_id);
    return r ? r->count : 0;
}

/**
 * @brief  Privileged implementation of sb_route_count().
 * @return Number of distinct msg_ids with at least one subscriber.
 */
ITCM_FUNC uint8_t __sb_route_count(void) {
    return route_used;
}

/* ============================================================================
 * PUBLIC API (SVC call gates — privileged access to DTCM_PRIV data)
 * ========================================================================= */

/**
 * @brief  Initialize the software bus via SVC gate.
 * @details Clears all routes in privileged mode.
 */
void sb_init(void) {
#ifndef HOST_TEST
    __asm__ volatile ("svc %0\n" : : "I" (SVC_SB_INIT));
#else
    __sb_init();
#endif
}

/**
 * @brief  Subscribe a pipe to a message ID via SVC gate.
 * @param[in] msg_id    Message identifier.
 * @param[in] pipe_idx  Kernel pipe index.
 * @retval true   Subscription added.
 * @retval false  Table full, limit hit, or duplicate.
 */
bool sb_subscribe(sb_msg_id_t msg_id, uint8_t pipe_idx) {
#ifndef HOST_TEST
    uint32_t result;
    __asm__ volatile (
        "mov r0, %1\n"
        "mov r1, %2\n"
        "svc %3\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "r" ((uint32_t)msg_id), "r" ((uint32_t)pipe_idx),
          "I" (SVC_SB_SUBSCRIBE)
        : "r0", "r1"
    );
    return (bool)result;
#else
    return __sb_subscribe(msg_id, pipe_idx);
#endif
}

/**
 * @brief  Remove a subscription via SVC gate.
 * @param[in] msg_id    Message identifier.
 * @param[in] pipe_idx  Pipe to unsubscribe.
 * @retval true   Subscription found and removed.
 * @retval false  No matching subscription.
 */
bool sb_unsubscribe(sb_msg_id_t msg_id, uint8_t pipe_idx) {
#ifndef HOST_TEST
    uint32_t result;
    __asm__ volatile (
        "mov r0, %1\n"
        "mov r1, %2\n"
        "svc %3\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "r" ((uint32_t)msg_id), "r" ((uint32_t)pipe_idx),
          "I" (SVC_SB_UNSUBSCRIBE)
        : "r0", "r1"
    );
    return (bool)result;
#else
    return __sb_unsubscribe(msg_id, pipe_idx);
#endif
}

/**
 * @brief  Publish a message to all subscribers via SVC gate.
 * @param[in] msg_id  Message identifier.
 * @param[in] data    Payload bytes.
 * @param[in] len     Payload length.
 * @return Number of subscribers successfully enqueued to.
 */
uint8_t sb_publish(sb_msg_id_t msg_id, const uint8_t *data, uint8_t len) {
#ifndef HOST_TEST
    uint32_t result;
    __asm__ volatile (
        "mov r0, %1\n"
        "mov r1, %2\n"
        "mov r2, %3\n"
        "svc %4\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "r" ((uint32_t)msg_id), "r" ((uint32_t)(uintptr_t)data),
          "r" ((uint32_t)len), "I" (SVC_SB_PUBLISH)
        : "r0", "r1", "r2"
    );
    return (uint8_t)result;
#else
    return __sb_publish(msg_id, data, len);
#endif
}

/**
 * @brief  Return the subscriber count for a message ID via SVC gate.
 * @param[in] msg_id  Message identifier.
 * @return Active subscriber count, or 0 if no route exists.
 */
uint8_t sb_subscriber_count(sb_msg_id_t msg_id) {
#ifndef HOST_TEST
    uint32_t result;
    __asm__ volatile (
        "mov r0, %1\n"
        "svc %2\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "r" ((uint32_t)msg_id), "I" (SVC_SB_SUBSCRIBER_COUNT)
        : "r0"
    );
    return (uint8_t)result;
#else
    return __sb_subscriber_count(msg_id);
#endif
}

/**
 * @brief  Return the total route count via SVC gate.
 * @return Number of distinct msg_ids with at least one subscriber.
 */
uint8_t sb_route_count(void) {
#ifndef HOST_TEST
    uint32_t result;
    __asm__ volatile (
        "svc %1\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "I" (SVC_SB_ROUTE_COUNT)
        : "r0"
    );
    return (uint8_t)result;
#else
    return __sb_route_count();
#endif
}
