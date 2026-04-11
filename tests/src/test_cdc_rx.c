/**
 * @file    test_cdc_rx.c
 * @brief   Host-side unit tests for icarus/cdc_rx.h
 *
 * @details Under HOST_TEST the public wrappers short-circuit to the
 *          __ implementations and the ring backing store is plain RAM
 *          (DTCM_PRIV macro is a no-op), so these tests exercise the
 *          producer/consumer logic end-to-end.
 *
 * @author  Souham Biswas
 * @date    2026
 *
 * @copyright Copyright 2025-2026 Souham Biswas
 *            Licensed under the Apache License, Version 2.0
 */

#include "unity.h"
#include "icarus/cdc_rx.h"
#include <string.h>

static void test_cdc_rx_init_empty(void) {
    cdc_rx_init();
    TEST_ASSERT_EQUAL_UINT32(0, cdc_rx_available());
    uint8_t b;
    TEST_ASSERT_FALSE(cdc_rx_read_byte(&b));
}

static void test_cdc_rx_push_and_drain(void) {
    cdc_rx_init();
    const uint8_t data[] = { 'H', 'e', 'l', 'l', 'o' };
    cdc_rx_push(data, sizeof(data));
    TEST_ASSERT_EQUAL_UINT32(5, cdc_rx_available());

    for (uint32_t i = 0; i < sizeof(data); i++) {
        uint8_t b;
        TEST_ASSERT_TRUE(cdc_rx_read_byte(&b));
        TEST_ASSERT_EQUAL_UINT8(data[i], b);
    }
    TEST_ASSERT_EQUAL_UINT32(0, cdc_rx_available());
}

static void test_cdc_rx_fifo_order(void) {
    cdc_rx_init();
    uint8_t buf[16];
    for (uint8_t i = 0; i < 16; i++) buf[i] = (uint8_t)(0xA0 + i);
    cdc_rx_push(buf, sizeof(buf));

    for (uint8_t i = 0; i < 16; i++) {
        uint8_t b;
        TEST_ASSERT_TRUE(cdc_rx_read_byte(&b));
        TEST_ASSERT_EQUAL_UINT8(0xA0 + i, b);
    }
}

/* The ring is 512 bytes; one slot is reserved for the head==tail wraparound
 * sentinel, so the effective capacity is 511 bytes. Pushing more than that
 * silently drops the overflow. */
static void test_cdc_rx_fills_to_capacity(void) {
    cdc_rx_init();
    uint8_t big[600];
    for (uint16_t i = 0; i < 600; i++) big[i] = (uint8_t)(i & 0xFF);
    cdc_rx_push(big, 600);

    /* Whatever the ring accepted should be readable, and ≤ 511 bytes. */
    uint32_t avail = cdc_rx_available();
    TEST_ASSERT_TRUE(avail > 0);
    TEST_ASSERT_TRUE(avail <= CDC_RX_BUF_SIZE - 1);

    uint8_t b;
    uint32_t consumed = 0;
    while (cdc_rx_read_byte(&b)) {
        TEST_ASSERT_EQUAL_UINT8((uint8_t)(consumed & 0xFF), b);
        consumed++;
    }
    TEST_ASSERT_EQUAL_UINT32(avail, consumed);
}

static void test_cdc_rx_wrap_around(void) {
    cdc_rx_init();
    uint8_t pattern[200];
    for (uint16_t i = 0; i < 200; i++) pattern[i] = (uint8_t)i;

    /* Push 200, drain 200, push 200 again — head should wrap past the
     * buffer end internally without losing FIFO order. */
    cdc_rx_push(pattern, 200);
    uint8_t b;
    for (uint16_t i = 0; i < 200; i++) {
        TEST_ASSERT_TRUE(cdc_rx_read_byte(&b));
    }
    cdc_rx_push(pattern, 200);
    for (uint16_t i = 0; i < 200; i++) {
        TEST_ASSERT_TRUE(cdc_rx_read_byte(&b));
        TEST_ASSERT_EQUAL_UINT8((uint8_t)i, b);
    }
}

static void test_cdc_rx_push_zero_length(void) {
    cdc_rx_init();
    const uint8_t data[1] = { 0xAA };
    cdc_rx_push(data, 0);
    TEST_ASSERT_EQUAL_UINT32(0, cdc_rx_available());
}

static void test_cdc_rx_read_byte_when_empty(void) {
    cdc_rx_init();
    uint8_t b = 0xCC;
    TEST_ASSERT_FALSE(cdc_rx_read_byte(&b));
    /* Sentinel preserved when read fails. */
    TEST_ASSERT_EQUAL_UINT8(0xCC, b);
}

void run_cdc_rx_tests(void) {
    RUN_TEST(test_cdc_rx_init_empty);
    RUN_TEST(test_cdc_rx_push_and_drain);
    RUN_TEST(test_cdc_rx_fifo_order);
    RUN_TEST(test_cdc_rx_fills_to_capacity);
    RUN_TEST(test_cdc_rx_wrap_around);
    RUN_TEST(test_cdc_rx_push_zero_length);
    RUN_TEST(test_cdc_rx_read_byte_when_empty);
}
