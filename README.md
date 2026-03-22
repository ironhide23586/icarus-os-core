<p align="center">
  <img src="logo_landscape.jpeg" alt="ICARUS OS Banner" width="100%">
</p>

# ICARUS OS Core

**Intelligent Certifiable Autonomous Real-time Unified System**

[![Documentation](https://img.shields.io/badge/docs-GitHub%20Pages-blue)](https://ironhide23586.github.io/icarus-os-core/)
[![Sponsor](https://img.shields.io/badge/sponsor-GitHub%20Sponsors-ea4aaa)](https://github.com/sponsors/ironhide23586)
[![Buy Me A Coffee](https://img.shields.io/badge/buy%20me%20a%20coffee-donate-yellow)](https://buymeacoffee.com/souham)

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

## Quick Start

### Prerequisites

- **ARM GCC Toolchain**: arm-none-eabi-gcc 13.3+ or 14.3+
- **Build Tools**: make, bash
- **Hardware**: STM32H750VBT6 development board
- **Debugger**: ST-Link or compatible
- **Terminal**: Any serial terminal (screen, minicom, PuTTY) or USB CDC viewer

### Building the Firmware

The project uses a unified build script that handles both firmware compilation and test execution:

```bash
# Clean build (recommended)
bash build/rebuild.sh

# Incremental build
bash build/build.sh

# Build output location
ls build/icarus_os.elf
ls build/icarus_os.hex
ls build/icarus_os.map
```

**Build artifacts:**
- `icarus_os.elf` - Executable with debug symbols (2.2 MB)
- `icarus_os.hex` - Flash programming file (147 KB)
- `icarus_os.map` - Memory map and symbol table

### Memory Layout

ICARUS OS uses optimized memory placement for maximum performance and security:

```
┌─────────────────────────────────────────┐
│ ITCM (0x00000000) - 64 KB               │
│   Critical kernel code (zero wait)      │
│   - Context switch (PendSV_Handler)     │
│   - Scheduler (os_yield)                │
│   - IPC (semaphores, pipes)             │
│   - SVC dispatcher                      │
│   Protection: Read-only for all (MPU)   │
│   Used: 7.5 KB (12%) | Free: 56.5 KB    │
├─────────────────────────────────────────┤
│ DTCM (0x20000000) - 128 KB              │
│   Fast data access (zero wait)          │
│   - Kernel data structures              │
│   - Task lists, semaphores, pipes       │
│   - Tick counter, scheduler state       │
│   Protection: Privileged-only (MPU)     │
│   Used: 8 KB (6%) | Free: 120 KB        │
├─────────────────────────────────────────┤
│ RAM_D1 (0x24000000) - 512 KB            │
│   Task stacks and shared buffers        │
│   - 128 task stacks (256 words each)    │
│   - Display buffers                     │
│   Protection: Full access               │
│   Used: 128 KB (25%) | Free: 384 KB     │
├─────────────────────────────────────────┤
│ RAM_D2 (0x30000000) - 288 KB            │
│   Task data regions (2KB-aligned)       │
│   - Per-task isolated 2KB regions       │
│   Protection: Dynamic MPU (Region 4)    │
│   Used: 256 KB (89%) | Free: 32 KB      │
├─────────────────────────────────────────┤
│ Flash (0x08000000) - 128 KB             │
│   Program code and constants            │
│   Protection: Read-only (MPU)           │
│   Used: ~50 KB (39%)                    │
├─────────────────────────────────────────┤
│ QSPI (0x90000000) - 8 MB                │
│   External flash (future use)           │
│   Protection: Read-only (MPU)           │
└─────────────────────────────────────────┘
```

**MPU Region Configuration:**

| Region | Base | Size | Access | Purpose |
|--------|------|------|--------|---------|
| 0 | 0x00000000 | 64KB | Priv+User RO+Exec | ITCM code protection |
| 1 | 0x90000000 | 8MB | Priv+User RO+Exec | QSPI Flash |
| 2 | 0x08000000 | 128KB | Priv+User RO+Exec | Internal Flash |
| 3 | DISABLED | - | - | Reserved |
| 4 | Dynamic | 2KB | Priv+User RW | Task data isolation |
| 5 | 0x20000000 | 128KB | Priv RW only | DTCM kernel data |
| 6 | 0x24000000 | 512KB | Full Access | RAM_D1 stacks |
| 7 | 0x40000000 | 512MB | Full Access | Peripherals |

### Flashing the Firmware

**Using ST-Link:**
```bash
# Flash the hex file
st-flash --format ihex write build/icarus_os.hex

# Or flash the binary
st-flash write build/icarus_os.bin 0x08000000
```

**Using OpenOCD:**
```bash
openocd -f interface/stlink.cfg -f target/stm32h7x.cfg \
  -c "program build/icarus_os.elf verify reset exit"
```

**Using STM32CubeProgrammer:**
```bash
STM32_Programmer_CLI -c port=SWD -w build/icarus_os.hex -v -rst
```

### Connecting to the Terminal

ICARUS OS outputs to USB CDC (Virtual COM Port). Connect using any serial terminal:

**Using screen (macOS/Linux):**
```bash
# Find the device
ls /dev/tty.usbmodem*  # macOS
ls /dev/ttyACM*        # Linux

# Connect (115200 baud, 8N1)
screen /dev/tty.usbmodem14203 115200

# Exit: Ctrl+A, then K, then Y
```

**Using minicom (Linux):**
```bash
# Configure once
sudo minicom -s
# Set: Serial Device = /dev/ttyACM0, Baud = 115200

# Connect
sudo minicom
```

**Using PuTTY (Windows):**
1. Open PuTTY
2. Connection type: Serial
3. Serial line: COM3 (check Device Manager)
4. Speed: 115200
5. Click "Open"

**Using the provided script (macOS):**
```bash
# Auto-detect and connect
bash icarus_terminal.sh
```

### What You'll See

Upon successful connection, you'll see the ICARUS OS terminal GUI with:
- ASCII art header
- Heartbeat banner (flashing with LED)
- Demo task progress bars
- Semaphore/pipe visualizations
- Message history panels
- Stress test statistics (if enabled)

**Example output:**
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

[>ICARUS_HEARTBEAT<] ★★★★★★★★★★★★★★★★★★★★ [>ICARUS_HEARTBEAT<]
[producer] ████████████────────  160/200 ticks  →[42]
[consumer] ██████████──────────  100/190 ticks  ←[42]
```

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

#### Core Kernel
- **Preemptive Round-Robin Scheduling**: Time-sliced task execution with configurable time quantum (50ms default)
- **Deterministic Context Switching**: Assembly-optimized context save/restore using PendSV
- **Task State Management**: Full lifecycle support (COLD, READY, RUNNING, BLOCKED, KILLED, FINISHED)
- **Bounded Semaphores**: Counting semaphores with blocking feed/consume for producer-consumer patterns
- **Message Queues (Pipes)**: FIFO byte-stream IPC with blocking enqueue/dequeue, supports multi-byte messages
- **Active Sleep**: Cooperative sleep that allows other tasks to run
- **Blocking Sleep**: Busy-wait sleep for critical timing

#### Memory Protection (NEW in v0.1.0)
- **Hardware-Enforced Isolation**: ARM Cortex-M7 MPU with 8-region configuration
- **DTCM Protection**: Kernel data isolated in privileged-only memory (Region 5)
- **ITCM Protection**: Kernel code marked read-only to prevent modification attacks (Region 0)
- **Task Data Isolation**: Each task gets isolated 2KB data region with MPU reconfiguration on context switch (Region 4)
- **Privilege Separation**: Tasks run unprivileged (CONTROL.nPRIV=1), kernel runs privileged via SVC mechanism
- **SVC Call Gates**: 39 SVC numbers for controlled privilege transitions with atomic kernel state access
- **Fault Recovery**: Graceful handling of MemManage faults with fault address/PC capture
- **Attack Validation**: 5 red team test tasks validate 100% protection effectiveness

#### Development & Debugging
- **Visual Debugging**: Terminal-based display with progress bars, message history, semaphore/pipe visualization
- **Stress Testing**: Built-in stress test suite with real-time verification (19 tasks, 10+ min stability)
- **USB CDC Support**: Serial communication via USB Virtual COM Port
- **MISRA C Compliant**: Follows MISRA C:2012 coding standards

---

## Memory Protection Architecture

ICARUS OS implements comprehensive hardware-enforced memory protection using the ARM Cortex-M7 Memory Protection Unit (MPU). This provides three layers of isolation essential for safety-critical applications.

### Protection Layers

#### 1. DTCM Protection (Kernel Data Isolation)
**Problem:** Unprivileged tasks could corrupt kernel state (task lists, semaphores, pipes, tick counter).

**Solution:** Kernel data structures isolated in privileged-only DTCM (Region 5: PRIV_RW).

**Implementation:**
- All kernel data marked with `DTCM_DATA_PRIV` attribute
- Unprivileged tasks cannot directly read/write DTCM
- All kernel data access forced through SVC call gates

**Verification:** `mpu_dtcm_attack_task` attempts DTCM read → MemManage fault (protection working)

#### 2. ITCM Protection (Code Integrity)
**Problem:** Malicious code could modify kernel handlers at runtime.

**Solution:** Kernel code marked read-only in ITCM (Region 0: RO for all).

**Implementation:**
- All kernel code placed in ITCM section
- MPU Region 0 configured as read-only + executable
- Write attempts trigger MemManage fault

**Verification:** `mpu_itcm_write_test` attempts ITCM write → MemManage fault (protection working)

#### 3. Task Data Isolation (Cross-Task Protection)
**Problem:** One task could corrupt another task's data, causing cascading failures.

**Solution:** Each task gets isolated 2KB data region with MPU reconfiguration on context switch.

**Implementation:**
- Task data regions allocated from RAM_D2 (2KB-aligned)
- MPU Region 4 reconfigured in PendSV handler
- Memory barriers (DSB/ISB) ensure MPU changes take effect

**Verification:** `mpu_redteam_task` attempts cross-task access → MemManage fault (protection working)

### SVC Call Gate Architecture

**Challenge:** Once DTCM is privileged-only, spinning functions (semaphore feed/consume, pipe enqueue/dequeue) cannot read kernel state from unprivileged mode.

**Solution:** SVC call gates that read/write kernel state atomically in privileged mode.

**39 SVC Numbers Defined:**

| SVC Range | Purpose | Examples |
|-----------|---------|----------|
| 0-15 | Core kernel operations | os_init, os_start, os_yield, task_sleep |
| 16-28 | IPC operations | semaphore_init, pipe_init, task lifecycle |
| 29-36 | Call gates (spinning) | sem_can_feed, pipe_can_enqueue, sem_increment |
| 37-39 | Metadata gates | os_get_tick_count, os_get_task_name |

**Spinning Pattern with Call Gates:**
```c
bool semaphore_feed(uint8_t semaphore_idx) {
    while (!sem_can_feed(semaphore_idx)) {  // SVC call gate (read DTCM)
        task_active_sleep(1);
    }
    sem_increment(semaphore_idx);  // SVC write gate (write DTCM)
    return true;
}
```

### Fault Handling

**MemManage Handler Features:**
- Captures fault address (MMFAR register)
- Captures fault PC (exception stack frame)
- Fault counter with limit (1000 faults before halt)
- Instruction skip recovery (+2 bytes for Thumb-2)
- Fault blink pattern for debugging (2 rapid blinks)

**Fault Statistics (10 min stress test with attack tasks):**
- Total faults: 240 (all intentional from attack tests)
- Fault recovery: 100% successful
- System uptime: 100%

### Performance Impact

| Metric | Before MPU | After MPU | Overhead |
|--------|------------|-----------|----------|
| Context switch | ~8μs | ~10μs | +2μs (+25%) |
| SVC call | N/A | ~0.8μs | N/A |
| Code size | 32KB | 39.5KB | +7.5KB (+23%) |
| Data size | 512KB | 768KB | +256KB (+50%) |

**Note:** Context switch overhead is negligible for 50ms time quantum (0.004% overhead).

### Attack Test Results

| Test | Attack Type | Expected | Result |
|------|-------------|----------|--------|
| `mpu_dtcm_attack_task` | Read kernel data | MemManage fault | ✅ Fault caught |
| `mpu_itcm_write_test` | Modify kernel code | MemManage fault | ✅ Fault caught |
| `mpu_redteam_task` | Cross-task access | MemManage fault | ✅ Fault caught |
| `mpu_kernel_bypass_test` | Direct kernel call | MemManage fault | ✅ Fault caught |
| `mpu_verify_task` | Data integrity | No corruption | ✅ 0 errors |

**Protection Effectiveness:** 100% (all attacks caught, system stable)

---

## DO-178C Alignment

ICARUS OS is being developed to support DO-178C DAL C certification objectives. Complete certification-aligned documentation is available in `docs/do178c/`.

### Documentation Suite

| Category | Documents |
|----------|-----------|
| **Plans** | PSAC, SDP, SVP, SCMP, SQAP |
| **Requirements** | SRS (71 requirements: 41 implemented, 30 planned) |
| **Design** | SDD with full traceability matrix |
| **Verification** | Coverage analysis, deactivated code analysis, test traceability |

**Memory Protection Requirements (HLR-KRN-063 to HLR-KRN-086):**
- 14 new requirements for MPU-based isolation
- All requirements implemented and verified
- Attack tests validate 100% protection effectiveness

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

## Support ICARUS OS

Building a certifiable RTOS from scratch is a massive undertaking. ICARUS OS is developed in the open because safety-critical software should be transparent, auditable, and accessible to everyone building systems where failure is not an option.

Your sponsorship directly funds:
- 🔬 **Formal verification tooling** for DAL A/B certification paths
- 📋 **Full MISRA C:2012 compliance** with commercial static analyzers
- 🧪 **MC/DC coverage infrastructure** for complete decision coverage
- 📚 **Expanded platform support** beyond STM32H7
- ⚡ **Performance optimization** and determinism guarantees

If ICARUS OS helps your project, or you believe in open-source safety-critical software, consider supporting its development:

<p align="center">
  <a href="https://github.com/sponsors/ironhide23586">
    <img src="https://img.shields.io/badge/Sponsor%20on-GitHub-ea4aaa?style=for-the-badge&logo=github" alt="Sponsor on GitHub">
  </a>
  &nbsp;&nbsp;
  <a href="https://buymeacoffee.com/souham">
    <img src="https://img.shields.io/badge/Buy%20Me%20A-Coffee-ffdd00?style=for-the-badge&logo=buy-me-a-coffee&logoColor=black" alt="Buy Me A Coffee">
  </a>
</p>

---
