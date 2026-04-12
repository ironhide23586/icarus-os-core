---
name: BKPRAM no MPU region
description: RAM_D3 (0x38000000, BKPRAM) has no MPU region — unprivileged task writes will MemManage fault on target hardware
type: project
---

RAM_D3 at 0x38000000 (64KB BKPRAM) is used by persist.c for battery-backed state.
The MPU configuration in mpu.c has no region granting unprivileged access to this address range.
With MPU_PRIVILEGED_DEFAULT background map, unprivileged tasks cannot access it.

persist_save() is called from sched_tick() which runs in unprivileged mode_manager_task context.
This will trigger a MemManage fault on real STM32H750 hardware.

**Why:** persist_save_tick is scheduler slot 1, called from mode_manager_task (unprivileged).
**How to apply:** Either add MPU region for RAM_D3 with FULL_ACCESS, or gate persist_save through SVC.
