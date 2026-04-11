/**
 * @file    test_tables.c
 * @brief   Host-side unit tests for icarus/tables.h (generic table engine)
 *
 * @details Exercises the registry / staging / activate / commit / dump
 *          flow plus the schema-CRC + data-CRC validation. Under
 *          HOST_TEST the public wrappers short-circuit to the __
 *          implementations and tbl_activate runs the prepare/commit
 *          straight through.
 *
 * @author  Souham Biswas
 * @date    2026
 *
 * @copyright Copyright 2025-2026 Souham Biswas
 *            Licensed under the Apache License, Version 2.0
 */

#include "unity.h"
#include "icarus/tables.h"
#include "icarus/crc.h"
#include <string.h>

#define TEST_TBL_ID    0x42
#define TEST_TBL_SIZE  16
#define TEST_TBL_SCRC  0xBEEFu

static volatile uint16_t g_activate_calls;
static volatile uint8_t  g_activate_first_byte;
static bool              g_activate_should_succeed = true;

static bool capture_activate(const void *data, uint16_t len) {
    g_activate_calls++;
    if (data && len > 0) {
        g_activate_first_byte = ((const uint8_t *)data)[0];
    }
    return g_activate_should_succeed;
}

static void register_test_table(void) {
    tbl_descriptor_t desc;
    memset(&desc, 0, sizeof(desc));
    desc.id         = TEST_TBL_ID;
    desc.name[0]    = 't'; desc.name[1] = 'e'; desc.name[2] = 's'; desc.name[3] = 't';
    desc.size       = TEST_TBL_SIZE;
    desc.schema_crc = TEST_TBL_SCRC;
    desc.activate   = capture_activate;
    TEST_ASSERT_TRUE(tbl_register(&desc));
}

static void seed_full_load(uint8_t fill) {
    uint8_t buf[TEST_TBL_SIZE];
    memset(buf, fill, sizeof(buf));
    TEST_ASSERT_TRUE(tbl_load(TEST_TBL_ID, buf, TEST_TBL_SIZE, TEST_TBL_SCRC));
}

static void common_setup(void) {
    tbl_init();
    g_activate_calls = 0;
    g_activate_first_byte = 0;
    g_activate_should_succeed = true;
    register_test_table();
}

static void test_tbl_init_clears_registry(void) {
    tbl_init();
    TEST_ASSERT_EQUAL_UINT8(0, tbl_count());
}

static void test_tbl_register_basic(void) {
    tbl_init();
    register_test_table();
    TEST_ASSERT_EQUAL_UINT8(1, tbl_count());
    const tbl_descriptor_t *d = tbl_get_descriptor(TEST_TBL_ID);
    TEST_ASSERT_NOT_NULL(d);
    TEST_ASSERT_EQUAL_UINT16(TEST_TBL_SIZE, d->size);
    TEST_ASSERT_EQUAL_UINT16(TEST_TBL_SCRC, d->schema_crc);
}

static void test_tbl_register_null_fails(void) {
    tbl_init();
    TEST_ASSERT_FALSE(tbl_register(NULL));
}

static void test_tbl_register_duplicate_fails(void) {
    tbl_init();
    register_test_table();
    /* Same id, second register attempt. */
    tbl_descriptor_t desc;
    memset(&desc, 0, sizeof(desc));
    desc.id   = TEST_TBL_ID;
    desc.size = TEST_TBL_SIZE;
    desc.schema_crc = TEST_TBL_SCRC;
    TEST_ASSERT_FALSE(tbl_register(&desc));
}

static void test_tbl_register_invalid_size_fails(void) {
    tbl_init();
    tbl_descriptor_t desc;
    memset(&desc, 0, sizeof(desc));
    desc.id   = 0x99;
    desc.size = 0;
    TEST_ASSERT_FALSE(tbl_register(&desc));
    desc.size = TBL_MAX_SIZE + 1;
    TEST_ASSERT_FALSE(tbl_register(&desc));
}

static void test_tbl_load_unknown_id_fails(void) {
    tbl_init();
    uint8_t buf[8] = { 0 };
    TEST_ASSERT_FALSE(tbl_load(0xAA, buf, 8, 0));
}

static void test_tbl_load_null_data_fails(void) {
    common_setup();
    TEST_ASSERT_FALSE(tbl_load(TEST_TBL_ID, NULL, 4, TEST_TBL_SCRC));
}

static void test_tbl_load_overflow_rejected(void) {
    common_setup();
    uint8_t huge[TBL_MAX_SIZE + 16];
    memset(huge, 0xCC, sizeof(huge));
    TEST_ASSERT_FALSE(
        tbl_load(TEST_TBL_ID, huge, TBL_MAX_SIZE + 1, TEST_TBL_SCRC));
}

static void test_tbl_activate_round_trip(void) {
    common_setup();
    seed_full_load(0xA5);
    TEST_ASSERT_TRUE(tbl_activate(TEST_TBL_ID));
    TEST_ASSERT_EQUAL_UINT16(1, g_activate_calls);
    TEST_ASSERT_EQUAL_HEX8(0xA5, g_activate_first_byte);
}

static void test_tbl_activate_without_load_fails(void) {
    common_setup();
    TEST_ASSERT_FALSE(tbl_activate(TEST_TBL_ID));
    TEST_ASSERT_EQUAL_UINT16(0, g_activate_calls);
}

static void test_tbl_activate_schema_crc_mismatch(void) {
    common_setup();
    uint8_t buf[TEST_TBL_SIZE];
    memset(buf, 0x11, sizeof(buf));
    /* Wrong schema_crc. */
    TEST_ASSERT_TRUE(tbl_load(TEST_TBL_ID, buf, TEST_TBL_SIZE, 0xDEAD));
    TEST_ASSERT_FALSE(tbl_activate(TEST_TBL_ID));
    TEST_ASSERT_EQUAL_UINT16(0, g_activate_calls);
}

static void test_tbl_activate_callback_rejection(void) {
    common_setup();
    g_activate_should_succeed = false;
    seed_full_load(0x77);
    TEST_ASSERT_FALSE(tbl_activate(TEST_TBL_ID));
    /* Callback was invoked but returned false → no commit. */
    TEST_ASSERT_EQUAL_UINT16(1, g_activate_calls);
    /* Active buffer untouched — dump should show no active table. */
    uint8_t out[TEST_TBL_SIZE];
    TEST_ASSERT_EQUAL_INT16(-1, tbl_dump(TEST_TBL_ID, out, sizeof(out)));
}

static void test_tbl_dump_returns_active(void) {
    common_setup();
    seed_full_load(0x33);
    TEST_ASSERT_TRUE(tbl_activate(TEST_TBL_ID));

    uint8_t out[TEST_TBL_SIZE];
    int16_t n = tbl_dump(TEST_TBL_ID, out, sizeof(out));
    TEST_ASSERT_EQUAL_INT16(TEST_TBL_SIZE, n);
    for (uint16_t i = 0; i < TEST_TBL_SIZE; i++) {
        TEST_ASSERT_EQUAL_HEX8(0x33, out[i]);
    }
}

static void test_tbl_dump_unknown_id_fails(void) {
    common_setup();
    uint8_t out[TEST_TBL_SIZE];
    TEST_ASSERT_EQUAL_INT16(-1, tbl_dump(0xAA, out, sizeof(out)));
}

static void test_tbl_chunked_load(void) {
    common_setup();
    /* Load in two chunks of 8 bytes each. */
    uint8_t chunk_a[8], chunk_b[8];
    for (uint8_t i = 0; i < 8; i++) {
        chunk_a[i] = (uint8_t)(0x10 + i);
        chunk_b[i] = (uint8_t)(0x20 + i);
    }
    TEST_ASSERT_TRUE(tbl_load(TEST_TBL_ID, chunk_a, 8, TEST_TBL_SCRC));
    TEST_ASSERT_TRUE(tbl_load(TEST_TBL_ID, chunk_b, 8, TEST_TBL_SCRC));
    TEST_ASSERT_TRUE(tbl_activate(TEST_TBL_ID));

    uint8_t out[TEST_TBL_SIZE];
    int16_t n = tbl_dump(TEST_TBL_ID, out, sizeof(out));
    TEST_ASSERT_EQUAL_INT16(TEST_TBL_SIZE, n);
    TEST_ASSERT_EQUAL_HEX8(0x10, out[0]);
    TEST_ASSERT_EQUAL_HEX8(0x17, out[7]);
    TEST_ASSERT_EQUAL_HEX8(0x20, out[8]);
    TEST_ASSERT_EQUAL_HEX8(0x27, out[15]);
}

static void test_tbl_get_descriptor_unknown(void) {
    common_setup();
    TEST_ASSERT_NULL(tbl_get_descriptor(0xBB));
}

void run_tables_tests(void) {
    RUN_TEST(test_tbl_init_clears_registry);
    RUN_TEST(test_tbl_register_basic);
    RUN_TEST(test_tbl_register_null_fails);
    RUN_TEST(test_tbl_register_duplicate_fails);
    RUN_TEST(test_tbl_register_invalid_size_fails);
    RUN_TEST(test_tbl_load_unknown_id_fails);
    RUN_TEST(test_tbl_load_null_data_fails);
    RUN_TEST(test_tbl_load_overflow_rejected);
    RUN_TEST(test_tbl_activate_round_trip);
    RUN_TEST(test_tbl_activate_without_load_fails);
    RUN_TEST(test_tbl_activate_schema_crc_mismatch);
    RUN_TEST(test_tbl_activate_callback_rejection);
    RUN_TEST(test_tbl_dump_returns_active);
    RUN_TEST(test_tbl_dump_unknown_id_fails);
    RUN_TEST(test_tbl_chunked_load);
    RUN_TEST(test_tbl_get_descriptor_unknown);
}
