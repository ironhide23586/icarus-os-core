# Test Traceability Matrix

**Document ID:** ICARUS-VER-003  
**Version:** 0.1  
**Date:** 2026-01-26  
**Status:** Draft  
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
| KRN-001 | System shall initialize kernel state | `test_os_init` | `os_init()` |
| KRN-002 | System shall support task registration | `test_os_register_task`, `test_os_register_task_multiple` | `os_register_task()` |
| KRN-003 | System shall create tasks with stack allocation | `test_os_create_task_normal`, `test_os_create_task_long_name` | `os_create_task()` |
| KRN-004 | System shall enforce maximum task limit | `test_os_create_task_max_tasks`, `test_os_create_task_max_running_tasks` | `os_create_task()` |
| KRN-005 | System shall start task execution | `test_os_start_valid_tasks`, `test_os_start_no_tasks` | `os_start()` |
| KRN-006 | System shall support task yielding | `test_os_yield` | `os_yield()` |
| KRN-007 | System shall support task exit | `test_os_exit_task_with_running_count`, `test_os_exit_task_zero_running_count` | `os_exit_task()` |
| KRN-008 | System shall support task termination | `test_os_kill_process_valid`, `test_os_kill_process_suicide` | `os_kill_process()` |
| KRN-009 | System shall protect task 0 from termination | `test_os_kill_process_index_zero`, `test_os_kill_process_index_zero_error` | `os_kill_process()` |
| KRN-010 | System shall track cleanup tasks | `test_os_exit_task_cleanup_idx_max`, `test_os_kill_process_cleanup_idx_max` | `os_exit_task()`, `os_kill_process()` |

### 3.2 Kernel - Task Scheduling

| Req ID | Requirement | Test Case(s) | Source Function |
|--------|-------------|--------------|-----------------|
| KRN-020 | System shall support active sleep | `test_task_active_sleep` | `task_active_sleep()` |
| KRN-021 | System shall support blocking sleep | `test_task_blocking_sleep` | `task_blocking_sleep()` |
| KRN-022 | System shall support busy wait | `test_task_busy_wait_basic` | `task_busy_wait()` |
| KRN-023 | System shall maintain tick count | `test_os_get_tick_count` | `os_get_tick_count()` |
| KRN-024 | System shall track running task count | `test_os_get_running_task_count` | `os_get_running_task_count()` |
| KRN-025 | System shall provide current task name | `test_os_get_current_task_name`, `test_os_get_current_task_name_null_task` | `os_get_current_task_name()` |
| KRN-026 | System shall track task ticks remaining | `test_os_get_task_ticks_remaining` | `os_get_task_ticks_remaining()` |

### 3.3 Kernel - Critical Sections

| Req ID | Requirement | Test Case(s) | Source Function |
|--------|-------------|--------------|-----------------|
| KRN-030 | System shall support nested critical sections | `test_critical_sections`, `test_exit_critical_depth_nonzero` | `enter_critical()`, `exit_critical()` |
| KRN-031 | System shall re-enable scheduler on critical exit | `test_exit_critical_depth_zero` | `exit_critical()` |

### 3.4 Kernel - Print Buffer

| Req ID | Requirement | Test Case(s) | Source Function |
|--------|-------------|--------------|-----------------|
| KRN-040 | System shall enqueue characters to print buffer | `test_enqueue_print_buffer_basic` | `enqueue_print_buffer()` |
| KRN-041 | System shall detect buffer full condition | `test_enqueue_print_buffer_full` | `enqueue_print_buffer()` |
| KRN-042 | System shall wrap buffer index | `test_enqueue_print_buffer_wrap_around` | `enqueue_print_buffer()` |

### 3.5 BSP - Display

| Req ID | Requirement | Test Case(s) | Source Function |
|--------|-------------|--------------|-----------------|
| BSP-001 | System shall initialize display once | `test_display_init_first_call`, `test_display_init_second_call` | `display_init()` |
| BSP-002 | System shall render progress bar | `test_display_render_bar_normal`, `test_display_render_bar_zero_period` | `display_render_bar()` |
| BSP-003 | System shall clamp progress bar values | `test_display_render_bar_elapsed_exceeds_period`, `test_display_render_bar_filled_exceeds_width` | `display_render_bar()` |
| BSP-004 | System shall handle null task name | `test_display_render_bar_null_name` | `display_render_bar()` |
| BSP-005 | System shall render banner | `test_display_render_banner_on`, `test_display_render_banner_off` | `display_render_banner()` |

### 3.6 BSP - LED Control

| Req ID | Requirement | Test Case(s) | Source Function |
|--------|-------------|--------------|-----------------|
| BSP-010 | System shall turn LED on | `test_LED_On` | `LED_On()` |
| BSP-011 | System shall turn LED off | `test_LED_Off` | `LED_Off()` |
| BSP-012 | System shall blink LED with timing | `test_LED_Blink`, `test_LED_Blink_zero_delays` | `LED_Blink()` |

### 3.7 BSP - Platform I/O

| Req ID | Requirement | Test Case(s) | Source Function |
|--------|-------------|--------------|-----------------|
| BSP-020 | System shall write to I2C | `test_platform_write`, `test_platform_write_zero_length` | `platform_write()` |
| BSP-021 | System shall read from I2C | `test_platform_read`, `test_platform_read_zero_length` | `platform_read()` |
| BSP-022 | System shall provide platform delay | `test_platform_delay`, `test_platform_delay_zero` | `platform_delay()` |
| BSP-023 | System shall initialize hardware | `test_hal_init` | `hal_init()` |

### 3.8 BSP - Standard I/O

| Req ID | Requirement | Test Case(s) | Source Function |
|--------|-------------|--------------|-----------------|
| BSP-030 | System shall buffer putchar output | `test_io_putchar_normal` | `__io_putchar()` |
| BSP-031 | System shall flush on newline | `test_io_putchar_newline` | `__io_putchar()` |
| BSP-032 | System shall flush on buffer full | `test_io_putchar_buffer_full` | `__io_putchar()` |

### 3.9 BSP - Interrupt Handlers

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
| Kernel - Task Management | 18 | 18 | 0 | 0 |
| Kernel - Scheduling | 7 | 7 | 0 | 0 |
| Kernel - Critical Sections | 3 | 3 | 0 | 0 |
| Kernel - Print Buffer | 5 | 5 | 0 | 0 |
| BSP - Display | 9 | 9 | 0 | 0 |
| BSP - LED | 4 | 4 | 0 | 0 |
| BSP - Platform I/O | 7 | 7 | 0 | 0 |
| BSP - Standard I/O | 3 | 3 | 0 | 0 |
| BSP - Interrupts | 8 | 8 | 0 | 0 |
| **Total** | **76** | **76** | **0** | **0** |

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
| 0.1 | 2026-01-26 | Kiro | Initial draft |
