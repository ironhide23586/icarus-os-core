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
| A-3: Requirements | `requirements/SRS.md` | ✅ Draft |
| A-4: Design | `design/SDD.md` | ✅ Draft |
| A-7: Structural Coverage Analysis | `verification/coverage_analysis.md` | ✅ Initial |
| A-7: Deactivated Code Analysis | `verification/deactivated_code.md` | ✅ Initial |
| A-5: Test Traceability | `verification/test_traceability.md` | ✅ Initial |

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
