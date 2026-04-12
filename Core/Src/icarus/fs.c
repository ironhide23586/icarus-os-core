/**
 * @file    fs.c
 * @brief   Internal RAM-backed flat-file filesystem implementation
 *
 * @details The store is a static byte array divided into FS_MAX_FILES
 *          fixed-size blocks. A parallel table of fs_entry_t structs
 *          tracks name, size, and occupancy for each block.
 *
 *          The __fs_* prefixed functions are the privileged implementations
 *          that execute inside the SVC handler with full access to kernel
 *          data.  The unprefixed public functions issue SVC instructions
 *          so that unprivileged tasks can use the filesystem without
 *          triggering MemManage faults.
 *
 * @author  Souham Biswas
 * @date    2026
 *
 * @copyright Copyright 2025-2026 Souham Biswas
 *            https://github.com/ironhide23586/icarus-os-core
 *            Licensed under the Apache License, Version 2.0
 */

#include "icarus/icarus.h"
#include "icarus/svc.h"
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

/** @brief Find slot index for @p name, or -1 if not found. */
static int16_t fs_find(const char *name) {
    for (uint8_t i = 0; i < FS_MAX_FILES; i++) {
        if (fs_table[i].used &&
            strncmp(fs_table[i].name, name, FS_MAX_NAME_LEN) == 0) {
            return (int16_t)i;
        }
    }
    return -1;
}

/** @brief Find first free slot, or -1 if full. */
static int16_t fs_free_slot(void) {
    for (uint8_t i = 0; i < FS_MAX_FILES; i++) {
        if (!fs_table[i].used) {
            return (int16_t)i;
        }
    }
    return -1;
}

/* ============================================================================
 * PRIVILEGED IMPLEMENTATIONS (called from SVC handler)
 * ========================================================================= */

/**
 * @brief  Privileged implementation of fs_init().
 * @details Zeroes the file table and data store.
 */
ITCM_FUNC void __fs_init(void) {
    memset(fs_table, 0, sizeof(fs_table));
    memset(fs_data,  0, sizeof(fs_data));
}

/**
 * @brief  Privileged implementation of fs_create().
 * @param[in]  name  Null-terminated file name.
 * @param[out] out   File handle written on success.
 * @retval true   File created.
 * @retval false  Full, duplicate, or invalid name.
 */
ITCM_FUNC bool __fs_create(const char *name, fs_file_t *out) {
    if (!name || !out || name[0] == '\0') {
        return false;
    }
    if (strnlen(name, FS_MAX_NAME_LEN) >= FS_MAX_NAME_LEN) {
        return false;
    }

    if (fs_find(name) >= 0) {
        return false;
    }

    int16_t slot = fs_free_slot();
    if (slot < 0) {
        return false;
    }

    strncpy(fs_table[slot].name, name, FS_MAX_NAME_LEN - 1);
    fs_table[slot].name[FS_MAX_NAME_LEN - 1] = '\0';
    fs_table[slot].size = 0;
    fs_table[slot].used = true;
    memset(fs_data[slot], 0, FS_MAX_FILE_SIZE);

    out->slot  = (uint8_t)slot;
    out->valid = true;

    return true;
}

/**
 * @brief  Privileged implementation of fs_open().
 * @param[in]  name  File name to look up.
 * @param[out] out   File handle written on success.
 * @retval true   File found and handle populated.
 * @retval false  File does not exist.
 */
ITCM_FUNC bool __fs_open(const char *name, fs_file_t *out) {
    if (!name || !out || name[0] == '\0') {
        return false;
    }

    int16_t slot = fs_find(name);
    if (slot < 0) {
        return false;
    }
    out->slot  = (uint8_t)slot;
    out->valid = true;
    return true;
}

/**
 * @brief  Privileged implementation of fs_write().
 * @param[in] f     File handle.
 * @param[in] data  Bytes to append.
 * @param[in] len   Number of bytes.
 * @retval true   Data appended.
 * @retval false  Invalid handle, null data, or would exceed max size.
 */
ITCM_FUNC bool __fs_write(fs_file_t *f, const uint8_t *data, uint16_t len) {
    if (!f || !f->valid || !data || len == 0) {
        return false;
    }

    uint8_t slot = f->slot;
    if (slot >= FS_MAX_FILES || !fs_table[slot].used) {
        return false;
    }

    uint16_t current = fs_table[slot].size;
    if ((uint32_t)current + len > FS_MAX_FILE_SIZE) {
        return false;
    }

    memcpy(&fs_data[slot][current], data, len);
    fs_table[slot].size = (uint16_t)(current + len);

    return true;
}

/**
 * @brief  Privileged implementation of fs_read().
 * @param[in]  f       File handle.
 * @param[out] buf     Destination buffer.
 * @param[in]  len     Maximum bytes to read.
 * @param[in]  offset  Byte offset into the file.
 * @return Bytes actually read (may be less than @p len near EOF), or 0 on error.
 */
ITCM_FUNC uint16_t __fs_read(fs_file_t *f, uint8_t *buf, uint16_t len,
                              uint16_t offset) {
    if (!f || !f->valid || !buf || len == 0) {
        return 0;
    }

    uint8_t slot = f->slot;
    if (slot >= FS_MAX_FILES || !fs_table[slot].used) {
        return 0;
    }

    uint16_t file_size = fs_table[slot].size;
    if (offset >= file_size) {
        return 0;
    }

    uint16_t available = (uint16_t)(file_size - offset);
    uint16_t to_read   = (len < available) ? len : available;

    memcpy(buf, &fs_data[slot][offset], to_read);

    return to_read;
}

/**
 * @brief  Privileged implementation of fs_delete().
 * @param[in] name  File name to remove.
 * @retval true   File found and deleted.
 * @retval false  File does not exist.
 */
ITCM_FUNC bool __fs_delete(const char *name) {
    if (!name || name[0] == '\0') {
        return false;
    }

    int16_t slot = fs_find(name);
    if (slot < 0) {
        return false;
    }

    memset(&fs_table[slot], 0, sizeof(fs_entry_t));
    memset(fs_data[slot],   0, FS_MAX_FILE_SIZE);

    return true;
}

/**
 * @brief  Privileged implementation of fs_list().
 * @param[out] out  Array of file info entries.
 * @param[in]  max  Maximum entries to write.
 * @return Number of entries written.
 */
ITCM_FUNC uint8_t __fs_list(fs_file_info_t *out, uint8_t max) {
    if (!out || max == 0) {
        return 0;
    }

    uint8_t count = 0;
    for (uint8_t i = 0; i < FS_MAX_FILES && count < max; i++) {
        if (fs_table[i].used) {
            strncpy(out[count].name, fs_table[i].name, FS_MAX_NAME_LEN);
            out[count].name[FS_MAX_NAME_LEN - 1] = '\0';
            out[count].size = fs_table[i].size;
            count++;
        }
    }

    return count;
}

/**
 * @brief  Privileged implementation of fs_stats().
 * @param[out] out  Aggregate filesystem statistics.
 */
ITCM_FUNC void __fs_stats(fs_stats_t *out) {
    if (!out) {
        return;
    }

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
}

/* ============================================================================
 * PUBLIC API (SVC call gates — privileged access to filesystem data)
 * ========================================================================= */

/**
 * @brief  Clear all file table entries and zero the data store via SVC gate.
 */
void fs_init(void) {
#ifndef HOST_TEST
    __asm__ volatile ("svc %0\n" : : "I" (SVC_FS_INIT));
#else
    __fs_init();
#endif
}

/**
 * @brief  Create a new file via SVC gate.
 * @param[in]  name  Null-terminated file name.
 * @param[out] out   File handle written on success.
 * @retval true   File created.
 * @retval false  Full, duplicate, or invalid name.
 */
bool fs_create(const char *name, fs_file_t *out) {
#ifndef HOST_TEST
    uint32_t result;
    __asm__ volatile (
        "mov r0, %1\n"
        "mov r1, %2\n"
        "svc %3\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "r" ((uint32_t)(uintptr_t)name), "r" ((uint32_t)(uintptr_t)out),
          "I" (SVC_FS_CREATE)
        : "r0", "r1"
    );
    return (bool)result;
#else
    return __fs_create(name, out);
#endif
}

/**
 * @brief  Open an existing file by name via SVC gate.
 * @param[in]  name  File name to look up.
 * @param[out] out   File handle written on success.
 * @retval true   File found.
 * @retval false  File does not exist.
 */
bool fs_open(const char *name, fs_file_t *out) {
#ifndef HOST_TEST
    uint32_t result;
    __asm__ volatile (
        "mov r0, %1\n"
        "mov r1, %2\n"
        "svc %3\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "r" ((uint32_t)(uintptr_t)name), "r" ((uint32_t)(uintptr_t)out),
          "I" (SVC_FS_OPEN)
        : "r0", "r1"
    );
    return (bool)result;
#else
    return __fs_open(name, out);
#endif
}

/**
 * @brief  Append data to an open file via SVC gate.
 * @param[in] f     File handle.
 * @param[in] data  Bytes to append.
 * @param[in] len   Number of bytes.
 * @retval true   Data appended.
 * @retval false  Invalid handle, null data, or would exceed max size.
 */
bool fs_write(fs_file_t *f, const uint8_t *data, uint16_t len) {
#ifndef HOST_TEST
    uint32_t result;
    __asm__ volatile (
        "mov r0, %1\n"
        "mov r1, %2\n"
        "mov r2, %3\n"
        "svc %4\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "r" ((uint32_t)(uintptr_t)f), "r" ((uint32_t)(uintptr_t)data),
          "r" ((uint32_t)len), "I" (SVC_FS_WRITE)
        : "r0", "r1", "r2"
    );
    return (bool)result;
#else
    return __fs_write(f, data, len);
#endif
}

/**
 * @brief  Read bytes from an open file at a given offset via SVC gate.
 * @param[in]  f       File handle.
 * @param[out] buf     Destination buffer.
 * @param[in]  len     Maximum bytes to read.
 * @param[in]  offset  Byte offset into the file.
 * @return Bytes actually read, or 0 on error.
 */
uint16_t fs_read(fs_file_t *f, uint8_t *buf, uint16_t len, uint16_t offset) {
#ifndef HOST_TEST
    uint32_t result;
    __asm__ volatile (
        "mov r0, %1\n"
        "mov r1, %2\n"
        "mov r2, %3\n"
        "mov r3, %4\n"
        "svc %5\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "r" ((uint32_t)(uintptr_t)f), "r" ((uint32_t)(uintptr_t)buf),
          "r" ((uint32_t)len), "r" ((uint32_t)offset),
          "I" (SVC_FS_READ)
        : "r0", "r1", "r2", "r3"
    );
    return (uint16_t)result;
#else
    return __fs_read(f, buf, len, offset);
#endif
}

/**
 * @brief  Delete a file by name via SVC gate.
 * @param[in] name  File name to remove.
 * @retval true   File deleted.
 * @retval false  File does not exist.
 */
bool fs_delete(const char *name) {
#ifndef HOST_TEST
    uint32_t result;
    __asm__ volatile (
        "mov r0, %1\n"
        "svc %2\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "r" ((uint32_t)(uintptr_t)name), "I" (SVC_FS_DELETE)
        : "r0"
    );
    return (bool)result;
#else
    return __fs_delete(name);
#endif
}

/**
 * @brief  Enumerate all files via SVC gate.
 * @param[out] out  Array of file info entries.
 * @param[in]  max  Maximum entries to write.
 * @return Number of entries written.
 */
uint8_t fs_list(fs_file_info_t *out, uint8_t max) {
#ifndef HOST_TEST
    uint32_t result;
    __asm__ volatile (
        "mov r0, %1\n"
        "mov r1, %2\n"
        "svc %3\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "r" ((uint32_t)(uintptr_t)out), "r" ((uint32_t)max),
          "I" (SVC_FS_LIST)
        : "r0", "r1"
    );
    return (uint8_t)result;
#else
    return __fs_list(out, max);
#endif
}

/**
 * @brief  Return aggregate filesystem statistics via SVC gate.
 * @param[out] out  Statistics structure to fill.
 */
void fs_stats(fs_stats_t *out) {
#ifndef HOST_TEST
    __asm__ volatile (
        "mov r0, %0\n"
        "svc %1\n"
        :
        : "r" ((uint32_t)(uintptr_t)out), "I" (SVC_FS_STATS)
        : "r0"
    );
#else
    __fs_stats(out);
#endif
}
