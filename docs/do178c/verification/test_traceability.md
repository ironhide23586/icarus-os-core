# Test Traceability Matrix

**Document ID:** ICARUS-VER-003  
**Version:** 0.3
**Date:** 2026-04-01  
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

### 3.4 Kernel - Semaphores

| Req ID | Requirement | Test Case(s) | Source Function |
|--------|-------------|--------------|-----------------|
| KRN-040 | System shall initialize semaphores | `test_semaphore_init_valid`, `test_semaphore_init_invalid_idx` | `semaphore_init()` |
| KRN-041 | System shall support semaphore feed | `test_semaphore_feed_valid`, `test_semaphore_feed_invalid_idx` | `semaphore_feed()` |
| KRN-042 | System shall support semaphore consume | `test_semaphore_consume_valid`, `test_semaphore_consume_invalid_idx` | `semaphore_consume()` |
| KRN-043 | System shall enforce bounded semaphore capacity | `test_semaphore_feed_valid` | `semaphore_feed()` |
| KRN-044 | System shall provide semaphore count query | `test_semaphore_get_count_valid`, `test_semaphore_get_count_invalid` | `semaphore_get_count()` |
| KRN-045 | System shall provide semaphore max count query | `test_semaphore_get_max_count_valid`, `test_semaphore_get_max_count_invalid` | `semaphore_get_max_count()` |

### 3.5 Kernel - Message Pipes

| Req ID | Requirement | Test Case(s) | Source Function |
|--------|-------------|--------------|-----------------|
| KRN-050 | System shall initialize message pipes | `test_pipe_init_valid`, `test_pipe_init_invalid_idx`, `test_pipe_init_zero_capacity` | `pipe_init()` |
| KRN-051 | System shall support pipe enqueue | `test_pipe_enqueue_valid`, `test_pipe_enqueue_invalid_idx` | `pipe_enqueue()` |
| KRN-052 | System shall support pipe dequeue | `test_pipe_dequeue_valid`, `test_pipe_dequeue_invalid_idx`, `test_pipe_dequeue_empty` | `pipe_dequeue()` |
| KRN-053 | System shall maintain FIFO ordering | `test_pipe_enqueue_valid`, `test_pipe_dequeue_valid` | `pipe_enqueue()`, `pipe_dequeue()` |
| KRN-054 | System shall enforce pipe capacity | `test_pipe_enqueue_valid` | `pipe_enqueue()` |
| KRN-055 | System shall provide pipe count query | `test_pipe_get_count_valid`, `test_pipe_get_count_invalid` | `pipe_get_count()` |
| KRN-056 | System shall provide pipe max count query | `test_pipe_get_max_count_valid`, `test_pipe_get_max_count_invalid` | `pipe_get_max_count()` |

### 3.6 Kernel - Print Buffer

| Req ID | Requirement | Test Case(s) | Source Function |
|--------|-------------|--------------|-----------------|
| KRN-060 | System shall enqueue characters to print buffer | `test_enqueue_print_buffer_basic` | `enqueue_print_buffer()` |
| KRN-061 | System shall detect buffer full condition | `test_enqueue_print_buffer_full` | `enqueue_print_buffer()` |
| KRN-062 | System shall wrap buffer index | `test_enqueue_print_buffer_wrap_around` | `enqueue_print_buffer()` |

### 3.7 BSP - Display

| Req ID | Requirement | Test Case(s) | Source Function |
|--------|-------------|--------------|-----------------|
| BSP-001 | System shall initialize display once | `test_display_init_first_call`, `test_display_init_second_call` | `display_init()` |
| BSP-002 | System shall render progress bar | `test_display_render_bar_normal`, `test_display_render_bar_zero_period` | `display_render_bar()` |
| BSP-003 | System shall clamp progress bar values | `test_display_render_bar_elapsed_exceeds_period`, `test_display_render_bar_filled_exceeds_width` | `display_render_bar()` |
| BSP-004 | System shall handle null task name | `test_display_render_bar_null_name` | `display_render_bar()` |
| BSP-005 | System shall render banner | `test_display_render_banner_on`, `test_display_render_banner_off` | `display_render_banner()` |
| BSP-006 | System shall render vertical bar | `test_display_render_vbar_normal`, `test_display_render_vbar_zero_max`, `test_display_render_vbar_value_exceeds_max` | `display_render_vbar()` |
| BSP-007 | System shall render pipe visualization | `test_display_render_pipe_normal`, `test_display_render_pipe_zero_capacity` | `display_render_pipe()` |
| BSP-008 | System shall render producer visualization | `test_display_render_producer_normal` | `display_render_producer()` |
| BSP-009 | System shall render consumer visualization | `test_display_render_consumer_normal` | `display_render_consumer()` |
| BSP-010 | System shall initialize message history | `test_msg_history_init` | `msg_history_init()` |
| BSP-011 | System shall add messages to history | `test_msg_history_add_normal`, `test_msg_history_add_wrap`, `test_msg_history_add_null_msg` | `msg_history_add()` |
| BSP-012 | System shall render message history | `test_display_render_msg_history_empty`, `test_display_render_msg_history_partial`, `test_display_render_msg_history_full`, `test_display_render_msg_history_wrapped` | `display_render_msg_history()` |

### 3.8 BSP - LED Control

| Req ID | Requirement | Test Case(s) | Source Function |
|--------|-------------|--------------|-----------------|
| BSP-010 | System shall turn LED on | `test_LED_On` | `LED_On()` |
| BSP-011 | System shall turn LED off | `test_LED_Off` | `LED_Off()` |
| BSP-012 | System shall blink LED with timing | `test_LED_Blink`, `test_LED_Blink_zero_delays` | `LED_Blink()` |

### 3.9 BSP - Platform I/O

| Req ID | Requirement | Test Case(s) | Source Function |
|--------|-------------|--------------|-----------------|
| BSP-020 | System shall write to I2C | `test_platform_write`, `test_platform_write_zero_length` | `platform_write()` |
| BSP-021 | System shall read from I2C | `test_platform_read`, `test_platform_read_zero_length` | `platform_read()` |
| BSP-022 | System shall initialize hardware | `test_hal_init` | `hal_init()` |

### 3.10 BSP - Standard I/O

| Req ID | Requirement | Test Case(s) | Source Function |
|--------|-------------|--------------|-----------------|
| BSP-030 | System shall buffer putchar output | `test_io_putchar_normal` | `__io_putchar()` |
| BSP-031 | System shall flush on newline | `test_io_putchar_newline` | `__io_putchar()` |
| BSP-032 | System shall flush on buffer full | `test_io_putchar_buffer_full` | `__io_putchar()` |

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

### 3.12 Kernel - SVC Dispatch

| Req ID | Requirement | Test Case(s) | Source Function |
|--------|-------------|--------------|-----------------|
| SVC-001 | SVC sem_can_feed shall return correct predicate | `test_svc_sem_can_feed_valid`, `test_svc_sem_can_feed_invalid` | `SVC_Handler_C()` |
| SVC-002 | SVC sem_can_consume shall return correct predicate | `test_svc_sem_can_consume_valid`, `test_svc_sem_can_consume_invalid` | `SVC_Handler_C()` |
| SVC-003 | SVC sem_increment shall atomically modify count | `test_svc_sem_increment_valid` | `SVC_Handler_C()` |
| SVC-004 | SVC sem_decrement shall atomically modify count | `test_svc_sem_decrement_valid` | `SVC_Handler_C()` |
| SVC-005 | SVC pipe_can_send shall return correct predicate | `test_svc_pipe_can_send_valid` | `SVC_Handler_C()` |
| SVC-006 | SVC pipe_can_receive shall return correct predicate | `test_svc_pipe_can_receive_valid` | `SVC_Handler_C()` |
| SVC-007 | SVC get_task_name shall return task name | `test_os_get_task_name_svc` | `SVC_Handler_C()` |
| SVC-008 | SVC get_num_tasks shall return created count | `test_os_get_num_created_tasks_svc` | `SVC_Handler_C()` |
| SVC-009 | SVC os_is_running shall return running flag | `test_os_is_running_svc` | `SVC_Handler_C()` |

### 3.13 Kernel - CRC16-CCITT helper *(v0.3.0)*

| Req ID | Requirement | Test Case(s) | Source Function |
|--------|-------------|--------------|-----------------|
| HLR-KRN-092 | Provide CRC16-CCITT (poly 0x1021, init 0xFFFF) | `test_crc16_ccitt_canonical_vector` | `crc16_ccitt()` |
| HLR-KRN-092.1 | Use STM32H7 on-chip CRC peripheral on target | covered by target smoke test (HW path short-circuited under HOST_TEST) | `crc16_ccitt()` |
| HLR-KRN-092.2 | Provide HOST_TEST fallback | `test_crc16_ccitt_canonical_vector`, `test_crc16_ccitt_single_byte_zero`, `test_crc16_ccitt_single_byte_ff`, `test_crc16_ccitt_zero_length`, `test_crc16_ccitt_null_data`, `test_crc16_ccitt_repeatable`, `test_crc16_ccitt_sensitive_to_single_bit`, `test_crc16_ccitt_length_sensitive` | `crc16_ccitt()` |

### 3.14 Kernel - CDC RX ring buffer *(v0.3.0)*

| Req ID | Requirement | Test Case(s) | Source Function |
|--------|-------------|--------------|-----------------|
| HLR-KRN-090 | Provide SPSC USB CDC receive ring buffer | `test_cdc_rx_init_empty`, `test_cdc_rx_push_and_drain`, `test_cdc_rx_fifo_order` | `cdc_rx_init()`, `cdc_rx_push()`, `cdc_rx_read_byte()` |
| HLR-KRN-090.1 | Non-blocking; overflow drops bytes silently | `test_cdc_rx_fills_to_capacity` | `cdc_rx_push()` |
| HLR-KRN-090.2 | Reads route through SVC gates so ring may live in DTCM_PRIV | `test_cdc_rx_wrap_around`, `test_cdc_rx_push_zero_length`, `test_cdc_rx_read_byte_when_empty` | `cdc_rx_read_byte()`, `cdc_rx_available()` |

### 3.15 Kernel - Event ring + per-module squelch *(v0.3.0)*

| Req ID | Requirement | Test Case(s) | Source Function |
|--------|-------------|--------------|-----------------|
| HLR-KRN-091 | Provide structured event ring buffer + squelch | `test_event_init_clears_state`, `test_event_emit_one_increments_count`, `test_event_drain_returns_what_was_emitted` | `event_init()`, `os_event()`, `event_drain()` |
| HLR-KRN-091.1 | Fixed 16-byte event entries | `test_event_drain_returns_what_was_emitted`, `test_event_payload_truncated_to_12` | `event_entry_t`, `os_event()` |
| HLR-KRN-091.2 | Transport-agnostic drain | `test_event_drain_partial`, `test_event_drain_empty_ring_returns_false` | `event_drain()` |
| HLR-KRN-091.3 | O(1) non-blocking emit; ring overflow overwrites oldest | `test_event_squelch_drops_low_severity`, `test_event_invalid_module_id_dropped`, `test_event_drain_full_ring` | `os_event()` |

### 3.16 Kernel - Internal flat-file filesystem *(v0.3.0)*

| Req ID | Requirement | Test Case(s) | Source Function |
|--------|-------------|--------------|-----------------|
| HLR-KRN-093 | Provide create/open/read/write/delete/list/stats | `test_fs_init_clean_state`, `test_fs_create_basic`, `test_fs_open_existing`, `test_fs_open_missing_fails`, `test_fs_write_read_roundtrip`, `test_fs_write_appends`, `test_fs_delete_existing`, `test_fs_delete_missing_fails`, `test_fs_list`, `test_fs_stats_after_writes`, `test_fs_read_offset` | `fs_init/create/open/write/read/delete/list/stats` |
| HLR-KRN-093.1 | At least 16 files × 2 KB = 32 KB capacity | `test_fs_create_full_disk`, `test_fs_write_overflow_rejected`, `test_fs_create_duplicate_fails`, `test_fs_create_invalid_name`, `test_fs_write_invalid_handle` | `fs_create()`, `fs_write()` |
| HLR-KRN-093.2 | Opaque on-disk format | covered by API contract — exercised across the full test set | n/a |

### 3.17 Kernel - Ground-loadable table engine *(v0.3.0)*

| Req ID | Requirement | Test Case(s) | Source Function |
|--------|-------------|--------------|-----------------|
| HLR-KRN-094 | Provide table engine with double-buffered swap | `test_tbl_init_clears_registry`, `test_tbl_register_basic`, `test_tbl_register_null_fails`, `test_tbl_register_duplicate_fails`, `test_tbl_register_invalid_size_fails`, `test_tbl_load_unknown_id_fails`, `test_tbl_load_null_data_fails`, `test_tbl_load_overflow_rejected`, `test_tbl_dump_returns_active`, `test_tbl_dump_unknown_id_fails`, `test_tbl_chunked_load`, `test_tbl_get_descriptor_unknown` | `tbl_init/register/load/dump/get_descriptor/count` |
| HLR-KRN-094.1 | Schema CRC + data CRC16 gated activation | `test_tbl_activate_schema_crc_mismatch`, `test_tbl_activate_round_trip` | `tbl_activate()` |
| HLR-KRN-094.2 | Activate callback runs in thread mode against scratch copy | `test_tbl_activate_round_trip` (verifies the callback observes the staged bytes via the scratch buffer) | `tbl_activate()` |
| HLR-KRN-094.3 | Active buffer immutable until callback OKs | `test_tbl_activate_callback_rejection`, `test_tbl_activate_without_load_fails` | `tbl_activate()` |

## 4. Test Case Summary

Run `cd tests && make test` to obtain current pass/fail results.

| Category | Test Cases |
|----------|------------|
| Kernel - Task Management | 18 |
| Kernel - Scheduling | 7 |
| Kernel - Critical Sections | 3 |
| Kernel - Semaphores | 16 |
| Kernel - Message Pipes | 19 |
| Kernel - Print Buffer | 5 |
| Kernel - SVC Dispatch | 11 |
| Kernel - CRC16-CCITT helper *(v0.3.0)* | 8 |
| Kernel - CDC RX ring buffer *(v0.3.0)* | 7 |
| Kernel - Event ring + squelch *(v0.3.0)* | 9 |
| Kernel - Internal filesystem *(v0.3.0)* | 16 |
| Kernel - Ground-loadable table engine *(v0.3.0)* | 16 |
| BSP - Display | 21 |
| BSP - LED | 4 |
| BSP - Platform I/O | 7 |
| BSP - Standard I/O | 3 |
| BSP - Interrupts | 8 |
| **Total** | **196** |

> **Note:** counts above are approximate; the canonical test count comes from the
> Unity runner output at test execution time.

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
| 0.2 | 2026-04-01 | Souham Biswas | Added SVC tests; updated counts to ~140; reconciled semaphore/pipe test names |
| 0.3 | 2026-04-11 | Souham Biswas | Added 56 host tests for the v0.3.0 shared modules across `test_crc.c` (8), `test_cdc_rx.c` (7), `test_event.c` (9), `test_fs.c` (16), and `test_tables.c` (16); total bumped 140 → 196 |
