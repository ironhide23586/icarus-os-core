/**
 * @file    test_crc.c
 * @brief   Host-side unit tests for icarus/crc.h (CRC16-CCITT helper)
 *
 * @details The HW peripheral path is short-circuited under HOST_TEST and
 *          replaced by a portable bytewise loop, so these tests verify
 *          the bytewise implementation against well-known test vectors
 *          and a few corner cases.
 *
 * @author  Souham Biswas
 * @date    2026
 *
 * @copyright Copyright 2025-2026 Souham Biswas
 *            Licensed under the Apache License, Version 2.0
 */

#include "unity.h"
#include "icarus/crc.h"
#include <string.h>

/* ---- Known-good vectors -------------------------------------------------- */

/* The canonical CCITT-FALSE test vector: ASCII "123456789" → 0x29B1 */
static void test_crc16_ccitt_canonical_vector(void) {
    const uint8_t data[] = "123456789";
    TEST_ASSERT_EQUAL_HEX16(0x29B1u, crc16_ccitt(data, 9));
}

/* Single-byte CRCs */
static void test_crc16_ccitt_single_byte_zero(void) {
    const uint8_t data[1] = { 0x00 };
    TEST_ASSERT_EQUAL_HEX16(0xE1F0u, crc16_ccitt(data, 1));
}

static void test_crc16_ccitt_single_byte_ff(void) {
    const uint8_t data[1] = { 0xFF };
    /* crc = 0xFFFF ^ (0xFF << 8) = 0x00FF; eight left shifts (no XOR needed
     * because the MSB stays clear until the final shift) → 0xFF00. */
    TEST_ASSERT_EQUAL_HEX16(0xFF00u, crc16_ccitt(data, 1));
}

/* Empty / NULL inputs return the sentinel 0xFFFF (init value) */
static void test_crc16_ccitt_zero_length(void) {
    const uint8_t data[1] = { 0xAA };
    TEST_ASSERT_EQUAL_HEX16(0xFFFFu, crc16_ccitt(data, 0));
}

static void test_crc16_ccitt_null_data(void) {
    TEST_ASSERT_EQUAL_HEX16(0xFFFFu, crc16_ccitt(NULL, 0));
    TEST_ASSERT_EQUAL_HEX16(0xFFFFu, crc16_ccitt(NULL, 16));
}

/* Symmetry: same buffer twice produces the same result (no internal state leak) */
static void test_crc16_ccitt_repeatable(void) {
    uint8_t buf[64];
    for (uint16_t i = 0; i < 64; i++) buf[i] = (uint8_t)(i * 7 + 3);
    uint16_t a = crc16_ccitt(buf, 64);
    uint16_t b = crc16_ccitt(buf, 64);
    TEST_ASSERT_EQUAL_HEX16(a, b);
}

/* Sensitivity: flipping a bit changes the CRC */
static void test_crc16_ccitt_sensitive_to_single_bit(void) {
    uint8_t buf[16];
    memset(buf, 0xA5, sizeof(buf));
    uint16_t a = crc16_ccitt(buf, sizeof(buf));
    buf[7] ^= 0x01;
    uint16_t b = crc16_ccitt(buf, sizeof(buf));
    TEST_ASSERT_NOT_EQUAL_HEX16(a, b);
}

/* Length sensitivity: same prefix with different length differs */
static void test_crc16_ccitt_length_sensitive(void) {
    const uint8_t data[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    uint16_t a = crc16_ccitt(data, 4);
    uint16_t b = crc16_ccitt(data, 8);
    TEST_ASSERT_NOT_EQUAL_HEX16(a, b);
}

void run_crc_tests(void) {
    RUN_TEST(test_crc16_ccitt_canonical_vector);
    RUN_TEST(test_crc16_ccitt_single_byte_zero);
    RUN_TEST(test_crc16_ccitt_single_byte_ff);
    RUN_TEST(test_crc16_ccitt_zero_length);
    RUN_TEST(test_crc16_ccitt_null_data);
    RUN_TEST(test_crc16_ccitt_repeatable);
    RUN_TEST(test_crc16_ccitt_sensitive_to_single_bit);
    RUN_TEST(test_crc16_ccitt_length_sensitive);
}
