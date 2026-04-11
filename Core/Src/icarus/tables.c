/**
 * @file    tables.c
 * @brief   Generic ground-loadable table engine implementation
 *
 * @details Maintains a registry of up to TBL_MAX_REGISTERED table
 *          descriptors. Each registered table has a staging buffer and
 *          an active buffer (double-buffered swap):
 *
 *            1. tbl_load()     — writes raw bytes to staging (chunked OK).
 *            2. tbl_activate() — validates CRC, calls activate callback,
 *                                copies staging → active on success.
 *            3. tbl_dump()     — returns the *active* buffer contents.
 *
 *          Integrity uses crc16_ccitt() from icarus/crc.h. The engine is
 *          silent — no logging dependency.
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

/* ---- Per-slot storage --------------------------------------------------- */

typedef struct {
    tbl_descriptor_t desc;

    /* Staging side */
    uint8_t  staging[TBL_MAX_SIZE];
    uint16_t staged_len;        /**< Bytes written to staging so far          */
    uint16_t staged_schema_crc; /**< schema_crc supplied with last tbl_load() */
    uint16_t staged_data_crc;   /**< CRC16 computed over the full staged data */
    bool     staged_valid;      /**< True once a full-size load completed     */

    /* Active side */
    uint8_t  active[TBL_MAX_SIZE];
    uint16_t active_len;        /**< Bytes in active buffer (0 = none)        */
} tbl_slot_t;

static tbl_slot_t registry[TBL_MAX_REGISTERED];
static uint8_t    reg_count;

/* ---- Init --------------------------------------------------------------- */

void tbl_init(void) {
    enter_critical();
    memset(registry, 0, sizeof(registry));
    reg_count = 0;
    exit_critical();
}

/* ---- Helper: find slot by id -------------------------------------------- */

static tbl_slot_t *find_slot(tbl_id_t id) {
    for (uint8_t i = 0; i < reg_count; i++) {
        if (registry[i].desc.id == id) {
            return &registry[i];
        }
    }
    return NULL;
}

/* ---- Register ----------------------------------------------------------- */

bool tbl_register(const tbl_descriptor_t *desc) {
    if (!desc) {
        return false;
    }
    if (reg_count >= TBL_MAX_REGISTERED) {
        return false;
    }
    if (desc->size == 0 || desc->size > TBL_MAX_SIZE) {
        return false;
    }

    enter_critical();
    /* Reject duplicate id */
    for (uint8_t i = 0; i < reg_count; i++) {
        if (registry[i].desc.id == desc->id) {
            exit_critical();
            return false;
        }
    }
    tbl_slot_t *slot = &registry[reg_count++];
    memset(slot, 0, sizeof(*slot));
    memcpy(&slot->desc, desc, sizeof(tbl_descriptor_t));
    /* Ensure NUL termination of name */
    slot->desc.name[TBL_NAME_LEN - 1] = '\0';
    exit_critical();

    return true;
}

/* ---- Load --------------------------------------------------------------- */

bool tbl_load(tbl_id_t id, const uint8_t *data, uint16_t len,
              uint16_t schema_crc) {
    if (!data || len == 0) {
        return false;
    }

    tbl_slot_t *slot = find_slot(id);
    if (!slot) {
        return false;
    }

    enter_critical();

    /* First chunk (or re-load after a completed/failed activation): reset
       staging.  We consider staging "ready for reset" when either it is
       empty (staged_len == 0) or a previous full-size load has already been
       validated (staged_valid == true, i.e. staged_len == desc.size). */
    if (slot->staged_len == 0 || slot->staged_valid) {
        memset(slot->staging, 0, sizeof(slot->staging));
        slot->staged_len        = 0;
        slot->staged_valid      = false;
        slot->staged_schema_crc = schema_crc;
    }

    /* Bounds check */
    if ((uint32_t)slot->staged_len + len > TBL_MAX_SIZE) {
        exit_critical();
        return false;
    }

    memcpy(&slot->staging[slot->staged_len], data, len);
    slot->staged_len = (uint16_t)(slot->staged_len + len);

    /* Mark valid and compute data CRC once we have a full descriptor-size load */
    if (slot->staged_len == slot->desc.size) {
        slot->staged_data_crc = crc16_ccitt(slot->staging, slot->staged_len);
        slot->staged_valid    = true;
    }

    exit_critical();
    return true;
}

/* ---- Activate ----------------------------------------------------------- */

bool tbl_activate(tbl_id_t id) {
    tbl_slot_t *slot = find_slot(id);
    if (!slot) {
        return false;
    }

    enter_critical();

    /* Step 1: size match */
    if (!slot->staged_valid || slot->staged_len != slot->desc.size) {
        exit_critical();
        return false;
    }

    /* Step 2: schema CRC */
    if (slot->staged_schema_crc != slot->desc.schema_crc) {
        exit_critical();
        return false;
    }

    /* Step 3: recompute data CRC for safety */
    uint16_t computed = crc16_ccitt(slot->staging, slot->staged_len);
    if (computed != slot->staged_data_crc) {
        exit_critical();
        return false;
    }

    /* Take local copies for the callback (released from critical before call) */
    uint8_t  tmp[TBL_MAX_SIZE];
    uint16_t tmp_len = slot->staged_len;
    memcpy(tmp, slot->staging, tmp_len);

    exit_critical();

    /* Step 4: call activate callback */
    if (slot->desc.activate) {
        if (!slot->desc.activate(tmp, tmp_len)) {
            return false;
        }
    }

    /* Step 5: copy staging -> active */
    enter_critical();
    memcpy(slot->active, tmp, tmp_len);
    slot->active_len = tmp_len;
    exit_critical();

    return true;
}

/* ---- Dump --------------------------------------------------------------- */

int16_t tbl_dump(tbl_id_t id, uint8_t *out, uint16_t max) {
    if (!out || max == 0) {
        return -1;
    }

    tbl_slot_t *slot = find_slot(id);
    if (!slot) {
        return -1;
    }

    enter_critical();
    uint16_t len = slot->active_len;
    if (len == 0) {
        exit_critical();
        return -1;
    }
    if (len > max) {
        len = max;
    }
    memcpy(out, slot->active, len);
    exit_critical();

    return (int16_t)len;
}

/* ---- Descriptor query --------------------------------------------------- */

const tbl_descriptor_t *tbl_get_descriptor(tbl_id_t id) {
    tbl_slot_t *slot = find_slot(id);
    return slot ? &slot->desc : NULL;
}

uint8_t tbl_count(void) {
    return reg_count;
}
