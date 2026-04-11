/**
 * @file    test_fs.c
 * @brief   Host-side unit tests for icarus/fs.h
 *
 * @details Exercises the RAM-backed flat-file filesystem end-to-end.
 *          Each test calls fs_init() so the store is always clean.
 *
 * @author  Souham Biswas
 * @date    2026
 *
 * @copyright Copyright 2025-2026 Souham Biswas
 *            Licensed under the Apache License, Version 2.0
 */

#include "unity.h"
#include "icarus/fs.h"
#include <string.h>

static void test_fs_init_clean_state(void) {
    fs_init();
    fs_stats_t s;
    fs_stats(&s);
    TEST_ASSERT_EQUAL_UINT32(FS_TOTAL_BYTES, s.total_bytes);
    TEST_ASSERT_EQUAL_UINT32(0, s.used_bytes);
    TEST_ASSERT_EQUAL_UINT32(FS_TOTAL_BYTES, s.free_bytes);
    TEST_ASSERT_EQUAL_UINT8(0, s.file_count);
}

static void test_fs_create_basic(void) {
    fs_init();
    fs_file_t f;
    TEST_ASSERT_TRUE(fs_create("hello.txt", &f));
    TEST_ASSERT_TRUE(f.valid);
}

static void test_fs_create_duplicate_fails(void) {
    fs_init();
    fs_file_t a, b;
    TEST_ASSERT_TRUE(fs_create("dupe", &a));
    TEST_ASSERT_FALSE(fs_create("dupe", &b));
}

static void test_fs_create_invalid_name(void) {
    fs_init();
    fs_file_t f;
    TEST_ASSERT_FALSE(fs_create(NULL, &f));
    TEST_ASSERT_FALSE(fs_create("", &f));
    /* Name longer than FS_MAX_NAME_LEN-1 (i.e. >=12 chars) is rejected. */
    TEST_ASSERT_FALSE(fs_create("toolongname12", &f));
}

/* Tiny inline name generator: "f00", "f01", … no libc dependency. */
static void make_name(char *out, uint8_t i) {
    out[0] = 'f';
    out[1] = (char)('0' + ((i / 10) % 10));
    out[2] = (char)('0' + (i % 10));
    out[3] = '\0';
}

static void test_fs_create_full_disk(void) {
    fs_init();
    char name[8];
    fs_file_t f;
    /* Fill all FS_MAX_FILES slots. */
    for (uint8_t i = 0; i < FS_MAX_FILES; i++) {
        make_name(name, i);
        TEST_ASSERT_TRUE(fs_create(name, &f));
    }
    /* One more should fail. */
    make_name(name, FS_MAX_FILES);
    TEST_ASSERT_FALSE(fs_create(name, &f));
}

static void test_fs_open_existing(void) {
    fs_init();
    fs_file_t a, b;
    TEST_ASSERT_TRUE(fs_create("alpha", &a));
    TEST_ASSERT_TRUE(fs_open("alpha", &b));
    TEST_ASSERT_EQUAL_UINT8(a.slot, b.slot);
}

static void test_fs_open_missing_fails(void) {
    fs_init();
    fs_file_t f;
    TEST_ASSERT_FALSE(fs_open("ghost", &f));
}

static void test_fs_write_read_roundtrip(void) {
    fs_init();
    fs_file_t f;
    TEST_ASSERT_TRUE(fs_create("data", &f));
    const uint8_t payload[] = "the quick brown fox";
    TEST_ASSERT_TRUE(fs_write(&f, payload, sizeof(payload)));

    uint8_t buf[64] = {0};
    uint16_t n = fs_read(&f, buf, sizeof(buf), 0);
    TEST_ASSERT_EQUAL_UINT16(sizeof(payload), n);
    TEST_ASSERT_EQUAL_MEMORY(payload, buf, sizeof(payload));
}

static void test_fs_write_appends(void) {
    fs_init();
    fs_file_t f;
    fs_create("append", &f);
    fs_write(&f, (const uint8_t *)"foo", 3);
    fs_write(&f, (const uint8_t *)"bar", 3);

    uint8_t buf[8] = {0};
    uint16_t n = fs_read(&f, buf, sizeof(buf), 0);
    TEST_ASSERT_EQUAL_UINT16(6, n);
    TEST_ASSERT_EQUAL_MEMORY("foobar", buf, 6);
}

static void test_fs_write_overflow_rejected(void) {
    fs_init();
    fs_file_t f;
    fs_create("big", &f);
    uint8_t huge[FS_MAX_FILE_SIZE + 16];
    memset(huge, 'X', sizeof(huge));
    /* The first FS_MAX_FILE_SIZE bytes fit; one more byte triggers overflow. */
    TEST_ASSERT_TRUE(fs_write(&f, huge, FS_MAX_FILE_SIZE));
    TEST_ASSERT_FALSE(fs_write(&f, huge, 1));
}

static void test_fs_delete_existing(void) {
    fs_init();
    fs_file_t f;
    fs_create("temp", &f);
    TEST_ASSERT_TRUE(fs_delete("temp"));
    fs_file_t reopen;
    TEST_ASSERT_FALSE(fs_open("temp", &reopen));
}

static void test_fs_delete_missing_fails(void) {
    fs_init();
    TEST_ASSERT_FALSE(fs_delete("nope"));
}

static void test_fs_list(void) {
    fs_init();
    fs_file_t f;
    fs_create("a", &f);
    fs_create("b", &f);
    fs_create("c", &f);

    fs_file_info_t infos[FS_MAX_FILES];
    uint8_t n = fs_list(infos, FS_MAX_FILES);
    TEST_ASSERT_EQUAL_UINT8(3, n);
}

static void test_fs_stats_after_writes(void) {
    fs_init();
    fs_file_t f;
    fs_create("s1", &f);
    fs_write(&f, (const uint8_t *)"hello", 5);
    fs_create("s2", &f);
    fs_write(&f, (const uint8_t *)"world!", 6);

    fs_stats_t s;
    fs_stats(&s);
    TEST_ASSERT_EQUAL_UINT8(2, s.file_count);
    TEST_ASSERT_EQUAL_UINT32(11, s.used_bytes);
    TEST_ASSERT_EQUAL_UINT32(FS_TOTAL_BYTES - 11, s.free_bytes);
}

static void test_fs_read_offset(void) {
    fs_init();
    fs_file_t f;
    fs_create("off", &f);
    fs_write(&f, (const uint8_t *)"abcdefghij", 10);

    uint8_t buf[8] = {0};
    /* Read 4 bytes from offset 3 → "defg". */
    uint16_t n = fs_read(&f, buf, 4, 3);
    TEST_ASSERT_EQUAL_UINT16(4, n);
    TEST_ASSERT_EQUAL_MEMORY("defg", buf, 4);
}

static void test_fs_write_invalid_handle(void) {
    fs_init();
    fs_file_t bogus = { .slot = 0, .valid = false };
    TEST_ASSERT_FALSE(fs_write(&bogus, (const uint8_t *)"x", 1));
}

void run_fs_tests(void) {
    RUN_TEST(test_fs_init_clean_state);
    RUN_TEST(test_fs_create_basic);
    RUN_TEST(test_fs_create_duplicate_fails);
    RUN_TEST(test_fs_create_invalid_name);
    RUN_TEST(test_fs_create_full_disk);
    RUN_TEST(test_fs_open_existing);
    RUN_TEST(test_fs_open_missing_fails);
    RUN_TEST(test_fs_write_read_roundtrip);
    RUN_TEST(test_fs_write_appends);
    RUN_TEST(test_fs_write_overflow_rejected);
    RUN_TEST(test_fs_delete_existing);
    RUN_TEST(test_fs_delete_missing_fails);
    RUN_TEST(test_fs_list);
    RUN_TEST(test_fs_stats_after_writes);
    RUN_TEST(test_fs_read_offset);
    RUN_TEST(test_fs_write_invalid_handle);
}
