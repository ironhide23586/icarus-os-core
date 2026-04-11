/**
 * @file    sb.h
 * @brief   ICARUS OS — Lightweight Software Bus (pub/sub message router)
 * @version 0.1.0
 *
 * @details A thin publish/subscribe routing layer on top of the kernel
 *          pipe IPC.  Tasks subscribe to message IDs; publishers push
 *          messages by ID and the bus copies the payload into every
 *          subscriber's pipe automatically.
 *
 *          Design constraints:
 *            - Zero dynamic allocation — all state is static in DTCM.
 *            - Each subscription binds a (msg_id, pipe_idx) pair.
 *            - A single msg_id can have up to @ref SB_MAX_SUBS_PER_MSG
 *              subscribers.
 *            - Publishing never blocks the caller; if a subscriber's
 *              pipe is full the message is silently dropped for that
 *              subscriber (best-effort delivery).
 *            - Hot-path functions are placed in ITCM for zero wait-state
 *              execution.
 *
 * @par Memory placement:
 *      - Route table: DTCM_DATA_PRIV (privileged-only, zero wait-state)
 *      - Functions:   ITCM_FUNC (zero wait-state instruction fetch)
 *
 * @see     icarus/pipe.h for the underlying byte-stream IPC
 * @see     docs/do178c/design/SDD.md Section 4.6 — Software Bus
 *
 * @author  Souham Biswas
 * @date    2026
 *
 * @copyright Copyright 2025-2026 Souham Biswas
 *            https://github.com/ironhide23586/icarus-os-core
 *            Licensed under the Apache License, Version 2.0
 */

#ifndef ICARUS_SB_H
#define ICARUS_SB_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/* ============================================================================
 * CONFIGURATION
 * ========================================================================= */

/**
 * @defgroup SB_CONFIG Software Bus Configuration
 * @brief    Compile-time sizing parameters
 * @{
 */

/**
 * @brief Maximum number of distinct message IDs that can have subscriptions.
 * @note  Each route entry uses ~12 bytes in DTCM.
 */
#ifndef SB_MAX_ROUTES
#define SB_MAX_ROUTES          32
#endif

/**
 * @brief Maximum subscribers per message ID.
 * @note  Typical value is 2–4.  Higher values increase per-route memory.
 */
#ifndef SB_MAX_SUBS_PER_MSG
#define SB_MAX_SUBS_PER_MSG    4
#endif

/** @} */

/* ============================================================================
 * TYPES
 * ========================================================================= */

/**
 * @brief  Software Bus message identifier.
 *
 * @details Application code defines the actual constants (e.g.
 *          @c SB_MSG_FAULT_CMD = 0x0001).  The bus treats them as
 *          opaque 16-bit values.
 */
typedef uint16_t sb_msg_id_t;

/* ============================================================================
 * PUBLIC API
 * ========================================================================= */

/**
 * @brief  Initialize the software bus.  Clears all routes.
 *
 * @pre    Kernel must be initialized (os_init() called).
 * @post   Route table is empty; ready for sb_subscribe() calls.
 */
void sb_init(void);

/**
 * @brief  Subscribe a pipe to a message ID.
 *
 * @param[in] msg_id    Message identifier to subscribe to.
 * @param[in] pipe_idx  Kernel pipe index that will receive copies of
 *                      published messages with this ID.  The pipe must
 *                      already be initialized via pipe_init().
 *
 * @retval true   Subscription added successfully.
 * @retval false  Route table full, subscriber limit reached for this
 *                msg_id, or duplicate (msg_id, pipe_idx) pair.
 */
bool sb_subscribe(sb_msg_id_t msg_id, uint8_t pipe_idx);

/**
 * @brief  Remove a subscription.
 *
 * @param[in] msg_id    Message identifier.
 * @param[in] pipe_idx  Pipe to unsubscribe.
 *
 * @retval true   Subscription found and removed.
 * @retval false  No matching subscription exists.
 */
bool sb_unsubscribe(sb_msg_id_t msg_id, uint8_t pipe_idx);

/**
 * @brief  Publish a message to all subscribers of @p msg_id.
 *
 * @param[in] msg_id  Message identifier.
 * @param[in] data    Payload bytes (must not be NULL when len > 0).
 * @param[in] len     Payload length in bytes.
 *
 * @return Number of subscribers the message was successfully enqueued to.
 *         Subscribers whose pipes are full are silently skipped.
 *
 * @note   Never blocks.  Uses pipe_can_enqueue() to test capacity
 *         before each write.
 */
uint8_t sb_publish(sb_msg_id_t msg_id, const uint8_t *data, uint8_t len);

/**
 * @brief  Return the number of active subscriptions for a message ID.
 *
 * @param[in] msg_id  Message identifier.
 * @return Subscriber count (0 if msg_id has no route).
 */
uint8_t sb_subscriber_count(sb_msg_id_t msg_id);

/**
 * @brief  Return the total number of route entries in use.
 * @return Count of distinct msg_ids that have at least one subscriber.
 */
uint8_t sb_route_count(void);

/* ============================================================================
 * PRIVILEGED IMPLEMENTATIONS (Internal — Do Not Call Directly)
 * ========================================================================= */

void    __sb_init(void);
bool    __sb_subscribe(sb_msg_id_t msg_id, uint8_t pipe_idx);
bool    __sb_unsubscribe(sb_msg_id_t msg_id, uint8_t pipe_idx);
uint8_t __sb_publish(sb_msg_id_t msg_id, const uint8_t *data, uint8_t len);
uint8_t __sb_subscriber_count(sb_msg_id_t msg_id);
uint8_t __sb_route_count(void);

#ifdef __cplusplus
}
#endif

#endif /* ICARUS_SB_H */
