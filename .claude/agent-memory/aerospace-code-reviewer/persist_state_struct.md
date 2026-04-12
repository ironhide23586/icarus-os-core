---
name: Persist state struct
description: persist_state_t needs _Static_assert for size and fault_counts[7] should use FDIR_NUM_FAULT_TYPES
type: project
---

persist_state_t in include/obc/persist.h has fault_counts[7] — hardcoded instead of FDIR_NUM_FAULT_TYPES.
persist_save() in persist.c also uses literal 7 in for loop.
No _Static_assert on struct size despite CRC depending on offsetof(persist_state_t, crc).

**Why:** If FDIR_NUM_FAULT_TYPES changes, persist will silently miss faults or overflow.
**How to apply:** Use FDIR_NUM_FAULT_TYPES in struct definition and loop. Add _Static_assert for size/offset.
