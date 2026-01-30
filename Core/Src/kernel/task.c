/*
 * task.c
 *
 *  Created on: May 14, 2025
 *      Author: Souham Biswas
 *      GitHub: https://github.com/ironhide23586/icarus-os-core
 */

#include "kernel/task.h"
#include "bsp/display.h"

/* ITCM attribute for hot path functions - zero wait state execution */
#ifndef HOST_TEST
#define ITCM_FUNC __attribute__((section(".itcm")))
#define DTCM_DATA __attribute__((section(".dtcm")))
#else
#define ITCM_FUNC
#define DTCM_DATA
#endif

#ifndef SKIP_STATIC_ASSERTS
_Static_assert(offsetof(task_t, stack_pointer) == 12, "task_t.stack_pointer offset mismatch");
_Static_assert(offsetof(task_t, task_state) == 16, "task_t.task_state offset mismatch");
_Static_assert(offsetof(task_t, global_tick_paused) == 20, "task_t.global_tick_paused offset mismatch");
_Static_assert(offsetof(task_t, ticks_to_pause) == 24, "task_t.ticks_to_pause offset mismatch");
#endif

DTCM_DATA task_t* task_list[MAX_TASKS];
DTCM_DATA semaphore_t* semaphore_list[MAX_SEMAPHORES];
DTCM_DATA message_pipe_t* message_pipe_list[MAX_MESSAGE_QUEUES];
DTCM_DATA static task_t task_pool[MAX_TASKS];
DTCM_DATA static semaphore_t semaphore_pool[MAX_SEMAPHORES];
DTCM_DATA static message_pipe_t message_pipe_pool[MAX_MESSAGE_QUEUES];
DTCM_DATA static int8_t cleanup_task_idx[MAX_TASKS];
DTCM_DATA uint8_t current_task_index;
DTCM_DATA uint8_t running_task_count;
DTCM_DATA uint8_t num_created_tasks;
DTCM_DATA volatile uint32_t current_task_ticks_remaining;
DTCM_DATA volatile uint32_t ticks_per_task;
DTCM_DATA uint32_t cpu_vregisters[CPU_VREGISTERS_SIZE];
DTCM_DATA volatile uint32_t os_tick_count;
DTCM_DATA volatile uint8_t os_running;
DTCM_DATA volatile uint8_t critical_stack_depth;
DTCM_DATA volatile bool scheduler_enabled;
DTCM_DATA int8_t current_cleanup_task_idx;

// Stack pool placed in RAM_D1 (same as other BSS data)
// With MAX_TASKS=128: 128 × 512 × 4 = 256KB
static uint32_t stack_pool[MAX_TASKS][STACK_WORDS];


extern void start_cold_task(task_t *task);
extern void os_yield_trampoline(void);
extern void os_yield_pendsv(void);


static inline void enter_critical(void) {
	scheduler_enabled = false;
	critical_stack_depth++;
}


static inline void exit_critical(void) {
	if (--critical_stack_depth == 0)
		scheduler_enabled = true;
}


uint32_t task_busy_wait(uint32_t ticks) {  // must aleardy be in critical section
	uint32_t st = os_tick_count;
	uint32_t delta;
	while (1) {
#ifdef HOST_TEST
		// In test mode, auto-advance ticks to prevent infinite loop
		os_tick_count++;
#endif
		// cppcheck-suppress duplicateExpression
		// Initial delta may be 0, but will increase on subsequent iterations
		delta = os_tick_count - st;
		// cppcheck-suppress unsignedLessThanZero
		// ticks is unsigned, but comparison is correct - checking if enough time passed
		if (delta >= ticks) {
			break;
		}
	}
	return delta;
}


static void os_idle_task(void) {
    // Initialize display (only first task to run will do this)
    display_init();
    while (1) {
        // __WFI();  // put cpu in low power state and wait for next interrupt.
        os_yield(); // handle this later
    }
}


static void os_heartbeart_task(void) {
#if ENABLE_HEARTBEAT_VISUALIZATION
    const char* task_name = os_get_current_task_name();
#endif
    
    while (1) {
        if (os_running) {
#if ENABLE_HEARTBEAT_VISUALIZATION
            // ON phase: match LED_Blink timing exactly (Hdelay - 1)
            // Update display first, then LED to minimize delay
            display_render_banner(ROW_HEARTBEAT, task_name, true);
            task_active_sleep(8);
            LED_On();
            task_active_sleep(HEARTBEAT_ON_TICKS - 1);  // Match LED_Blink: Hdelay - 1
            
            // OFF phase: match LED_Blink timing exactly (Ldelay - 1)
            // Update display first, then LED to minimize delay
            display_render_banner(ROW_HEARTBEAT, task_name, false);
            task_active_sleep(8);
            LED_Off();
            task_active_sleep(HEARTBEAT_OFF_TICKS - 1);  // Match LED_Blink: Ldelay - 1
#else
            // Visualization disabled - just blink LED
            LED_Blink(HEARTBEAT_ON_TICKS, HEARTBEAT_OFF_TICKS);
#endif
        } else {
            // OS not running
#if ENABLE_HEARTBEAT_VISUALIZATION
            // Clear banner
            display_render_banner(ROW_HEARTBEAT, task_name, false);
            task_active_sleep(100);
#else
            // Just sleep
            task_active_sleep(100);
#endif
        }
    }
}


static inline void __init_sem(uint8_t semaphore_idx, uint32_t semaphore_count, bool should_engage) {
    semaphore_list[semaphore_idx]->count = semaphore_count;
    semaphore_list[semaphore_idx]->max_count = semaphore_count;
    semaphore_list[semaphore_idx]->tick_updated_at = os_tick_count;
    semaphore_list[semaphore_idx]->engaged = should_engage;
}


static inline void __init_pipe(uint8_t message_pipe_idx, uint8_t max_messages, bool should_engage) {
    message_pipe_list[message_pipe_idx]->count = 0;
    message_pipe_list[message_pipe_idx]->max_count = max_messages;
    for (uint16_t i = 0; i < MAX_MESSAGE_BUFFER_BYTES; i++) {
        message_pipe_list[message_pipe_idx]->buffer[i] = 0;
    }
    message_pipe_list[message_pipe_idx]->enqueue_idx = 0;
    message_pipe_list[message_pipe_idx]->dequeue_idx = 0;
    message_pipe_list[message_pipe_idx]->tick_updated_at = os_tick_count;
    message_pipe_list[message_pipe_idx]->engaged = should_engage;
}



void os_init(void) {
    os_running = 0;
    ticks_per_task = TICKS_PER_TASK;
    running_task_count = 0;
    current_task_index = 0;
    current_cleanup_task_idx = -1;
    uint8_t i;
    current_task_ticks_remaining = ticks_per_task;
    for (i = 0; i < MAX_TASKS; i++) {
        task_list[i] = &task_pool[i];
        cleanup_task_idx[i] = -1;
    }
    for (i = 0; i < MAX_SEMAPHORES; i++) {
        semaphore_list[i] = &semaphore_pool[i];
        __init_sem(i, 0, false);
    }
    for (i = 0; i < MAX_MESSAGE_QUEUES; i++) {
        message_pipe_list[i] = &message_pipe_pool[i];
        __init_pipe(i, 0, false);
    }
    for (i = 0; i < CPU_VREGISTERS_SIZE; i++) {
        cpu_vregisters[i] = 0;
    }

    os_register_task(os_idle_task, "ICARUS_KEEPALIVE_TASK");
    os_register_task(os_heartbeart_task, ">ICARUS_HEARTBEART<");
    scheduler_enabled = true;
}


ITCM_FUNC uint32_t os_get_tick_count(void) {
    return os_tick_count;
}


uint8_t os_get_running_task_count(void) {
    return running_task_count;
}


const char* os_get_current_task_name(void) {
    if (current_task_index < num_created_tasks && task_list[current_task_index] != NULL) {
        return task_list[current_task_index]->name;
    }
    return "unknown";
}


uint32_t os_get_task_ticks_remaining(void) {
    return current_task_ticks_remaining;
}


void task_start(task_t *task) {
    start_cold_task(task);
}


ITCM_FUNC void os_yield(void) {
    current_task_ticks_remaining = ticks_per_task;
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}


ITCM_FUNC uint32_t task_active_sleep(uint32_t ticks) {
    task_list[current_task_index]->global_tick_paused = os_tick_count;
    task_list[current_task_index]->ticks_to_pause = ticks;
    task_list[current_task_index]->task_state = TASK_BLOCKED; // BLOCKED
    os_yield();
    return os_tick_count - task_list[current_task_index]->global_tick_paused;
}


uint32_t task_blocking_sleep(uint32_t ticks) {
	enter_critical();
	uint32_t delta = task_busy_wait(ticks);
	exit_critical();
	return delta;
}


void os_start(void) {
    if (num_created_tasks == 0 || num_created_tasks > MAX_TASKS) {
        return;
    }
    task_start(task_list[current_task_index]);
}


void os_exit_task(void) {
    task_list[current_task_index]->task_state = TASK_FINISHED;  // FINISHED
    if (running_task_count > 0)
    	running_task_count--;
    if (current_cleanup_task_idx < (MAX_TASKS - 1)) {
        cleanup_task_idx[++current_cleanup_task_idx] = (int8_t)current_task_index;
    }
    os_yield();
}


void os_kill_process(uint8_t task_index) {
    if (task_index >= num_created_tasks) {
        return;
    }
    // After the check above, we know task_index < num_created_tasks
    // Explicit check for static analysis (cppcheck can't track num_created_tasks <= MAX_TASKS)
    if (task_index > 0 && task_index < num_created_tasks && task_list[task_index]->task_state < 4) {
        printf("[INFO] %s task killed %s task\r\n", task_list[current_task_index]->name, task_list[task_index]->name);
        task_list[task_index]->task_state = TASK_KILLED;  // KILLED
        if (running_task_count > 0)
        	running_task_count--;
        if (current_cleanup_task_idx < (MAX_TASKS - 1)) {
            cleanup_task_idx[++current_cleanup_task_idx] = (int8_t)task_index;
        }
        if (task_index == current_task_index)  // suicide 
            os_yield();
    } else {
        printf("[ERROR] Cannot kill task %d (index=%d, state=%d)\r\n", task_index, task_index, task_list[task_index]->task_state);
    }
}


void os_task_suicide(void) {
    printf("[INFO] %s task committed suicide.", task_list[current_task_index]->name);
    os_kill_process(current_task_index);
}


void os_create_task(task_t *task, void (*function)(void), uint32_t *stack, uint32_t stack_size, const char *name) {
    if (running_task_count >= MAX_TASKS) {
        return;
    }
    task->function = function;
    task->stack_base = stack;
    task->stack_size = stack_size;
    strncpy(task->name, name, MAX_TASK_NAME_LENGTH);
    task->name[MAX_TASK_NAME_LENGTH - 1] = '\0';

    task->task_state = TASK_COLD; // COLD
    task->task_priority = PRI_LOW; // LOW
    task->global_tick_paused = 0;
    task->ticks_to_pause = 0;
    uint32_t *stack_top = stack + stack_size - 1;
    
    // Push in reverse order (high to low address)
    // Cast through uintptr_t to avoid warnings on 64-bit hosts during testing

    *(stack_top--) = 0x01000000;                      // PSR
    *(stack_top--) = (uint32_t)(uintptr_t)function;   // PC

    *(stack_top--) = (uint32_t)(uintptr_t)os_exit_task;  // LR
    *(stack_top--) = 0;                  // R12
    *(stack_top--) = 0;                  // R3
    *(stack_top--) = 0;                  // R2
    *(stack_top--) = 0;                  // R1
    *(stack_top) = 0;                    // R0 ← SP points here

    task->stack_pointer = stack_top;
    
    num_created_tasks++;
}


void os_register_task(void (*function)(void), const char *name) {
    os_create_task(task_list[num_created_tasks], function, stack_pool[num_created_tasks], STACK_WORDS, name);
}


bool pipe_init(uint8_t pipe_idx, uint8_t pipe_capacity_bytes) {
    enter_critical();
    if (pipe_idx < MAX_MESSAGE_QUEUES && pipe_capacity_bytes > 0 && pipe_capacity_bytes <= MAX_MESSAGE_BUFFER_BYTES) {
        if (!message_pipe_list[pipe_idx]->engaged) {
            __init_pipe(pipe_idx, pipe_capacity_bytes, true);
            exit_critical();
            return true;
        }
        exit_critical();
        return false;
    }
    exit_critical();
    return false;
}


ITCM_FUNC bool pipe_enqueue(uint8_t pipe_idx, uint8_t* message, uint8_t message_bytes) {
    if (pipe_idx >= MAX_MESSAGE_QUEUES || !message_pipe_list[pipe_idx]->engaged || message_bytes > message_pipe_list[pipe_idx]->max_count)
        return false;
    while ((message_pipe_list[pipe_idx]->max_count - message_pipe_list[pipe_idx]->count) < message_bytes) {
        task_active_sleep(1);
        if ((message_pipe_list[pipe_idx]->max_count - message_pipe_list[pipe_idx]->count) >= message_bytes)
            scheduler_enabled = false;
    }
    enter_critical();
    for (uint8_t i = 0; i < message_bytes; i++) {
        message_pipe_list[pipe_idx]->buffer[message_pipe_list[pipe_idx]->enqueue_idx] = message[i];
        message_pipe_list[pipe_idx]->enqueue_idx = (uint8_t) (message_pipe_list[pipe_idx]->enqueue_idx + 1) % message_pipe_list[pipe_idx]->max_count;
        message_pipe_list[pipe_idx]->count++;
    }
    message_pipe_list[pipe_idx]->tick_updated_at = os_tick_count;
    exit_critical();
    return true;
}


ITCM_FUNC bool pipe_dequeue(uint8_t pipe_idx, uint8_t* message, uint8_t message_bytes) {
    if (pipe_idx >= MAX_MESSAGE_QUEUES || !message_pipe_list[pipe_idx]->engaged || message_bytes > message_pipe_list[pipe_idx]->max_count)
        return false;
    while (message_pipe_list[pipe_idx]->count < message_bytes) {
        task_active_sleep(1);
        if (message_pipe_list[pipe_idx]->count >= message_bytes)
            scheduler_enabled = false;
    }
    enter_critical();
    for (uint8_t i = 0; i < message_bytes; i++) {
        message[i] = message_pipe_list[pipe_idx]->buffer[message_pipe_list[pipe_idx]->dequeue_idx];
        message_pipe_list[pipe_idx]->dequeue_idx = (uint8_t) (message_pipe_list[pipe_idx]->dequeue_idx + 1) % message_pipe_list[pipe_idx]->max_count;
        message_pipe_list[pipe_idx]->count--;
    }
    message_pipe_list[pipe_idx]->tick_updated_at = os_tick_count;
    exit_critical();
    return true;
}


bool semaphore_init(uint8_t semaphore_idx, uint32_t semaphore_count) {
    enter_critical();
    if (semaphore_idx < MAX_SEMAPHORES && semaphore_count > 0) {
        if (!semaphore_list[semaphore_idx]->engaged) {
            __init_sem(semaphore_idx, semaphore_count, true);
            exit_critical();
            return true;
        }
        exit_critical();
        return false;
    }
    exit_critical();
    return false;
}


ITCM_FUNC bool semaphore_feed(uint8_t semaphore_idx) {
    if (semaphore_idx >= MAX_SEMAPHORES || !semaphore_list[semaphore_idx]->engaged)
        return false;
    while (semaphore_list[semaphore_idx]->count >= semaphore_list[semaphore_idx]->max_count) {
        task_active_sleep(1);
    }
    enter_critical();
    ++semaphore_list[semaphore_idx]->count;
    semaphore_list[semaphore_idx]->tick_updated_at = os_tick_count;
    exit_critical();
    return true;
}


ITCM_FUNC bool semaphore_consume(uint8_t semaphore_idx) {
    if (semaphore_idx >= MAX_SEMAPHORES || !semaphore_list[semaphore_idx]->engaged)
        return false;
    while (semaphore_list[semaphore_idx]->count == 0) {
        task_active_sleep(1);
    }
    enter_critical();
    --semaphore_list[semaphore_idx]->count;
    semaphore_list[semaphore_idx]->tick_updated_at = os_tick_count;
    exit_critical();
    return true;
}


uint32_t semaphore_get_count(uint8_t semaphore_idx) {
    if (semaphore_idx >= MAX_SEMAPHORES || !semaphore_list[semaphore_idx]->engaged)
        return 0;
    return semaphore_list[semaphore_idx]->count;
}


uint32_t semaphore_get_max_count(uint8_t semaphore_idx) {
    if (semaphore_idx >= MAX_SEMAPHORES || !semaphore_list[semaphore_idx]->engaged)
        return 0;
    return semaphore_list[semaphore_idx]->max_count;
}


uint8_t pipe_get_count(uint8_t pipe_idx) {
    if (pipe_idx >= MAX_MESSAGE_QUEUES || !message_pipe_list[pipe_idx]->engaged)
        return 0;
    return message_pipe_list[pipe_idx]->count;
}


uint8_t pipe_get_max_count(uint8_t pipe_idx) {
    if (pipe_idx >= MAX_MESSAGE_QUEUES || !message_pipe_list[pipe_idx]->engaged)
        return 0;
    return message_pipe_list[pipe_idx]->max_count;
}


