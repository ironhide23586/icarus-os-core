# Software Quality Assurance Plan (SQAP)

**Document ID:** ICARUS-SQAP-001  
**Version:** 0.1  
**Date:** 2025-01-26  
**Status:** Draft  
**Classification:** Public (Open Source)  

---

## Document Control

| Role | Name | Date | Signature |
|------|------|------|-----------|
| Author | | | |
| Reviewer | | | |
| Approver | | | |

---

## Revision History

| Version | Date | Author | Description |
|---------|------|--------|-------------|
| 0.1 | 2025-01-26 | Souham Biswas | Initial draft |

---

## 1. Introduction

### 1.1 Purpose

This Software Quality Assurance Plan (SQAP) defines the quality assurance activities for ICARUS OS development. It ensures that software life cycle processes comply with approved plans and standards per DO-178C.

### 1.2 Scope

QA activities cover:
- Process compliance monitoring
- Product quality verification
- Standards conformance
- Audit and review activities

### 1.3 QA Independence

Per DO-178C DAL C:
- QA personnel may participate in development
- QA reviews must be independent of the author
- QA has authority to escalate non-conformances

---

## 2. Quality Assurance Organization

### 2.1 Roles and Responsibilities

| Role | Responsibilities |
|------|------------------|
| QA Lead | Overall QA program, audits, escalation |
| QA Engineer | Reviews, checklists, compliance verification |
| Developer | Follow processes, address findings |
| Project Lead | Resource allocation, finding resolution |

### 2.2 Authority

QA has authority to:
- Stop release if critical non-conformances exist
- Escalate unresolved issues to project leadership
- Require re-work for non-compliant artifacts
- Access all project artifacts and records

---

## 3. Quality Assurance Activities

### 3.1 Activity Overview

```
┌─────────────────────────────────────────────────────────────┐
│                    QA Activities                             │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│  PROCESS ASSURANCE                                          │
│  ├── Plan compliance audits                                 │
│  ├── Process adherence reviews                              │
│  └── Standards conformance checks                           │
│                                                              │
│  PRODUCT ASSURANCE                                          │
│  ├── Requirements reviews                                   │
│  ├── Design reviews                                         │
│  ├── Code reviews                                           │
│  ├── Test reviews                                           │
│  └── Documentation reviews                                  │
│                                                              │
│  TRANSITION ASSURANCE                                       │
│  ├── Baseline audits                                        │
│  ├── Release readiness reviews                              │
│  └── Certification evidence review                          │
│                                                              │
└─────────────────────────────────────────────────────────────┘
```

### 3.2 Process Assurance

| Activity | Frequency | Method |
|----------|-----------|--------|
| Plan compliance | Per phase | Checklist audit |
| Coding standard compliance | Per PR | Automated + manual |
| CM process compliance | Weekly | Audit |
| Test process compliance | Per test cycle | Review |

### 3.3 Product Assurance

| Artifact | Review Type | Criteria |
|----------|-------------|----------|
| Requirements | Inspection | Complete, consistent, testable |
| Design | Walkthrough | Meets requirements, feasible |
| Code | Peer review | Standards, correctness, coverage |
| Tests | Inspection | Traces to requirements, adequate |
| Documentation | Review | Accurate, complete, current |


---

## 4. Reviews and Audits

### 4.1 Review Types

| Review | Purpose | Participants | Output |
|--------|---------|--------------|--------|
| **Requirements Review** | Verify completeness, consistency | QA, Dev Lead | Checklist, findings |
| **Design Review** | Verify design meets requirements | QA, Developers | Approval, action items |
| **Code Review** | Verify code quality, standards | Developers, QA | PR approval |
| **Test Review** | Verify test adequacy | QA, Verification | Test approval |
| **Release Review** | Verify release readiness | All stakeholders | Go/No-go decision |

### 4.2 Audit Types

| Audit | Purpose | Frequency |
|-------|---------|-----------|
| **Process Audit** | Verify process adherence | Quarterly |
| **Configuration Audit** | Verify CM compliance | Per release |
| **Physical Configuration Audit** | Verify build matches baseline | Per release |
| **Functional Configuration Audit** | Verify functionality matches requirements | Per release |

### 4.3 Review Checklists

#### 4.3.1 Requirements Review Checklist

- [ ] Each requirement has unique identifier
- [ ] Requirements are unambiguous
- [ ] Requirements are testable
- [ ] Requirements are traceable to system requirements
- [ ] No conflicts between requirements
- [ ] Performance requirements are quantified
- [ ] Safety requirements are identified

#### 4.3.2 Code Review Checklist

- [ ] Code follows naming conventions
- [ ] Code follows formatting standards
- [ ] No MISRA violations (or documented deviations)
- [ ] All functions documented
- [ ] Error handling complete
- [ ] No dead code
- [ ] No magic numbers
- [ ] Unit tests provided
- [ ] Coverage target met

#### 4.3.3 Test Review Checklist

- [ ] Test traces to requirement
- [ ] Test procedure is clear
- [ ] Expected results defined
- [ ] Pass/fail criteria explicit
- [ ] Test environment documented
- [ ] Test is repeatable

---

## 5. Standards and Procedures

### 5.1 Applicable Standards

| Standard | Application |
|----------|-------------|
| DO-178C | Software development process |
| MISRA C:2012 | Coding standard |
| IEEE 830 | Requirements documentation |
| Git Flow | Branching strategy |

### 5.2 Coding Standards Summary

Key rules enforced:

| Rule | Description | Enforcement |
|------|-------------|-------------|
| Naming | module_action_object | Review |
| Formatting | 4-space indent, K&R braces | Automated |
| Comments | Doxygen style | Review |
| Complexity | Max cyclomatic: 15 | Automated |
| MISRA | Mandatory + Required subset | Automated |

### 5.3 Documentation Standards

| Document | Format | Template |
|----------|--------|----------|
| Plans | Markdown | docs/do178c/plans/ |
| Requirements | Markdown | docs/do178c/requirements/ |
| Verification | Markdown | docs/do178c/verification/ |
| Code | Doxygen comments | In source |

---

## 6. Non-Conformance Management

### 6.1 Non-Conformance Categories

| Category | Severity | Response Time |
|----------|----------|---------------|
| **Critical** | Blocks release, safety impact | 24 hours |
| **Major** | Significant deviation | 1 week |
| **Minor** | Small deviation | Next release |
| **Observation** | Improvement opportunity | Backlog |

### 6.2 Non-Conformance Process

```
┌──────────────┐    ┌──────────────┐    ┌──────────────┐
│   Identify   │───▶│   Document   │───▶│   Analyze    │
│              │    │   (Issue)    │    │   (Root Cause)│
└──────────────┘    └──────────────┘    └──────────────┘
                                               │
       ┌───────────────────────────────────────┘
       ▼
┌──────────────┐    ┌──────────────┐    ┌──────────────┐
│   Correct    │───▶│   Verify     │───▶│    Close     │
│   (Fix)      │    │   (Review)   │    │              │
└──────────────┘    └──────────────┘    └──────────────┘
```

### 6.3 Escalation Path

1. Developer → Reviewer (PR comments)
2. Reviewer → QA Lead (unresolved findings)
3. QA Lead → Project Lead (critical issues)
4. Project Lead → Stakeholders (release blockers)

---

## 7. Metrics and Reporting

### 7.1 Quality Metrics

| Metric | Target | Current |
|--------|--------|---------|
| Test pass rate | 100% | Run `cd tests && make test` |
| Code coverage (host stmt) | >80% | ~91% (see `coverage_analysis.md`) |
| Open critical issues | 0 | 0 |
| MISRA violations | 0 (mandatory) | TBD |
| Review turnaround | <3 days | TBD |

### 7.2 Quality Dashboard

Automated via CI:

```
┌─────────────────────────────────────────────────────────────┐
│                 ICARUS Quality Dashboard                     │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│  Build Status:     (see CI / local build output)             │
│  Test Results:     140 tests (run `make test` for status)    │
│  Code Coverage:    ~91% stmt (kernel+BSP, `lcov` host)      │
│  Static Analysis:  cppcheck clean (see build/Makefile)       │
│  Open Issues:      (check GitHub Issues)                     │
│  Current Version:  v0.1.0                                    │
│                                                              │
└─────────────────────────────────────────────────────────────┘
```

### 7.3 Reporting Schedule

| Report | Frequency | Audience |
|--------|-----------|----------|
| CI Status | Per commit | Developers |
| Weekly Summary | Weekly | Team |
| Release Report | Per release | Stakeholders |
| Audit Report | Per audit | QA, Management |

---

## 8. Training

### 8.1 Required Training

| Topic | Audience | Frequency |
|-------|----------|-----------|
| DO-178C Overview | All | Onboarding |
| Coding Standards | Developers | Onboarding |
| CM Procedures | All | Onboarding |
| QA Procedures | QA, Reviewers | Onboarding |
| Tool Training | As needed | Per tool |

### 8.2 Training Records

Training completion tracked in:
- Onboarding checklist
- Training log (maintained by QA)

---

## 9. Records Management

### 9.1 QA Records

| Record | Retention | Location |
|--------|-----------|----------|
| Review records | Project life + 5 years | GitHub PRs |
| Audit reports | Project life + 5 years | docs/do178c/quality/ |
| Non-conformance reports | Project life + 5 years | GitHub Issues |
| Training records | Project life + 5 years | Internal |

### 9.2 Record Access

- All project records accessible to team
- Certification records archived per release
- Audit trail maintained via Git history

---

## 10. Continuous Improvement

### 10.1 Process Improvement

Sources of improvement:
- Audit findings
- Retrospectives
- Industry best practices
- Tool advancements

### 10.2 Lessons Learned

Captured after:
- Each release
- Significant issues
- Process changes

Documented in:
- Release retrospective notes
- Process improvement backlog

---

*End of Document*
