# Deactivated Code Analysis

**Document ID:** ICARUS-VER-002  
**Version:** 0.1  
**Date:** 2025-01-26  
**Status:** Draft  
**Classification:** Public (Open Source)  

## 1. Purpose

This document identifies and justifies deactivated code (code that cannot be executed during normal operation or testing) in accordance with DO-178C objectives. Deactivated code must be analyzed to ensure it does not adversely affect safety.

## 2. Definitions

Per DO-178C:
- **Deactivated Code:** Executable code that is not intended to be executed in any configuration of the target computer environment.
- **Dead Code:** Executable code that cannot be reached during any execution path.

## 3. Deactivated Code Inventory

### 3.1 Fault Handlers (Dead Code)

These handlers execute only when the processor encounters a fault condition. They are intentionally designed as infinite loops to halt the system safely.

#### 3.1.1 Error_Handler

| Attribute | Value |
|-----------|-------|
| **File** | `Core/Src/bsp/error.c` |
| **Function** | `Error_Handler(void)` |
| **Lines** | 4 |
| **Category** | Dead Code |
| **Trigger** | Called on unrecoverable HAL errors |

**Code:**
```c
void Error_Handler(void) {
    __disable_irq();
    while (1) {
    }
}
```

**Justification:** This function is called only when the HAL detects an unrecoverable error. The infinite loop is intentional to halt the system and prevent undefined behavior. In a certified system, this would trigger a watchdog reset.

**Verification Method:** Code review, static analysis

---

#### 3.1.2 NMI_Handler

| Attribute | Value |
|-----------|-------|
| **File** | `Core/Src/bsp/stm32h7xx_it.c` |
| **Function** | `NMI_Handler(void)` |
| **Lines** | ~3 |
| **Category** | Dead Code |
| **Trigger** | Non-maskable interrupt (hardware fault) |

**Justification:** NMI is triggered by hardware faults (e.g., clock security system). Cannot be triggered in unit tests.

**Verification Method:** Code review

---

#### 3.1.3 HardFault_Handler

| Attribute | Value |
|-----------|-------|
| **File** | `Core/Src/bsp/stm32h7xx_it.c` |
| **Function** | `HardFault_Handler(void)` |
| **Lines** | ~3 |
| **Category** | Dead Code |
| **Trigger** | Processor hard fault (invalid memory access, etc.) |

**Code Pattern:**
```c
void HardFault_Handler(void) {
    while (1) {
    }
}
```

**Justification:** Hard faults occur on invalid memory access, undefined instructions, or other processor exceptions. The infinite loop prevents further corruption. Cannot be safely triggered in unit tests.

**Verification Method:** Code review, static analysis

---

#### 3.1.4 MemManage_Handler

| Attribute | Value |
|-----------|-------|
| **File** | `Core/Src/bsp/stm32h7xx_it.c` |
| **Function** | `MemManage_Handler(void)` |
| **Lines** | ~3 |
| **Category** | Dead Code |
| **Trigger** | Memory protection unit (MPU) violation |

**Justification:** Triggered by MPU violations. Cannot be triggered without hardware MPU.

**Verification Method:** Code review

---

#### 3.1.5 BusFault_Handler

| Attribute | Value |
|-----------|-------|
| **File** | `Core/Src/bsp/stm32h7xx_it.c` |
| **Function** | `BusFault_Handler(void)` |
| **Lines** | ~3 |
| **Category** | Dead Code |
| **Trigger** | Bus error during memory access |

**Justification:** Triggered by bus errors (e.g., accessing invalid peripheral address). Cannot be triggered in host tests.

**Verification Method:** Code review

---

#### 3.1.6 UsageFault_Handler

| Attribute | Value |
|-----------|-------|
| **File** | `Core/Src/bsp/stm32h7xx_it.c` |
| **Function** | `UsageFault_Handler(void)` |
| **Lines** | ~3 |
| **Category** | Dead Code |
| **Trigger** | Usage fault (undefined instruction, unaligned access) |

**Justification:** Triggered by usage faults. Cannot be triggered in host tests.

**Verification Method:** Code review

---

### 3.2 Context Switch Handler (Target-Only Code)

#### 3.2.1 PendSV_Handler

| Attribute | Value |
|-----------|-------|
| **File** | `Core/Src/bsp/stm32h7xx_it.c` |
| **Function** | `PendSV_Handler(void)` |
| **Lines** | ~5 |
| **Category** | Target-Only Code |
| **Trigger** | PendSV interrupt for context switching |

**Code:**
```c
void PendSV_Handler(void) {
    context_switch();  // ARM assembly function
}
```

**Justification:** This handler calls `context_switch()` which is implemented in ARM assembly (`context_switch.s`). The assembly code manipulates ARM-specific registers (PSP, LR, etc.) that don't exist on the host platform.

**Verification Method:** Target integration testing, code review

---

### 3.3 Infinite Loop Tasks (Target-Only Code)

These are RTOS tasks that run continuously. They cannot be unit tested because they never return.

#### 3.3.1 os_idle_task

| Attribute | Value |
|-----------|-------|
| **File** | `Core/Src/icarus/task.c` |
| **Function** | `os_idle_task(void)` |
| **Lines** | ~8 |
| **Category** | Target-Only Code |
| **Purpose** | System idle task, initializes display |

**Code:**
```c
static void os_idle_task(void) {
    display_init();
    while (1) {
        os_yield();
    }
}
```

**Justification:** This task runs when no other tasks are ready. The `while(1)` loop is intentional RTOS design. Individual functions called within (`display_init`, `os_yield`) are tested separately.

**Verification Method:** Target integration testing, component unit tests

---

#### 3.3.2 os_heartbeart_task

| Attribute | Value |
|-----------|-------|
| **File** | `Core/Src/icarus/task.c` |
| **Function** | `os_heartbeart_task(void)` |
| **Lines** | ~25 |
| **Category** | Target-Only Code |
| **Purpose** | System heartbeat LED and display |

**Justification:** Heartbeat task blinks LED and updates display continuously. Individual functions (`LED_Blink`, `display_render_banner`, `task_active_sleep`) are tested separately.

**Verification Method:** Target integration testing, component unit tests

---

#### 3.3.3 os_transmit_printf_task

| Attribute | Value |
|-----------|-------|
| **File** | `Core/Src/icarus/task.c` |
| **Function** | `os_transmit_printf_task(void)` |
| **Lines** | ~25 |
| **Category** | Target-Only Code |
| **Purpose** | Asynchronous printf transmission over USB |

**Justification:** This task dequeues characters from the print buffer and transmits via USB CDC. The `while(1)` loop is intentional. Individual functions (`dequeue_print_buffer`, `CDC_Transmit_FS`, `task_busy_wait`, `task_active_sleep`) are tested where possible.

**Verification Method:** Target integration testing, component unit tests

---

### 3.4 Static Helper Functions

#### 3.4.1 dequeue_print_buffer

| Attribute | Value |
|-----------|-------|
| **File** | `Core/Src/icarus/task.c` |
| **Function** | `dequeue_print_buffer(uint8_t *out_c)` |
| **Lines** | ~12 |
| **Category** | Indirectly Deactivated |
| **Caller** | `os_transmit_printf_task` only |

**Justification:** This static function is only called by `os_transmit_printf_task`, which cannot be tested. The complementary function `enqueue_print_buffer` is fully tested, providing confidence in the buffer implementation.

**Verification Method:** Code review, `enqueue_print_buffer` tests provide indirect verification

---

## 4. Safety Analysis

### 4.1 Impact Assessment

| Code Category | Safety Impact | Mitigation |
|---------------|---------------|------------|
| Fault Handlers | Low - only execute on faults | Watchdog timer resets system |
| Context Switch | Medium - critical for scheduling | Target integration tests |
| Infinite Loop Tasks | Low - non-safety-critical features | Component tests cover internals |
| Static Helpers | Low - buffer management | Complementary function tested |

### 4.2 Conclusion

All identified deactivated code has been analyzed and justified. The deactivated code:
- Does not contain safety-critical logic that could cause hazards
- Is either fault-handling code (intentionally unreachable) or requires target hardware
- Has been verified through code review and/or component testing

## 5. Approval

| Role | Name | Signature | Date |
|------|------|-----------|------|
| Author | | | |
| Reviewer | | | |
| Safety Engineer | | | |

## 6. Revision History

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 0.1 | 2025-01-26 | Souham Biswas | Initial draft |
