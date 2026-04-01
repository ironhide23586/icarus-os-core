# Changelog

All notable changes to ICARUS OS are documented in this file.
Format follows [Keep a Changelog](https://keepachangelog.com/).

## [0.2.0] - 2026-04-01

### Added

- **MPU-based memory protection** — DTCM privileged-only region for kernel data
  structures, ITCM read-only + execute protection, per-task data isolation via
  dynamic MPU region reconfiguration on context switch.
- **SVC call-gate architecture** — 40 SVC IDs (0-39) providing unprivileged
  tasks safe access to privileged kernel services, including atomic DTCM
  read/write helpers (IDs 29-39).
- **ICARUS Runner game** — 4-task real-time obstacle-dodge demo showcasing
  preemptive scheduling, IPC pipes, and terminal GUI rendering.
- **Interactive button-LED demo** — hardware button input with LED feedback,
  selectable via compile-time flag.
- **Message pipes** — FIFO byte-stream IPC primitive (`pipe.c`) with blocking
  read/write and configurable buffer sizes.
- **Memory map visualizer** — runtime tool that prints the active linker-section
  layout over USB CDC.
- **Stress test suite** — MPU attack tests, multi-alloc verification, and
  red-team memory probes.
- **DO-178C documentation suite** — PSAC, SDP, SVP, SDD, SRS, SQAP, SCMP,
  coverage analysis, deactivated code analysis, and requirement traceability
  matrix.

### Changed

- Kernel sources moved from `kernel/` to `Core/Src/icarus/` (headers under
  `Core/Inc/icarus/`).
- Unit test suite expanded from 76 to 140 Unity tests.
- Host-based code coverage improved to ~91% line, ~89.5% function.
- Default terminal GUI changed from IPC dashboard to ICARUS Runner game
  (`ENABLE_GAME=1` in `main.c`).
- Copyright headers updated to 2025-2026.

### Fixed

- USB enumeration timing for reliable CDC connection on boot.
- Float handling in unprivileged mode for game physics.
- Task name buffer placement for DTCM protection compatibility.
- MemManage handler made recoverable for MPU fault testing.

## [0.1.0] - 2025-06-15

Initial release — preemptive RTOS kernel with round-robin scheduler,
semaphores, context switching, and basic terminal output on STM32H750VBT6.
