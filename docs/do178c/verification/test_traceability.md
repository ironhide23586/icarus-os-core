# Test Traceability Matrix

**Document ID:** ICARUS-VER-003  
**Version:** 1.0  
**Date:** 2025-02-07  
**Status:** Current  
**Classification:** Public (Open Source)  

## 1. Purpose

This document provides traceability between software requirements, test cases, and source code in accordance with DO-178C objectives A-5 (Software Verification Traceability).

## 2. Traceability Overview

```
Requirements (HLR/LLR) → Test Cases → Source Code
         ↓                    ↓            ↓
    Verified by          Executes      Implements
```

## 3. Functional Requirements Traceability

### 3.1 Kernel - Task Management

| Req ID | Requirement | Test Case(s) | Source Function |
|--------|-------------|--------------|-----------------|
| KRN-001 | System shall initialize kernel state | `test_os_init` | `__os_init()` |
| KRN-002 | System shall support task registration | `test_os_register_task`, `test_os_register_task_multiple` | `__os_register_task()` |
| KRN-003 | System shall create tasks with stack allocation | `test_os_create_task_normal`, `test_os_create_task_long_name`, `test_os_create_task_boundary_max_minus_one` | `__os_register_task()` |
| KRN-004 | System shall enforce maximum task limit | `test_os_create_task_max_tasks`, `test_os_create_task_max_running_tasks` | `__os_register_task()` |
| KRN-005 | System shall start task execution | `test_os_start_valid_tasks`, `test_os_start_no_tasks`, `test_os_start_too_many_tasks`, `test_os_start_boundary_max_tasks` | `__os_start()` |
| KRN-006 | System shall support task yielding | `test_os_yield` | `__os_yield()` |
| KRN-007 | System shall support task exit | `test_os_exit_task_with_running_count`, `test_os_exit_task_zero_running_count` | `__os_exit_task()` |
| KRN-008 | System shall support task termination | `test_os_kill_process_valid`, `test_os_kill_process_suicide`, `test_os_kill_process_already_killed` | `__os_kill_process()` |
| KRN-009 | System shall protect task 0 from termination | `test_os_kill_process_index_zero`, `test_os_kill_process_index_zero_error`, `test_os_kill_process_index_equal` | `__os_kill_process()` |
| KRN-010 | System shall track cleanup tasks | `test_os_exit_task_cleanup_idx_max`, `test_os_kill_process_cleanup_idx_max` | `__os_exit_task()`, `__os_kill_process()` |
| KRN-011 | System shall support task suicide | `test_os_task_suicide` | `__os_task_suicide()` |

### 3.2 Kernel - Task Scheduling

| Req ID | Requirement | Test Case(s) | Source Function |
|--------|-------------|--------------|-----------------|
| KRN-020 | System shall support active sleep | `test_task_active_sleep` | `__task_active_sleep()` |
| KRN-021 | System shall support blocking sleep | `test_task_blocking_sleep` | `__task_blocking_sleep()` |
| KRN-022 | System shall support busy wait | `test_task_busy_wait_basic` | `__task_busy_wait()` |
| KRN-023 | System shall maintain tick count | `test_os_get_tick_count` | `__os_get_tick_count()` |
| KRN-024 | System shall track running task count | `test_os_get_running_task_count` | `__os_get_running_task_count()` |
| KRN-025 | System shall provide current task name | `test_os_get_current_task_name`, `test_os_get_current_task_name_null_task`, `test_os_get_current_task_name_invalid_index`, `test_os_get_current_task_name_boundary` | `__os_get_current_task_name()` |
| KRN-026 | System shall track task ticks remaining | `test_os_get_task_ticks_remaining` | `__os_get_task_ticks_remaining()` |

### 3.3 Kernel - Critical Sections

| Req ID | Requirement | Test Case(s) | Source Function |
|--------|-------------|--------------|-----------------|
| KRN-030 | System shall support nested critical sections | `test_critical_sections` | `__enter_critical()`, `__exit_critical()` |

### 3.4 Kernel - Protected Data

| Req ID | Requirement | Test Case(s) | Source Function |
|--------|-------------|--------------|-----------------|
| KRN-060 | System shall allocate protected data for tasks | `test_kernel_protected_data_basic`, `test_kernel_protected_data_multiple` | `__kernel_protected_data()` |
| KRN-061 | System shall reject zero-word allocations | `test_kernel_protected_data_zero_words` | `__kernel_protected_data()` |
| KRN-062 | System shall enforce data pool limits | `test_kernel_protected_data_exceed_space`, `test_kernel_protected_data_exact_size` | `__kernel_protected_data()` |
| KRN-063 | System shall support data read/write | `test_kernel_protected_data_write_read` | `__kernel_protected_data()` |
| KRN-064 | System shall provide stack pool access | `test_kernel_get_stack_basic`, `test_kernel_get_stack_multiple`, `test_kernel_get_stack_boundary` | `__kernel_get_stack()` |
| KRN-065 | System shall provide data pool access | `test_kernel_get_data_basic`, `test_kernel_get_data_multiple`, `test_kernel_get_data_boundary`, `test_kernel_get_data_write_read` | `__kernel_get_data()` |

### 3.5 Kernel - Semaphores

| Req ID | Requirement | Test Case(s) | Source Function |
|--------|-------------|--------------|-----------------|
| KRN-040 | System shall initialize semaphores | `test_semaphore_init_basic`, `test_semaphore_init_invalid_index`, `test_semaphore_init_zero_count`, `test_semaphore_init_multiple`, `test_semaphore_init_boundary` | `__semaphore_init()` |
| KRN-041 | System shall prevent re-initialization | `test_semaphore_init_already_engaged` | `__semaphore_init()` |
| KRN-042 | System shall support semaphore feed | `test_semaphore_feed_basic`, `test_semaphore_feed_invalid_index`, `test_semaphore_feed_not_engaged`, `test_semaphore_feed_to_max` | `__semaphore_feed()` |
| KRN-043 | System shall support semaphore consume | `test_semaphore_consume_basic`, `test_semaphore_consume_invalid_index`, `test_semaphore_consume_not_engaged`, `test_semaphore_consume_multiple`, `test_semaphore_consume_to_zero` | `__semaphore_consume()` |
| KRN-044 | System shall support feed/consume together | `test_semaphore_feed_consume_together` | `__semaphore_feed()`, `__semaphore_consume()` |
| KRN-045 | System shall provide semaphore count query | `test_semaphore_get_count_basic`, `test_semaphore_get_count_invalid`, `test_semaphore_get_count_not_engaged` | `__semaphore_get_count()` |
| KRN-046 | System shall provide semaphore max count query | `test_semaphore_get_max_count_basic`, `test_semaphore_get_max_count_invalid` | `__semaphore_get_max_count()` |

### 3.6 Kernel - Message Pipes

| Req ID | Requirement | Test Case(s) | Source Function |
|--------|-------------|--------------|-----------------|
| KRN-050 | System shall initialize message pipes | `test_pipe_init_basic`, `test_pipe_init_invalid_index`, `test_pipe_init_zero_capacity`, `test_pipe_init_capacity_max`, `test_pipe_init_multiple`, `test_pipe_init_boundary` | `__pipe_init()` |
| KRN-051 | System shall prevent re-initialization | `test_pipe_init_already_engaged` | `__pipe_init()` |
| KRN-052 | System shall enforce capacity limits | `test_pipe_init_capacity_too_large` | `__pipe_init()` |
| KRN-053 | System shall support pipe enqueue | `test_pipe_enqueue_basic`, `test_pipe_enqueue_invalid_index`, `test_pipe_enqueue_not_engaged`, `test_pipe_enqueue_message_too_large`, `test_pipe_enqueue_multiple` | `__pipe_enqueue()` |
| KRN-054 | System shall support pipe dequeue | `test_pipe_dequeue_basic`, `test_pipe_dequeue_invalid_index`, `test_pipe_dequeue_not_engaged`, `test_pipe_dequeue_message_too_large` | `__pipe_dequeue()` |
| KRN-055 | System shall maintain FIFO ordering | `test_pipe_enqueue_dequeue_together` | `__pipe_enqueue()`, `__pipe_dequeue()` |
| KRN-056 | System shall support circular buffer wrap | `test_pipe_circular_wrap` | `__pipe_enqueue()`, `__pipe_dequeue()` |
| KRN-057 | System shall provide pipe count query | `test_pipe_get_count_basic`, `test_pipe_get_count_invalid`, `test_pipe_get_count_not_engaged` | `__pipe_get_count()` |
| KRN-058 | System shall provide pipe max count query | `test_pipe_get_max_count_basic`, `test_pipe_get_max_count_invalid` | `__pipe_get_max_count()` |

### 3.7 BSP - Display

| Req ID | Requirement | Test Case(s) | Source Function |
|--------|-------------|--------------|-----------------|
| BSP-001 | System shall initialize display once | `test_display_init_first_call`, `test_display_init_second_call` | `display_init()` |
| BSP-002 | System shall render progress bar | `test_display_render_bar_normal`, `test_display_render_bar_zero_period`, `test_display_render_bar_zero_width` | `display_render_bar()` |
| BSP-003 | System shall clamp progress bar values | `test_display_render_bar_elapsed_exceeds_period`, `test_display_render_bar_filled_exceeds_width` | `display_render_bar()` |
| BSP-004 | System shall handle null task name | `test_display_render_bar_null_name` | `display_render_bar()` |
| BSP-005 | System shall render banner | `test_display_render_banner_on`, `test_display_render_banner_off` | `display_render_banner()` |
| BSP-006 | System shall render vertical bar | `test_display_render_vbar_basic`, `test_display_render_vbar_overflow` | `display_render_vbar()` |
| BSP-007 | System shall render pipe visualization | `test_display_render_pipe_normal`, `test_display_render_pipe_zero_max` | `display_render_pipe()` |
| BSP-008 | System shall render producer visualization | `test_display_render_producer_normal`, `test_display_render_producer_zero_period` | `display_render_producer()` |
| BSP-009 | System shall render consumer visualization | `test_display_render_consumer_normal`, `test_display_render_consumer_zero_period` | `display_render_consumer()` |
| BSP-010 | System shall initialize message history | `test_msg_history_init_basic`, `test_msg_history_init_null` | `msg_history_init()` |
| BSP-011 | System shall add messages to history | `test_msg_history_add_basic`, `test_msg_history_add_wrap`, `test_msg_history_add_null`, `test_msg_history_add_clamp_len` | `msg_history_add()` |
| BSP-012 | System shall render message history | `test_display_render_msg_history_basic`, `test_display_render_msg_history_null` | `display_render_msg_history()` |

### 3.8 BSP - LED Control

| Req ID | Requirement | Test Case(s) | Source Function |
|--------|-------------|--------------|-----------------|
| BSP-020 | System shall turn LED on | `test_LED_On` | `LED_On()` |
| BSP-021 | System shall turn LED off | `test_LED_Off` | `LED_Off()` |
| BSP-022 | System shall blink LED with timing | `test_LED_Blink`, `test_LED_Blink_zero_delays` | `LED_Blink()` |

### 3.9 BSP - Platform I/O

| Req ID | Requirement | Test Case(s) | Source Function |
|--------|-------------|--------------|-----------------|
| BSP-030 | System shall write to platform | `test_platform_write`, `test_platform_write_zero_length` | `platform_write()` |
| BSP-031 | System shall read from platform | `test_platform_read`, `test_platform_read_zero_length` | `platform_read()` |
| BSP-032 | System shall initialize hardware | `test_hal_init` | `hal_init()` |

### 3.10 BSP - Standard I/O

| Req ID | Requirement | Test Case(s) | Source Function |
|--------|-------------|--------------|-----------------|
| BSP-040 | System shall buffer putchar output | `test_io_putchar_normal` | `__io_putchar()` |
| BSP-041 | System shall flush on newline | `test_io_putchar_newline` | `__io_putchar()` |
| BSP-042 | System shall flush on buffer full | `test_io_putchar_buffer_full` | `__io_putchar()` |

### 3.11 BSP - Interrupt Handlers

| Req ID | Requirement | Test Case(s) | Source Function |
|--------|-------------|--------------|-----------------|
| INT-001 | SysTick shall increment tick count | `test_SysTick_Handler`, `test_SysTick_Handler_multiple_ticks` | `SysTick_Handler()` |
| INT-002 | SysTick shall decrement task ticks | `test_SysTick_Handler`, `test_SysTick_Handler_scheduler_disabled` | `SysTick_Handler()` |
| INT-003 | SysTick shall trigger scheduler on timeout | `test_SysTick_Handler_trigger_scheduler` | `SysTick_Handler()` |
| INT-004 | SysTick shall respect os_running flag | `test_SysTick_Handler_os_not_running` | `SysTick_Handler()` |
| INT-005 | USB interrupt shall call HAL handler | `test_OTG_FS_IRQHandler` | `OTG_FS_IRQHandler()` |
| INT-006 | SVC handler shall be callable | `test_SVC_Handler` | `SVC_Handler()` |
| INT-007 | Debug monitor shall be callable | `test_DebugMon_Handler` | `DebugMon_Handler()` |

## 4. Test Case Summary

| Category | Test Cases | Pass | Fail | Ignored |
|----------|------------|------|------|---------|
| Kernel - Initialization | 2 | 2 | 0 | 0 |
| Kernel - Task Management | 20 | 20 | 0 | 0 |
| Kernel - Scheduling | 8 | 8 | 0 | 0 |
| Kernel - Critical Sections | 1 | 1 | 0 | 0 |
| Kernel - Protected Data | 13 | 13 | 0 | 0 |
| Kernel - Semaphores | 15 | 15 | 0 | 0 |
| Kernel - Message Pipes | 23 | 23 | 0 | 0 |
| BSP - Display | 18 | 18 | 0 | 0 |
| BSP - LED | 4 | 4 | 0 | 0 |
| BSP - Platform I/O | 5 | 5 | 0 | 0 |
| BSP - Standard I/O | 3 | 3 | 0 | 0 |
| BSP - HAL | 1 | 1 | 0 | 0 |
| BSP - Interrupts | 7 | 7 | 0 | 0 |
| BSP - USB | 1 | 1 | 0 | 0 |
| BSP - Debug | 1 | 1 | 0 | 0 |
| **Total** | **142** | **142** | **0** | **0** |

**Coverage Metrics:**
- Line Coverage: 88.5%
- Function Coverage: 85.9%
- Branch Coverage: In progress

## 5. Untested Requirements

The following requirements require target integration testing:

| Req ID | Requirement | Reason |
|--------|-------------|--------|
| KRN-050 | Context switch shall save/restore registers | ARM assembly code |
| KRN-051 | Idle task shall run when no tasks ready | Infinite loop task |
| KRN-052 | Heartbeat task shall blink LED | Infinite loop task |
| KRN-053 | Printf task shall transmit buffered data | Infinite loop task |
| INT-010 | Fault handlers shall halt system | Dead code by design |

## 6. Approval

| Role | Name | Signature | Date |
|------|------|-----------|------|
| Author | | | |
| Reviewer | | | |
| QA | | | |

## 7. Revision History

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 0.1 | 2025-01-26 | Souham Biswas | Initial draft |
| 1.0 | 2025-02-07 | Souham Biswas | Updated with 142 tests, added protected data tests, coverage metrics |
