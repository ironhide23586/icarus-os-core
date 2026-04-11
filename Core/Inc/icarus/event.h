/**
 * @file    event.h
 * @brief   Generic structured event ring buffer for ICARUS OS
 *
 * @details Lock-protected ring of fixed-size 16-byte event entries with
 *          per-module severity squelch filtering. Transport-agnostic:
 *          drains into a caller-provided buffer; downstream consumers
 *          decide how to ship the entries (e.g. CCSDS TM, file logging).
 *
 * @author  Souham Biswas
 * @date    2026
 *
 * @copyright Copyright 2025-2026 Souham Biswas
 *            https://github.com/ironhide23586/icarus-os-core
 *            Licensed under the Apache License, Version 2.0
 */

#ifndef ICARUS_EVENT_H
#define ICARUS_EVENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/* ---- Severity levels ---------------------------------------------------- */

typedef enum {
    EVENT_DEBUG    = 0,
    EVENT_INFO     = 1,
    EVENT_WARN     = 2,
    EVENT_CRITICAL = 3
} event_severity_t;

/* ---- Sizing ------------------------------------------------------------- */

#ifndef EVENT_MOD_MAX
#define EVENT_MOD_MAX        16
#endif

#ifndef EVENT_RING_SIZE
#define EVENT_RING_SIZE      32
#endif

/* ---- Event entry: 4B header + 12B payload = 16B ------------------------- */

typedef struct {
    uint8_t  module_id;
    uint8_t  severity;       /* event_severity_t */
    uint16_t event_id;
    uint8_t  payload[12];
} __attribute__((packed)) event_entry_t;

#ifndef SKIP_STATIC_ASSERTS
_Static_assert(sizeof(event_entry_t) == 16, "event_entry_t must be 16 bytes");
#endif

/* ---- Public API --------------------------------------------------------- */

/**
 * @brief  Initialize the event system. Clears the ring buffer and resets
 *         every module's squelch level to EVENT_DEBUG (accept all).
 */
void event_init(void);

/**
 * @brief  Emit a structured event into the ring buffer.
 * @param  module_id   Source module identifier (0..EVENT_MOD_MAX-1).
 * @param  severity    Event severity level.
 * @param  event_id    Application-defined event identifier.
 * @param  payload     Optional payload bytes (may be NULL).
 * @param  payload_len Payload length in bytes (truncated to 12).
 * @note   Never blocks. When the ring is full the oldest entry is
 *         implicitly overwritten.
 */
void os_event(uint8_t module_id, event_severity_t severity, uint16_t event_id,
              const void *payload, uint8_t payload_len);

/**
 * @brief  Set the minimum severity accepted for a given module. Events
 *         below the threshold are silently dropped at emission time.
 */
void event_set_squelch(uint8_t module_id, event_severity_t min_severity);

/**
 * @brief  Get the current squelch level for a given module.
 */
event_severity_t event_get_squelch(uint8_t module_id);

/**
 * @brief  Drain queued events into a caller-provided buffer.
 * @param  out_buf      Destination array of event_entry_t.
 * @param  max_entries  Maximum number of entries to copy.
 * @param  num_drained  [out] Number of entries actually copied.
 * @return true if at least one entry was copied; false if the ring was
 *         empty or out_buf was NULL.
 */
bool event_drain(event_entry_t *out_buf, uint8_t max_entries,
                 uint8_t *num_drained);

/**
 * @brief  Number of events currently buffered.
 */
uint32_t event_get_count(void);

#ifdef __cplusplus
}
#endif

#endif /* ICARUS_EVENT_H */
