# DO-178C Documentation

This folder contains software lifecycle documentation organized according to DO-178C/ED-12C standards for airborne software certification.

## Document Structure

```
docs/do178c/
├── README.md                           # This file
├── plans/                              # Software Planning Documents
│   └── PSAC.md                         # Plan for Software Aspects of Certification
├── standards/                          # Software Standards
│   └── (future: coding standards, etc.)
├── requirements/                       # Software Requirements
│   └── (future: HLR, LLR documents)
├── design/                             # Software Design Documents
│   └── (future: architecture, etc.)
├── verification/                       # Verification Documents
│   ├── coverage_analysis.md            # Structural Coverage Analysis
│   ├── deactivated_code.md             # Deactivated Code Analysis
│   └── test_traceability.md            # Requirements-to-Test Traceability
├── configuration/                      # Configuration Management
│   └── (future: CI/SCI documents)
└── quality/                            # Quality Assurance
    └── (future: QA records)
```

## DO-178C Objectives Addressed

| Objective | Document | Status |
|-----------|----------|--------|
| A-1: Software Planning | `plans/PSAC.md` | ✅ Draft |
| A-7: Structural Coverage Analysis | `verification/coverage_analysis.md` | ✅ Initial |
| A-7: Deactivated Code Analysis | `verification/deactivated_code.md` | ✅ Initial |
| A-5: Test Traceability | `verification/test_traceability.md` | ✅ Initial |

## Design Assurance Level (DAL)

Target DAL: **To Be Determined**

Current test coverage supports:
- DAL D: ✅ Ready (no structural coverage required)
- DAL C: ⚠️ Partial (statement coverage ~79%, needs justification)
- DAL B: ❌ Not ready (needs 100% statement + decision coverage)
- DAL A: ❌ Not ready (needs MC/DC coverage)

## Version History

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 0.1 | 2026-01-26 | Kiro | Initial documentation structure |
