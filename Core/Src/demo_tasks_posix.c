/**
 * @file    demo_tasks_posix.c
 * @brief   POSIX API Demo Tasks Implementation
 * @version 0.1.0
 *
 * @details Demonstration of ICARUS POSIX-compatible APIs including:
 *          - pthread_create / pthread_create_named
 *          - sem_init / sem_wait / sem_post
 *          - mq_open / mq_send / mq_receive
 *          - nanosleep / usleep
 *
 * @see     IEEE Std 1003.1-2017 (POSIX.1)
 *
 * @author  Souham Biswas
 * @date    2026
 *
 * @copyright Copyright (c) 2026 ICARUS Project
 */

#include "demo_tasks_posix.h"
#include "icarus/posix/posix.h"
#include "bsp/bsp_display.h"

#include <stdio.h>

/* ============================================================================
 * CONFIGURATION
 * ========================================================================= */

/* Semaphore demo configuration */
#define SEM_CAPACITY        10      /**< Semaphore max count */
#define PRODUCER_DELAY_MS   200     /**< Producer period in ms */
#define CONSUMER_DELAY_MS   190     /**< Consumer period in ms */

/* Message queue configuration */
#define MQ_SS_NAME          "/0"    /**< Single-Single queue */
#define MQ_SM_NAME          "/1"    /**< Single-Multi queue */
#define MQ_MS_NAME          "/2"    /**< Multi-Single queue */
#define MQ_MM_NAME          "/3"    /**< Multi-Multi queue */
#define MQ_CAPACITY         32      /**< Queue capacity in bytes */

/* Message sizes */
#define MSG_SIZE_SS         1       /**< Single byte for SS */
#define MSG_SIZE_SM         2       /**< 2 bytes for SM */
#define MSG_SIZE_MS         1       /**< 1 byte for MS */
#define MSG_SIZE_MM         3       /**< 3 bytes for MM */

/* Display rows */
#define ROW_PIPE_SS_P       14
#define ROW_PIPE_SS_C       15
#define ROW_PIPE_SM_P       16
#define ROW_PIPE_SM_C1      17
#define ROW_PIPE_SM_C2      18
#define ROW_PIPE_MS_P1      19
#define ROW_PIPE_MS_P2      20
#define ROW_PIPE_MS_C       21
#define ROW_PIPE_MM_P1      22
#define ROW_PIPE_MM_P2      23
#define ROW_PIPE_MM_C1      24
#define ROW_PIPE_MM_C2      25

/* History display columns */
#define HIST_COL_SS         78
#define HIST_COL_SM         92
#define HIST_COL_MS         106
#define HIST_COL_MM         120

/* Timing */
#define PIPE_SEND_DELAY_MS  400
#define PIPE_RECV_DELAY_MS  350
#define MSG_FLASH_MS        150
#define RENDER_INTERVAL_MS  20

/* ============================================================================
 * GLOBAL STATE
 * ========================================================================= */

/* POSIX semaphore for producer-consumer demo */
static sem_t g_sem;

/* POSIX message queue descriptors */
static mqd_t g_mq_ss = MQ_INVALID;
static mqd_t g_mq_sm = MQ_INVALID;
static mqd_t g_mq_ms = MQ_INVALID;
static mqd_t g_mq_mm = MQ_INVALID;

/* Message history buffers */
static msg_history_t hist_ss;
static msg_history_t hist_sm;
static msg_history_t hist_ms;
static msg_history_t hist_mm;

/* Thread handles (for reference, not used for join in this demo) */
static pthread_t g_threads[15];
static int g_thread_count = 0;

/* ============================================================================
 * HELPER FUNCTIONS
 * ========================================================================= */

/**
 * @brief Get current thread name (ICARUS extension)
 */
static const char* get_thread_name(void)
{
    return os_get_current_task_name();
}

/**
 * @brief Get current tick count
 */
static uint32_t get_ticks(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint32_t)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

/**
 * @brief Sleep for milliseconds using POSIX nanosleep
 */
static void sleep_ms(uint32_t ms)
{
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000L;
    nanosleep(&ts, NULL);
}

/* ============================================================================
 * SEMAPHORE DEMO TASKS
 * ========================================================================= */

/**
 * @brief Producer task using POSIX semaphore
 */
static void producer_task_posix(void)
{
    const char* task_name = get_thread_name();
    int sem_val;

    while (1) {
        uint32_t period_start = get_ticks();
        uint32_t elapsed;

        do {
            elapsed = get_ticks() - period_start;
            
            sem_getvalue(&g_sem, &sem_val);
            display_render_bar(ROW_TASK_A, task_name, elapsed, PRODUCER_DELAY_MS);
            display_render_vbar(ROW_HEARTBEAT, VBAR_COL, (uint32_t)sem_val, SEM_CAPACITY);

            if (elapsed < PRODUCER_DELAY_MS) {
                sleep_ms(RENDER_INTERVAL_MS);
            }
            elapsed = get_ticks() - period_start;
        } while (elapsed < PRODUCER_DELAY_MS);

        /* POSIX semaphore post (signal) */
        sem_post(&g_sem);

        sem_getvalue(&g_sem, &sem_val);
        display_render_vbar(ROW_HEARTBEAT, VBAR_COL, (uint32_t)sem_val, SEM_CAPACITY);
    }
}

/**
 * @brief Consumer task using POSIX semaphore
 */
static void consumer_task_posix(void)
{
    const char* task_name = get_thread_name();
    int sem_val;

    while (1) {
        uint32_t period_start = get_ticks();
        uint32_t elapsed;

        do {
            elapsed = get_ticks() - period_start;
            
            sem_getvalue(&g_sem, &sem_val);
            display_render_bar(ROW_TASK_B, task_name, elapsed, CONSUMER_DELAY_MS);
            display_render_vbar(ROW_HEARTBEAT, VBAR_COL, (uint32_t)sem_val, SEM_CAPACITY);

            if (elapsed < CONSUMER_DELAY_MS) {
                sleep_ms(RENDER_INTERVAL_MS);
            }
            elapsed = get_ticks() - period_start;
        } while (elapsed < CONSUMER_DELAY_MS);

        /* POSIX semaphore wait */
        sem_wait(&g_sem);

        sem_getvalue(&g_sem, &sem_val);
        display_render_vbar(ROW_HEARTBEAT, VBAR_COL, (uint32_t)sem_val, SEM_CAPACITY);
    }
}

/**
 * @brief Reference task using POSIX sleep
 */
static void reference_task_posix(void)
{
    const char* task_name = get_thread_name();
    const uint32_t period_ms = TASK_C_PERIOD_TICKS;  /* Reuse existing constant */

    while (1) {
        uint32_t period_start = get_ticks();
        uint32_t elapsed;

        do {
            elapsed = get_ticks() - period_start;
            display_render_bar(ROW_TASK_C, task_name, elapsed, period_ms);

            uint32_t remaining = (elapsed < period_ms) ? (period_ms - elapsed) : 0;
            if (remaining > 0) {
                uint32_t sleep_time = (remaining < RENDER_INTERVAL_MS) ? remaining : RENDER_INTERVAL_MS;
                sleep_ms(sleep_time);
            }
            elapsed = get_ticks() - period_start;
        } while (elapsed < period_ms);

        display_render_bar(ROW_TASK_C, task_name, period_ms, period_ms);
        sleep_ms(CYCLE_PAUSE_TICKS);
    }
}

/* ============================================================================
 * MESSAGE QUEUE DEMO - Single Producer -> Single Consumer
 * ========================================================================= */

static void mq_ss_producer(void)
{
    const char* task_name = get_thread_name();
    uint8_t msg = 0;
    uint32_t send_tick = 0;

    while (1) {
        uint32_t period_start = get_ticks();
        uint32_t elapsed;

        do {
            elapsed = get_ticks() - period_start;
            bool show_flash = (get_ticks() - send_tick) < MSG_FLASH_MS;
            display_render_producer(ROW_PIPE_SS_P, task_name, elapsed, PIPE_SEND_DELAY_MS,
                                   (uint8_t)(msg - 1), show_flash);
            display_render_msg_history(ROW_PIPE_SS_P, HIST_COL_SS, &hist_ss, "SS");

            if (elapsed < PIPE_SEND_DELAY_MS) {
                sleep_ms(RENDER_INTERVAL_MS);
            }
            elapsed = get_ticks() - period_start;
        } while (elapsed < PIPE_SEND_DELAY_MS);

        /* POSIX mq_send */
        mq_send(g_mq_ss, (const char*)&msg, MSG_SIZE_SS, 0);
        msg_history_add(&hist_ss, &msg, MSG_SIZE_SS, 0, true);
        send_tick = get_ticks();
        msg++;
    }
}

static void mq_ss_consumer(void)
{
    const char* task_name = get_thread_name();
    uint8_t msg = 0;
    uint8_t last_msg = 0;
    uint32_t recv_tick = 0;

    while (1) {
        uint32_t period_start = get_ticks();
        uint32_t elapsed;

        do {
            elapsed = get_ticks() - period_start;
            bool show_flash = (get_ticks() - recv_tick) < MSG_FLASH_MS;
            display_render_consumer(ROW_PIPE_SS_C, task_name, elapsed, PIPE_RECV_DELAY_MS,
                                   last_msg, show_flash);

            if (elapsed < PIPE_RECV_DELAY_MS) {
                sleep_ms(RENDER_INTERVAL_MS);
            }
            elapsed = get_ticks() - period_start;
        } while (elapsed < PIPE_RECV_DELAY_MS);

        /* POSIX mq_receive */
        mq_receive(g_mq_ss, (char*)&msg, MSG_SIZE_SS, NULL);
        msg_history_add(&hist_ss, &msg, MSG_SIZE_SS, 0, false);
        last_msg = msg;
        recv_tick = get_ticks();
    }
}

/* ============================================================================
 * MESSAGE QUEUE DEMO - Single Producer -> Multiple Consumers
 * ========================================================================= */

#define MQ_SM_SEND_DELAY_MS  150

static void mq_sm_producer(void)
{
    const char* task_name = get_thread_name();
    uint16_t seq = 0;
    uint8_t msg[2];
    uint32_t send_tick = 0;

    while (1) {
        uint32_t period_start = get_ticks();
        uint32_t elapsed;

        do {
            elapsed = get_ticks() - period_start;
            bool show_flash = (get_ticks() - send_tick) < MSG_FLASH_MS;
            display_render_producer(ROW_PIPE_SM_P, task_name, elapsed, MQ_SM_SEND_DELAY_MS,
                                   (uint8_t)(seq & 0xFF), show_flash);
            display_render_msg_history(ROW_PIPE_SM_P, HIST_COL_SM, &hist_sm, "SM");

            if (elapsed < MQ_SM_SEND_DELAY_MS) {
                sleep_ms(RENDER_INTERVAL_MS);
            }
            elapsed = get_ticks() - period_start;
        } while (elapsed < MQ_SM_SEND_DELAY_MS);

        msg[0] = (uint8_t)(seq >> 8);
        msg[1] = (uint8_t)(seq & 0xFF);
        mq_send(g_mq_sm, (const char*)msg, MSG_SIZE_SM, 0);
        msg_history_add(&hist_sm, msg, MSG_SIZE_SM, 0, true);
        send_tick = get_ticks();
        seq++;
    }
}

static void mq_sm_consumer1(void)
{
    const char* task_name = get_thread_name();
    uint8_t msg[2];
    uint8_t last_lo = 0;
    uint32_t recv_tick = 0;

    while (1) {
        uint32_t period_start = get_ticks();
        uint32_t elapsed;

        do {
            elapsed = get_ticks() - period_start;
            bool show_flash = (get_ticks() - recv_tick) < MSG_FLASH_MS;
            display_render_consumer(ROW_PIPE_SM_C1, task_name, elapsed, PIPE_RECV_DELAY_MS,
                                   last_lo, show_flash);

            if (elapsed < PIPE_RECV_DELAY_MS) {
                sleep_ms(RENDER_INTERVAL_MS);
            }
            elapsed = get_ticks() - period_start;
        } while (elapsed < PIPE_RECV_DELAY_MS);

        mq_receive(g_mq_sm, (char*)msg, MSG_SIZE_SM, NULL);
        msg_history_add(&hist_sm, msg, MSG_SIZE_SM, 0, false);
        last_lo = msg[1];
        recv_tick = get_ticks();
    }
}

static void mq_sm_consumer2(void)
{
    const char* task_name = get_thread_name();
    uint8_t msg[2];
    uint8_t last_lo = 0;
    uint32_t recv_tick = 0;
    const uint32_t delay = PIPE_RECV_DELAY_MS + 100;

    while (1) {
        uint32_t period_start = get_ticks();
        uint32_t elapsed;

        do {
            elapsed = get_ticks() - period_start;
            bool show_flash = (get_ticks() - recv_tick) < MSG_FLASH_MS;
            display_render_consumer(ROW_PIPE_SM_C2, task_name, elapsed, delay, last_lo, show_flash);

            if (elapsed < delay) {
                sleep_ms(RENDER_INTERVAL_MS);
            }
            elapsed = get_ticks() - period_start;
        } while (elapsed < delay);

        mq_receive(g_mq_sm, (char*)msg, MSG_SIZE_SM, NULL);
        msg_history_add(&hist_sm, msg, MSG_SIZE_SM, 1, false);
        last_lo = msg[1];
        recv_tick = get_ticks();
    }
}

/* ============================================================================
 * MESSAGE QUEUE DEMO - Multiple Producers -> Single Consumer
 * ========================================================================= */

static void mq_ms_producer1(void)
{
    const char* task_name = get_thread_name();
    uint8_t msg = 0;
    uint32_t send_tick = 0;

    while (1) {
        uint32_t period_start = get_ticks();
        uint32_t elapsed;

        do {
            elapsed = get_ticks() - period_start;
            bool show_flash = (get_ticks() - send_tick) < MSG_FLASH_MS;
            display_render_producer(ROW_PIPE_MS_P1, task_name, elapsed, PIPE_SEND_DELAY_MS,
                                   (uint8_t)(msg == 0 ? 99 : msg - 1), show_flash);
            display_render_msg_history(ROW_PIPE_MS_P1, HIST_COL_MS, &hist_ms, "MS");

            if (elapsed < PIPE_SEND_DELAY_MS) {
                sleep_ms(RENDER_INTERVAL_MS);
            }
            elapsed = get_ticks() - period_start;
        } while (elapsed < PIPE_SEND_DELAY_MS);

        mq_send(g_mq_ms, (const char*)&msg, MSG_SIZE_MS, 0);
        msg_history_add(&hist_ms, &msg, MSG_SIZE_MS, 0, true);
        send_tick = get_ticks();
        msg++;
        if (msg >= 100) msg = 0;
    }
}

static void mq_ms_producer2(void)
{
    const char* task_name = get_thread_name();
    uint8_t msg = 100;
    uint32_t send_tick = 0;
    const uint32_t delay = PIPE_SEND_DELAY_MS + 150;

    while (1) {
        uint32_t period_start = get_ticks();
        uint32_t elapsed;

        do {
            elapsed = get_ticks() - period_start;
            bool show_flash = (get_ticks() - send_tick) < MSG_FLASH_MS;
            display_render_producer(ROW_PIPE_MS_P2, task_name, elapsed, delay,
                                   (uint8_t)(msg == 100 ? 199 : msg - 1), show_flash);

            if (elapsed < delay) {
                sleep_ms(RENDER_INTERVAL_MS);
            }
            elapsed = get_ticks() - period_start;
        } while (elapsed < delay);

        mq_send(g_mq_ms, (const char*)&msg, MSG_SIZE_MS, 0);
        msg_history_add(&hist_ms, &msg, MSG_SIZE_MS, 1, true);
        send_tick = get_ticks();
        msg++;
        if (msg >= 200) msg = 100;
    }
}

static void mq_ms_consumer(void)
{
    const char* task_name = get_thread_name();
    uint8_t msg = 0;
    uint8_t last_msg = 0;
    uint32_t recv_tick = 0;

    while (1) {
        uint32_t period_start = get_ticks();
        uint32_t elapsed;

        do {
            elapsed = get_ticks() - period_start;
            bool show_flash = (get_ticks() - recv_tick) < MSG_FLASH_MS;
            display_render_consumer(ROW_PIPE_MS_C, task_name, elapsed, PIPE_RECV_DELAY_MS,
                                   last_msg, show_flash);

            if (elapsed < PIPE_RECV_DELAY_MS) {
                sleep_ms(RENDER_INTERVAL_MS);
            }
            elapsed = get_ticks() - period_start;
        } while (elapsed < PIPE_RECV_DELAY_MS);

        mq_receive(g_mq_ms, (char*)&msg, MSG_SIZE_MS, NULL);
        msg_history_add(&hist_ms, &msg, MSG_SIZE_MS, 0, false);
        last_msg = msg;
        recv_tick = get_ticks();
    }
}

/* ============================================================================
 * MESSAGE QUEUE DEMO - Multiple Producers -> Multiple Consumers
 * ========================================================================= */

static void mq_mm_producer1(void)
{
    const char* task_name = get_thread_name();
    uint16_t seq = 0;
    uint8_t msg[3];
    uint32_t send_tick = 0;

    while (1) {
        uint32_t period_start = get_ticks();
        uint32_t elapsed;

        do {
            elapsed = get_ticks() - period_start;
            bool show_flash = (get_ticks() - send_tick) < MSG_FLASH_MS;
            display_render_producer(ROW_PIPE_MM_P1, task_name, elapsed, PIPE_SEND_DELAY_MS,
                                   (uint8_t)(seq & 0xFF), show_flash);
            display_render_msg_history(ROW_PIPE_MM_P1, HIST_COL_MM, &hist_mm, "MM");

            if (elapsed < PIPE_SEND_DELAY_MS) {
                sleep_ms(RENDER_INTERVAL_MS);
            }
            elapsed = get_ticks() - period_start;
        } while (elapsed < PIPE_SEND_DELAY_MS);

        msg[0] = 0;
        msg[1] = (uint8_t)(seq >> 8);
        msg[2] = (uint8_t)(seq & 0xFF);
        mq_send(g_mq_mm, (const char*)msg, MSG_SIZE_MM, 0);
        msg_history_add(&hist_mm, msg, MSG_SIZE_MM, 0, true);
        send_tick = get_ticks();
        seq++;
    }
}

static void mq_mm_producer2(void)
{
    const char* task_name = get_thread_name();
    uint16_t seq = 0;
    uint8_t msg[3];
    uint32_t send_tick = 0;
    const uint32_t delay = PIPE_SEND_DELAY_MS + 200;

    while (1) {
        uint32_t period_start = get_ticks();
        uint32_t elapsed;

        do {
            elapsed = get_ticks() - period_start;
            bool show_flash = (get_ticks() - send_tick) < MSG_FLASH_MS;
            display_render_producer(ROW_PIPE_MM_P2, task_name, elapsed, delay,
                                   (uint8_t)(seq & 0xFF), show_flash);

            if (elapsed < delay) {
                sleep_ms(RENDER_INTERVAL_MS);
            }
            elapsed = get_ticks() - period_start;
        } while (elapsed < delay);

        msg[0] = 1;
        msg[1] = (uint8_t)(seq >> 8);
        msg[2] = (uint8_t)(seq & 0xFF);
        mq_send(g_mq_mm, (const char*)msg, MSG_SIZE_MM, 0);
        msg_history_add(&hist_mm, msg, MSG_SIZE_MM, 1, true);
        send_tick = get_ticks();
        seq++;
    }
}

static void mq_mm_consumer1(void)
{
    const char* task_name = get_thread_name();
    uint8_t msg[3];
    uint8_t last_lo = 0;
    uint32_t recv_tick = 0;

    while (1) {
        uint32_t period_start = get_ticks();
        uint32_t elapsed;

        do {
            elapsed = get_ticks() - period_start;
            bool show_flash = (get_ticks() - recv_tick) < MSG_FLASH_MS;
            display_render_consumer(ROW_PIPE_MM_C1, task_name, elapsed, PIPE_RECV_DELAY_MS,
                                   last_lo, show_flash);

            if (elapsed < PIPE_RECV_DELAY_MS) {
                sleep_ms(RENDER_INTERVAL_MS);
            }
            elapsed = get_ticks() - period_start;
        } while (elapsed < PIPE_RECV_DELAY_MS);

        mq_receive(g_mq_mm, (char*)msg, MSG_SIZE_MM, NULL);
        msg_history_add(&hist_mm, msg, MSG_SIZE_MM, 0, false);
        last_lo = msg[2];
        recv_tick = get_ticks();
    }
}

static void mq_mm_consumer2(void)
{
    const char* task_name = get_thread_name();
    uint8_t msg[3];
    uint8_t last_lo = 0;
    uint32_t recv_tick = 0;
    const uint32_t delay = PIPE_RECV_DELAY_MS + 50;

    while (1) {
        uint32_t period_start = get_ticks();
        uint32_t elapsed;

        do {
            elapsed = get_ticks() - period_start;
            bool show_flash = (get_ticks() - recv_tick) < MSG_FLASH_MS;
            display_render_consumer(ROW_PIPE_MM_C2, task_name, elapsed, delay, last_lo, show_flash);

            if (elapsed < delay) {
                sleep_ms(RENDER_INTERVAL_MS);
            }
            elapsed = get_ticks() - period_start;
        } while (elapsed < delay);

        mq_receive(g_mq_mm, (char*)msg, MSG_SIZE_MM, NULL);
        msg_history_add(&hist_mm, msg, MSG_SIZE_MM, 1, false);
        last_lo = msg[2];
        recv_tick = get_ticks();
    }
}

/* ============================================================================
 * INITIALIZATION
 * ========================================================================= */

void demo_tasks_posix_init(void)
{
    /* Initialize POSIX subsystem */
    posix_init();

    /* Initialize message history buffers */
    msg_history_init(&hist_ss);
    msg_history_init(&hist_sm);
    msg_history_init(&hist_ms);
    msg_history_init(&hist_mm);

    /* Initialize POSIX semaphore */
    if (sem_init(&g_sem, 0, SEM_CAPACITY) != 0) {
        printf("[ERROR] Failed to init semaphore\r\n");
    }

    /* Open POSIX message queues */
    struct mq_attr attr = {
        .mq_flags = 0,
        .mq_maxmsg = MQ_CAPACITY,
        .mq_msgsize = 4,
        .mq_curmsgs = 0
    };

    g_mq_ss = mq_open(MQ_SS_NAME, O_CREAT | O_RDWR, 0644, &attr);
    g_mq_sm = mq_open(MQ_SM_NAME, O_CREAT | O_RDWR, 0644, &attr);
    g_mq_ms = mq_open(MQ_MS_NAME, O_CREAT | O_RDWR, 0644, &attr);
    g_mq_mm = mq_open(MQ_MM_NAME, O_CREAT | O_RDWR, 0644, &attr);

    if (g_mq_ss == MQ_INVALID || g_mq_sm == MQ_INVALID ||
        g_mq_ms == MQ_INVALID || g_mq_mm == MQ_INVALID) {
        printf("[ERROR] Failed to open message queues\r\n");
    }

    /* Create threads using POSIX pthread_create_named (ICARUS extension) */
    g_thread_count = 0;

    /* Semaphore demo threads */
    pthread_create_named(&g_threads[g_thread_count++], NULL, producer_task_posix, NULL, "producer");
    pthread_create_named(&g_threads[g_thread_count++], NULL, consumer_task_posix, NULL, "consumer");
    pthread_create_named(&g_threads[g_thread_count++], NULL, reference_task_posix, NULL, "reference");

    /* Message queue demo: Single Producer -> Single Consumer */
    pthread_create_named(&g_threads[g_thread_count++], NULL, mq_ss_producer, NULL, "ss_prod");
    pthread_create_named(&g_threads[g_thread_count++], NULL, mq_ss_consumer, NULL, "ss_cons");

    /* Message queue demo: Single Producer -> Multiple Consumers */
    pthread_create_named(&g_threads[g_thread_count++], NULL, mq_sm_producer, NULL, "sm_prod");
    pthread_create_named(&g_threads[g_thread_count++], NULL, mq_sm_consumer1, NULL, "sm_con1");
    pthread_create_named(&g_threads[g_thread_count++], NULL, mq_sm_consumer2, NULL, "sm_con2");

    /* Message queue demo: Multiple Producers -> Single Consumer */
    pthread_create_named(&g_threads[g_thread_count++], NULL, mq_ms_producer1, NULL, "ms_prd1");
    pthread_create_named(&g_threads[g_thread_count++], NULL, mq_ms_producer2, NULL, "ms_prd2");
    pthread_create_named(&g_threads[g_thread_count++], NULL, mq_ms_consumer, NULL, "ms_cons");

    /* Message queue demo: Multiple Producers -> Multiple Consumers */
    pthread_create_named(&g_threads[g_thread_count++], NULL, mq_mm_producer1, NULL, "mm_prd1");
    pthread_create_named(&g_threads[g_thread_count++], NULL, mq_mm_producer2, NULL, "mm_prd2");
    pthread_create_named(&g_threads[g_thread_count++], NULL, mq_mm_consumer1, NULL, "mm_con1");
    pthread_create_named(&g_threads[g_thread_count++], NULL, mq_mm_consumer2, NULL, "mm_con2");

    printf("[INFO] POSIX demo: %d threads created\r\n", g_thread_count);
}
