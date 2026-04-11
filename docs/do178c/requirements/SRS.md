# Software Requirements Specification (SRS)

**Document ID:** ICARUS-SRS-001
**Version:** 0.4
**Date:** 2026-06-15
**Status:** Draft
**Classification:** Public (Open Source)

---

## Document Control

| Role | Name | Date | Signature |
|------|------|------|-----------|
| Author | | | |
| Reviewer | | | |
| Approver | | | |

---

## Revision History

| Version | Date | Author | Description |
|---------|------|--------|-------------|
| 0.1 | 2025-01-26 | Souham Biswas | Initial draft |
| 0.2 | 2026-04-01 | Souham Biswas | Added 14 memory protection requirements (HLR-KRN-063 to HLR-KRN-086) |
| 0.3 | 2026-04-11 | Souham Biswas | Added 5 shared service module requirements (HLR-KRN-090 to HLR-KRN-094): CDC RX ring buffer, event ring + squelch, CRC16 helper, internal filesystem, ground-loadable table engine |
| 0.4 | 2026-06-15 | Souham Biswas | Added Software Bus (HLR-KRN-095), Background Checksum (HLR-KRN-096), task restart (HLR-KRN-097), timed semaphore (HLR-KRN-098), wider pipes (HLR-KRN-053 updated), task diagnostics (HLR-KRN-099), BSP watchdog/button/CDC write (HLR-BSP-025 to HLR-BSP-027) |

---

## 1. Introduction

### 1.1 Purpose

This Software Requirements Specification (SRS) defines the functional and performance requirements for ICARUS OS. These requirements form the basis for design, implementation, and verification activities.

### 1.2 Scope

ICARUS OS is a real-time operating system providing:
- Preemptive multitasking
- Deterministic scheduling
- Hardware abstraction
- AI inference runtime
- Inter-process communication

### 1.3 Definitions

| Term | Definition |
|------|------------|
| Task | Independent unit of execution with own stack |
| Tick | Fundamental time unit (default: 1ms) |
| WCET | Worst-Case Execution Time |
| IPC | Inter-Process Communication |
| BSP | Board Support Package |

---

## 2. System Requirements Allocation

The following system-level requirements are allocated to ICARUS OS:

| System Req | Description | Allocated To |
|------------|-------------|--------------|
| SYS-001 | Provide deterministic task scheduling | Kernel |
| SYS-002 | Support minimum 8 concurrent tasks | Kernel |
| SYS-003 | Provide hardware abstraction | BSP |
| SYS-004 | Support AI inference <50ms | AI Runtime |
| SYS-005 | Boot to operational <500ms | Kernel + BSP |

---

## 3. High-Level Requirements (HLR)

### 3.1 Kernel Requirements

#### 3.1.1 Task Management

| ID | Requirement | Priority | Status |
|----|-------------|----------|--------|
| HLR-KRN-001 | The kernel shall support creation of up to MAX_TASKS concurrent tasks | Must | ✅ Implemented |
| HLR-KRN-002 | The kernel shall assign each task a unique identifier | Must | ✅ Implemented |
| HLR-KRN-003 | The kernel shall allocate a dedicated stack for each task | Must | ✅ Implemented |
| HLR-KRN-004 | The kernel shall support task termination by the task itself | Must | ✅ Implemented |
| HLR-KRN-005 | The kernel shall support task termination by other tasks | Must | ✅ Implemented |
| HLR-KRN-006 | The kernel shall protect system tasks from termination | Must | ✅ Implemented |
| HLR-KRN-007 | The kernel shall track task state (COLD, RUNNING, READY, BLOCKED, KILLED, FINISHED) | Must | ✅ Implemented |

#### 3.1.2 Scheduling

| ID | Requirement | Priority | Status |
|----|-------------|----------|--------|
| HLR-KRN-010 | The kernel shall implement preemptive scheduling | Must | ✅ Implemented |
| HLR-KRN-011 | The kernel shall use round-robin scheduling among equal-priority tasks | Must | ✅ Implemented |
| HLR-KRN-012 | The kernel shall support configurable time quantum (default: 50ms) | Must | ✅ Implemented |
| HLR-KRN-013 | The kernel shall support voluntary yield by tasks | Must | ✅ Implemented |
| HLR-KRN-014 | The kernel shall support priority-based scheduling | Should | Planned |
| HLR-KRN-015 | The kernel shall prevent priority inversion | Should | Planned |

#### 3.1.3 Timing Services

| ID | Requirement | Priority | Status |
|----|-------------|----------|--------|
| HLR-KRN-020 | The kernel shall maintain a monotonic tick counter | Must | ✅ Implemented |
| HLR-KRN-021 | The kernel shall support task sleep (non-blocking) | Must | ✅ Implemented |
| HLR-KRN-022 | The kernel shall support task delay (blocking) | Must | ✅ Implemented |
| HLR-KRN-023 | The kernel shall provide tick count query | Must | ✅ Implemented |
| HLR-KRN-024 | Sleep accuracy shall be ±1 tick | Must | ✅ Implemented |

#### 3.1.4 Critical Sections

| ID | Requirement | Priority | Status |
|----|-------------|----------|--------|
| HLR-KRN-030 | The kernel shall support disabling preemption for critical sections | Must | ✅ Implemented |
| HLR-KRN-031 | The kernel shall support nested critical sections | Must | ✅ Implemented |
| HLR-KRN-032 | Critical section duration shall be minimized (<100μs typical) | Should | ✅ Implemented |


#### 3.1.5 Inter-Process Communication (IPC)

| ID | Requirement | Priority | Status |
|----|-------------|----------|--------|
| HLR-KRN-040 | The kernel shall support message queues (pipes) | Must | ✅ Implemented |
| HLR-KRN-041 | The kernel shall support binary semaphores | Must | ✅ Implemented |
| HLR-KRN-042 | The kernel shall support counting semaphores | Should | ✅ Implemented |
| HLR-KRN-043 | The kernel shall support bounded semaphores with max capacity | Must | ✅ Implemented |
| HLR-KRN-044 | The kernel shall support mutexes with priority inheritance | Should | Planned |
| HLR-KRN-045 | The kernel shall support event flags | Should | Planned |
| HLR-KRN-046 | IPC operations shall have bounded WCET | Must | ✅ Implemented |
| HLR-KRN-047 | Message queues shall support FIFO ordering | Must | ✅ Implemented |
| HLR-KRN-048 | Message queues shall support variable-length messages | Must | ✅ Implemented |
| HLR-KRN-049 | Semaphore operations shall block when resource unavailable | Must | ✅ Implemented |
| HLR-KRN-050 | Pipe operations shall block when full/empty | Must | ✅ Implemented |
| HLR-KRN-051 | The kernel shall support up to ICARUS_MAX_SEMAPHORES (64) semaphores | Must | ✅ Implemented |
| HLR-KRN-052 | The kernel shall support up to ICARUS_MAX_MESSAGE_QUEUES (64) pipes | Must | ✅ Implemented |
| HLR-KRN-053 | Each pipe shall support up to 512 bytes capacity | Must | ✅ Implemented |

#### 3.1.6 Memory Management

| ID | Requirement | Priority | Status |
|----|-------------|----------|--------|
| HLR-KRN-060 | The kernel shall use static memory allocation only | Must | ✅ Implemented |
| HLR-KRN-061 | The kernel shall detect stack overflow | Should | ✅ Implemented |
| HLR-KRN-062 | The kernel shall support MPU-based memory protection | Must | ✅ Implemented |
| HLR-KRN-063 | The kernel shall isolate kernel code in privileged ITCM region | Must | ✅ Implemented |
| HLR-KRN-064 | The kernel shall isolate kernel data in privileged DTCM region | Must | ✅ Implemented |
| HLR-KRN-065 | The kernel shall isolate task data regions from each other | Must | ✅ Implemented |
| HLR-KRN-066 | The kernel shall enforce read-only protection on code regions | Must | ✅ Implemented |
| HLR-KRN-067 | The kernel shall provide SVC call gates for kernel services | Must | ✅ Implemented |
| HLR-KRN-068 | The kernel shall execute tasks in unprivileged mode | Must | ✅ Implemented |
| HLR-KRN-069 | The kernel shall execute kernel functions in privileged mode | Must | ✅ Implemented |
| HLR-KRN-070 | The kernel shall prevent unprivileged access to kernel data | Must | ✅ Implemented |
| HLR-KRN-071 | The kernel shall prevent cross-task memory access | Must | ✅ Implemented |
| HLR-KRN-072 | The kernel shall recover from recoverable memory faults | Must | ✅ Implemented |
| HLR-KRN-073 | The kernel shall support memory partitioning (ARINC 653) | Could | Planned |

#### 3.1.7 Fault Handling

| ID | Requirement | Priority | Status |
|----|-------------|----------|--------|
| HLR-KRN-080 | The kernel shall handle processor faults gracefully | Must | ✅ Implemented |
| HLR-KRN-081 | The kernel shall handle MemManage faults from unprivileged tasks | Must | ✅ Implemented |
| HLR-KRN-082 | The kernel shall recover from data access violations | Must | ✅ Implemented |
| HLR-KRN-083 | The kernel shall halt on instruction fetch violations | Must | ✅ Implemented |
| HLR-KRN-084 | The kernel shall support watchdog integration | Should | ✅ Implemented |
| HLR-KRN-085 | The kernel shall log fault information for diagnostics | Should | ✅ Implemented |
| HLR-KRN-086 | The kernel shall support fault recovery (task restart) | Could | ✅ Implemented |

#### 3.1.8 Shared Service Modules

The shared service modules provide reusable kernel infrastructure for
serial input buffering, structured event logging, integrity checks,
scratch storage, and ground-loadable configuration tables. All five
follow the kernel's MPU-aware privilege-separation pattern (DTCM_PRIV
backing data, ITCM hot path, SVC-gated public API). See SDD §3.10 for
the detailed design.

| ID | Requirement | Priority | Status |
|----|-------------|----------|--------|
| HLR-KRN-090 | The kernel shall provide a single-producer / single-consumer USB CDC receive ring buffer with capacity ≥ 256 bytes, callable from a privileged ISR producer and an unprivileged thread-mode consumer | Must | ✅ Implemented |
| HLR-KRN-090.1 | The CDC RX ring buffer shall not block the USB ISR — overflow shall silently drop incoming bytes rather than wait | Must | ✅ Implemented |
| HLR-KRN-090.2 | Thread-mode consumer reads of the CDC RX ring shall route through SVC gates so the ring data may live in privileged-only DTCM | Must | ✅ Implemented |
| HLR-KRN-091 | The kernel shall provide a generic structured event ring buffer with per-module severity squelch filtering | Must | ✅ Implemented |
| HLR-KRN-091.1 | Event entries shall be fixed at 16 bytes (header + ≤12 byte payload) for deterministic memory budget | Must | ✅ Implemented |
| HLR-KRN-091.2 | The event module shall be transport-agnostic — it shall drain into a caller-provided buffer rather than encoding any specific telemetry format | Must | ✅ Implemented |
| HLR-KRN-091.3 | Event emission shall be O(1) and non-blocking; ring overflow shall overwrite the oldest entry rather than block the producer | Must | ✅ Implemented |
| HLR-KRN-092 | The kernel shall provide a CRC16-CCITT helper (polynomial 0x1021, initial value 0xFFFF, no reflection) | Must | ✅ Implemented |
| HLR-KRN-092.1 | On STM32H7 target, the CRC16 helper shall use the on-chip CRC peripheral on the AHB4 bus rather than a software loop | Should | ✅ Implemented |
| HLR-KRN-092.2 | A portable software fallback shall be available under HOST_TEST so unit tests run unchanged off-target | Must | ✅ Implemented |
| HLR-KRN-093 | The kernel shall provide a minimal flat-file storage layer with create/open/read/write/delete/list/stats operations | Must | ✅ Implemented |
| HLR-KRN-093.1 | The filesystem shall support at least 16 named files of at least 2 KB each, totalling at least 32 KB capacity | Must | ✅ Implemented |
| HLR-KRN-093.2 | The on-disk format shall be opaque to allow a real flash backend to be substituted later without changing the public API | Should | ✅ Implemented |
| HLR-KRN-094 | The kernel shall provide a generic ground-loadable table engine with double-buffered staging/active swap | Must | ✅ Implemented |
| HLR-KRN-094.1 | Table activation shall be gated by both a schema CRC and a data CRC16; mismatches shall reject the activation atomically | Must | ✅ Implemented |
| HLR-KRN-094.2 | The activate callback registered by a table producer shall execute in unprivileged thread mode against a stack scratch copy of the staged bytes — never against the live DTCM_PRIV staging buffer | Must | ✅ Implemented |
| HLR-KRN-094.3 | The active table buffer shall not be modified until the activate callback returns success | Must | ✅ Implemented |
| HLR-KRN-095 | The kernel shall provide a lightweight pub/sub Software Bus that routes messages by ID to subscriber pipes | Must | ✅ Implemented |
| HLR-KRN-095.1 | The Software Bus shall support at least 32 distinct message routes with up to 4 subscribers per route | Must | ✅ Implemented |
| HLR-KRN-095.2 | Publishing shall be best-effort: if a subscriber's pipe is full the message shall be silently dropped for that subscriber without blocking the publisher | Must | ✅ Implemented |
| HLR-KRN-095.3 | The Software Bus route table shall reside in DTCM_DATA_PRIV; hot-path functions shall be placed in ITCM_FUNC | Must | ✅ Implemented |
| HLR-KRN-096 | The kernel shall provide a periodic background checksum integrity monitor using CRC16-CCITT over registered memory regions | Must | ✅ Implemented |
| HLR-KRN-096.1 | The checksum monitor shall support at least 8 independently enabled memory regions with baselines captured at registration time | Must | ✅ Implemented |
| HLR-KRN-096.2 | CRC mismatches shall be reported through a user-supplied callback invoked from within cs_check_all() | Must | ✅ Implemented |
| HLR-KRN-096.3 | The checksum module shall perform a hardware CRC self-test at initialization (expected value 0x29B1) | Must | ✅ Implemented |
| HLR-KRN-097 | The kernel shall provide os_restart_task(task_index) to cold-restart a killed or finished task in-place from its original entry point without allocating a new stack slot | Must | ✅ Implemented |
| HLR-KRN-097.1 | os_restart_task shall only accept tasks in TASK_STATE_KILLED or TASK_STATE_FINISHED; the restarted task shall enter the scheduler as TASK_STATE_COLD | Must | ✅ Implemented |
| HLR-KRN-098 | The kernel shall provide semaphore_consume_timeout(idx, max_ticks) for timed semaphore acquisition; 0 max_ticks shall be a non-blocking try | Must | ✅ Implemented |
| HLR-KRN-098.1 | semaphore_consume_timeout shall return false if the semaphore is not acquired within max_ticks | Must | ✅ Implemented |
| HLR-KRN-099 | The kernel shall provide per-task diagnostic fields: dispatch_count (scheduling counter) and stack_watermark (minimum free stack words using 0xDEADC0DE sentinel) | Must | ✅ Implemented |
| HLR-KRN-099.1 | os_get_task_state(task_index) shall return the current task state via an SVC-gated query | Must | ✅ Implemented |
| HLR-KRN-099.2 | os_update_stack_watermark(task_index) shall scan the task stack for the sentinel pattern and update the watermark field in the TCB | Must | ✅ Implemented |

---

### 3.2 BSP Requirements

#### 3.2.1 Platform Abstraction

| ID | Requirement | Priority | Status |
|----|-------------|----------|--------|
| HLR-BSP-001 | The BSP shall provide platform-independent kernel interface | Must | ✅ Implemented |
| HLR-BSP-002 | The BSP shall initialize processor and clocks | Must | ✅ Implemented |
| HLR-BSP-003 | The BSP shall configure interrupt priorities | Must | ✅ Implemented |
| HLR-BSP-004 | The BSP shall provide context switch mechanism | Must | ✅ Implemented |

#### 3.2.2 Peripheral Drivers

| ID | Requirement | Priority | Status |
|----|-------------|----------|--------|
| HLR-BSP-010 | The BSP shall support GPIO control | Must | ✅ Implemented |
| HLR-BSP-011 | The BSP shall support I2C communication | Must | ✅ Implemented |
| HLR-BSP-012 | The BSP shall support SPI communication | Must | ✅ Implemented |
| HLR-BSP-013 | The BSP shall support UART/USB serial output | Must | ✅ Implemented |
| HLR-BSP-014 | The BSP shall support display output | Should | ✅ Implemented |
| HLR-BSP-015 | The BSP shall support PWM output | Should | Planned |
| HLR-BSP-016 | The BSP shall support ADC input | Should | Planned |
| HLR-BSP-017 | The BSP shall support CAN bus | Could | Planned |

#### 3.2.3 Timing Hardware

| ID | Requirement | Priority | Status |
|----|-------------|----------|--------|
| HLR-BSP-020 | The BSP shall configure SysTick for 1ms tick | Must | ✅ Implemented |
| HLR-BSP-021 | The BSP shall support high-resolution timer | Should | Planned |
| HLR-BSP-022 | The BSP shall support RTC for wall-clock time | Should | ✅ Implemented |
| HLR-BSP-025 | The BSP shall provide an Independent Watchdog (IWDG) abstraction with init, refresh, reset-reason query, and flag clear | Must | ✅ Implemented |
| HLR-BSP-026 | The BSP shall provide a K1 user button read function returning the raw active-low pin state | Must | ✅ Implemented |
| HLR-BSP-027 | The BSP shall provide a CDC raw write helper (CDC_Write, CDC_WriteString) with busy-retry via task_active_sleep | Must | ✅ Implemented |

---

### 3.3 AI Runtime Requirements

#### 3.3.1 Model Management

| ID | Requirement | Priority | Status |
|----|-------------|----------|--------|
| HLR-AI-001 | The AI runtime shall load models from memory | Must | Planned |
| HLR-AI-002 | The AI runtime shall validate model integrity (CRC) | Must | Planned |
| HLR-AI-003 | The AI runtime shall validate model structure | Must | Planned |
| HLR-AI-004 | The AI runtime shall reject models with unsupported operators | Must | Planned |
| HLR-AI-005 | The AI runtime shall support multiple concurrent models | Should | Planned |

#### 3.3.2 Inference Execution

| ID | Requirement | Priority | Status |
|----|-------------|----------|--------|
| HLR-AI-010 | The AI runtime shall execute inference deterministically | Must | Planned |
| HLR-AI-011 | The AI runtime shall use fixed-point arithmetic only | Must | Planned |
| HLR-AI-012 | The AI runtime shall complete inference within WCET budget | Must | Planned |
| HLR-AI-013 | The AI runtime shall not allocate memory during inference | Must | Planned |
| HLR-AI-014 | The AI runtime shall support int8 quantized models | Must | Planned |
| HLR-AI-015 | The AI runtime shall support int16 quantized models | Should | Planned |

#### 3.3.3 Supported Operators

| ID | Requirement | Priority | Status |
|----|-------------|----------|--------|
| HLR-AI-020 | The AI runtime shall support Conv2D operator | Must | Planned |
| HLR-AI-021 | The AI runtime shall support Dense (fully connected) operator | Must | Planned |
| HLR-AI-022 | The AI runtime shall support ReLU activation | Must | Planned |
| HLR-AI-023 | The AI runtime shall support Softmax activation | Must | Planned |
| HLR-AI-024 | The AI runtime shall support MaxPool2D operator | Must | Planned |
| HLR-AI-025 | The AI runtime shall support AveragePool2D operator | Should | Planned |
| HLR-AI-026 | The AI runtime shall support Add/Concatenate operators | Should | Planned |
| HLR-AI-027 | The AI runtime shall support BatchNormalization | Should | Planned |

#### 3.3.4 Safety Features

| ID | Requirement | Priority | Status |
|----|-------------|----------|--------|
| HLR-AI-030 | The AI runtime shall check output bounds | Must | Planned |
| HLR-AI-031 | The AI runtime shall provide confidence scores | Must | Planned |
| HLR-AI-032 | The AI runtime shall support fallback on low confidence | Should | Planned |
| HLR-AI-033 | The AI runtime shall detect numerical overflow | Must | Planned |
| HLR-AI-034 | The AI runtime shall isolate AI execution from kernel | Should | Planned |


---

## 4. Performance Requirements

### 4.1 Timing Requirements

| ID | Requirement | Value | Status |
|----|-------------|-------|--------|
| PRF-001 | Context switch time | <10μs | ✅ Met |
| PRF-002 | Interrupt latency | <5μs | ✅ Met |
| PRF-003 | Task creation time | <100μs | ✅ Met |
| PRF-004 | Boot to first task | <100ms | ✅ Met |
| PRF-005 | AI inference (small model) | <10ms | Planned |
| PRF-006 | AI inference (medium model) | <50ms | Planned |
| PRF-007 | Pipe enqueue (uncontended) | <5μs | ✅ Met |
| PRF-008 | Semaphore acquire (uncontended) | <2μs | ✅ Met |
| PRF-009 | Pipe dequeue (uncontended) | <5μs | ✅ Met |
| PRF-010 | Semaphore release (uncontended) | <2μs | ✅ Met |

### 4.2 Resource Requirements

| ID | Requirement | Value | Status |
|----|-------------|-------|--------|
| PRF-010 | Kernel code size | <32KB | ✅ Met |
| PRF-011 | Kernel RAM usage | <8KB + stacks | ✅ Met |
| PRF-012 | Per-task stack | 2 KB (ICARUS_STACK_WORDS=512 words) | ✅ Met |
| PRF-013 | AI runtime code size | <64KB | Planned |
| PRF-014 | AI runtime RAM (base) | <16KB | Planned |

### 4.3 Scalability Requirements

| ID | Requirement | Value | Status |
|----|-------------|-------|--------|
| PRF-020 | Maximum tasks | 128 (ICARUS_MAX_TASKS, configurable) | ✅ Met |
| PRF-021 | Maximum priority levels | 8 | Planned |
| PRF-022 | Maximum message queues | 64 (ICARUS_MAX_MESSAGE_QUEUES) | ✅ Met |
| PRF-023 | Maximum semaphores | 64 (ICARUS_MAX_SEMAPHORES) | ✅ Met |
| PRF-024 | Maximum AI models loaded | 4 | Planned |

---

## 5. Interface Requirements

### 5.1 Kernel API

```c
// Task Management
void os_init(void);
void os_start(void);
void os_register_task(void (*function)(void), const char *name);
void os_create_task(task_t *task, void (*function)(void), 
                    uint32_t *stack, uint32_t stack_size, const char *name);
void os_exit_task(void);
void os_kill_process(uint8_t task_index);

// Scheduling
void os_yield(void);
uint32_t task_active_sleep(uint32_t ticks);
uint32_t task_blocking_sleep(uint32_t ticks);

// Information
uint32_t os_get_tick_count(void);
uint8_t os_get_running_task_count(void);
const char* os_get_current_task_name(void);

// Semaphores (Implemented)
bool semaphore_init(uint8_t semaphore_idx, uint32_t semaphore_count);
bool semaphore_feed(uint8_t semaphore_idx);
bool semaphore_consume(uint8_t semaphore_idx);
uint32_t semaphore_get_count(uint8_t semaphore_idx);
uint32_t semaphore_get_max_count(uint8_t semaphore_idx);

// Message Pipes (Implemented)
bool pipe_init(uint8_t pipe_idx, uint8_t capacity);
bool pipe_enqueue(uint8_t pipe_idx, uint8_t data);
int16_t pipe_dequeue(uint8_t pipe_idx);
uint8_t pipe_get_count(uint8_t pipe_idx);
uint8_t pipe_get_max_count(uint8_t pipe_idx);
```

### 5.2 AI Runtime API

```c
// Model Management (Planned)
int ai_model_load(ai_model_t *model, const void *model_data, size_t size);
int ai_model_validate(ai_model_t *model);
void ai_model_unload(ai_model_t *model);

// Inference (Planned)
int ai_inference_run(ai_model_t *model, const void *input, void *output);
int ai_inference_get_confidence(ai_model_t *model, float *confidence);
uint32_t ai_inference_get_wcet(ai_model_t *model);

// Operator Library (Planned)
void ai_op_conv2d(const ai_tensor_t *input, const ai_tensor_t *weights,
                  const ai_tensor_t *bias, ai_tensor_t *output,
                  const ai_conv_params_t *params);
void ai_op_dense(const ai_tensor_t *input, const ai_tensor_t *weights,
                 const ai_tensor_t *bias, ai_tensor_t *output);
void ai_op_relu(ai_tensor_t *tensor);
void ai_op_softmax(const ai_tensor_t *input, ai_tensor_t *output);
```

### 5.3 BSP Interface

```c
// Platform Abstraction (Planned unified interface)
void icarus_hal_init(void);
uint32_t icarus_hal_get_tick(void);
void icarus_hal_enter_critical(void);
void icarus_hal_exit_critical(void);
void icarus_hal_context_switch(void);
void icarus_hal_start_first_task(task_t *task);

// Current Implementation
void hal_init(void);
void LED_On(void);
void LED_Off(void);
void LED_Blink(uint32_t on_ticks, uint32_t off_ticks);
int32_t platform_write(void *handle, uint8_t reg, uint8_t *data, uint16_t len);
int32_t platform_read(void *handle, uint8_t reg, uint8_t *data, uint16_t len);
```

---

## 6. Safety Requirements

### 6.1 Fault Detection

| ID | Requirement | Priority | Status |
|----|-------------|----------|--------|
| SAF-001 | The system shall detect stack overflow | Must | Planned |
| SAF-002 | The system shall detect null pointer dereference | Should | Planned |
| SAF-003 | The system shall detect watchdog timeout | Must | Planned |
| SAF-004 | The system shall detect AI model corruption | Must | Planned |
| SAF-005 | The system shall detect AI output out of bounds | Must | Planned |

### 6.2 Fault Response

| ID | Requirement | Priority | Status |
|----|-------------|----------|--------|
| SAF-010 | The system shall halt on unrecoverable fault | Must | ✅ Implemented |
| SAF-011 | The system shall log fault information | Should | Planned |
| SAF-012 | The system shall support graceful degradation | Should | Planned |
| SAF-013 | The system shall support task restart on fault | Could | Planned |

---

## 7. Traceability

### 7.1 Requirements to Tests

See `ICARUS-VER-003 Test Traceability Matrix` for complete mapping.

### 7.2 Requirements Status Summary

| Category | Total | Implemented | Planned |
|----------|-------|-------------|---------|
| Kernel (KRN) | 69 | 58 | 11 |
| BSP | 18 | 14 | 4 |
| AI Runtime | 24 | 0 | 24 |
| Performance | 18 | 14 | 4 |
| Safety | 9 | 1 | 8 |
| **Total** | **138** | **87** | **51** |

> Counts include MPU/fault requirements HLR-KRN-060 through HLR-KRN-086,
> shared service modules HLR-KRN-090 through HLR-KRN-094, Software Bus
> (HLR-KRN-095), Background Checksum (HLR-KRN-096), task restart
> (HLR-KRN-097), timed semaphore (HLR-KRN-098), task diagnostics
> (HLR-KRN-099), BSP watchdog/button/CDC write (HLR-BSP-025 to
> HLR-BSP-027), and the updated IPC scalability numbers (PRF-022/023).
> Recount after adding or removing requirements.

---

*End of Document*
