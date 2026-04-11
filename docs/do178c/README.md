# DO-178C Documentation

This folder contains software lifecycle documentation organized according to DO-178C/ED-12C standards for airborne software certification.

## Document Structure

```
docs/do178c/
├── README.md                           # This file
├── plans/                              # Software Planning Documents
│   ├── PSAC.md                         # Plan for Software Aspects of Certification
│   ├── SDP.md                          # Software Development Plan
│   ├── SVP.md                          # Software Verification Plan
│   ├── SCMP.md                         # Software Configuration Management Plan
│   └── SQAP.md                         # Software Quality Assurance Plan
├── requirements/                       # Software Requirements
│   └── SRS.md                          # Software Requirements Specification
├── design/                             # Software Design Documents
│   └── SDD.md                          # Software Design Description
├── verification/                       # Verification Documents
│   ├── coverage_analysis.md            # Structural Coverage Analysis
│   ├── deactivated_code.md             # Deactivated Code Analysis
│   └── test_traceability.md            # Requirements-to-Test Traceability
├── configuration/                      # Configuration Management
│   └── (future: SCI documents)
└── quality/                            # Quality Assurance
    └── (future: audit records)
```

## DO-178C Objectives Addressed

| Objective | Document | Status |
|-----------|----------|--------|
| A-1: Software Planning | `plans/PSAC.md` | ✅ Draft |
| A-1: Software Planning | `plans/SDP.md` | ✅ Draft |
| A-1: Software Planning | `plans/SVP.md` | ✅ Draft |
| A-1: Software Planning | `plans/SCMP.md` | ✅ Draft |
| A-1: Software Planning | `plans/SQAP.md` | ✅ Draft |
| A-3: Requirements | `requirements/SRS.md` | ✅ Updated with MPU requirements |
| A-4: Design | `design/SDD.md` | ✅ Updated with MPU architecture |
| A-7: Structural Coverage Analysis | `verification/coverage_analysis.md` | ✅ Initial |
| A-7: Deactivated Code Analysis | `verification/deactivated_code.md` | ✅ Initial |
| A-5: Test Traceability | `verification/test_traceability.md` | ✅ Initial |

## Recent updates (MPU protection)

The following memory protection features are implemented on the main development line and documented in the SRS/SDD:

### Memory protection features implemented

The following memory protection features have been implemented and documented:

1. **DTCM Protection (HLR-KRN-064, HLR-KRN-070)**
   - Kernel data structures isolated in privileged-only DTCM region
   - Prevents unprivileged tasks from accessing kernel state
   - Verified by `mpu_dtcm_attack_task` and `mpu_kernel_bypass_test`

2. **ITCM Protection (HLR-KRN-063, HLR-KRN-066)**
   - Kernel code in read-only ITCM region
   - Prevents code modification attacks
   - Verified by `mpu_itcm_write_test`

3. **Task Data Isolation (HLR-KRN-065, HLR-KRN-071)**
   - Each task has isolated 2KB data region
   - MPU reconfigured on context switch
   - Prevents cross-task memory corruption
   - Verified by `mpu_redteam_task`

4. **Privilege Separation (HLR-KRN-068, HLR-KRN-069)**
   - Tasks run in unprivileged mode
   - Kernel functions run in privileged mode
   - Enforced by ARM Cortex-M7 privilege levels

5. **SVC call gates (HLR-KRN-067)**
   - Kernel services are invoked from unprivileged code via SVC (57 SVC IDs, 0–56, see `svc.h`)
   - IDs 29–39 cover atomic DTCM read/write helpers (`sem_can_*`, `pipe_can_*`, etc.)
   - IDs 40–56 are the v0.3.0 shared service module gates (cdc_rx, event, tables — see SDD §3.10)
   - Controlled transition between privilege levels

6. **Fault Handling (HLR-KRN-081, HLR-KRN-082, HLR-KRN-083)**
   - Graceful recovery from data access violations
   - Fault address and PC captured for debugging
   - System remains stable after faults

### Documentation Updates

- **SRS.md**: Added 14 memory protection requirements (HLR-KRN-063 through HLR-KRN-086) plus 16 shared-module requirements (HLR-KRN-090 through HLR-KRN-094.3)
- **SDD.md**: Added comprehensive MPU Protection Architecture section (4.2) and shared service modules section (3.10)
- **SVP.md**: Added Memory Protection Tests section (4.5) with verification procedures; the v0.3.0 shared modules add 56 new host-side unit tests across `test_crc.c`, `test_cdc_rx.c`, `test_event.c`, `test_fs.c`, `test_tables.c`

## Recent updates (v0.3.0 shared service modules)

The v0.3.0 release adds five reusable kernel modules with the same
MPU-aware ITCM/DTCM_PRIV/SVC-gate hardening pattern used by
semaphore.c and pipe.c:

| Module | SVC range | Backing storage | Tests |
|---|---|---|---|
| `icarus/cdc_rx.h` (USB CDC RX ring) | 40–42 | DTCM_PRIV | 7 |
| `icarus/event.h` (event ring + squelch) | 43–48 | DTCM_PRIV | 9 |
| `icarus/crc.h` (CRC16-CCITT, HW peripheral) | n/a | n/a (pure fn) | 8 |
| `icarus/fs.h` (flat-file FS, 32 KB) | n/a | regular SRAM | 16 |
| `icarus/tables.h` (ground-loadable tables) | 49–56 | DTCM_PRIV | 16 |

See `design/SDD.md` §3.10 for the per-module design and
`requirements/SRS.md` §3.1.8 for the requirement set.

## Design Assurance Level (DAL)

Target DAL: **DAL C (Major)**

Current documentation supports:
- DAL D: ✅ Ready
- DAL C: ✅ Documentation complete, verification in progress
- DAL B: ⚠️ Needs decision coverage analysis
- DAL A: ❌ Needs MC/DC coverage analysis

## Version History

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 0.1 | 2025-01-26 | Souham Biswas | Initial documentation structure |
