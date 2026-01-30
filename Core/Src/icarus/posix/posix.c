/**
 * @file    posix.c
 * @brief   ICARUS OS POSIX API Implementation
 * @version 0.1.0
 *
 * @details Implementation of POSIX-compatible APIs mapping to native
 *          ICARUS kernel primitives.
 *
 * @see     IEEE Std 1003.1-2017 (POSIX.1)
 *
 * @author  Souham Biswas
 * @date    2026
 *
 * @copyright Copyright (c) 2026 ICARUS Project
 */

#include "icarus/posix/posix.h"
#include "icarus/icarus_task.h"
#include <string.h>

/* ============================================================================
 * GLOBAL ERROR NUMBER
 * ========================================================================= */

/** @brief Thread-local error number (global in single-core ICARUS) */
int errno = 0;

/* ============================================================================
 * INTERNAL STATE
 * ========================================================================= */

/** @brief Track which semaphore indices are used by POSIX layer */
static uint8_t posix_sem_map[ICARUS_MAX_SEMAPHORES];

/** @brief Track which pipe indices are used by POSIX layer */
static uint8_t posix_mq_map[ICARUS_MAX_MESSAGE_QUEUES];

/** @brief Next available semaphore index */
static uint8_t next_sem_idx = 0;

/** @brief Next available message queue index */
static uint8_t next_mq_idx = 0;

/* ============================================================================
 * POSIX INITIALIZATION
 * ========================================================================= */

void posix_init(void)
{
    errno = 0;
    next_sem_idx = 0;
    next_mq_idx = 0;
    memset(posix_sem_map, 0, sizeof(posix_sem_map));
    memset(posix_mq_map, 0, sizeof(posix_mq_map));
}

/* ============================================================================
 * PTHREAD ATTRIBUTE FUNCTIONS
 * ========================================================================= */

int pthread_attr_init(pthread_attr_t *attr)
{
    if (attr == NULL) {
        errno = EINVAL;
        return EINVAL;
    }

    attr->stacksize = ICARUS_STACK_WORDS * sizeof(uint32_t);
    attr->detachstate = PTHREAD_CREATE_JOINABLE;
    attr->schedpolicy = SCHED_RR;
    attr->priority = 0;

    return 0;
}

int pthread_attr_destroy(pthread_attr_t *attr)
{
    if (attr == NULL) {
        errno = EINVAL;
        return EINVAL;
    }
    /* Nothing to free - attributes are stack-allocated */
    return 0;
}

int pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize)
{
    if (attr == NULL || stacksize < 256) {
        errno = EINVAL;
        return EINVAL;
    }
    attr->stacksize = stacksize;
    return 0;
}

int pthread_attr_getstacksize(const pthread_attr_t *attr, size_t *stacksize)
{
    if (attr == NULL || stacksize == NULL) {
        errno = EINVAL;
        return EINVAL;
    }
    *stacksize = attr->stacksize;
    return 0;
}

int pthread_attr_setdetachstate(pthread_attr_t *attr, int detachstate)
{
    if (attr == NULL ||
        (detachstate != PTHREAD_CREATE_JOINABLE &&
         detachstate != PTHREAD_CREATE_DETACHED)) {
        errno = EINVAL;
        return EINVAL;
    }
    attr->detachstate = detachstate;
    return 0;
}

int pthread_attr_getdetachstate(const pthread_attr_t *attr, int *detachstate)
{
    if (attr == NULL || detachstate == NULL) {
        errno = EINVAL;
        return EINVAL;
    }
    *detachstate = attr->detachstate;
    return 0;
}

/* ============================================================================
 * PTHREAD THREAD FUNCTIONS
 * ========================================================================= */

/* External kernel state */
extern uint8_t current_task_index;
extern uint8_t num_created_tasks;
extern icarus_task_t* task_list[];

int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                   void (*start_routine)(void), void *arg)
{
    (void)arg;  /* arg not supported in current implementation */

    if (thread == NULL || start_routine == NULL) {
        errno = EINVAL;
        return EINVAL;
    }

    if (num_created_tasks >= ICARUS_MAX_TASKS) {
        errno = EAGAIN;
        return EAGAIN;
    }

    /* Register task with kernel */
    uint8_t task_idx = num_created_tasks;
    os_register_task(start_routine, "pthread");

    *thread = (pthread_t)task_idx;
    return 0;
}

void pthread_exit(void *retval)
{
    (void)retval;  /* Return value not supported */
    os_exit_task();
    /* Never returns */
    while (1) { }
}

int pthread_join(pthread_t thread, void **retval)
{
    if (retval != NULL) {
        *retval = NULL;  /* Return values not supported */
    }

    if (thread >= num_created_tasks) {
        errno = ESRCH;
        return ESRCH;
    }

    if (thread == current_task_index) {
        errno = EDEADLK;
        return EDEADLK;
    }

    /* Wait for thread to finish */
    while (task_list[thread]->task_state != TASK_STATE_FINISHED &&
           task_list[thread]->task_state != TASK_STATE_KILLED) {
        task_active_sleep(1);
    }

    return 0;
}

int pthread_detach(pthread_t thread)
{
    if (thread >= num_created_tasks) {
        errno = ESRCH;
        return ESRCH;
    }
    /* Detached threads are not tracked differently in ICARUS */
    return 0;
}

pthread_t pthread_self(void)
{
    return (pthread_t)current_task_index;
}

int pthread_equal(pthread_t t1, pthread_t t2)
{
    return (t1 == t2) ? 1 : 0;
}

int sched_yield(void)
{
    os_yield();
    return 0;
}

/* ============================================================================
 * SEMAPHORE FUNCTIONS
 * ========================================================================= */

/**
 * @brief Find a free semaphore slot
 * @return Semaphore index, or 0xFF if none available
 */
static uint8_t find_free_sem(void)
{
    for (uint8_t i = 0; i < ICARUS_MAX_SEMAPHORES; i++) {
        if (posix_sem_map[i] == 0) {
            return i;
        }
    }
    return 0xFF;
}

int sem_init(sem_t *sem, int pshared, unsigned int value)
{
    if (sem == NULL) {
        errno = EINVAL;
        return -1;
    }

    if (pshared != 0) {
        /* Process-shared semaphores not supported */
        errno = ENOSYS;
        return -1;
    }

    uint8_t idx = find_free_sem();
    if (idx == 0xFF) {
        errno = ENOSPC;
        return -1;
    }

    if (!semaphore_init(idx, value)) {
        errno = EINVAL;
        return -1;
    }

    posix_sem_map[idx] = 1;
    sem->__idx = idx;
    sem->__valid = 1;

    return 0;
}

int sem_destroy(sem_t *sem)
{
    if (sem == NULL || sem->__valid != 1) {
        errno = EINVAL;
        return -1;
    }

    posix_sem_map[sem->__idx] = 0;
    sem->__valid = 0;

    return 0;
}

int sem_wait(sem_t *sem)
{
    if (sem == NULL || sem->__valid != 1) {
        errno = EINVAL;
        return -1;
    }

    if (!semaphore_consume(sem->__idx)) {
        errno = EINVAL;
        return -1;
    }

    return 0;
}

int sem_trywait(sem_t *sem)
{
    if (sem == NULL || sem->__valid != 1) {
        errno = EINVAL;
        return -1;
    }

    if (semaphore_get_count(sem->__idx) == 0) {
        errno = EAGAIN;
        return -1;
    }

    if (!semaphore_consume(sem->__idx)) {
        errno = EINVAL;
        return -1;
    }

    return 0;
}

int sem_timedwait(sem_t *sem, const struct timespec *abstime)
{
    if (sem == NULL || sem->__valid != 1 || abstime == NULL) {
        errno = EINVAL;
        return -1;
    }

    /* Convert absolute time to relative ticks */
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);

    int32_t timeout_ms = (int32_t)((abstime->tv_sec - now.tv_sec) * 1000 +
                         (abstime->tv_nsec - now.tv_nsec) / 1000000);

    if (timeout_ms <= 0) {
        return sem_trywait(sem);
    }

    uint32_t start_tick = os_get_tick_count();
    while (semaphore_get_count(sem->__idx) == 0) {
        if ((os_get_tick_count() - start_tick) >= (uint32_t)timeout_ms) {
            errno = ETIMEDOUT;
            return -1;
        }
        task_active_sleep(1);
    }

    return sem_wait(sem);
}

int sem_post(sem_t *sem)
{
    if (sem == NULL || sem->__valid != 1) {
        errno = EINVAL;
        return -1;
    }

    if (!semaphore_feed(sem->__idx)) {
        errno = EOVERFLOW;
        return -1;
    }

    return 0;
}

int sem_getvalue(sem_t *sem, int *sval)
{
    if (sem == NULL || sem->__valid != 1 || sval == NULL) {
        errno = EINVAL;
        return -1;
    }

    *sval = (int)semaphore_get_count(sem->__idx);
    return 0;
}

/* ============================================================================
 * TIME FUNCTIONS
 * ========================================================================= */

int clock_gettime(clockid_t clk_id, struct timespec *tp)
{
    if (tp == NULL) {
        errno = EINVAL;
        return -1;
    }

    uint32_t ticks = os_get_tick_count();

    switch (clk_id) {
    case CLOCK_MONOTONIC:
    case CLOCK_REALTIME:
        /* Convert ticks to timespec (1 tick = 1ms) */
        tp->tv_sec = ticks / 1000;
        tp->tv_nsec = (ticks % 1000) * 1000000L;
        break;

    default:
        errno = EINVAL;
        return -1;
    }

    return 0;
}

int clock_getres(clockid_t clk_id, struct timespec *res)
{
    if (res == NULL) {
        errno = EINVAL;
        return -1;
    }

    switch (clk_id) {
    case CLOCK_MONOTONIC:
    case CLOCK_REALTIME:
        /* 1ms resolution */
        res->tv_sec = 0;
        res->tv_nsec = 1000000L;  /* 1ms in nanoseconds */
        break;

    default:
        errno = EINVAL;
        return -1;
    }

    return 0;
}

int nanosleep(const struct timespec *rqtp, struct timespec *rmtp)
{
    if (rqtp == NULL) {
        errno = EINVAL;
        return -1;
    }

    /* Convert to milliseconds (minimum 1ms) */
    uint32_t ms = (uint32_t)(rqtp->tv_sec * 1000 + rqtp->tv_nsec / 1000000);
    if (ms == 0 && (rqtp->tv_sec > 0 || rqtp->tv_nsec > 0)) {
        ms = 1;  /* Minimum sleep of 1ms */
    }

    task_active_sleep(ms);

    if (rmtp != NULL) {
        rmtp->tv_sec = 0;
        rmtp->tv_nsec = 0;
    }

    return 0;
}

unsigned int sleep(unsigned int seconds)
{
    task_active_sleep(seconds * 1000);
    return 0;
}

int usleep(useconds_t usec)
{
    if (usec >= 1000000) {
        errno = EINVAL;
        return -1;
    }

    /* Convert to milliseconds (minimum 1ms) */
    uint32_t ms = usec / 1000;
    if (ms == 0 && usec > 0) {
        ms = 1;
    }

    task_active_sleep(ms);
    return 0;
}

/* ============================================================================
 * MESSAGE QUEUE FUNCTIONS
 * ========================================================================= */

/**
 * @brief Parse queue name to index
 * @param name  Queue name (format: "/N" where N is 0-63)
 * @return Queue index, or -1 if invalid
 */
static int parse_mq_name(const char *name)
{
    if (name == NULL || name[0] != '/') {
        return -1;
    }

    int idx = 0;
    const char *p = name + 1;
    while (*p >= '0' && *p <= '9') {
        idx = idx * 10 + (*p - '0');
        p++;
    }

    if (*p != '\0' || idx >= ICARUS_MAX_MESSAGE_QUEUES) {
        return -1;
    }

    return idx;
}

mqd_t mq_open(const char *name, int oflag, ...)
{
    int idx = parse_mq_name(name);
    if (idx < 0) {
        errno = EINVAL;
        return MQ_INVALID;
    }

    bool exists = (posix_mq_map[idx] != 0);

    if ((oflag & O_CREAT) && (oflag & O_EXCL) && exists) {
        errno = EEXIST;
        return MQ_INVALID;
    }

    if (!(oflag & O_CREAT) && !exists) {
        errno = ENOENT;
        return MQ_INVALID;
    }

    if (!exists) {
        /* Create new queue with default attributes */
        if (!pipe_init((uint8_t)idx, ICARUS_MAX_MESSAGE_BYTES)) {
            errno = ENOSPC;
            return MQ_INVALID;
        }
        posix_mq_map[idx] = 1;
    }

    return (mqd_t)idx;
}

int mq_close(mqd_t mqdes)
{
    if (mqdes < 0 || mqdes >= ICARUS_MAX_MESSAGE_QUEUES) {
        errno = EBADF;
        return -1;
    }
    /* In ICARUS, close is a no-op (queue persists until unlink) */
    return 0;
}

int mq_unlink(const char *name)
{
    int idx = parse_mq_name(name);
    if (idx < 0 || posix_mq_map[idx] == 0) {
        errno = ENOENT;
        return -1;
    }

    posix_mq_map[idx] = 0;
    return 0;
}

int mq_send(mqd_t mqdes, const char *msg_ptr, size_t msg_len,
            unsigned int msg_prio)
{
    (void)msg_prio;  /* Priority not supported */

    if (mqdes < 0 || mqdes >= ICARUS_MAX_MESSAGE_QUEUES ||
        posix_mq_map[mqdes] == 0) {
        errno = EBADF;
        return -1;
    }

    if (msg_len > ICARUS_MAX_MESSAGE_BYTES) {
        errno = EMSGSIZE;
        return -1;
    }

    if (!pipe_enqueue((uint8_t)mqdes, (uint8_t *)msg_ptr, (uint8_t)msg_len)) {
        errno = EAGAIN;
        return -1;
    }

    return 0;
}

ssize_t mq_receive(mqd_t mqdes, char *msg_ptr, size_t msg_len,
                   unsigned int *msg_prio)
{
    if (msg_prio != NULL) {
        *msg_prio = 0;  /* Priority always 0 */
    }

    if (mqdes < 0 || mqdes >= ICARUS_MAX_MESSAGE_QUEUES ||
        posix_mq_map[mqdes] == 0) {
        errno = EBADF;
        return -1;
    }

    uint8_t count = pipe_get_count((uint8_t)mqdes);
    if (count == 0) {
        /* Block until data available */
        while ((count = pipe_get_count((uint8_t)mqdes)) == 0) {
            task_active_sleep(1);
        }
    }

    /* Read available bytes (up to msg_len) */
    uint8_t to_read = (count < msg_len) ? count : (uint8_t)msg_len;

    if (!pipe_dequeue((uint8_t)mqdes, (uint8_t *)msg_ptr, to_read)) {
        errno = EIO;
        return -1;
    }

    return (ssize_t)to_read;
}

int mq_getattr(mqd_t mqdes, struct mq_attr *attr)
{
    if (mqdes < 0 || mqdes >= ICARUS_MAX_MESSAGE_QUEUES ||
        posix_mq_map[mqdes] == 0 || attr == NULL) {
        errno = EBADF;
        return -1;
    }

    attr->mq_flags = 0;
    attr->mq_maxmsg = pipe_get_max_count((uint8_t)mqdes);
    attr->mq_msgsize = ICARUS_MAX_MESSAGE_BYTES;
    attr->mq_curmsgs = pipe_get_count((uint8_t)mqdes);

    return 0;
}

int mq_setattr(mqd_t mqdes, const struct mq_attr *newattr,
               struct mq_attr *oldattr)
{
    if (mqdes < 0 || mqdes >= ICARUS_MAX_MESSAGE_QUEUES ||
        posix_mq_map[mqdes] == 0) {
        errno = EBADF;
        return -1;
    }

    if (oldattr != NULL) {
        mq_getattr(mqdes, oldattr);
    }

    /* Only mq_flags can be changed (O_NONBLOCK) - not implemented */
    (void)newattr;

    return 0;
}

int mq_timedsend(mqd_t mqdes, const char *msg_ptr, size_t msg_len,
                 unsigned int msg_prio, const struct timespec *abstime)
{
    (void)abstime;  /* Timeout not fully implemented */
    return mq_send(mqdes, msg_ptr, msg_len, msg_prio);
}

ssize_t mq_timedreceive(mqd_t mqdes, char *msg_ptr, size_t msg_len,
                        unsigned int *msg_prio, const struct timespec *abstime)
{
    (void)abstime;  /* Timeout not fully implemented */
    return mq_receive(mqdes, msg_ptr, msg_len, msg_prio);
}
