.syntax unified
.cpu cortex-m7
.thumb

.text


/*
 * os_yield_pendsv - yielded function tailored to pending service call method
 */

.global os_yield_pendsv
.type os_yield_pendsv %function

os_yield_pendsv:  // context switch to next task using pendsv style stack called in handler mode
    mrs r0, psp  // read current process stack pointer to r0
    stmdb r0!, {r4-r11}  // store current task's context on current process stack
    mov r4, r0  // save current process stack pointer to r4 after storing context

    ldr r0, =current_task_index
    ldr r1, =task_list
    ldr r5, =num_created_tasks
    ldrb r5, [r5]  // load task_count to r5
    ldrb r9, [r0]  // load current_task_index to r9

    ldrb r2, [r0] // load current_task_index to r2
    lsl r2, r2, #2  // current_task_index * 4 computation by left bit shift op (shift left by 2 bits)
    ldr r3, [r1, r2]  // load task_list[current_task_index] to r3
    str r4, [r3, #12] // store current process stack pointer to task_list[current_task_index]->stack_pointer
    
    ldrb r4, [r3, #16] // load task_tcb->task_state to r4
    cmp r4, #1 // RUNNING
    beq set_task_to_ready // if current task is RUNNING, set it to READY

    find_next_task:
        mov r6, r9    
        add r6, r6, #1
        udiv r7, r6, r5
        mul r8, r7, r5
        sub r9, r6, r8  // r9 = next_task_index
        lsl r10, r9, #2 // next_task_index * 4 computation by left bit shift op (shift left by 2 bits)
        ldr r11, [r1, r10] // load task_list[next_task_index] to r11
        ldrb r4, [r11, #16]  // load next task's task_state to r4

        cmp r4, #0  // COLD
        beq yield_postprocess

        cmp r4, #2  // READY
        beq yield_postprocess

        cmp r4, #4  // KILLED
        beq find_next_task

        cmp r4, #5  // FINISHED
        beq find_next_task

        cmp r4, #3  // BLOCKED
        beq check_sleep_ticks
    
    yield_postprocess:
        strb r9, [r0]  // update current_task_index to next_task_index
        ldr r1, [r11, #12]  // load next task's process stack pointer to r1
        mov r5, #1
        strb r5, [r11, #16]  // set next task's task_state to RUNNING
        cmp r4, #0  // if next task is COLD, branch to next task
        beq increment_runing_task_count  
        ldmia r1!, {r4-r11}  // if next task is warm, also resume software saved registers
        branch_to_next_task:
            msr psp, r1 // switch to next task's process stack
            bx lr

    check_sleep_ticks:
        ldr r7, =os_tick_count
        ldr r7, [r7]
        ldr r6, [r11, #20] // load next task's global_tick_paused to r6
        ldr r8, [r11, #24]  // load next task's ticks_to_pause to r8
        sub r10, r7, r6 // r10 -> elapsed_ticks = global_tick - tick at which next task was previously blocked
        cmp r8, r10 // ticks_to_pause vs elapsed_ticks
        bhs find_next_task  // if ticks_to_wait >= elapsed_ticks, continue to find next task
        b yield_postprocess

    set_task_to_ready:
        ldr r6, =scheduler_enabled
        ldrb r6, [r6]
        cmp r6, #0
        beq defer_ctx_switch

        mov r4, #2 // READY
        strb r4, [r3, #16]  // set task_tcb->task_state to READY
        b find_next_task

    increment_runing_task_count:
        ldr r7, =running_task_count
        ldrb r6, [r7]
        add r6, r6, #1
        strb r6, [r7]
        b branch_to_next_task

    defer_ctx_switch:
    	mov r11, r3
    	b yield_postprocess




/*
 * start_cold_task(uint32_t *stack_pointer)
 * argument *stack_pointer passed in register r0
 */

.global start_cold_task
.type start_cold_task %function

start_cold_task:  // simulates cold resume from pendsv style stack
    ldr r1, [r0, #12]  // load task_tcb->stack_pointer to r1
    msr psp, r1  // write r1 into PSP
    mov r1, #0x02 // Setting SPSEL bit in CONTROL register to enable PSP and not MSP
    msr control, r1
    isb               // Instruction sync barrier - flushes cpu instruction cache (FETCH, DECODE, EXECUTE pipelining)

    mov r1, #1
    strb r1, [r0, #16]  // set task_tcb->task_state to RUNNING

    pop {r0-r3, r12, lr}  // uses PSP not MSP as SPSEL was set to 1
    pop {r4}  // PC  
    pop {r5}  // PSR
    msr apsr_nzcvq, r5
    
    ldr r5, =os_running
    mov r6, #1
    strb r6, [r5]

    ldr r5, =running_task_count
    strb r6, [r5]

    cpsie i  // enable all IRQ maskable interrupts (since this is the first task)
    bx r4 // branch to PC stored in r4

