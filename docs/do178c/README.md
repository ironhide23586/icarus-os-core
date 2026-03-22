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

## Recent Updates (MPU Protection - Branch: feature/mpu-protection)

### Memory Protection Features Implemented

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

5. **SVC Call Gates (HLR-KRN-067)**
   - All kernel services accessed via SVC instructions
   - 11 SVC call gates implemented (SVCs 29-39)
   - Controlled transition between privilege levels

6. **Fault Handling (HLR-KRN-081, HLR-KRN-082, HLR-KRN-083)**
   - Graceful recovery from data access violations
   - Fault address and PC captured for debugging
   - System remains stable after faults

### Documentation Updates

- **SRS.md**: Added 14 new memory protection requirements (HLR-KRN-063 through HLR-KRN-086)
- **SDD.md**: Added comprehensive MPU Protection Architecture section (4.2)
- **SVP.md**: Added Memory Protection Tests section (4.5) with verification procedures

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
