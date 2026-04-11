/**
 * @file    test_iwdg.c
 * @brief   Host-side unit tests for bsp/iwdg.h
 *
 * @details Under HOST_TEST the BSP IWDG module is backed by a software
 *          model (in `Core/Src/bsp/iwdg.c`) so the API can be exercised
 *          off-target. The model exposes a small set of `__iwdg_host_*`
 *          hooks that the tests below use to inspect refresh counts and
 *          inject reset-flag state.
 *
 * @author  Souham Biswas
 * @date    2026
 *
 * @copyright Copyright 2025-2026 Souham Biswas
 *            Licensed under the Apache License, Version 2.0
 */

#include "unity.h"
#include "bsp/iwdg.h"

/* Host-only hooks exported from the iwdg.c HOST_TEST stub. */
extern uint32_t __iwdg_host_refresh_count(void);
extern void     __iwdg_host_set_reset_flag(bool value);
extern uint8_t  __iwdg_host_started(void);
extern void     __iwdg_host_reset_state(void);

static void test_iwdg_init_starts_module(void) {
    __iwdg_host_reset_state();
    TEST_ASSERT_EQUAL_UINT8(0, __iwdg_host_started());
    IWDG_Init(IWDG_PRESCALER_64, 1000);
    TEST_ASSERT_EQUAL_UINT8(1, __iwdg_host_started());
}

static void test_iwdg_refresh_counts(void) {
    __iwdg_host_reset_state();
    IWDG_Init(IWDG_PRESCALER_64, 1000);
    TEST_ASSERT_EQUAL_UINT32(0, __iwdg_host_refresh_count());

    IWDG_Refresh();
    IWDG_Refresh();
    IWDG_Refresh();
    TEST_ASSERT_EQUAL_UINT32(3, __iwdg_host_refresh_count());
}

static void test_iwdg_refresh_before_init_is_noop(void) {
    __iwdg_host_reset_state();
    /* Without an Init, refresh should not increment because the watchdog
     * has not been started. (Real hardware would simply ignore the key.) */
    IWDG_Refresh();
    TEST_ASSERT_EQUAL_UINT32(0, __iwdg_host_refresh_count());
}

static void test_iwdg_was_reset_default_false(void) {
    __iwdg_host_reset_state();
    TEST_ASSERT_FALSE(IWDG_WasReset());
}

static void test_iwdg_was_reset_after_inject(void) {
    __iwdg_host_reset_state();
    __iwdg_host_set_reset_flag(true);
    TEST_ASSERT_TRUE(IWDG_WasReset());
}

static void test_iwdg_clear_reset_flag(void) {
    __iwdg_host_reset_state();
    __iwdg_host_set_reset_flag(true);
    TEST_ASSERT_TRUE(IWDG_WasReset());
    IWDG_ClearResetFlag();
    TEST_ASSERT_FALSE(IWDG_WasReset());
}

static void test_iwdg_init_clamps_oversized_args(void) {
    __iwdg_host_reset_state();
    /* Should not crash on out-of-range prescaler / reload values. */
    IWDG_Init(99, 0xFFFF);
    IWDG_Init(0, 0);
    TEST_ASSERT_EQUAL_UINT8(1, __iwdg_host_started());
}

void run_iwdg_tests(void) {
    RUN_TEST(test_iwdg_init_starts_module);
    RUN_TEST(test_iwdg_refresh_counts);
    RUN_TEST(test_iwdg_refresh_before_init_is_noop);
    RUN_TEST(test_iwdg_was_reset_default_false);
    RUN_TEST(test_iwdg_was_reset_after_inject);
    RUN_TEST(test_iwdg_clear_reset_flag);
    RUN_TEST(test_iwdg_init_clamps_oversized_args);
}
