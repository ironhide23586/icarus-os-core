/**
 * @file    context_switch.s
 * @brief   ICARUS OS Context Switch Implementation (ARM Cortex-M7)
 * @version 0.1.0
 *
 * @details Low-level assembly routines for task context switching.
 *          Implements PendSV-based preemptive context switch.
 *
 * @note    This code runs in ITCM for zero wait-state execution.
 *
 * @see     ARMv7-M Architecture Reference Manual
 * @see     docs/do178c/design/SDD.md Section 5.2 - Context Switch
 *
 * @author  Souham Biswas
 * @date    2026
 *
 * @copyright Copyright (c) 2026 ICARUS Project
 */

.syntax unified
.cpu cortex-m7
.thumb

/* Place in ITCM for zero wait-state execution */
.section .itcm,"ax",%progbits

/* ============================================================================
 * TCB FIELD OFFSETS (must match icarus_types.h)
 * ========================================================================= */
.equ TCB_STACK_PTR,     12      /* offsetof(icarus_task_t, stack_pointer) */
.equ TCB_TASK_STATE,    16      /* offsetof(icarus_task_t, task_state) */
.equ TCB_TICK_PAUSED,   20      /* offsetof(icarus_task_t, global_tick_paused) */
.equ TCB_TICKS_PAUSE,   24      /* offsetof(icarus_task_t, ticks_to_pause) */
.equ TCB_DATA_PTR,      28      /* offsetof(icarus_task_t, data_pointer) */

/* Task state constants */
.equ STATE_COLD,        0
.equ STATE_RUNNING,     1
.equ STATE_READY,       2
.equ STATE_BLOCKED,     3
.equ STATE_KILLED,      4
.equ STATE_FINISHED,    5

/* ============================================================================
 * os_yield_pendsv - PendSV Context Switch Handler
 * ============================================================================
 *
 * Called from PendSV_Handler to perform context switch.
 * Saves current task context, selects next ready task, restores its context.
 *
 * Register usage:
 *   r0  - current_task_index address / scratch
 *   r1  - task_list address / next task SP
 *   r2  - scratch (index calculations)
 *   r3  - current TCB pointer
 *   r4  - current PSP / task state
 *   r5  - num_created_tasks
 *   r6  - scheduler_enabled / scratch
 *   r7  - os_tick_count / scratch
 *   r8  - scratch (tick calculations)
 *   r9  - next_task_index
 *   r10 - scratch (index * 4)
 *   r11 - next TCB pointer
 */

.global os_yield_pendsv
.type os_yield_pendsv, %function

os_yield_pendsv:
    /* Save current task context */
    mrs     r0, psp                     /* Read Process Stack Pointer */
    stmdb   r0!, {r4-r11}               /* Push R4-R11 to task stack */
    mov     r4, r0                      /* Save updated PSP */

    /* Load kernel state pointers */
    ldr     r0, =current_task_index
    ldr     r1, =task_list
    ldr     r5, =num_created_tasks
    ldrb    r5, [r5]                    /* r5 = task count */
    ldrb    r9, [r0]                    /* r9 = current index (will become next) */

    /* Save current task's stack pointer to TCB */
    ldrb    r2, [r0]                    /* r2 = current_task_index */
    lsl     r2, r2, #2                  /* r2 = index * 4 (pointer size) */
    ldr     r3, [r1, r2]                /* r3 = task_list[current_task_index] */
    str     r4, [r3, #TCB_STACK_PTR]    /* Save PSP to TCB */

    /* Check if current task should transition to READY */
    ldrb    r4, [r3, #TCB_TASK_STATE]
    cmp     r4, #STATE_RUNNING
    beq     set_task_to_ready

find_next_task:
    /* Round-robin: next = (current + 1) % num_tasks */
    mov     r6, r9
    add     r6, r6, #1
    udiv    r7, r6, r5                  /* r7 = (current+1) / num_tasks */
    mul     r8, r7, r5                  /* r8 = quotient * num_tasks */
    sub     r9, r6, r8                  /* r9 = next_task_index (remainder) */

    /* Load next task's TCB */
    lsl     r10, r9, #2                 /* r10 = next_index * 4 */
    ldr     r11, [r1, r10]              /* r11 = task_list[next_index] */
    ldrb    r4, [r11, #TCB_TASK_STATE]  /* r4 = next task state */

    /* State machine: determine if task is runnable */
    cmp     r4, #STATE_COLD
    beq     yield_postprocess           /* COLD tasks can start */

    cmp     r4, #STATE_READY
    beq     yield_postprocess           /* READY tasks can run */

    cmp     r4, #STATE_KILLED
    beq     find_next_task              /* Skip KILLED tasks */

    cmp     r4, #STATE_FINISHED
    beq     find_next_task              /* Skip FINISHED tasks */

    cmp     r4, #STATE_BLOCKED
    beq     check_sleep_ticks           /* Check if sleep expired */

    b       find_next_task              /* Unknown state, skip */

yield_postprocess:
    /* Switch to next task */
    strb    r9, [r0]                    /* Update current_task_index */

    /* Set up Memory Protection Unit for task data section */
    push    {r0-r3, r12, lr}
    ldr     r0, [r11, #TCB_DATA_PTR]
    bl      MPU_ConfigureTaskData
    pop     {r0-r3, r12, lr}

    ldr     r1, [r11, #TCB_STACK_PTR]   /* Load next task's SP */
    mov     r5, #STATE_RUNNING
    strb    r5, [r11, #TCB_TASK_STATE]  /* Set state to RUNNING */

    /* Check if task is cold (first run) */
    cmp     r4, #STATE_COLD
    beq     increment_running_count

    /* Warm task: restore software-saved registers */
    ldmia   r1!, {r4-r11}               /* Pop R4-R11 from task stack */

branch_to_next_task:
    msr     psp, r1                     /* Switch to next task's stack */
    bx      lr                          /* Return from exception */

check_sleep_ticks:
    /* Check if blocked task's sleep has expired */
    ldr     r7, =os_tick_count
    ldr     r7, [r7]                    /* r7 = current tick */
    ldr     r6, [r11, #TCB_TICK_PAUSED] /* r6 = tick when blocked */
    ldr     r8, [r11, #TCB_TICKS_PAUSE] /* r8 = ticks to sleep */
    sub     r10, r7, r6                 /* r10 = elapsed ticks */
    cmp     r8, r10                     /* ticks_to_pause vs elapsed */
    bhs     find_next_task              /* Still sleeping, try next */
    b       yield_postprocess           /* Sleep done, run this task */

set_task_to_ready:
    /* Check if scheduler is enabled before transitioning */
    ldr     r6, =scheduler_enabled
    ldrb    r6, [r6]
    cmp     r6, #0
    beq     defer_ctx_switch            /* Scheduler disabled, defer */

    mov     r4, #STATE_READY
    strb    r4, [r3, #TCB_TASK_STATE]   /* Set current to READY */
    b       find_next_task

increment_running_count:
    /* First time running this task */
    ldr     r7, =running_task_count
    ldrb    r6, [r7]
    add     r6, r6, #1
    strb    r6, [r7]
    b       branch_to_next_task

defer_ctx_switch:
    /* Scheduler disabled: re-run current task */
    mov     r11, r3                     /* Next = current */
    b       yield_postprocess

.size os_yield_pendsv, .-os_yield_pendsv

/* ============================================================================
 * start_cold_task - Initialize and Start First Task
 * ============================================================================
 *
 * Called once from os_start() to begin task execution.
 * Sets up PSP, enables interrupts, and jumps to first task.
 *
 * @param r0  Pointer to task control block (TCB)
 *
 * Stack frame expected (set up by os_create_task):
 *   [SP+24] xPSR
 *   [SP+20] PC (entry point)
 *   [SP+16] LR (os_exit_task)
 *   [SP+12] R12
 *   [SP+8]  R3
 *   [SP+4]  R2
 *   [SP+0]  R1
 *   [SP-4]  R0  <- Initial SP points here
 */

.global start_cold_task
.type start_cold_task, %function

start_cold_task:
    /* Load task's initial stack pointer */
    ldr     r1, [r0, #TCB_STACK_PTR]
    msr     psp, r1                     /* Set Process Stack Pointer */

    /* Mark task as RUNNING */
    mov     r1, #STATE_RUNNING
    strb    r1, [r0, #TCB_TASK_STATE]

    /* Set OS running flags BEFORE switching to unprivileged mode */
    ldr     r5, =os_running
    mov     r6, #1
    strb    r6, [r5]

    ldr     r5, =running_task_count
    strb    r6, [r5]

    /* Switch to PSP for thread mode */
    mov     r1, #0x03                   /* SPSEL = 1 (use PSP), nPRIV = 1 (unprivileged) */
    msr     control, r1
    isb                                 /* Instruction barrier */

    /* Pop exception frame from stack */
    pop     {r0-r3, r12, lr}            /* R0-R3, R12, LR */
    pop     {r4}                        /* PC -> r4 */
    pop     {r5}                        /* xPSR -> r5 */
    msr     apsr_nzcvq, r5              /* Restore flags */

    /* Enable interrupts and jump to task */
    cpsie   i                           /* Enable IRQ */
    bx      r4                          /* Branch to task entry point */

.size start_cold_task, .-start_cold_task

/* End of context_switch.s */
