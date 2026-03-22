/**
 * @file    main.c
 * @brief   ICARUS OS Application Entry Point
 * @version 0.1.0
 *
 * @details Main entry point for the ICARUS Real-Time Operating System.
 *          Initializes hardware, registers demo/stress test tasks, and
 *          starts the kernel scheduler.
 *
 * @par Boot Sequence:
 *      1. hal_init() - Hardware initialization (clocks, peripherals)
 *      2. os_init() - Kernel initialization (task pools, system tasks)
 *      3. demo_tasks_init() - Register demo producer/consumer tasks
 *      4. stress_test_init() - Register stress test tasks (if enabled)
 *      5. os_start() - Start scheduler (never returns)
 *
 * @par Configuration:
 *      Set ENABLE_STRESS_TEST to 1 for stress testing, 0 for demo mode only.
 *
 * @see     docs/do178c/design/SDD.md Section 2 - System Architecture
 *
 * @author  Souham Biswas
 * @date    2025
 *
 * @copyright Copyright 2025 Souham Biswas
 *            https://github.com/ironhide23586/icarus-os-core
 *            Licensed under the Apache License, Version 2.0
 */

#include "main.h"
#include "interactive_tasks.h"

/* ============================================================================
 * CONFIGURATION
 * ========================================================================= */

/**
 * @brief Enable stress testing mode
 * @note  Set to 1 to enable stress test tasks, 0 for demo mode only
 */
#define ENABLE_STRESS_TEST  0

/**
 * @brief Enable demo tasks mode
 * @note  Set to 1 to enable demo producer/consumer tasks, 0 to disable
 */
#define ENABLE_DEMO_TASKS   0

/**
 * @brief Enable interactive input demo
 * @note  Set to 1 to enable interactive button/keyboard demo, 0 to disable
 */
#define ENABLE_INTERACTIVE  1

/** @brief ANSI escape code to show cursor (used in error path) */
#define ANSI_SHOW_CURSOR()  printf("\033[?25h")

/* ============================================================================
 * MAIN ENTRY POINT
 * ========================================================================= */

/**
 * @brief   Application entry point
 *
 * @details Initializes the system and starts the ICARUS kernel.
 *          This function should never return under normal operation.
 *
 * @return  int  Never returns (infinite loop on error)
 *
 * @par Execution Flow:
 *      1. Initialize hardware abstraction layer
 *      2. Initialize ICARUS kernel
 *      3. Register demo tasks (producer/consumer patterns)
 *      4. Register stress test tasks (if enabled)
 *      5. Start scheduler (does not return)
 *      6. Error handler (only reached on failure)
 *
 * @warning If this function returns, a critical error has occurred.
 *          The error path blinks the LED rapidly to indicate failure.
 */
int main(void)
{
    hal_init();
    
    os_init();

#if ENABLE_DEMO_TASKS
    demo_tasks_init();
#endif

#if ENABLE_STRESS_TEST
    stress_test_init();
#endif

#if ENABLE_INTERACTIVE
    interactive_tasks_init();
#endif

    os_start();

    /* Error path: restore cursor before error message */
    ANSI_SHOW_CURSOR();
    printf("ERROR: Should not be here\r\n");
    while (1) {
        LED_Blink(50, 900);
    }
}
