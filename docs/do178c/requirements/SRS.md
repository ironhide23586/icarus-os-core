# Software Requirements Specification (SRS)

**Document ID:** ICARUS-SRS-001  
**Version:** 0.1  
**Date:** 2025-01-26  
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
| HLR-KRN-051 | The kernel shall support up to MAX_SEMAPHORES (32) semaphores | Must | ✅ Implemented |
| HLR-KRN-052 | The kernel shall support up to MAX_MESSAGE_QUEUES (32) pipes | Must | ✅ Implemented |
| HLR-KRN-053 | Each pipe shall support up to 128 bytes capacity | Must | ✅ Implemented |

#### 3.1.6 Memory Management

| ID | Requirement | Priority | Status |
|----|-------------|----------|--------|
| HLR-KRN-060 | The kernel shall use static memory allocation only | Must | ✅ Implemented |
| HLR-KRN-061 | The kernel shall detect stack overflow | Should | Planned |
| HLR-KRN-062 | The kernel shall support MPU-based memory protection | Should | Planned |
| HLR-KRN-063 | The kernel shall support memory partitioning (ARINC 653) | Could | Planned |

#### 3.1.7 Fault Handling

| ID | Requirement | Priority | Status |
|----|-------------|----------|--------|
| HLR-KRN-070 | The kernel shall handle processor faults gracefully | Must | ✅ Implemented |
| HLR-KRN-071 | The kernel shall support watchdog integration | Should | Planned |
| HLR-KRN-072 | The kernel shall log fault information for diagnostics | Should | Planned |
| HLR-KRN-073 | The kernel shall support fault recovery (task restart) | Could | Planned |

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
| PRF-012 | Per-task stack (minimum) | 512 bytes | ✅ Met |
| PRF-013 | AI runtime code size | <64KB | Planned |
| PRF-014 | AI runtime RAM (base) | <16KB | Planned |

### 4.3 Scalability Requirements

| ID | Requirement | Value | Status |
|----|-------------|-------|--------|
| PRF-020 | Maximum tasks | 16 (configurable) | ✅ Met |
| PRF-021 | Maximum priority levels | 8 | Planned |
| PRF-022 | Maximum message queues | 8 | Planned |
| PRF-023 | Maximum semaphores | 20 | ✅ Met |
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

// IPC (Planned)
int ipc_queue_create(queue_t *queue, size_t item_size, size_t capacity);
int ipc_queue_send(queue_t *queue, const void *item, uint32_t timeout);
int ipc_queue_receive(queue_t *queue, void *item, uint32_t timeout);

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
| Kernel | 35 | 22 | 13 |
| BSP | 15 | 11 | 4 |
| AI Runtime | 24 | 0 | 24 |
| Performance | 18 | 12 | 6 |
| Safety | 9 | 1 | 8 |
| **Total** | **101** | **46** | **55** |

---

*End of Document*
