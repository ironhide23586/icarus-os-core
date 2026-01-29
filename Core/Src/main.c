/*
 * main.c
 *
 *  Created on: Jan 23, 2026
 *      Author: Souham Biswas
 *      GitHub: https://github.com/ironhide23586/icarus-os-core
 */

#include "kernel/task.h"
#include "demo_tasks.h"
#include "stress_test.h"

// Set to 1 to enable stress testing, 0 for normal demo mode
#define ENABLE_STRESS_TEST  1

// ANSI escape code helper: show cursor (used in error path)
#define ANSI_SHOW_CURSOR()  printf("\033[?25h")

int main(void)
{
    hal_init();
    os_init();

    demo_tasks_init();

#if ENABLE_STRESS_TEST
    stress_test_init();
#endif

    os_start();

    // Error path: restore cursor before error message
    ANSI_SHOW_CURSOR();
    printf("ERROR: Should not be here\r\n");
    while (1) {
        LED_Blink(50, 900);
    }
}
