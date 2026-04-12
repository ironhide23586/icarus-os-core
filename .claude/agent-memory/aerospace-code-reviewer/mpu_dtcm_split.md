---
name: MPU layout and DTCM split
description: MPU Region 5 covers lower 64KB DTCM (PRIV_RW, SRD=0xF0), Region 3 covers upper 64KB (FULL_ACCESS) for application data
type: project
---

DTCM 128KB split into kernel (lower 64KB, 0x20000000-0x2000FFFF) and application (upper 64KB, 0x20010000-0x2001FFFF).
Region 5: 128KB, PRIV_RW, SubRegionDisable=0xF0 (subregions 4-7 disabled = upper 64KB not governed).
Region 3: 64KB at 0x20010000, FULL_ACCESS (application hot data, unprivileged RW).
Linker ASSERT prevents .dtcm_priv from overflowing into application region.

**Why:** MPU isolation between kernel and application data in DTCM.
**How to apply:** DTCM_DATA_PRIV goes to lower 64KB (kernel only). DTCM_DATA_APP goes to upper 64KB (task accessible).
