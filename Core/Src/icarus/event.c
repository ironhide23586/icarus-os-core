/**
 * @file    event.c
 * @brief   Generic event ring buffer implementation
 *
 * @details Lock-protected (enter_critical / exit_critical) circular ring
 *          of fixed-size event entries with per-module severity squelch.
 *          Transport-agnostic: drains into a caller-provided buffer.
 *
 *          The ring, head/count cursors, and squelch table all live in
 *          DTCM_DATA_PRIV (privileged-only TCM). Public access goes
 *          through SVC gates so unprivileged thread-mode tasks can use
 *          the API once the MPU is locked.
 *
 * @author  Souham Biswas
 * @date    2026
 *
 * @copyright Copyright 2025-2026 Souham Biswas
 *            https://github.com/ironhide23586/icarus-os-core
 *            Licensed under the Apache License, Version 2.0
 */

#include "icarus/icarus.h"
#include <string.h>

/* ---- Backing storage in DTCM_PRIV --------------------------------------- */

DTCM_DATA_PRIV static event_entry_t    ring[EVENT_RING_SIZE];
DTCM_DATA_PRIV static uint32_t         ring_head;   /* next write index    */
DTCM_DATA_PRIV static uint32_t         ring_count;  /* valid entry count   */
DTCM_DATA_PRIV static event_severity_t squelch[EVENT_MOD_MAX];

/* ---- Privileged implementations ----------------------------------------- */

ITCM_FUNC void __event_init(void) {
    memset(ring, 0, sizeof(ring));
    ring_head  = 0;
    ring_count = 0;
    for (uint32_t i = 0; i < EVENT_MOD_MAX; i++) {
        squelch[i] = EVENT_DEBUG;
    }
}

ITCM_FUNC void __os_event(uint8_t module_id, event_severity_t severity,
                          uint16_t event_id, const void *payload,
                          uint8_t payload_len) {
    if (module_id >= EVENT_MOD_MAX) {
        return;
    }
    if ((uint8_t)severity < (uint8_t)squelch[module_id]) {
        return;
    }
    if (payload_len > 12) {
        payload_len = 12;
    }

    event_entry_t *entry = &ring[ring_head];
    entry->module_id = module_id;
    entry->severity  = (uint8_t)severity;
    entry->event_id  = event_id;

    memset(entry->payload, 0, sizeof(entry->payload));
    if (payload && payload_len > 0) {
        memcpy(entry->payload, payload, payload_len);
    }

    ring_head = (ring_head + 1) % EVENT_RING_SIZE;
    if (ring_count < EVENT_RING_SIZE) {
        ring_count++;
    }
}

ITCM_FUNC void __event_set_squelch(uint8_t module_id,
                                   event_severity_t min_severity) {
    if (module_id >= EVENT_MOD_MAX) {
        return;
    }
    squelch[module_id] = min_severity;
}

ITCM_FUNC event_severity_t __event_get_squelch(uint8_t module_id) {
    if (module_id >= EVENT_MOD_MAX) {
        return EVENT_DEBUG;
    }
    return squelch[module_id];
}

ITCM_FUNC bool __event_drain(event_entry_t *out_buf, uint8_t max_entries,
                             uint8_t *num_drained) {
    if (!out_buf || max_entries == 0) {
        if (num_drained) *num_drained = 0;
        return false;
    }

    if (ring_count == 0) {
        if (num_drained) *num_drained = 0;
        return false;
    }

    /* Index of the oldest entry */
    uint32_t tail;
    if (ring_count < EVENT_RING_SIZE) {
        tail = 0;
    } else {
        tail = ring_head; /* head points to oldest when full */
    }

    uint32_t to_drain = ring_count;
    if (to_drain > max_entries) {
        to_drain = max_entries;
    }

    for (uint32_t i = 0; i < to_drain; i++) {
        uint32_t idx = (tail + i) % EVENT_RING_SIZE;
        memcpy(&out_buf[i], &ring[idx], sizeof(event_entry_t));
    }

    ring_count -= to_drain;
    if (ring_count == 0) {
        ring_head = 0;
    }

    if (num_drained) *num_drained = (uint8_t)to_drain;
    return true;
}

ITCM_FUNC uint32_t __event_get_count(void) {
    return ring_count;
}
