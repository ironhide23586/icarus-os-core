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
 *          Registry, staging, and active buffers all live in
 *          DTCM_DATA_PRIV. Public access goes through SVC gates.
 *
 *          tbl_activate is split across two privileged calls so the
 *          user activate callback can run in thread mode between them:
 *
 *            __tbl_activate_prepare()  — validate + copy staging into a
 *                                        caller-provided scratch buffer
 *            user activate(scratch)    — runs in thread mode, no SVC
 *            __tbl_activate_commit()   — copy scratch into active
 *
 *          The engine itself is silent — no logging dependency.
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

DTCM_DATA_PRIV static tbl_slot_t registry[TBL_MAX_REGISTERED];
DTCM_DATA_PRIV static uint8_t    reg_count;

/* ---- Helper: find slot by id (priv mode only) -------------------------- */

ITCM_FUNC static tbl_slot_t *find_slot(tbl_id_t id) {
    for (uint8_t i = 0; i < reg_count; i++) {
        if (registry[i].desc.id == id) {
            return &registry[i];
        }
    }
    return NULL;
}

/* ---- Privileged implementations ---------------------------------------- */

ITCM_FUNC void __tbl_init(void) {
    (void)memset(registry, 0, sizeof(registry));
    reg_count = 0;
}

ITCM_FUNC bool __tbl_register(const tbl_descriptor_t *desc) {
    if (!desc) {
        return false;
    }
    if (reg_count >= TBL_MAX_REGISTERED) {
        return false;
    }
    if (desc->size == 0 || desc->size > TBL_MAX_SIZE) {
        return false;
    }
    /* Reject duplicate id */
    for (uint8_t i = 0; i < reg_count; i++) {
        if (registry[i].desc.id == desc->id) {
            return false;
        }
    }
    tbl_slot_t *slot = &registry[reg_count++];
    (void)memset(slot, 0, sizeof(*slot));
    (void)memcpy(&slot->desc, desc, sizeof(tbl_descriptor_t));
    /* Ensure NUL termination of name */
    slot->desc.name[TBL_NAME_LEN - 1] = '\0';
    return true;
}

ITCM_FUNC bool __tbl_load(tbl_id_t id, const uint8_t *data, uint16_t len,
                          uint16_t schema_crc) {
    if ((data == NULL) || (len == 0u)) {
        return false;
    }

    tbl_slot_t *slot = find_slot(id);
    if (slot == NULL) {
        return false;
    }

    /* First chunk (or re-load after a completed/failed activation): reset
       staging.  We consider staging "ready for reset" when either it is
       empty (staged_len == 0) or a previous full-size load has already been
       validated (staged_valid == true). */
    if ((slot->staged_len == 0u) || slot->staged_valid) {
        (void)memset(slot->staging, 0, sizeof(slot->staging));
        slot->staged_len        = 0;
        slot->staged_valid      = false;
        slot->staged_schema_crc = schema_crc;
    }

    /* Bounds check */
    if ((uint32_t)slot->staged_len + len > TBL_MAX_SIZE) {
        return false;
    }

    (void)memcpy(&slot->staging[slot->staged_len], data, len);
    slot->staged_len = (uint16_t)(slot->staged_len + len);

    /* Mark valid and compute data CRC once we have a full descriptor-size load */
    if (slot->staged_len == slot->desc.size) {
        slot->staged_data_crc = crc16_ccitt(slot->staging, slot->staged_len);
        slot->staged_valid    = true;
    }

    return true;
}

ITCM_FUNC bool __tbl_activate_prepare(tbl_id_t id, uint8_t *out_data,
                                      uint16_t *out_len,
                                      tbl_activate_fn *out_activate) {
    if ((out_data == NULL) || (out_len == NULL) || (out_activate == NULL)) {
        return false;
    }

    tbl_slot_t *slot = find_slot(id);
    if (slot == NULL) {
        return false;
    }

    /* Step 1: size match */
    if ((!slot->staged_valid) || (slot->staged_len != slot->desc.size)) {
        return false;
    }

    /* Step 2: schema CRC */
    if (slot->staged_schema_crc != slot->desc.schema_crc) {
        return false;
    }

    /* Step 3: recompute data CRC for safety */
    uint16_t computed = crc16_ccitt(slot->staging, slot->staged_len);
    if (computed != slot->staged_data_crc) {
        return false;
    }

    /* Step 4: copy into the caller-provided scratch buffer so the user
     *         activate callback can run from thread mode without touching
     *         DTCM_PRIV directly. */
    (void)memcpy(out_data, slot->staging, slot->staged_len);
    *out_len      = slot->staged_len;
    *out_activate = slot->desc.activate;
    return true;
}

ITCM_FUNC bool __tbl_activate_commit(tbl_id_t id, const uint8_t *data,
                                     uint16_t len) {
    if ((data == NULL) || (len == 0u) || (len > (uint16_t)TBL_MAX_SIZE)) {
        return false;
    }
    tbl_slot_t *slot = find_slot(id);
    if (slot == NULL) {
        return false;
    }
    (void)memcpy(slot->active, data, len);
    slot->active_len = len;
    return true;
}

ITCM_FUNC int16_t __tbl_dump(tbl_id_t id, uint8_t *out, uint16_t max) {
    if ((out == NULL) || (max == 0u)) {
        return -1;
    }

    tbl_slot_t *slot = find_slot(id);
    if (slot == NULL) {
        return -1;
    }

    uint16_t len = slot->active_len;
    if (len == 0u) {
        return -1;
    }
    if (len > max) {
        len = max;
    }
    (void)memcpy(out, slot->active, len);
    return (int16_t)len;
}

ITCM_FUNC const tbl_descriptor_t *__tbl_get_descriptor(tbl_id_t id) {
    tbl_slot_t *slot = find_slot(id);
    return slot ? &slot->desc : NULL;
}

ITCM_FUNC uint8_t __tbl_count(void) {
    return reg_count;
}
