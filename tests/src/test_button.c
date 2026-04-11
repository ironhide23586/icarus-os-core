/**
 * @file    test_button.c
 * @brief   Host-side unit tests for bsp/button.h
 *
 * @details Under HOST_TEST the BSP button module is backed by a
 *          software state variable that the unit tests drive via the
 *          `__button_host_*` hooks declared in `Core/Src/bsp/button.c`.
 *
 * @author  Souham Biswas
 * @date    2026
 *
 * @copyright Copyright 2025-2026 Souham Biswas
 *            Licensed under the Apache License, Version 2.0
 */

#include "unity.h"
#include "bsp/button.h"

extern void __button_host_set_pressed(bool pressed);
extern void __button_host_reset_state(void);

static void test_button_default_released(void) {
    __button_host_reset_state();
    TEST_ASSERT_FALSE(Button_IsPressed());
}

static void test_button_press_release(void) {
    __button_host_reset_state();
    __button_host_set_pressed(true);
    TEST_ASSERT_TRUE(Button_IsPressed());
    __button_host_set_pressed(false);
    TEST_ASSERT_FALSE(Button_IsPressed());
}

static void test_button_repeated_reads_stable(void) {
    __button_host_reset_state();
    __button_host_set_pressed(true);
    for (int i = 0; i < 10; i++) {
        TEST_ASSERT_TRUE(Button_IsPressed());
    }
    __button_host_set_pressed(false);
    for (int i = 0; i < 10; i++) {
        TEST_ASSERT_FALSE(Button_IsPressed());
    }
}

void run_button_tests(void) {
    RUN_TEST(test_button_default_released);
    RUN_TEST(test_button_press_release);
    RUN_TEST(test_button_repeated_reads_stable);
}
