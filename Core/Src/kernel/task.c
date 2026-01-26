/*
 * task.c
 *
 *  Created on: May 14, 2025
 *      Author: Souham Biswas
 *      GitHub: https://github.com/ironhide23586/icarus-os-core
 */

#include "kernel/task.h"
#include "bsp/display.h"


#ifndef SKIP_STATIC_ASSERTS
_Static_assert(offsetof(task_t, stack_pointer) == 12, "task_t.stack_pointer offset mismatch");
_Static_assert(offsetof(task_t, task_state) == 16, "task_t.task_state offset mismatch");
_Static_assert(offsetof(task_t, global_tick_paused) == 20, "task_t.global_tick_paused offset mismatch");
_Static_assert(offsetof(task_t, ticks_to_pause) == 24, "task_t.ticks_to_pause offset mismatch");
#endif


task_t* task_list[MAX_TASKS];
static task_t task_pool[MAX_TASKS];
static uint32_t stack_pool[MAX_TASKS][STACK_WORDS];
static int8_t cleanup_task_idx[MAX_TASKS]; // when a task needs to be cleanup, its idx is stored here
static uint8_t print_buffer[PRINT_BUFFER_BYTES];

uint8_t current_task_index = 0;
uint8_t running_task_count = 0;
uint8_t num_created_tasks = 0;
int8_t current_cleanup_task_idx = -1;

volatile uint8_t print_buffer_dequeue_idx = 0;
volatile uint8_t print_buffer_enqueue_idx = 0;

volatile uint32_t current_task_ticks_remaining;
volatile uint32_t ticks_per_task;

uint32_t cpu_vregisters[CPU_VREGISTERS_SIZE];  // virtual registers for CPU to save and restore context

volatile uint32_t os_tick_count = 0;
volatile uint8_t os_running = 0;
volatile bool scheduler_enabled = true;

extern void start_cold_task(task_t *task);
extern void os_yield_trampoline(void);
extern void os_yield_pendsv(void);

/* Function prototypes */
void os_transmit_printf_task(void);

volatile uint8_t critical_stack_depth = 0;


static inline void enter_critical(void) {
	scheduler_enabled = false;
	critical_stack_depth++;
}


static inline void exit_critical(void) {
	if (--critical_stack_depth == 0)
		scheduler_enabled = true;
//	task_active_sleep(3);
}


static bool dequeue_print_buffer(uint8_t *out_c) {
	enter_critical();
	if (print_buffer_enqueue_idx == print_buffer_dequeue_idx) {
		exit_critical();
		return false;
	}
	*out_c = print_buffer[print_buffer_dequeue_idx++];
	if (print_buffer_dequeue_idx >= PRINT_BUFFER_BYTES)
		print_buffer_dequeue_idx = 0;
	exit_critical();
	return true;
}


bool enqueue_print_buffer(uint8_t c) {
	enter_critical();
	uint8_t next_enqueue_idx = (uint8_t)((print_buffer_enqueue_idx + 1) % PRINT_BUFFER_BYTES);
	if (next_enqueue_idx != print_buffer_dequeue_idx) {
		print_buffer[print_buffer_enqueue_idx] = c;
		print_buffer_enqueue_idx = next_enqueue_idx;
		exit_critical();
		return true;
	}
	exit_critical();
	return false;
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



void os_transmit_printf_task(void) {
	uint8_t retry_count;
	static uint8_t send_buffer[PRINT_BUFFER_BYTES];
	while (1) {
		uint8_t num_chars_to_print = 0;
		while (num_chars_to_print < sizeof(send_buffer)) {
			if(!dequeue_print_buffer(&send_buffer[num_chars_to_print]))
				break;
			num_chars_to_print++;
			if (send_buffer[num_chars_to_print - 1] == '\n')
				break;
		}
		if (num_chars_to_print > 0) {
			enter_critical();
			retry_count = 0;
			while (retry_count++ < MAX_PRINT_RETRIES) {
				if (CDC_Transmit_FS(send_buffer, num_chars_to_print) == USBD_OK)
					break;
				task_busy_wait(1);
			}
			task_busy_wait(10);
			exit_critical();
		} else {
			task_active_sleep(10);
		}
	}
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


void os_init() {
    os_running = 0;
    ticks_per_task = TICKS_PER_TASK;
    running_task_count = 0;
    current_task_index = 0;

    print_buffer_dequeue_idx = 0;
    print_buffer_enqueue_idx = 0;

    int i;
    current_task_ticks_remaining = ticks_per_task;
    for (i = 0; i < MAX_TASKS; i++) {
        task_list[i] = &task_pool[i];
        cleanup_task_idx[i] = -1;
    }
    for (i = 0; i < CPU_VREGISTERS_SIZE; i++) {
        cpu_vregisters[i] = 0;
    }
    for (i = 0; i < PRINT_BUFFER_BYTES; i++) {
    	print_buffer[i] = 0;
    }

    os_register_task(os_idle_task, "ICARUS_KEEPALIVE_TASK");
    os_register_task(os_heartbeart_task, ">ICARUS_HEARTBEART<");
    // os_register_task(os_transmit_printf_task, "ICARUS_PRINT_LISTENER_TASK");
}


uint32_t os_get_tick_count() {
    return os_tick_count;
}


uint8_t os_get_running_task_count() {
    return running_task_count;
}


const char* os_get_current_task_name() {
    if (current_task_index < num_created_tasks && task_list[current_task_index] != NULL) {
        return task_list[current_task_index]->name;
    }
    return "unknown";
}


uint32_t os_get_task_ticks_remaining() {
    return current_task_ticks_remaining;
}


void task_start(task_t *task) {
    start_cold_task(task);
}


void os_yield() {
    current_task_ticks_remaining = ticks_per_task;
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}


uint32_t task_active_sleep(uint32_t ticks) {
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


void os_start() {
    if (num_created_tasks == 0 || num_created_tasks > MAX_TASKS) {
        return;
    }
    task_start(task_list[current_task_index]);
}


void os_exit_task() {
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


void suicide() {
    printf("[INFO] %s task committed suicide.", task_list[current_task_index]->name);
    os_kill_process(current_task_index);
}


void print_finished_tasks() {
    printf("Finished task indices -> \t");
    for (int i = current_cleanup_task_idx; i >= 0; i--) {
        printf("%d\t", cleanup_task_idx[i]);
    }
    printf("\r\n");
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

    *(stack_top--) = 0x01000000;         // PSR
    *(stack_top--) = (uint32_t)function; // PC

    *(stack_top--) = (uint32_t)os_exit_task;         // LR
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
