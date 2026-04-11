/**
 * @file    fs.c
 * @brief   Internal RAM-backed flat-file filesystem implementation
 *
 * @details The store is a static byte array divided into FS_MAX_FILES
 *          fixed-size blocks. A parallel table of fs_entry_t structs
 *          tracks name, size, and occupancy for each block. All public
 *          functions are re-entrant-safe via enter_critical() /
 *          exit_critical().
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

/* ---- Internal file table entry ----------------------------------------- */

typedef struct {
    char     name[FS_MAX_NAME_LEN]; /**< Null-terminated file name. */
    uint16_t size;                  /**< Bytes written so far. */
    bool     used;                  /**< Slot is occupied. */
} fs_entry_t;

#ifndef SKIP_STATIC_ASSERTS
_Static_assert(sizeof(fs_entry_t) == (FS_MAX_NAME_LEN + 2 + 1 + 1 /* pad */),
               "fs_entry_t unexpected size");
#endif

/* ---- Storage ----------------------------------------------------------- */

static fs_entry_t fs_table[FS_MAX_FILES];
static uint8_t    fs_data[FS_MAX_FILES][FS_MAX_FILE_SIZE];

/* ---- Internal helpers -------------------------------------------------- */

/* Caller must hold critical section. */
static int16_t fs_find(const char *name) {
    for (uint8_t i = 0; i < FS_MAX_FILES; i++) {
        if (fs_table[i].used &&
            strncmp(fs_table[i].name, name, FS_MAX_NAME_LEN) == 0) {
            return (int16_t)i;
        }
    }
    return -1;
}

/* Caller must hold critical section. */
static int16_t fs_free_slot(void) {
    for (uint8_t i = 0; i < FS_MAX_FILES; i++) {
        if (!fs_table[i].used) {
            return (int16_t)i;
        }
    }
    return -1;
}

/* ---- Public API -------------------------------------------------------- */

void fs_init(void) {
    enter_critical();
    memset(fs_table, 0, sizeof(fs_table));
    memset(fs_data,  0, sizeof(fs_data));
    exit_critical();
}

bool fs_create(const char *name, fs_file_t *out) {
    if (!name || !out || name[0] == '\0') {
        return false;
    }
    if (strnlen(name, FS_MAX_NAME_LEN) >= FS_MAX_NAME_LEN) {
        return false;
    }

    enter_critical();

    if (fs_find(name) >= 0) {
        exit_critical();
        return false;
    }

    int16_t slot = fs_free_slot();
    if (slot < 0) {
        exit_critical();
        return false;
    }

    strncpy(fs_table[slot].name, name, FS_MAX_NAME_LEN - 1);
    fs_table[slot].name[FS_MAX_NAME_LEN - 1] = '\0';
    fs_table[slot].size = 0;
    fs_table[slot].used = true;
    memset(fs_data[slot], 0, FS_MAX_FILE_SIZE);

    out->slot  = (uint8_t)slot;
    out->valid = true;

    exit_critical();
    return true;
}

bool fs_open(const char *name, fs_file_t *out) {
    if (!name || !out || name[0] == '\0') {
        return false;
    }

    enter_critical();
    int16_t slot = fs_find(name);
    if (slot < 0) {
        exit_critical();
        return false;
    }
    out->slot  = (uint8_t)slot;
    out->valid = true;
    exit_critical();
    return true;
}

bool fs_write(fs_file_t *f, const uint8_t *data, uint16_t len) {
    if (!f || !f->valid || !data || len == 0) {
        return false;
    }

    enter_critical();

    uint8_t slot = f->slot;
    if (slot >= FS_MAX_FILES || !fs_table[slot].used) {
        exit_critical();
        return false;
    }

    uint16_t current = fs_table[slot].size;
    if ((uint32_t)current + len > FS_MAX_FILE_SIZE) {
        exit_critical();
        return false;
    }

    memcpy(&fs_data[slot][current], data, len);
    fs_table[slot].size = (uint16_t)(current + len);

    exit_critical();
    return true;
}

uint16_t fs_read(fs_file_t *f, uint8_t *buf, uint16_t len, uint16_t offset) {
    if (!f || !f->valid || !buf || len == 0) {
        return 0;
    }

    enter_critical();

    uint8_t slot = f->slot;
    if (slot >= FS_MAX_FILES || !fs_table[slot].used) {
        exit_critical();
        return 0;
    }

    uint16_t file_size = fs_table[slot].size;
    if (offset >= file_size) {
        exit_critical();
        return 0;
    }

    uint16_t available = (uint16_t)(file_size - offset);
    uint16_t to_read   = (len < available) ? len : available;

    memcpy(buf, &fs_data[slot][offset], to_read);

    exit_critical();
    return to_read;
}

bool fs_delete(const char *name) {
    if (!name || name[0] == '\0') {
        return false;
    }

    enter_critical();

    int16_t slot = fs_find(name);
    if (slot < 0) {
        exit_critical();
        return false;
    }

    memset(&fs_table[slot], 0, sizeof(fs_entry_t));
    memset(fs_data[slot],   0, FS_MAX_FILE_SIZE);

    exit_critical();
    return true;
}

uint8_t fs_list(fs_file_info_t *out, uint8_t max) {
    if (!out || max == 0) {
        return 0;
    }

    enter_critical();

    uint8_t count = 0;
    for (uint8_t i = 0; i < FS_MAX_FILES && count < max; i++) {
        if (fs_table[i].used) {
            strncpy(out[count].name, fs_table[i].name, FS_MAX_NAME_LEN);
            out[count].name[FS_MAX_NAME_LEN - 1] = '\0';
            out[count].size = fs_table[i].size;
            count++;
        }
    }

    exit_critical();
    return count;
}

void fs_stats(fs_stats_t *out) {
    if (!out) {
        return;
    }

    enter_critical();

    uint32_t used = 0;
    uint8_t  count = 0;
    for (uint8_t i = 0; i < FS_MAX_FILES; i++) {
        if (fs_table[i].used) {
            used += fs_table[i].size;
            count++;
        }
    }

    out->total_bytes = FS_TOTAL_BYTES;
    out->used_bytes  = used;
    out->free_bytes  = FS_TOTAL_BYTES - used;
    out->file_count  = count;

    exit_critical();
}
