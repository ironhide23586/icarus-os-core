/**
 * @file    mqueue.h
 * @brief   ICARUS OS POSIX Message Queues API
 * @version 0.1.0
 *
 * @details POSIX.1-2017 (IEEE Std 1003.1) compatible message queue interface.
 *          Maps to native ICARUS message pipes.
 *
 * @par Supported Features:
 *      - Queue creation (mq_open with O_CREAT)
 *      - Send/receive (mq_send, mq_receive)
 *      - Timed operations (mq_timedsend, mq_timedreceive)
 *      - Attribute query (mq_getattr)
 *
 * @par Limitations:
 *      - Named queues use index-based naming ("/0", "/1", etc.)
 *      - Message priorities not supported (always 0)
 *      - mq_notify not supported
 *
 * @see     IEEE Std 1003.1-2017 (POSIX.1)
 * @see     https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/mqueue.h.html
 *
 * @author  Souham Biswas
 * @date    2026
 *
 * @copyright Copyright (c) 2026 ICARUS Project
 */

#ifndef ICARUS_POSIX_MQUEUE_H
#define ICARUS_POSIX_MQUEUE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include "icarus/posix/types.h"
#include "icarus/posix/errno.h"
#include "icarus/posix/time.h"
#include "icarus/posix/fcntl.h"

/* ============================================================================
 * MESSAGE QUEUE TYPES
 * ========================================================================= */

/**
 * @brief Message queue descriptor
 */
typedef int mqd_t;

/** @brief Invalid message queue descriptor */
#define MQ_INVALID      ((mqd_t)-1)

/**
 * @brief Message queue attributes
 */
struct mq_attr {
    long mq_flags;      /**< Message queue flags (O_NONBLOCK) */
    long mq_maxmsg;     /**< Maximum number of messages */
    long mq_msgsize;    /**< Maximum message size */
    long mq_curmsgs;    /**< Current number of messages */
};

/* ============================================================================
 * MESSAGE QUEUE FUNCTIONS
 * ========================================================================= */

/**
 * @brief Open a message queue
 *
 * @param[in] name   Queue name (format: "/N" where N is 0-63)
 * @param[in] oflag  Open flags (O_RDONLY, O_WRONLY, O_RDWR, O_CREAT, O_EXCL)
 * @param[in] mode   Permissions (ignored in ICARUS)
 * @param[in] attr   Queue attributes (NULL for defaults)
 *
 * @return Message queue descriptor, or MQ_INVALID on error
 *
 * @par Errors (errno):
 *      - EINVAL: Invalid name or attributes
 *      - EEXIST: O_CREAT|O_EXCL and queue exists
 *      - ENOENT: Queue doesn't exist and O_CREAT not set
 *      - EMFILE: Too many open queues
 *
 * @par Example:
 * @code
 *     struct mq_attr attr = { .mq_maxmsg = 10, .mq_msgsize = 32 };
 *     mqd_t mq = mq_open("/0", O_CREAT | O_RDWR, 0644, &attr);
 * @endcode
 */
mqd_t mq_open(const char *name, int oflag, ...);

/**
 * @brief Close a message queue
 *
 * @param[in] mqdes  Message queue descriptor
 *
 * @retval 0   Success
 * @retval -1  Error (check errno)
 */
int mq_close(mqd_t mqdes);

/**
 * @brief Remove a message queue
 *
 * @param[in] name  Queue name
 *
 * @retval 0   Success
 * @retval -1  Error (check errno)
 *
 * @note Queue is destroyed when last reference is closed
 */
int mq_unlink(const char *name);

/**
 * @brief Send a message to a queue
 *
 * @param[in] mqdes    Message queue descriptor
 * @param[in] msg_ptr  Message data
 * @param[in] msg_len  Message length
 * @param[in] msg_prio Message priority (ignored, always 0)
 *
 * @retval 0   Success
 * @retval -1  Error (check errno)
 *
 * @par Errors:
 *      - EBADF: Invalid descriptor
 *      - EMSGSIZE: Message too large
 *      - EAGAIN: Queue full and O_NONBLOCK set
 *
 * @note Maps to pipe_enqueue()
 */
int mq_send(mqd_t mqdes, const char *msg_ptr, size_t msg_len,
            unsigned int msg_prio);

/**
 * @brief Send a message with timeout
 *
 * @param[in] mqdes    Message queue descriptor
 * @param[in] msg_ptr  Message data
 * @param[in] msg_len  Message length
 * @param[in] msg_prio Message priority (ignored)
 * @param[in] abstime  Absolute timeout
 *
 * @retval 0   Success
 * @retval -1  Error (check errno)
 */
int mq_timedsend(mqd_t mqdes, const char *msg_ptr, size_t msg_len,
                 unsigned int msg_prio, const struct timespec *abstime);

/**
 * @brief Receive a message from a queue
 *
 * @param[in]  mqdes    Message queue descriptor
 * @param[out] msg_ptr  Buffer for message
 * @param[in]  msg_len  Buffer size
 * @param[out] msg_prio Message priority (always 0)
 *
 * @return Number of bytes received, or -1 on error
 *
 * @par Errors:
 *      - EBADF: Invalid descriptor
 *      - EMSGSIZE: Buffer too small
 *      - EAGAIN: Queue empty and O_NONBLOCK set
 *
 * @note Maps to pipe_dequeue()
 */
ssize_t mq_receive(mqd_t mqdes, char *msg_ptr, size_t msg_len,
                   unsigned int *msg_prio);

/**
 * @brief Receive a message with timeout
 *
 * @param[in]  mqdes    Message queue descriptor
 * @param[out] msg_ptr  Buffer for message
 * @param[in]  msg_len  Buffer size
 * @param[out] msg_prio Message priority (always 0)
 * @param[in]  abstime  Absolute timeout
 *
 * @return Number of bytes received, or -1 on error
 */
ssize_t mq_timedreceive(mqd_t mqdes, char *msg_ptr, size_t msg_len,
                        unsigned int *msg_prio, const struct timespec *abstime);

/**
 * @brief Get message queue attributes
 *
 * @param[in]  mqdes  Message queue descriptor
 * @param[out] attr   Attribute structure
 *
 * @retval 0   Success
 * @retval -1  Error (check errno)
 */
int mq_getattr(mqd_t mqdes, struct mq_attr *attr);

/**
 * @brief Set message queue attributes
 *
 * @param[in]  mqdes    Message queue descriptor
 * @param[in]  newattr  New attributes (only mq_flags used)
 * @param[out] oldattr  Previous attributes (may be NULL)
 *
 * @retval 0   Success
 * @retval -1  Error (check errno)
 */
int mq_setattr(mqd_t mqdes, const struct mq_attr *newattr,
               struct mq_attr *oldattr);

#ifdef __cplusplus
}
#endif

#endif /* ICARUS_POSIX_MQUEUE_H */
