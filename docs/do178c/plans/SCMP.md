# Software Configuration Management Plan (SCMP)

**Document ID:** ICARUS-SCMP-001  
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

This Software Configuration Management Plan (SCMP) defines the processes and procedures for managing ICARUS OS configuration items throughout the software life cycle, per DO-178C objectives.

### 1.2 Scope

This plan covers:
- Source code and documentation
- Build artifacts and tools
- Test cases and results
- Release management

---

## 2. Configuration Management Organization

### 2.1 Roles

| Role | Responsibilities |
|------|------------------|
| Configuration Manager | Repository management, release process, baseline control |
| Developer | Create branches, submit PRs, follow CM procedures |
| Reviewer | Approve changes, verify traceability |
| Release Manager | Tag releases, generate artifacts, publish |

### 2.2 Tools

| Tool | Purpose | Location |
|------|---------|----------|
| Git | Version control | Local + GitHub |
| GitHub | Repository hosting, PRs, issues | github.com |
| GitHub Actions | CI/CD automation | .github/workflows/ |
| Make/CMake | Build automation | Makefile, CMakeLists.txt |

---

## 3. Configuration Identification

### 3.1 Configuration Items

| Item | Type | Location | Control Level |
|------|------|----------|---------------|
| Source Code | Software | Core/, Drivers/ | CC1 |
| Test Code | Software | tests/ | CC1 |
| Build Scripts | Software | Makefile, build/ | CC1 |
| Documentation | Document | docs/ | CC2 |
| CI Configuration | Config | .github/ | CC2 |
| Third-Party Code | Software | Drivers/STM32*, Middlewares/ | CC2 |

### 3.2 Naming Conventions

**Branches:**
```
main                    # Stable release branch
dev                     # Development integration
feature/<name>          # Feature development
bugfix/<name>           # Bug fixes
release/v<X.Y.Z>        # Release preparation
```

**Tags:**
```
v<MAJOR>.<MINOR>.<PATCH>        # Release tags (e.g., v1.0.0)
v<MAJOR>.<MINOR>.<PATCH>-rc<N>  # Release candidates
```

**Documents:**
```
ICARUS-<TYPE>-<SEQ>     # e.g., ICARUS-SRS-001
```

### 3.3 Version Numbering

Semantic Versioning (SemVer):
- **MAJOR:** Incompatible API changes
- **MINOR:** New features, backward compatible
- **PATCH:** Bug fixes, backward compatible

---

## 4. Baseline Management

### 4.1 Baseline Types

| Baseline | Content | Trigger |
|----------|---------|---------|
| Development | Current dev branch | Continuous |
| Release Candidate | Tagged RC | Pre-release review |
| Release | Tagged release | Certification milestone |

### 4.2 Baseline Contents

A release baseline includes:

```
Release v1.0.0/
├── Source Code (tagged commit)
├── Documentation
│   ├── PSAC
│   ├── SDP
│   ├── SVP
│   ├── SRS
│   ├── SCMP
│   └── SQAP
├── Verification Results
│   ├── Test Results
│   ├── Coverage Report
│   └── Static Analysis Report
├── Build Artifacts
│   ├── icarus_os.elf
│   ├── icarus_os.hex
│   └── icarus_os.map
└── Release Notes
```


---

## 5. Change Control

### 5.1 Change Process

```
┌──────────────┐    ┌──────────────┐    ┌──────────────┐
│   Issue/PR   │───▶│    Review    │───▶│    Merge     │
│   Created    │    │   Approval   │    │   to dev     │
└──────────────┘    └──────────────┘    └──────────────┘
       │                   │                   │
       ▼                   ▼                   ▼
  • Description       • Code review      • CI passes
  • Traceability      • Test review      • Coverage met
  • Impact analysis   • Doc review       • No regressions
```

### 5.2 Pull Request Requirements

All changes must:

- [ ] Reference an issue or requirement
- [ ] Include appropriate tests
- [ ] Pass CI checks (build, test, coverage)
- [ ] Have at least one approval
- [ ] Update documentation if needed
- [ ] Follow coding standards

### 5.3 Change Categories

| Category | Approval | CI Required |
|----------|----------|-------------|
| Bug fix | 1 reviewer | Yes |
| Feature | 2 reviewers | Yes |
| Documentation | 1 reviewer | No |
| Safety-critical | 2 reviewers + QA | Yes |

### 5.4 Emergency Changes

For critical fixes:
1. Create hotfix branch from main
2. Implement minimal fix
3. Expedited review (1 reviewer minimum)
4. Merge to main and dev
5. Document deviation from normal process

---

## 6. Build Management

### 6.1 Build Environment

| Component | Specification |
|-----------|---------------|
| OS | Linux (CI), macOS/Windows (dev) |
| Compiler | ARM GCC 12.x |
| Build Tool | GNU Make 4.x |
| Container | Docker (optional, for reproducibility) |

### 6.2 Build Targets

```makefile
# Primary targets
make all          # Build release firmware
make debug        # Build debug firmware
make clean        # Clean build artifacts

# Test targets
make test         # Run host-based tests
make coverage     # Generate coverage report

# Analysis targets
make cppcheck     # Run static analysis
make misra        # Run MISRA checker (planned)
```

### 6.3 Build Reproducibility

To ensure reproducible builds:
- Compiler version locked in CI
- All dependencies version-controlled
- Build flags documented
- Docker image available for exact reproduction

---

## 7. Release Management

### 7.1 Release Process

```
1. Feature Freeze
   └── No new features on release branch

2. Release Candidate
   ├── Tag: v1.0.0-rc1
   ├── Full test execution
   └── Documentation review

3. Release Approval
   ├── All tests pass
   ├── Coverage targets met
   ├── Documentation complete
   └── QA sign-off

4. Release
   ├── Tag: v1.0.0
   ├── Generate artifacts
   ├── Publish release notes
   └── Archive baseline
```

### 7.2 Release Artifacts

| Artifact | Format | Purpose |
|----------|--------|---------|
| Source archive | .tar.gz, .zip | Distribution |
| Binary (ELF) | .elf | Debug loading |
| Binary (HEX) | .hex | Production flashing |
| Map file | .map | Memory analysis |
| Documentation | PDF | Certification |
| Test results | HTML, XML | Verification evidence |

### 7.3 Release Notes Template

```markdown
# ICARUS OS v1.0.0 Release Notes

## Highlights
- [Brief summary of major changes]

## New Features
- [Feature 1]
- [Feature 2]

## Bug Fixes
- [Fix 1] (#issue)
- [Fix 2] (#issue)

## Breaking Changes
- [Change 1]

## Known Issues
- [Issue 1]

## Verification Status
- Tests: X passed, 0 failed
- Coverage: XX% statement
- Static Analysis: Clean

## Upgrade Notes
- [Migration instructions if needed]
```

---

## 8. Problem Reporting

### 8.1 Issue Tracking

All problems tracked via GitHub Issues with labels:

| Label | Description |
|-------|-------------|
| `bug` | Software defect |
| `enhancement` | Feature request |
| `documentation` | Doc issue |
| `safety` | Safety-related issue |
| `priority:critical` | Immediate attention |
| `priority:high` | Next release |
| `priority:medium` | Future release |
| `priority:low` | Backlog |

### 8.2 Problem Report Template

```markdown
## Description
[Clear description of the problem]

## Steps to Reproduce
1. [Step 1]
2. [Step 2]

## Expected Behavior
[What should happen]

## Actual Behavior
[What actually happens]

## Environment
- Hardware: [e.g., STM32H750]
- Version: [e.g., v0.1.0]
- Compiler: [e.g., ARM GCC 12.2]

## Impact
- [ ] Safety-related
- [ ] Functional
- [ ] Performance
- [ ] Documentation
```

---

## 9. Audit and Status Accounting

### 9.1 Configuration Status Reports

Generated automatically via CI:
- Build status (pass/fail)
- Test results summary
- Coverage metrics
- Open issues count

### 9.2 Audit Trail

Git provides complete audit trail:
- Who made each change
- When changes were made
- What was changed (diff)
- Why (commit message, PR description)

### 9.3 Periodic Reviews

| Review | Frequency | Content |
|--------|-----------|---------|
| CI Status | Daily | Build/test results |
| Issue Triage | Weekly | New issues, priorities |
| Baseline Review | Per release | Complete baseline audit |

---

## 10. Supplier Control

### 10.1 Third-Party Components

| Component | Supplier | Version | Control |
|-----------|----------|---------|---------|
| STM32 HAL | STMicroelectronics | 1.11.x | Vendor release |
| CMSIS | ARM | 5.9.x | Vendor release |
| USB Middleware | STMicroelectronics | 2.11.x | Vendor release |
| Unity | ThrowTheSwitch | 2.5.x | Tagged release |

### 10.2 Supplier Change Management

- Third-party updates reviewed before integration
- Changes documented in release notes
- Regression testing after updates
- Version locked until explicit update decision

---

*End of Document*
