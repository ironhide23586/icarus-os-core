/**
 * @file    test_event.c
 * @brief   Host-side unit tests for icarus/event.h
 *
 * @details Exercises the generic ring buffer + per-module severity squelch
 *          + drain semantics. Under HOST_TEST the public wrappers
 *          short-circuit to the __ implementations.
 *
 * @author  Souham Biswas
 * @date    2026
 *
 * @copyright Copyright 2025-2026 Souham Biswas
 *            Licensed under the Apache License, Version 2.0
 */

#include "unity.h"
#include "icarus/event.h"
#include <string.h>

static void test_event_init_clears_state(void) {
    event_init();
    TEST_ASSERT_EQUAL_UINT32(0, event_get_count());
    /* All squelch levels reset to EVENT_DEBUG (accept everything). */
    for (uint8_t i = 0; i < EVENT_MOD_MAX; i++) {
        TEST_ASSERT_EQUAL_INT(EVENT_DEBUG, event_get_squelch(i));
    }
}

static void test_event_emit_one_increments_count(void) {
    event_init();
    os_event(0, EVENT_INFO, 0x1234, NULL, 0);
    TEST_ASSERT_EQUAL_UINT32(1, event_get_count());
}

static void test_event_drain_returns_what_was_emitted(void) {
    event_init();
    uint8_t payload[4] = { 0xDE, 0xAD, 0xBE, 0xEF };
    os_event(2, EVENT_WARN, 0xABCD, payload, 4);

    event_entry_t out[1];
    uint8_t n = 0;
    TEST_ASSERT_TRUE(event_drain(out, 1, &n));
    TEST_ASSERT_EQUAL_UINT8(1, n);
    TEST_ASSERT_EQUAL_UINT8(2, out[0].module_id);
    TEST_ASSERT_EQUAL_UINT8((uint8_t)EVENT_WARN, out[0].severity);
    TEST_ASSERT_EQUAL_UINT16(0xABCD, out[0].event_id);
    TEST_ASSERT_EQUAL_HEX8(0xDE, out[0].payload[0]);
    TEST_ASSERT_EQUAL_HEX8(0xEF, out[0].payload[3]);
}

static void test_event_drain_empty_ring_returns_false(void) {
    event_init();
    event_entry_t out[4];
    uint8_t n = 99;
    TEST_ASSERT_FALSE(event_drain(out, 4, &n));
    TEST_ASSERT_EQUAL_UINT8(0, n);
}

static void test_event_squelch_drops_low_severity(void) {
    event_init();
    event_set_squelch(5, EVENT_WARN);
    TEST_ASSERT_EQUAL_INT(EVENT_WARN, event_get_squelch(5));

    /* DEBUG is below WARN — dropped. */
    os_event(5, EVENT_DEBUG, 1, NULL, 0);
    TEST_ASSERT_EQUAL_UINT32(0, event_get_count());

    /* WARN passes. */
    os_event(5, EVENT_WARN, 2, NULL, 0);
    TEST_ASSERT_EQUAL_UINT32(1, event_get_count());

    /* CRITICAL passes. */
    os_event(5, EVENT_CRITICAL, 3, NULL, 0);
    TEST_ASSERT_EQUAL_UINT32(2, event_get_count());
}

static void test_event_invalid_module_id_dropped(void) {
    event_init();
    os_event(EVENT_MOD_MAX, EVENT_INFO, 0, NULL, 0);
    os_event(EVENT_MOD_MAX + 5, EVENT_INFO, 0, NULL, 0);
    TEST_ASSERT_EQUAL_UINT32(0, event_get_count());
}

static void test_event_payload_truncated_to_12(void) {
    event_init();
    uint8_t big[20];
    for (uint8_t i = 0; i < 20; i++) big[i] = i;
    os_event(0, EVENT_INFO, 0, big, 20);

    event_entry_t out[1];
    uint8_t n = 0;
    TEST_ASSERT_TRUE(event_drain(out, 1, &n));
    /* Only the first 12 bytes should land in payload. */
    for (uint8_t i = 0; i < 12; i++) {
        TEST_ASSERT_EQUAL_UINT8(i, out[0].payload[i]);
    }
}

static void test_event_drain_partial(void) {
    event_init();
    for (uint8_t i = 0; i < 5; i++) {
        os_event(0, EVENT_INFO, (uint16_t)(i + 100), NULL, 0);
    }
    TEST_ASSERT_EQUAL_UINT32(5, event_get_count());

    event_entry_t out[3];
    uint8_t n = 0;
    TEST_ASSERT_TRUE(event_drain(out, 3, &n));
    TEST_ASSERT_EQUAL_UINT8(3, n);
    TEST_ASSERT_EQUAL_UINT16(100, out[0].event_id);
    TEST_ASSERT_EQUAL_UINT16(101, out[1].event_id);
    TEST_ASSERT_EQUAL_UINT16(102, out[2].event_id);
    TEST_ASSERT_EQUAL_UINT32(2, event_get_count());
}

static void test_event_drain_full_ring(void) {
    event_init();
    /* Fill the ring exactly. */
    for (uint16_t i = 0; i < EVENT_RING_SIZE; i++) {
        os_event(0, EVENT_INFO, i, NULL, 0);
    }
    TEST_ASSERT_EQUAL_UINT32(EVENT_RING_SIZE, event_get_count());

    /* One more emission overwrites the oldest entry (ring count stays at max). */
    os_event(0, EVENT_INFO, 999, NULL, 0);
    TEST_ASSERT_EQUAL_UINT32(EVENT_RING_SIZE, event_get_count());
}

void run_event_tests(void) {
    RUN_TEST(test_event_init_clears_state);
    RUN_TEST(test_event_emit_one_increments_count);
    RUN_TEST(test_event_drain_returns_what_was_emitted);
    RUN_TEST(test_event_drain_empty_ring_returns_false);
    RUN_TEST(test_event_squelch_drops_low_severity);
    RUN_TEST(test_event_invalid_module_id_dropped);
    RUN_TEST(test_event_payload_truncated_to_12);
    RUN_TEST(test_event_drain_partial);
    RUN_TEST(test_event_drain_full_ring);
}
