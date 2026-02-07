# Plan for Software Aspects of Certification (PSAC)

**Document ID:** ICARUS-PSAC-001  
**Version:** 1.0  
**Date:** 2025-02-07  
**Status:** Current  
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
| 1.0 | 2025-02-07 | Souham Biswas | Updated with SVC architecture, 142 tests, privilege separation |

---

## Table of Contents

1. [Introduction](#1-introduction)
2. [System Overview](#2-system-overview)
3. [Software Overview](#3-software-overview)
4. [Certification Considerations](#4-certification-considerations)
5. [Software Development Environment](#5-software-development-environment)
6. [Software Life Cycle](#6-software-life-cycle)
7. [Software Life Cycle Data](#7-software-life-cycle-data)
8. [Schedule](#8-schedule)
9. [Additional Considerations](#9-additional-considerations)

---

## 1. Introduction

### 1.1 Purpose

This Plan for Software Aspects of Certification (PSAC) describes the software development and verification processes for the ICARUS Operating System (ICARUS OS). This document provides the certification authority with visibility into the software development activities and demonstrates compliance with DO-178C objectives.

### 1.2 Scope

This PSAC covers the ICARUS OS real-time operating system kernel and board support package (BSP) software executing on the STM32H750 microcontroller platform.

### 1.3 Applicable Documents

| Document ID | Title | Version |
|-------------|-------|---------|
| DO-178C | Software Considerations in Airborne Systems and Equipment Certification | - |
| DO-330 | Software Tool Qualification Considerations | - |
| DO-331 | Model-Based Development and Verification Supplement | - |
| ARP4754A | Guidelines for Development of Civil Aircraft and Systems | - |

### 1.4 Reference Documents

| Document ID | Title |
|-------------|-------|
| ICARUS-SDP-001 | Software Development Plan |
| ICARUS-SVP-001 | Software Verification Plan |
| ICARUS-SCMP-001 | Software Configuration Management Plan |
| ICARUS-SQAP-001 | Software Quality Assurance Plan |
| ICARUS-SRS-001 | Software Requirements Specification |


---

## 2. System Overview

### 2.1 System Description

ICARUS OS is a preemptive real-time operating system designed for embedded aerospace applications. The system provides task scheduling, inter-task communication, privilege separation via SVC mechanism, and hardware abstraction for the STM32H750 Cortex-M7 microcontroller with MPU-based memory protection.

### 2.2 System Functions

The system provides the following high-level functions:

| Function | Description |
|----------|-------------|
| Task Scheduling | Preemptive round-robin scheduling with 50ms time quantum |
| Task Management | Task creation, termination, sleep, and yield operations |
| Privilege Separation | SVC-based kernel protection with 28 entry points |
| Memory Protection | MPU-ready architecture with ITCM/DTCM optimization |
| IPC | Semaphores (32) and message pipes (32) for inter-task communication |
| Interrupt Handling | SysTick timer, fault handlers, USB interrupts |
| Hardware Abstraction | GPIO, I2C, SPI, display, LED control |
| Debug Output | USB CDC serial output for diagnostics |

### 2.3 System Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    Application Tasks                         │
│                    (Unprivileged Mode)                       │
├─────────────────────────────────────────────────────────────┤
│                    Public API Layer                          │
│                    (SVC Wrappers)                            │
├─────────────────────────────────────────────────────────────┤
│                    SVC Handler                               │
│                    (Privilege Boundary)                      │
├─────────────────────────────────────────────────────────────┤
│                    ICARUS OS Kernel                          │
│                    (Privileged Mode, MPU Protected)          │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────┐  │
│  │  Scheduler  │  │Task Manager │  │   IPC (Sem/Pipe)    │  │
│  │  (ITCM)     │  │             │  │   (ITCM)            │  │
│  └─────────────┘  └─────────────┘  └─────────────────────┘  │
│  ┌─────────────────────────────────────────────────────────┐ │
│  │  Kernel Data (DTCM - Zero Wait State)                   │ │
│  └─────────────────────────────────────────────────────────┘ │
├─────────────────────────────────────────────────────────────┤
│                 Board Support Package (BSP)                  │
│  ┌───────┐ ┌─────┐ ┌─────┐ ┌─────┐ ┌───────┐ ┌──────────┐  │
│  │Display│ │GPIO │ │ I2C │ │ SPI │ │  USB  │ │Interrupts│  │
│  └───────┘ └─────┘ └─────┘ └─────┘ └───────┘ └──────────┘  │
├─────────────────────────────────────────────────────────────┤
│                    STM32 HAL Drivers                         │
├─────────────────────────────────────────────────────────────┤
│                 STM32H750 Hardware (Cortex-M7)               │
│                 ITCM (64KB) | DTCM (128KB) | Flash (128KB)   │
└─────────────────────────────────────────────────────────────┘
```

### 2.4 Hardware Platform

| Component | Specification |
|-----------|---------------|
| Processor | STM32H750VBT6 (ARM Cortex-M7) |
| Clock Speed | Up to 480 MHz |
| Flash | 128 KB internal + external QSPI |
| ITCM | 64 KB (zero wait-state code) |
| DTCM | 128 KB (zero wait-state data) |
| AXI SRAM | 512 KB (general purpose) |
| Peripherals | USB OTG, SPI, I2C, GPIO, TIM |
| MPU | 16 regions, privilege separation |


---

## 3. Software Overview

### 3.1 Software Functions

#### 3.1.1 Kernel Functions

| Function | Description | Safety Relevance |
|----------|-------------|------------------|
| `os_init()` | Initialize kernel state | High - system startup |
| `os_start()` | Begin task execution | High - scheduler start |
| `os_yield()` | Voluntary context switch | Medium - task cooperation |
| `os_create_task()` | Create new task | Medium - resource allocation |
| `os_kill_process()` | Terminate task | Medium - resource cleanup |
| `task_active_sleep()` | Non-blocking sleep | Low - timing |
| `task_blocking_sleep()` | Blocking sleep | Low - timing |

#### 3.1.2 BSP Functions

| Function | Description | Safety Relevance |
|----------|-------------|------------------|
| `hal_init()` | Initialize hardware | High - hardware setup |
| `LED_Blink()` | Heartbeat indication | Low - status display |
| `display_init()` | Initialize LCD | Low - user interface |
| `platform_read/write()` | I2C communication | Medium - sensor data |

### 3.2 Software Architecture

The software is organized into the following components:

| Component | Location | Lines of Code | Description |
|-----------|----------|---------------|-------------|
| Kernel | `Core/Src/kernel/` | ~400 | Task scheduler and management |
| BSP | `Core/Src/bsp/` | ~300 | Hardware abstraction |
| Context Switch | `Core/Src/kernel/context_switch.s` | ~100 | ARM assembly context switch |
| Startup | `Core/Startup/` | ~200 | Processor initialization |

### 3.3 Software Partitioning

The software does not implement partitioning. All tasks execute in the same memory space with shared access to kernel resources.

**Rationale:** The target application does not require memory protection between tasks. All tasks are developed and verified together as a single software load.

### 3.4 Deactivated Code

The following code is present but not executed during normal operation:

| Code | Purpose | Justification |
|------|---------|---------------|
| Fault handlers | Error recovery | Execute only on hardware faults |
| `os_transmit_printf_task` | Debug output | Currently disabled in `os_init()` |

See `ICARUS-VER-002 Deactivated Code Analysis` for complete analysis.


---

## 4. Certification Considerations

### 4.1 Software Level

**Assigned Software Level: DAL C (Major)**

**Rationale:** Failure of the ICARUS OS could result in:
- Loss of task scheduling leading to system unresponsiveness
- Incorrect timing affecting dependent systems
- These effects are classified as "Major" per ARP4754A

### 4.2 DO-178C Objectives

The following DO-178C objectives apply to DAL C software:

| Table | Objective | Applicable |
|-------|-----------|------------|
| A-1 | Software Planning Process | Yes |
| A-2 | Software Development Process | Yes |
| A-3 | Verification of Outputs of Software Requirements Process | Yes |
| A-4 | Verification of Outputs of Software Design Process | Yes |
| A-5 | Verification of Outputs of Software Coding & Integration | Yes |
| A-6 | Testing of Outputs of Integration Process | Yes |
| A-7 | Verification of Verification Process Results | Yes |
| A-8 | Software Configuration Management Process | Yes |
| A-9 | Software Quality Assurance Process | Yes |
| A-10 | Certification Liaison Process | Yes |

### 4.3 Structural Coverage

Per DO-178C Table A-7, DAL C requires:

| Coverage Type | Required | Method |
|---------------|----------|--------|
| Statement Coverage | Yes | Host-based unit tests + target integration |
| Decision Coverage | No | - |
| MC/DC | No | - |

**Current Status:** 79.3% statement coverage achieved via host-based testing. Remaining coverage requires target integration testing for infinite-loop tasks and assembly code.

### 4.4 Software Tool Qualification

| Tool | Purpose | Category | Qualification |
|------|---------|----------|---------------|
| GCC (ARM) | Compilation | Development | Not required |
| GCC (Host) | Test compilation | Verification | Not required |
| Unity | Test framework | Verification | TQL-5 (criteria-based) |
| lcov/gcov | Coverage analysis | Verification | TQL-5 (criteria-based) |
| Git | Version control | Development | Not required |

**Rationale:** Per DO-330, verification tools that could fail to detect errors require qualification. Unity and lcov are used to verify coverage; their output is reviewed manually, reducing qualification burden.


---

## 5. Software Development Environment

### 5.1 Development Hardware

| Item | Description |
|------|-------------|
| Development Host | macOS workstation |
| Target Hardware | STM32H750VBT6 development board |
| Debug Probe | ST-Link V2 or J-Link |
| Display | ST7735 LCD module |

### 5.2 Development Software

| Tool | Version | Purpose |
|------|---------|---------|
| STM32CubeIDE | 1.x | IDE and project management |
| ARM GCC | 10.x+ | Cross-compilation |
| GNU Make | 4.x | Build automation |
| Git | 2.x | Version control |
| GitHub | - | Repository hosting |

### 5.3 Test Environment

#### 5.3.1 Host-Based Testing

| Tool | Version | Purpose |
|------|---------|---------|
| GCC (native) | System | Host compilation |
| Unity | 2.5.x | Unit test framework |
| lcov | 2.x | Coverage collection |
| genhtml | 2.x | Coverage reporting |

#### 5.3.2 Target-Based Testing

| Tool | Purpose |
|------|---------|
| ST-Link GDB Server | Debug connection |
| Serial terminal | USB CDC output capture |
| Logic analyzer | Timing verification |

### 5.4 Configuration Management

- **Repository:** GitHub (private)
- **Branching Model:** Feature branches merged to `dev`, releases from `master`
- **Tagging:** Semantic versioning (vX.Y.Z)

See `ICARUS-SCMP-001 Software Configuration Management Plan` for details.


---

## 6. Software Life Cycle

### 6.1 Life Cycle Model

ICARUS OS follows an iterative development model with the following phases:

```
┌──────────────┐     ┌──────────────┐     ┌──────────────┐
│  Planning    │────▶│ Requirements │────▶│    Design    │
└──────────────┘     └──────────────┘     └──────────────┘
                                                  │
       ┌──────────────────────────────────────────┘
       ▼
┌──────────────┐     ┌──────────────┐     ┌──────────────┐
│    Coding    │────▶│ Integration  │────▶│ Verification │
└──────────────┘     └──────────────┘     └──────────────┘
                                                  │
       ┌──────────────────────────────────────────┘
       ▼
┌──────────────┐     ┌──────────────┐
│Certification │────▶│   Release    │
│   Liaison    │     │              │
└──────────────┘     └──────────────┘
```

### 6.2 Software Planning Process

**Objective:** Establish plans for software development and verification.

**Outputs:**
- Plan for Software Aspects of Certification (PSAC) - this document
- Software Development Plan (SDP)
- Software Verification Plan (SVP)
- Software Configuration Management Plan (SCMP)
- Software Quality Assurance Plan (SQAP)

### 6.3 Software Requirements Process

**Objective:** Define high-level and low-level software requirements.

**Inputs:**
- System requirements (from system-level analysis)
- Hardware interface specifications

**Outputs:**
- Software Requirements Specification (SRS)
- Interface Control Document (ICD)

### 6.4 Software Design Process

**Objective:** Define software architecture and detailed design.

**Outputs:**
- Software Architecture Document
- Detailed design (in code comments and headers)

### 6.5 Software Coding Process

**Objective:** Implement software per design.

**Standards:**
- MISRA C:2012 (subset)
- Project coding standards

**Outputs:**
- Source code
- Build scripts

### 6.6 Integration Process

**Objective:** Combine software components and load onto target.

**Outputs:**
- Executable software load
- Integration test results

### 6.7 Software Verification Process

**Objective:** Verify software meets requirements.

**Methods:**
- Reviews (requirements, design, code)
- Analysis (coverage, static analysis)
- Testing (unit, integration, system)

**Outputs:**
- Test procedures and results
- Coverage analysis
- Verification reports


---

## 7. Software Life Cycle Data

### 7.1 Data Items

The following data items are produced per DO-178C:

| Data Item | Document ID | Status |
|-----------|-------------|--------|
| Plan for Software Aspects of Certification | ICARUS-PSAC-001 | Draft |
| Software Development Plan | ICARUS-SDP-001 | Draft |
| Software Verification Plan | ICARUS-SVP-001 | Draft |
| Software Configuration Management Plan | ICARUS-SCMP-001 | Draft |
| Software Quality Assurance Plan | ICARUS-SQAP-001 | Draft |
| Software Requirements Specification | ICARUS-SRS-001 | Draft |
| Software Design Description | ICARUS-SDD-001 | Draft |
| Source Code | Repository | In Progress |
| Executable Object Code | Build artifacts | In Progress |
| Software Verification Cases and Procedures | ICARUS-SVCP-001 | In Progress |
| Software Verification Results | ICARUS-SVR-001 | In Progress |
| Software Configuration Index | ICARUS-SCI-001 | Planned |
| Software Accomplishment Summary | ICARUS-SAS-001 | Planned |

### 7.2 Data Control Categories

Per DO-178C Table A-9:

| Category | Control Level | Items |
|----------|---------------|-------|
| CC1 | Configuration identification, change control, baseline | Source code, executables |
| CC2 | Configuration identification, change control | Plans, requirements, design |

### 7.3 Traceability

The following traceability is maintained:

```
System Requirements
        │
        ▼
Software Requirements (HLR) ◄──── Test Cases
        │                              │
        ▼                              ▼
Low-Level Requirements ◄───────── Test Procedures
        │                              │
        ▼                              ▼
Source Code ◄─────────────────── Test Results
```

See `ICARUS-VER-003 Test Traceability Matrix` for current traceability.


---

## 8. Schedule

### 8.1 Development Milestones

| Milestone | Target Date | Status |
|-----------|-------------|--------|
| PSAC Draft | 2025-01-26 | ✅ Complete |
| Software Requirements | TBD | Planned |
| Design Complete | TBD | Planned |
| Code Complete | TBD | In Progress |
| Unit Test Complete | 2025-01-26 | ✅ Complete |
| Integration Test Complete | TBD | Planned |
| Coverage Analysis Complete | 2025-01-26 | ✅ Complete (host) |
| Target Testing Complete | TBD | Planned |
| SAS Submission | TBD | Planned |

### 8.2 Certification Milestones

| Milestone | Target Date | Status |
|-----------|-------------|--------|
| Stage 1 - Planning Review | TBD | Planned |
| Stage 2 - Development Review | TBD | Planned |
| Stage 3 - Verification Review | TBD | Planned |
| Stage 4 - Final Review | TBD | Planned |

---

## 9. Additional Considerations

### 9.1 Previously Developed Software

The following previously developed software is used:

| Component | Source | Qualification |
|-----------|--------|---------------|
| STM32 HAL | STMicroelectronics | Vendor-qualified |
| CMSIS | ARM | Industry standard |
| USB Middleware | STMicroelectronics | Vendor-qualified |

**Approach:** These components are treated as COTS (Commercial Off-The-Shelf). Verification focuses on correct integration and usage.

### 9.2 User-Modifiable Software

ICARUS OS does not contain user-modifiable software. All configuration is compile-time.

### 9.3 Field-Loadable Software

The software is field-loadable via:
- ST-Link debug interface
- USB DFU (Device Firmware Update) - if enabled

**Protection:** CRC verification of loaded image (planned).

### 9.4 Multiple-Version Dissimilar Software

Not applicable. Single version only.

### 9.5 Certification Authority Coordination

| Authority | Contact | Status |
|-----------|---------|--------|
| FAA (if US) | TBD | Not initiated |
| EASA (if EU) | TBD | Not initiated |

---

## Appendix A: Acronyms

| Acronym | Definition |
|---------|------------|
| BSP | Board Support Package |
| CC | Control Category |
| COTS | Commercial Off-The-Shelf |
| DAL | Design Assurance Level |
| DER | Designated Engineering Representative |
| HAL | Hardware Abstraction Layer |
| HLR | High-Level Requirement |
| LLR | Low-Level Requirement |
| MC/DC | Modified Condition/Decision Coverage |
| PSAC | Plan for Software Aspects of Certification |
| RTOS | Real-Time Operating System |
| SAS | Software Accomplishment Summary |
| SCMP | Software Configuration Management Plan |
| SDP | Software Development Plan |
| SQAP | Software Quality Assurance Plan |
| SRS | Software Requirements Specification |
| SVP | Software Verification Plan |
| TQL | Tool Qualification Level |

---

## Appendix B: Document Cross-Reference

| Section | Related Documents |
|---------|-------------------|
| 3. Software Overview | ICARUS-SRS-001, ICARUS-SDD-001 |
| 4. Certification | DO-178C, ARP4754A |
| 5. Development Environment | ICARUS-SDP-001 |
| 6. Life Cycle | ICARUS-SDP-001, ICARUS-SVP-001 |
| 7. Life Cycle Data | ICARUS-SCMP-001 |

---

*End of Document*
