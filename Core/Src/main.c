/*
 * main.c
 *
 *  Created on: Jan 23, 2026
 *      Author: Souham Biswas
 *      GitHub: https://github.com/ironhide23586/icarus-os-core
 */

#include "icarus/icarus_task.h"
#include "bsp/retarget_hal.h"
#include "demo_tasks.h"
#include "demo_tasks_posix.h"
#include "stress_test.h"

#include <stdio.h>

/* Demo mode selection:
 * 0 = Native ICARUS API demo (os_register_task, semaphore_*, pipe_*)
 * 1 = POSIX API demo (pthread_create, sem_*, mq_*)
 */
#define USE_POSIX_DEMO      1

/* Set to 1 to enable stress testing, 0 for normal demo mode */
#define ENABLE_STRESS_TEST  1

/* ANSI escape code helper: show cursor (used in error path) */
#define ANSI_SHOW_CURSOR()  printf("\033[?25h")

int main(void)
{
    hal_init();
    os_init();

#if USE_POSIX_DEMO
    /* Use POSIX-compatible API demo */
    demo_tasks_posix_init();
#else
    /* Use native ICARUS API demo */
    demo_tasks_init();
#endif

#if ENABLE_STRESS_TEST
    stress_test_init();
#endif

    os_start();

    /* Error path: restore cursor before error message */
    ANSI_SHOW_CURSOR();
    printf("ERROR: Should not be here\r\n");
    while (1) {
        LED_Blink(50, 900);
    }
}
