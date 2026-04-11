/**
 * @file    cs.h
 * @brief   ICARUS OS — Background Checksum integrity monitor
 * @version 0.1.0
 *
 * @details Periodically computes CRC16-CCITT over registered memory
 *          regions and compares against baseline values captured at
 *          startup (or set by the application).  Mismatches are
 *          reported through a user-supplied callback.
 *
 *          Typical use: monitor flash code segments, critical data
 *          tables, and SRAM guard patterns to detect bit-flips from
 *          radiation (SEU) or software corruption.
 *
 * @par Memory placement:
 *      - Region table: DTCM_DATA_PRIV (privileged-only, zero wait-state)
 *      - Functions:    ITCM_FUNC (zero wait-state instruction fetch)
 *
 * @par Thread safety:
 *      All public functions use enter_critical() / exit_critical().
 *      cs_check_all() is designed to be called from a periodic task
 *      (e.g. 1 Hz from the FDIR monitor loop).
 *
 * @see     icarus/crc.h for the underlying CRC16-CCITT implementation
 *
 * @author  Souham Biswas
 * @date    2026
 *
 * @copyright Copyright 2025-2026 Souham Biswas
 *            https://github.com/ironhide23586/icarus-os-core
 *            Licensed under the Apache License, Version 2.0
 */

#ifndef ICARUS_CS_H
#define ICARUS_CS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/* ============================================================================
 * CONFIGURATION
 * ========================================================================= */

/**
 * @defgroup CS_CONFIG Checksum Configuration
 * @brief    Compile-time sizing parameters
 * @{
 */

/**
 * @brief Maximum number of monitored memory regions.
 * @note  Each entry uses ~20 bytes in DTCM.
 */
#ifndef CS_MAX_REGIONS
#define CS_MAX_REGIONS  8
#endif

/** @} */

/* ============================================================================
 * TYPES
 * ========================================================================= */

/**
 * @brief  Mismatch callback signature.
 *
 * @param[in] region_idx  Index of the region that failed (0-based).
 * @param[in] expected    Baseline CRC that was stored at registration.
 * @param[in] actual      CRC computed during the latest scan.
 *
 * @details The callback runs inside the critical section of
 *          cs_check_all().  Keep it short — typically just a
 *          fault injection call.
 */
typedef void (*cs_mismatch_fn)(uint8_t region_idx, uint16_t expected,
                               uint16_t actual);

/**
 * @brief  Descriptor for a monitored memory region.
 */
typedef struct {
    const uint8_t *addr;     /**< Start address of the region.            */
    uint32_t       size;     /**< Size in bytes.                          */
    uint16_t       baseline; /**< Expected CRC16-CCITT value.             */
    bool           enabled;  /**< True if this region is actively scanned.*/
} cs_region_t;

/* ============================================================================
 * PUBLIC API
 * ========================================================================= */

/**
 * @brief  Initialize the checksum monitor.  Clears all regions and sets
 *         the mismatch callback to NULL.
 */
void cs_init(void);

/**
 * @brief  Register the mismatch callback.
 *
 * @param[in] fn  Function to call when a CRC mismatch is detected.
 *                Pass NULL to disable mismatch reporting.
 */
void cs_set_callback(cs_mismatch_fn fn);

/**
 * @brief  Add a memory region to the monitor.
 *
 * @param[in] idx   Region index (0 .. CS_MAX_REGIONS-1).
 * @param[in] addr  Start address of the region.
 * @param[in] size  Size in bytes (must be > 0).
 *
 * @retval true   Region registered; baseline CRC computed and stored.
 * @retval false  Invalid index, NULL addr, or zero size.
 *
 * @note   The baseline CRC is computed immediately from the current
 *         memory contents.  Call this after the region is fully
 *         initialized (e.g. after flash boot or table activation).
 */
bool cs_add_region(uint8_t idx, const uint8_t *addr, uint32_t size);

/**
 * @brief  Enable or disable scanning of a region.
 *
 * @param[in] idx      Region index.
 * @param[in] enabled  True to enable, false to disable.
 * @retval true   Success.
 * @retval false  Invalid index.
 */
bool cs_enable(uint8_t idx, bool enabled);

/**
 * @brief  Re-compute and store the baseline CRC for a region.
 *
 * @param[in] idx  Region index.
 * @retval true   Baseline updated.
 * @retval false  Invalid index or region not registered.
 *
 * @note   Call after a legitimate write to a monitored region
 *         (e.g. table activation) to prevent false mismatch reports.
 */
bool cs_rebaseline(uint8_t idx);

/**
 * @brief  Scan all enabled regions and invoke the mismatch callback
 *         for any CRC failures.
 *
 * @return Number of regions that failed the CRC check.
 *
 * @note   Designed to be called periodically from a low-priority task
 *         (e.g. 1 Hz from the FDIR monitor loop).
 */
uint8_t cs_check_all(void);

/**
 * @brief  Read back a region's configuration.
 *
 * @param[in]  idx  Region index.
 * @param[out] out  Destination for the region descriptor.
 * @retval true   Success.
 * @retval false  Invalid index or out is NULL.
 */
bool cs_get_region(uint8_t idx, cs_region_t *out);

/**
 * @brief  Return the number of registered (non-empty) regions.
 */
uint8_t cs_region_count(void);

/* ============================================================================
 * PRIVILEGED IMPLEMENTATIONS (Internal — Do Not Call Directly)
 * ========================================================================= */

void    __cs_init(void);
void    __cs_set_callback(cs_mismatch_fn fn);
bool    __cs_add_region(uint8_t idx, const uint8_t *addr, uint32_t size);
bool    __cs_enable(uint8_t idx, bool enabled);
bool    __cs_rebaseline(uint8_t idx);
uint8_t __cs_check_all(void);
bool    __cs_get_region(uint8_t idx, cs_region_t *out);
uint8_t __cs_region_count(void);

#ifdef __cplusplus
}
#endif

#endif /* ICARUS_CS_H */
