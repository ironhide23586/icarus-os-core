# Changelog

All notable changes to ICARUS OS are documented in this file.
Format follows [Keep a Changelog](https://keepachangelog.com/).

## [0.4.0] - 2026-06-15

### Added

- **Software Bus** (`icarus/sb.h` / `sb.c`) — lightweight pub/sub message
  router built on top of kernel pipes. 32 routes × 4 subscribers per message
  ID. Best-effort delivery: if a subscriber's pipe is full the message is
  silently dropped for that subscriber. Route table in `DTCM_DATA_PRIV`,
  hot-path functions in `ITCM_FUNC`. API: `sb_init`, `sb_subscribe`,
  `sb_unsubscribe`, `sb_publish`, `sb_subscriber_count`, `sb_route_count`.
- **Background Checksum integrity monitor** (`icarus/cs.h` / `cs.c`) —
  periodic CRC16-CCITT scanner over up to 8 registered memory regions.
  Baselines captured at registration time; mismatches reported through a
  user-supplied callback. Hardware CRC self-test on init (expected 0x29B1).
  Region table in `DTCM_DATA_PRIV`, functions in `ITCM_FUNC`. API:
  `cs_init`, `cs_set_callback`, `cs_add_region`, `cs_enable`,
  `cs_rebaseline`, `cs_check_all`, `cs_get_region`, `cs_region_count`.
- **`os_restart_task(task_index)`** — cold-restart a killed or finished task
  in-place from its original entry point. No new stack slot is allocated;
  the task re-enters the scheduler as `TASK_STATE_COLD`. SVC 57.
- **`semaphore_consume_timeout(idx, max_ticks)`** — timed semaphore wait.
  Returns `false` if the semaphore is not acquired within `max_ticks`
  (0 = non-blocking try). SVC 58.
- **`os_get_task_state(task_index)`** — SVC-gated query (SVC 59) returning
  the current `icarus_task_state_t` for any task index.
- **`os_get_task_dispatch_count(task_index)`** — per-task scheduling counter
  (SVC 60). Incremented each time the scheduler selects the task.
- **`os_get_stack_watermark(task_index)`** / **`os_update_stack_watermark(task_index)`**
  — stack high-water mark tracking using a `0xDEADC0DE` sentinel pattern.
  `os_update_stack_watermark` scans the stack and updates the TCB field;
  `os_get_stack_watermark` returns the cached minimum free words. SVC 61–62.
- **BSP: IWDG watchdog** (`bsp/iwdg.h` / `iwdg.c`) — thin wrapper around
  the STM32H7 Independent Watchdog (IWDG1). `IWDG_Init`, `IWDG_Refresh`,
  `IWDG_WasReset`, `IWDG_ClearResetFlag`.
- **BSP: K1 user button** (`bsp/button.h` / `button.c`) — `Button_IsPressed()`
  raw read of the K1 button on PC13 (active low).
- **BSP: CDC raw write helper** (`bsp/cdc.h` / `cdc.c`) — `CDC_Write` and
  `CDC_WriteString` for pushing raw bytes to the USB CDC IN endpoint with
  busy-retry via `task_active_sleep`.
- **6 new SVC numbers (57–62)** — `SVC_OS_RESTART_TASK` (57),
  `SVC_SEMAPHORE_CONSUME_TIMEOUT` (58), `SVC_GET_TASK_STATE` (59),
  `SVC_GET_TASK_DISPATCH_COUNT` (60), `SVC_GET_STACK_WATERMARK` (61),
  `SVC_UPDATE_STACK_WATERMARK` (62). All routed through `SVC_Handler_C`.

### Changed

- **Pipe capacity widened** — `ICARUS_MAX_MESSAGE_BYTES` increased from 128
  to 512 bytes; pipe internal counters widened from `uint8_t` to `uint16_t`
  to support the larger buffers. Enables Software Bus and CFDP PDU routing.
- **`dispatch_count` field added to TCB** — tracks how many times each task
  has been scheduled. Useful for load-balance diagnostics.
- **`stack_watermark` field added to TCB** — minimum free stack words
  observed, updated lazily via `os_update_stack_watermark()`. Stack is
  painted with `0xDEADC0DE` sentinel at creation time.
- **SVC count grew 57 → 63** (IDs 0–62).
- **`icarus/icarus.h` umbrella header** now also exports `sb.h` and `cs.h`.

## [0.3.0] - 2026-04-11

### Added

- **CDC RX ring buffer** (`icarus/cdc_rx.h` / `cdc_rx.c`) — generic SPSC USB
  CDC receive ring buffer (512 B). Producer is the USB CDC ISR; consumer is
  any RTOS task. Backing store in `DTCM_DATA_PRIV`, hot path in `ITCM_FUNC`,
  thread-mode reads through SVC gates.
- **Generic event ring buffer + per-module severity squelch** (`icarus/event.h`
  / `event.c`) — 32-slot ring of compact 16-byte event entries with a 16-entry
  squelch table. Transport-agnostic: drains into a caller-provided buffer
  via `event_drain()`. Emission via `os_event(module_id, severity, event_id,
  payload, len)`. Silent (no logging dependency).
- **CRC16-CCITT helper** (`icarus/crc.h` / `crc.c`) — `crc16_ccitt(data, len)`
  with poly 0x1021, init 0xFFFF. **Hardware-accelerated** on STM32H7 via the
  on-chip CRC peripheral (AHB4 bus, configurable polynomial), lazy-initialised
  on first call. Approximately 4× faster than the bytewise software loop on
  the buffer sizes used by downstream consumers. Portable bytewise fallback
  under `HOST_TEST`.
- **Internal RAM-backed flat-file filesystem** (`icarus/fs.h` / `fs.c`) — 16
  files × 2 KB = 32 KB total, with create/open/write/read/delete/list/stats.
  On-disk format opaque so a real flash backend can be substituted later
  without changing the public API.
- **Generic ground-loadable table engine** (`icarus/tables.h` / `tables.c`)
  — registry of up to 8 tables, each with a staging buffer and an active
  buffer (double-buffered swap). Tables validated by schema CRC and CRC16
  data checksum, then committed via a registered activate callback.
  `tbl_activate` is split across two SVCs (`prepare` + `commit`) so the
  user activate callback runs in unprivileged thread mode against a stack
  scratch copy — never sees DTCM_PRIV.
- **17 new SVC numbers (40–56)** — `SVC_CDC_RX_*` (40–42), `SVC_EVENT_*`
  (43–48), `SVC_TBL_*` (49–56). All routed through `SVC_Handler_C`. Each
  module's public API in `svc.c` follows the existing wrapper pattern (asm
  `svc %imm` on target, direct `__` call under `HOST_TEST`).
- **Showcase demo** (`Core/Src/showcase_tasks.c`, `Core/Inc/showcase_tasks.h`)
  — three sleep-and-poll tasks (`sensor_task`, `ground_task`, `shell_task`)
  that drive every public entry point of the new modules together. Gated
  behind `ENABLE_SHOWCASE` in `main.c` (default off so the existing game
  demo stays primary). When enabled the firmware grows ~2.5 KB text + 33 KB
  bss for the three task stacks.

### Changed

- **`Core/Inc/icarus/icarus.h` umbrella header** now also exports
  `cdc_rx.h`, `event.h`, `crc.h`, `fs.h`, and `tables.h`, so a single
  `#include "icarus/icarus.h"` makes the new modules available alongside
  kernel/scheduler/task/semaphore/pipe.
- **Kernel host test suite** (`tests/Makefile`) gained the new module
  sources (`cdc_rx.c`, `event.c`, `crc.c`, `fs.c`, `tables.c`). The host
  build now links the entire kernel surface — previously the new modules
  were not in `KERNEL_SRC` so `make -C tests` would fail with undefined
  `__cdc_rx_*` / `__os_event` / `__tbl_*` references when the application consumer
  reached for them.
- **`__os_init`**: only one system task is registered now (`ICARUS_KEEPALIVE_TASK`).
  The heartbeat task line is commented out at `Core/Src/icarus/kernel.c:244`
  for the interactive demo. Test `test_os_init` was updated to match.

### Fixed

- **`test_os_init` host test** was asserting `num_created_tasks == 2` and
  probing `task_list[1] / ">ICARUS_HEARTBEAT<"`, causing `make -C tests` to
  report 140/141 passing on every dev rebuild. Heartbeat task has been
  disabled since 0.2.0; test now expects 1 system task.
- **`-Werror=bad-function-cast` warnings in `svc.c`** — eight new SVC
  dispatch arms (cdc_rx_read_byte, event_get_squelch, event_drain, all
  five tbl_*) cast function-call results directly to `uint32_t`, which the
  standalone kernel build's stricter warnings caught. Each cast now goes
  via an intermediate variable.

### Hardening

- **MPU compatibility for new modules** — `cdc_rx`, `event`, and `tables`
  follow the kernel's existing `semaphore` / `pipe` pattern: backing data
  in `DTCM_DATA_PRIV`, hot functions in `ITCM_FUNC`, public entry points
  split into `__`-prefixed privileged implementations + thread-mode
  wrappers that issue SVC instructions. Once the MPU is locked priv-only,
  unprivileged tasks reach these modules through the SVC gates exactly as
  they do for semaphores and pipes.
  - `cdc_rx`: 156 B ITCM / 520 B DTCM_PRIV
  - `event`:  336 B ITCM / 536 B DTCM_PRIV
  - `tables`: 796 B ITCM / 8.3 KB DTCM_PRIV
  - `crc`:    pure function, ITCM only
  - `fs`:     32 KB store stays in regular SRAM (won't fit DTCM); functions
    use the standard `enter_critical()` / `exit_critical()` pattern. The
    deliberate trade-off is documented in `fs.c`.
- **`cdc_rx_push` ISR fast path** — the public wrapper calls
  `__cdc_rx_push` directly without an SVC because the USB CDC ISR is
  already in privileged handler context, and issuing an SVC from a
  high-priority interrupt would either fault or cause a priority
  inversion. Mirrors the pattern used by `pipe_enqueue`.
- **`tbl_activate` priv↔thread split** — the registered activate callback
  is user code that may issue further SVCs, so it cannot run inside the
  SVC handler. The wrapper splits the operation across two SVCs:
  `__tbl_activate_prepare` (validate + copy staging into a thread-mode
  scratch buffer), the user callback runs in thread mode against the
  scratch copy, then `__tbl_activate_commit` (copy scratch into the active
  buffer). The active buffer never moves until the callback succeeds.
- **`os_event` 5-arg packing** — `module_id`, `severity`, `event_id`,
  `payload`, `payload_len` packs into three SVC registers as
  `R0 = (event_id << 16) | (severity << 8) | module_id`, `R1 = payload`,
  `R2 = payload_len`, staying inside the AAPCS R0–R3 budget without
  spilling to the caller's stack.

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
