---
name: CRC16 len truncation risk
description: crc16_ccitt() takes uint16_t len but cs_add_region accepts uint32_t size — regions >64KB silently truncate CRC computation
type: project
---

crc16_ccitt(const uint8_t *data, uint16_t len) — max 65535 bytes.
__cs_add_region and __cs_rebaseline cast size to uint16_t: crc16_ccitt(addr, (uint16_t)size).
register_cs_regions in obc_init.c has a guard (text_size <= 0xFFFF) but the kernel API has no guard.

**Why:** .text section could exceed 64KB on STM32H750 (128KB flash). Silent truncation means CRC covers wrong byte count.
**How to apply:** Either widen crc16_ccitt len to uint32_t, or add bounds check in __cs_add_region.
