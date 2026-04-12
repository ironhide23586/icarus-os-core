/**
 * @file    fs.h
 * @brief   ICARUS OS — Internal RAM-backed flat-file filesystem
 *
 * @details A minimal flat-file storage layer backed by a static RAM array
 *          divided into FS_MAX_FILES fixed-size blocks. Suitable as a
 *          drop-in target for code that wants a tiny POSIX-ish file API
 *          before a real flash filesystem is wired up. The on-disk format
 *          is opaque; a real flash backend can be substituted later
 *          without changing the public API.
 *
 * @author  Souham Biswas
 * @date    2026
 *
 * @copyright Copyright 2025-2026 Souham Biswas
 *            https://github.com/ironhide23586/icarus-os-core
 *            Licensed under the Apache License, Version 2.0
 */

#ifndef ICARUS_FS_H
#define ICARUS_FS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/* ---- Sizing ------------------------------------------------------------ */

#define FS_MAX_FILES      16
#define FS_MAX_NAME_LEN   12   /* null-terminated, so 11 usable chars */
#define FS_MAX_FILE_SIZE  2048 /* bytes per file */
#define FS_TOTAL_BYTES    (FS_MAX_FILES * FS_MAX_FILE_SIZE) /* 32 KB */

/* ---- Types ------------------------------------------------------------- */

/** Opaque file handle — callers hold one of these per open file. */
typedef struct {
    uint8_t  slot;    /**< Index into internal file table (0-based). */
    bool     valid;   /**< True if handle references a live file entry. */
} fs_file_t;

/** Per-file metadata returned by fs_list(). */
typedef struct {
    char     name[FS_MAX_NAME_LEN]; /**< Null-terminated file name. */
    uint16_t size;                  /**< Current number of bytes written. */
} fs_file_info_t;

/** Aggregate filesystem statistics. */
typedef struct {
    uint32_t total_bytes; /**< Total storage capacity in bytes. */
    uint32_t used_bytes;  /**< Bytes occupied by all file data. */
    uint32_t free_bytes;  /**< Bytes available for new data. */
    uint8_t  file_count;  /**< Number of files currently stored. */
} fs_stats_t;

/* ---- API --------------------------------------------------------------- */

/**
 * @brief  Clear all file table entries and zero the data store.
 * @note   Call once before any other fs_* function.
 */
void fs_init(void);

/**
 * @brief  Create a new file with the given name.
 * @return true on success; false if the filesystem is full, the name is
 *         already in use, or the name is invalid.
 */
bool fs_create(const char *name, fs_file_t *out);

/**
 * @brief  Open an existing file by name.
 * @return true on success; false if the file does not exist.
 */
bool fs_open(const char *name, fs_file_t *out);

/**
 * @brief  Append data to an open file.
 * @return true on success; false on invalid handle, null data, or if the
 *         write would exceed FS_MAX_FILE_SIZE.
 */
bool fs_write(fs_file_t *f, const uint8_t *data, uint16_t len);

/**
 * @brief  Read bytes from an open file at a given offset.
 * @return Number of bytes actually read (may be less than len near EOF),
 *         or 0 on error.
 */
uint16_t fs_read(fs_file_t *f, uint8_t *buf, uint16_t len, uint16_t offset);

/**
 * @brief  Delete a file by name.
 * @return true on success; false if the file does not exist.
 */
bool fs_delete(const char *name);

/**
 * @brief  Enumerate all files currently in the filesystem.
 * @return Number of entries written into @p out.
 */
uint8_t fs_list(fs_file_info_t *out, uint8_t max);

/**
 * @brief  Return aggregate filesystem statistics.
 */
void fs_stats(fs_stats_t *out);

/* ---- Privileged implementations (internal — do not call directly) ------ */

void     __fs_init(void);
bool     __fs_create(const char *name, fs_file_t *out);
bool     __fs_open(const char *name, fs_file_t *out);
bool     __fs_write(fs_file_t *f, const uint8_t *data, uint16_t len);
uint16_t __fs_read(fs_file_t *f, uint8_t *buf, uint16_t len, uint16_t offset);
bool     __fs_delete(const char *name);
uint8_t  __fs_list(fs_file_info_t *out, uint8_t max);
void     __fs_stats(fs_stats_t *out);

#ifdef __cplusplus
}
#endif

#endif /* ICARUS_FS_H */
