---
inclusion: always
---

# ICARUS OS — MPU Architecture & Learnings

## Current Region Map (Step 6)

| # | Name | Base | Size | Access | Notes |
|---|------|------|------|--------|-------|
| 0 | ITCM_BASE | 0x00000000 | 64KB | PRIV_RO_URO + exec | Base covering all ITCM |
| 1 | QSPI_FLASH | 0x90000000 | 8MB | PRIV_RO_URO + exec | Cacheable |
| 2 | FLASH | 0x08000000 | 128KB | PRIV_RO_URO + exec | Internal flash |
| 3 | ITCM_PRIV | 0x00000000 | 1KB | PRIV_RO + exec | Overlay, SubRegionDisable=0xFC |
| 4 | TASK_DATA | dynamic | 2KB | FULL_ACCESS | Set per context switch |
| 5 | DTCM | 0x20000000 | 128KB | FULL_ACCESS | Unprotected — see Step 7 |
| 6 | RAM_D1 | 0x24000000 | 512KB | FULL_ACCESS | Stacks, heap, data pools |
| 7 | PERIPH | 0x40000000 | 512MB | FULL_ACCESS, Device | GPIO, USB, SPI, etc. |

Enabled with `MPU_PRIVILEGED_DEFAULT` — blocks unprivileged access to SCS (NVIC, SCB, SysTick, MPU registers).

## ITCM Layout

```
0x00000000  _sitcm_priv   SVC_Handler, PendSV_Handler, __os_yield,
                          __task_active_sleep, __os_get_tick_count
0x00000098  _eitcm_code   (actual code ends here, 152 bytes)
0x00000098–0x000003FF     linker padding (872 bytes, zeros)
0x00000400  _sitcm_user   SysTick_Handler, __pipe_enqueue/dequeue,
                          __semaphore_feed/consume, os_yield_pendsv,
                          start_cold_task, veneers
0x00000778  _eitcm_user   (888 bytes used)
0x00000778–0x0000FFFF     ~62KB free for future ITCM_FUNC code
```

Total ITCM used: ~1.9KB of 64KB.

## Region 3 Subregion Disable (Critical Detail)

Region 3 uses `SubRegionDisable = 0xFC` (binary `11111100`):
- Subregions 0-1 active → covers `0x000–0x1FF` as PRIV_RO (protects actual kernel code)
- Subregions 2-7 disabled → `0x200–0x3FF` falls back to region 0 (PRIV_RO_URO)

**Why this is necessary:** C library code (`iprintf`, `strncmp`, newlib internals) performs
data reads from the ITCM padding area (`~0x210`) while running unprivileged. Without the
subregion disable, the full 1KB overlay blocks these reads and causes a MemManage fault
(DACCVIOL, MMFAR valid, PC in flash).

If priv code ever grows past 512 bytes (subregions 0-1), bump the overlay to 2KB and
update the linker script padding from `ALIGN(1024)` to `ALIGN(2048)`.

## MPU Hardware Rules (ARMv7-M)

- Region size must be a power of 2, minimum 32 bytes
- Base address must be a multiple of region size (natural alignment)
- Higher-numbered regions override lower-numbered ones for overlapping addresses
- Disabled subregions fall back to the next lower-priority matching region
- Each subregion = region_size / 8; `SubRegionDisable` bit N disables subregion N
- `DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE` sets the XN (Execute Never) bit
- **XN bug to avoid:** when moving a region to a different number, always explicitly
  set `DisableExec` — it does NOT inherit from the previous config of that region slot

## What's Protected vs. Not

**Protected:**
- ITCM `0x000–0x1FF`: unprivileged code cannot read, write, or execute kernel handlers
- Flash / QSPI / ITCM_USER: read+execute only, no writes from any mode
- SCS (`0xE000E000`): unprivileged code cannot touch NVIC, SCB, SysTick, MPU registers

**NOT protected (Step 7 work):**
- DTCM: full access — tasks can corrupt `task_list`, `semaphore_list`, `scheduler_enabled`, `os_tick_count`
- RAM_D1: full access — tasks can read/write each other's stacks and data pools
- Peripherals: full access — tasks can directly drive GPIO, USB, SPI without kernel

## Step 7 Plan (DTCM Protection)

Blocker: spinning functions read kernel data directly from unprivileged thread mode:
- `__semaphore_feed/consume` → `semaphore_list[idx]->count/max_count/engaged`
- `__pipe_enqueue/dequeue` → `message_pipe_list[idx]->...`
- `__task_blocking_sleep/__task_busy_wait` → `os_tick_count`

These cannot be moved into the SVC handler (PendSV deadlock). The solution is a
kernel call gate: each spinning iteration issues an SVC to atomically read/update
a single field, returns to thread mode, spins, repeats. Once done, DTCM can be
split: `.dtcm_priv` → PRIV_RO, `.dtcm` → PRIV_RO_URO for task-accessible data.

## Diagnostic MemManage Handler Pattern

When debugging MPU faults, the MemManage handler can be extended to blink:
1. Fault type: IACCVIOL (instruction) vs DACCVIOL (data) from `SCB->CFSR & 0x1`
2. MMFAR validity: `SCB->CFSR & 0x80` — if set, `SCB->MMFAR` holds the faulting address
3. PC range: extracted from stacked frame `sp[6]` (use MSP or PSP based on `LR & 4`)
4. MMFAR range: which memory region was accessed

Key insight: the stacked PC is the instruction that *caused* the fault, not the handler PC.
Use `tst lr, #4 / ite eq / mrseq r0, msp / mrsne r0, psp` to get the right stack.
