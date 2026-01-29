# ICARUS OS Core

**Intelligent Certifiable Autonomous Real-time Unified System**

A minimal, deterministic real-time kernel for Cortex-M designed to support DO-178C DAL C certification objectives. Built from the ground up with safety-critical aerospace and defense applications in mind.

```
┌──────────────────────────────────────────────────────────────┐
│   ██╗ ██████╗  █████╗ ██████╗ ██╗   ██╗ ██████╗              │
│   ██║██╔════╝ ██╔══██╗██╔══██╗██║   ██║██╔════╝              │
│   ██║██║      ███████║██████╔╝██║   ██║╚█████╗               │
│   ██║██║      ██╔══██║██╔══██╗██║   ██║ ╚═══██╗              │
│   ██║╚██████╗ ██║  ██║██║  ██║╚██████╔╝██████╔╝              │
│   ╚═╝ ╚═════╝ ╚═╝  ╚═╝╚═╝  ╚═╝ ╚═════╝ ╚═════╝               │
│   Preemptive Kernel • ARMv7E-M • STM32H750                   │
└──────────────────────────────────────────────────────────────┘
```

> **⚠️ WARNING: This is work in progress and NOT production ready.**
> 
> This kernel is under active development. It may contain bugs, incomplete features, and is not suitable for use in production systems. Use at your own risk.

---

## Highlights

| Metric | Status |
|--------|--------|
| **Test Coverage** | 88.5% line, 85.9% function |
| **Unit Tests** | 131 tests, 0 failures |
| **Static Analysis** | cppcheck clean |
| **Certification Target** | DO-178C DAL C |
| **Coding Standard** | MISRA C:2012 subset |

---

## Terminal GUI

ICARUS OS features a real-time terminal-based GUI that visualizes kernel activity, IPC operations, and stress test metrics. The display uses ANSI escape codes for cursor positioning and colors.

### GUI Layout Overview

```
┌──────────────────────────────────────────────────────────────────────────────────────────────────────────┐
│                                              HEADER                                                       │
│  ICARUS ASCII art logo + version info                                                                    │
├──────────────────────────────────────────────────────────────────────────────────────────────────────────┤
│                                         DEMO TASKS SECTION                                                │
│                                                                                                           │
│  [heartbeat]  ★★★★★★★★★★★★★★★★★★★★ [heartbeat]          SEM     +---SS---+  +---SM---+                   │
│  [producer]   ████████████────────  160/200 ticks       +---+   |>P0: 42|  |>P0:0042|                    │
│  [consumer]   ██████████──────────  100/190 ticks       |###|   |<C0: 42|  |<C0:0042|                    │
│  [reference]  ████████████████────  1200/3000 ticks     |###|   |>P0: 43|  |<C1:0043|                    │
│  [ss_prod]    ██████████████──────  220/400  →[106]     |###|   |<C0: 43|  |>P0:0044|                    │
│  [ss_cons]    ████████████────────  200/350  ←[105]     |   |   +--------+  +--------+                   │
│  [sm_prod]    ████████────────────   90/150  →[ 68]     |   |                                            │
│  [sm_con1]    ██████████████──────  225/350  ←[ 66]     +---+   +---MS---+  +---MM---+                   │
│  [sm_con2]    ████████████████────  300/450  ←[ 67]      3/10   |>P0: 42|  |>P0:006B|                    │
│  [ms_prd1]    ████████████████────  200/400  →[ 42]             |>P1:115|  |<C0:006B|                    │
│  [ms_prd2]    ██████████████████──  390/550  →[115]             |<C0: 42|  |>P1:0048|                    │
│  [ms_cons]    ████████────────────  150/350  ←[114]             |<C0:115|  |<C1:0048|                    │
│  [mm_prd1]    ██████████────────    175/400  →[ 23]             +--------+  +--------+                   │
│  [mm_prd2]    ████████████████████  480/600  →[ 11]                                                      │
│  [mm_con1]    ████████████████────  250/350  ←[ 22]                                                      │
│  [mm_con2]    ██████████████──────  280/400  ←[ 10]                                                      │
├──────────────────────────────────────────────────────────────────────────────────────────────────────────┤
│                                       STRESS TEST SECTION                                                 │
│                                                                                                           │
│  ═══════════════════════════════════════════════════════════════════════════════                         │
│    ⚡ STRESS TEST ACTIVE ⚡  Semaphores: 4  Pipes: 4  Tasks: 19                                           │
│  ═══════════════════════════════════════════════════════════════════════════════                         │
│                                                                                                           │
│  [sem_ham0]   ████████████████████   20/20  →[ 75]      SEM    SEM    SEM    SEM                         │
│  [sem_ham1]   ██████████████──────   14/20  ←[ 74]     +---+  +---+  +---+  +---+                        │
│  [sem_med0]   ████████████────────   60/100            |###|  |###|  |   |  |###|                        │
│  [sem_med1]   ██████████──────────   50/107            |###|  |###|  |   |  +---+                        │
│  [sem_slow]   ████████████████────   80/100            |###|  |   |  |   |   1/1                         │
│  [sem_mtx0]   ████████────────────   40/50  →[ 49]     |   |  |   |  |   |                               │
│  [sem_mtx1]   ██████████████──────   70/50  ←[ 48]     +---+  +---+  +---+                               │
│  [pf_send]    ████████████████████   10/10  →[255]      5/5    3/5    2/5                                │
│  [pf_recv]    ██████████████──────   14/20  ←[254]                                                       │
│  [pm_prd0]    ████████████────────   30/50  →[ 42]                                                       │
│  [pm_prd1]    ██████████████──────   35/61  →[ 41]                                                       │
│  [pm_prd2]    ████████████████────   40/73  →[ 40]                                                       │
│  [pm_cons]    ████████────────────   20/16  ←[ 39]                                                       │
│  [pv_send]    ██████████████████──   45/50  →[ 12]                                                       │
│  [pv_recv]    ████████████████────   40/55  ←[ 11]                                                       │
│  [yielder]    ████████████────────   12/20                                                               │
│  [sleeper]    ██████████──────────   25/50                                                               │
│  [cpu_hog]    ████████████████────   35/50                                                               │
│                                                                                                           │
│  STRESS: sem_f=1542 sem_c=1538 pipe_s=892 pipe_r=887 yields=4521 sleeps=3892                             │
│  WAITS: sem_max=45 pipe_max=12 full=234 empty=156                                                        │
│  VERIFY: seq=0 data=0 overflow=0 underflow=0  [PASS]                                                     │
└──────────────────────────────────────────────────────────────────────────────────────────────────────────┘
```

### GUI Components Explained

#### 1. Header Section
The ICARUS ASCII art logo with platform information (ARMv7E-M, STM32H750).

#### 2. Heartbeat Banner
```
[>ICARUS_HEARTBEAT<] ★★★★★★★★★★★★★★★★★★★★ [>ICARUS_HEARTBEAT<]
```
Flashes on/off synchronized with the onboard LED. Indicates the kernel is alive and scheduling tasks.

#### 3. Task Progress Bars
```
[task_name] ████████████────────────────  elapsed/period ticks
```
- **Green bars** (`████`): Producers sending messages
- **Magenta bars** (`████`): Consumers receiving messages
- **White bars**: Reference/utility tasks
- **Elapsed/Period**: Current tick count vs total period duration
- **Arrow indicators**: `→[value]` for sent messages, `←[value]` for received

#### 4. Semaphore Vertical Bars
```
  SEM
+---+
|###|  ← Filled portion (current count)
|###|
|   |  ← Empty portion
|   |
+---+
 3/10  ← current/max count
```
Visual representation of semaphore fill level. Fills from bottom to top as count increases.

#### 5. Message History Panels
```
+---SS---+
|>P0: 42|  ← Producer 0 sent value 42
|<C0: 42|  ← Consumer 0 received value 42
|>P0: 43|
|<C0: 43|
+--------+
```
Rolling history of the last 8 messages for each pipe configuration:
- **SS**: Single Producer → Single Consumer
- **SM**: Single Producer → Multiple Consumers
- **MS**: Multiple Producers → Single Consumer
- **MM**: Multiple Producers → Multiple Consumers

Format: `>Pn:` = Producer n sent, `<Cn:` = Consumer n received

#### 6. Stress Test Statistics

**STRESS line** - Operation counts:
| Metric | Description |
|--------|-------------|
| `sem_f` | Total semaphore feed (signal) operations |
| `sem_c` | Total semaphore consume (wait) operations |
| `pipe_s` | Total messages sent to pipes |
| `pipe_r` | Total messages received from pipes |
| `yields` | Times tasks voluntarily yielded CPU |
| `sleeps` | Times tasks called active sleep |

**WAITS line** - Contention metrics:
| Metric | Description |
|--------|-------------|
| `sem_max` | Longest wait time (ticks) for any semaphore operation |
| `pipe_max` | Longest wait time (ticks) for any pipe operation |
| `full` | Count of times sender blocked on full pipe |
| `empty` | Count of times receiver blocked on empty pipe |

**VERIFY line** - Data integrity verification:
| Metric | Description |
|--------|-------------|
| `seq` | Out-of-order message errors (FIFO violation) |
| `data` | Data corruption detected (wrong content) |
| `overflow` | Semaphore/pipe overflow errors |
| `underflow` | Semaphore/pipe underflow errors |
| `[PASS]` | Green if all zeros - no errors detected |
| `[FAIL]` | Red if any errors - bugs detected |

### Color Coding

| Color | Meaning |
|-------|---------|
| **Green** | Producers, successful operations, PASS status |
| **Magenta** | Consumers, receive operations |
| **Cyan** | Headers, labels, informational |
| **Yellow** | Warnings, stress test header |
| **Red** | Errors, FAIL status |
| **White** | Normal text, reference tasks |

---

## Overview

ICARUS OS is a lightweight, preemptive real-time operating system kernel designed for ARM Cortex-M7 microcontrollers (specifically STM32H750). The kernel provides deterministic task scheduling, context switching, and a clean API for embedded real-time applications.

### Vision

ICARUS is designed to be the first open-source RTOS with:
- **Native AI integration** with certifiable determinism (planned)
- **DO-178C alignment by design** (not retrofitted)
- **Hardware-agnostic portability** across safety-critical platforms
- **Formal verification readiness** for DAL A applications (future)

### Key Features

- **Preemptive Round-Robin Scheduling**: Time-sliced task execution with configurable time quantum (50ms default)
- **Deterministic Context Switching**: Assembly-optimized context save/restore using PendSV
- **Task State Management**: Full lifecycle support (COLD, READY, RUNNING, BLOCKED, KILLED, FINISHED)
- **Bounded Semaphores**: Counting semaphores with blocking feed/consume for producer-consumer patterns
- **Message Queues (Pipes)**: FIFO byte-stream IPC with blocking enqueue/dequeue, supports multi-byte messages
- **Active Sleep**: Cooperative sleep that allows other tasks to run
- **Blocking Sleep**: Busy-wait sleep for critical timing
- **Visual Debugging**: Terminal-based display with progress bars, message history, semaphore/pipe visualization
- **Stress Testing**: Built-in stress test suite with real-time verification
- **USB CDC Support**: Serial communication via USB Virtual COM Port
- **MPU Configuration**: Memory Protection Unit setup for QSPI flash access
- **MISRA C Compliant**: Follows MISRA C:2012 coding standards

---

## DO-178C Alignment

ICARUS OS is being developed to support DO-178C DAL C certification objectives. Complete certification-aligned documentation is available in `docs/do178c/`.

### Documentation Suite

| Category | Documents |
|----------|-----------|
| **Plans** | PSAC, SDP, SVP, SCMP, SQAP |
| **Requirements** | SRS (101 requirements: 41 implemented, 60 planned) |
| **Design** | SDD with full traceability matrix |
| **Verification** | Coverage analysis, deactivated code analysis, test traceability |

### Compliance Status

| Objective | Status |
|-----------|--------|
| Static analysis (cppcheck) | ✅ Complete |
| MISRA C:2012 subset | ✅ Complete |
| Unit testing (Unity) | ✅ 131 tests |
| Line coverage | ✅ 88.5% |
| Function coverage | ✅ 85.9% |
| Requirements traceability | ✅ SRS complete |
| Design traceability | ✅ SDD complete |
| MC/DC coverage | 🔄 In progress |
| PC-lint Plus (full MISRA) | 📋 Planned |
| Formal verification | 📋 Planned (DAL B) |

### Running Verification

```bash
# Run all tests
cd tests && make clean test

# Generate coverage report
cd tests && make COVERAGE=yes clean test coverage-html

# Run static analysis
cd build && make cppcheck
```

---
