/*

This file contains the structs and definitions supporting a task on ICARUS OS.

*/

//#pragma once

#ifndef __ICARUS_TASK_H__
#define __ICARUS_TASK_H__

#include <stdint.h>
#include <stdbool.h>

#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "usbd_cdc_if.h"
#include "usbd_def.h"
#include "cmsis_gcc.h"
#include <stddef.h>
#include "gpio.h"

#define MAX_TASKS 10
#define STACK_WORDS 512  // 512 - 4 byte words per stack
#define CPU_VREGISTERS_SIZE 16
#define PRINT_BUFFER_BYTES 64  // DO NOT CROSS 64
#define MAX_PRINT_RETRIES 10
#define MAX_TASK_NAME_LENGTH 32

#include "main.h"
#include "retarget_hal.h"


#if (STACK_WORDS % 2) != 0
#error "STACK_WORDS must keep stack 8-byte aligned (even number of 32-bit words)."
#endif

#if (PRINT_BUFFER_BYTES > 64)
#error "PRINT_BUFFER_BYTES must be <= 64 for USB FS CDC packet sizing."
#endif



//typedef enum {
//    TASK_COLD     = 0,
//    TASK_RUNNING  = 1,
//    TASK_READY    = 2,
//    TASK_BLOCKED  = 3,
//    TASK_KILLED   = 4,
//    TASK_FINISHED = 5
//} task_state_t;
//
//typedef enum {
//    PRI_LOW  = 0,
//    PRI_MED  = 1,
//    PRI_HIGH = 2
//} task_pri_t;


typedef uint8_t task_state_t;
typedef uint8_t task_pri_t;

enum {
    TASK_COLD = 0, TASK_RUNNING, TASK_READY, TASK_BLOCKED, TASK_KILLED, TASK_FINISHED
};
enum {
    PRI_LOW = 0, PRI_MED, PRI_HIGH
};


typedef struct {
    void (*function)(void);
    uint32_t *stack_base;
    uint32_t stack_size;
    uint32_t *stack_pointer;
    task_state_t task_state; // 0 - COLD, 1 - RUNNING, 2 - READY, 3 - BLOCKED, 4 - KILLED, 5 - FINISHED
    uint32_t global_tick_paused; // global tick at which task was paused
    uint32_t ticks_to_pause; // number of ticks to remain paused
    task_pri_t task_priority; // 0 - LOW, 1 - MEDIUM, 2 - HIGH
    char name[MAX_TASK_NAME_LENGTH];
} task_t;


void os_init(void);
uint32_t os_get_tick_count(void);
void os_create_task(task_t *task, void (*function)(void), uint32_t *stack, 
                    uint32_t stack_size, const char *name);
void os_register_task(void (*function)(void), const char *name);
void os_start(void);
void os_exit_task(void);
void os_kill_process(uint8_t task_index);
void suicide(void);
void os_yield(void);

uint32_t task_active_sleep(uint32_t ticks);
uint32_t task_blocking_sleep(uint32_t ticks);
uint32_t task_busy_wait(uint32_t ticks);

void task_start(task_t *task);
uint32_t os_get_task_ticks_remaining(void);
uint8_t os_get_running_task_count(void);
const char* os_get_current_task_name(void);

bool enqueue_print_buffer(uint8_t c);
void print_finished_tasks(void);


//void enter_critical();
//void exit_critical();

#endif /* __ICARUS_TASK_H__ */
