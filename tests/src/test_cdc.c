/**
 * @file    test_cdc.c
 * @brief   Host-side unit tests for bsp/cdc.h
 *
 * @details Under HOST_TEST the BSP CDC module is backed by a small
 *          in-memory sink (see `Core/Src/bsp/cdc.c`) so we can verify
 *          the wrapper API without needing the ST USB CDC middleware.
 *          The host hooks (`__cdc_host_*`) let the tests inspect the
 *          last payload, retry-loop coverage, and the failure path.
 *
 * @author  Souham Biswas
 * @date    2026
 *
 * @copyright Copyright 2025-2026 Souham Biswas
 *            Licensed under the Apache License, Version 2.0
 */

#include "unity.h"
#include "bsp/cdc.h"
#include <string.h>

extern const uint8_t *__cdc_host_sink(void);
extern uint16_t       __cdc_host_sink_len(void);
extern uint32_t       __cdc_host_call_count(void);
extern void           __cdc_host_set_fail(bool fail);
extern void           __cdc_host_reset_state(void);

static void test_cdc_write_basic(void) {
    __cdc_host_reset_state();
    const uint8_t payload[] = { 'h', 'e', 'l', 'l', 'o' };
    TEST_ASSERT_TRUE(CDC_Write(payload, sizeof(payload)));
    TEST_ASSERT_EQUAL_UINT16(sizeof(payload), __cdc_host_sink_len());
    TEST_ASSERT_EQUAL_MEMORY(payload, __cdc_host_sink(), sizeof(payload));
    TEST_ASSERT_EQUAL_UINT32(1, __cdc_host_call_count());
}

static void test_cdc_write_zero_len_is_noop_success(void) {
    __cdc_host_reset_state();
    TEST_ASSERT_TRUE(CDC_Write((const uint8_t *)"x", 0));
    TEST_ASSERT_EQUAL_UINT16(0, __cdc_host_sink_len());
    TEST_ASSERT_EQUAL_UINT32(1, __cdc_host_call_count());
}

static void test_cdc_write_null_data_with_nonzero_len_fails(void) {
    __cdc_host_reset_state();
    TEST_ASSERT_FALSE(CDC_Write(NULL, 4));
}

static void test_cdc_write_failure_propagates(void) {
    __cdc_host_reset_state();
    __cdc_host_set_fail(true);
    const uint8_t payload[] = { 1, 2, 3 };
    TEST_ASSERT_FALSE(CDC_Write(payload, sizeof(payload)));
    /* Forced failure should not have populated the sink. */
    TEST_ASSERT_EQUAL_UINT16(0, __cdc_host_sink_len());
}

static void test_cdc_write_string_basic(void) {
    __cdc_host_reset_state();
    TEST_ASSERT_TRUE(CDC_WriteString("hello, world"));
    TEST_ASSERT_EQUAL_UINT16(12, __cdc_host_sink_len());
    TEST_ASSERT_EQUAL_MEMORY("hello, world", __cdc_host_sink(), 12);
}

static void test_cdc_write_string_null_is_success_noop(void) {
    __cdc_host_reset_state();
    TEST_ASSERT_TRUE(CDC_WriteString(NULL));
    /* No call into the underlying CDC_Write expected. */
    TEST_ASSERT_EQUAL_UINT32(0, __cdc_host_call_count());
}

static void test_cdc_write_string_empty_is_success(void) {
    __cdc_host_reset_state();
    TEST_ASSERT_TRUE(CDC_WriteString(""));
    /* Empty string still calls through with len=0. */
    TEST_ASSERT_EQUAL_UINT32(1, __cdc_host_call_count());
    TEST_ASSERT_EQUAL_UINT16(0, __cdc_host_sink_len());
}

void run_cdc_tests(void) {
    RUN_TEST(test_cdc_write_basic);
    RUN_TEST(test_cdc_write_zero_len_is_noop_success);
    RUN_TEST(test_cdc_write_null_data_with_nonzero_len_fails);
    RUN_TEST(test_cdc_write_failure_propagates);
    RUN_TEST(test_cdc_write_string_basic);
    RUN_TEST(test_cdc_write_string_null_is_success_noop);
    RUN_TEST(test_cdc_write_string_empty_is_success);
}
