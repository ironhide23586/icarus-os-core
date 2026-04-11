/**
 * @file    cs.c
 * @brief   Background Checksum integrity monitor implementation
 * @version 0.1.0
 *
 * @details Maintains a table of up to CS_MAX_REGIONS memory regions,
 *          each with a baseline CRC16-CCITT.  cs_check_all() recomputes
 *          the CRC over each enabled region and invokes the mismatch
 *          callback on any discrepancy.
 *
 *          CRC computation uses crc16_ccitt() from icarus/crc.h, which
 *          runs on the STM32H7 hardware CRC peripheral on target and
 *          falls back to a software loop under HOST_TEST.
 *
 *          cs_init() performs a self-test of the CRC engine against a
 *          known test vector ("123456789" → 0x29B1 for CRC16-CCITT).
 *          If the self-test fails (e.g. HW CRC peripheral damaged by
 *          radiation), the cs_hw_ok flag is cleared and cs_check_all()
 *          reports region index 0xFF via the mismatch callback to
 *          signal a CRC engine failure.
 *
 * @par Memory placement:
 *      - Region table + state: DTCM_DATA_PRIV
 *      - All functions:        ITCM_FUNC
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

/* ============================================================================
 * INTERNAL STATE (DTCM)
 * ========================================================================= */

/**
 * @brief  Per-region entry in the monitor table.
 */
typedef struct {
    const uint8_t *addr;     /**< Start address of the monitored region.  */
    uint32_t       size;     /**< Size in bytes.                          */
    uint16_t       baseline; /**< Expected CRC16-CCITT.                   */
    bool           enabled;  /**< True if actively scanned.               */
    bool           registered; /**< True if this slot has been configured. */
} cs_entry_t;

/** @brief Region table — privileged DTCM, zero wait-state. */
DTCM_DATA_PRIV static cs_entry_t  regions[CS_MAX_REGIONS];

/** @brief User-supplied mismatch callback. */
DTCM_DATA_PRIV static cs_mismatch_fn mismatch_cb;

/** @brief CRC engine self-test result.  False if HW CRC is suspect. */
DTCM_DATA_PRIV static bool cs_hw_ok;

/**
 * @brief  Known CRC16-CCITT test vector.
 * @details "123456789" (9 bytes) → CRC = 0x29B1 per ITU-T V.41.
 */
#define CS_SELFTEST_EXPECTED  0x29B1u

/* ============================================================================
 * INTERNAL HELPERS (ITCM)
 * ========================================================================= */

/**
 * @brief  Run the CRC engine self-test against a known vector.
 * @retval true   CRC engine produces the expected result.
 * @retval false  CRC engine is damaged or misconfigured.
 */
ITCM_FUNC static bool cs_selftest(void) {
    static const uint8_t vec[] = { '1','2','3','4','5','6','7','8','9' };
    uint16_t result = crc16_ccitt(vec, 9);
    return (result == CS_SELFTEST_EXPECTED);
}

/* ============================================================================
 * PRIVILEGED IMPLEMENTATIONS
 * ========================================================================= */

/**
 * @brief  Privileged implementation of cs_init().
 * @details Clears all regions, runs the CRC self-test, and stores the
 *          result in cs_hw_ok.
 */
ITCM_FUNC void __cs_init(void) {
    memset(regions, 0, sizeof(regions));
    mismatch_cb = NULL;
    cs_hw_ok = cs_selftest();
}

/**
 * @brief  Privileged implementation of cs_set_callback().
 * @param[in] fn  Mismatch callback, or NULL to disable.
 */
ITCM_FUNC void __cs_set_callback(cs_mismatch_fn fn) {
    mismatch_cb = fn;
}

/**
 * @brief  Privileged implementation of cs_add_region().
 *
 * @param[in] idx   Region index.
 * @param[in] addr  Start address.
 * @param[in] size  Size in bytes.
 * @retval true   Region registered and baseline CRC computed.
 * @retval false  Invalid parameters.
 *
 * @details Computes the baseline CRC immediately from the current
 *          memory contents using crc16_ccitt() (HW-accelerated on
 *          target, software fallback under HOST_TEST).
 */
ITCM_FUNC bool __cs_add_region(uint8_t idx, const uint8_t *addr,
                                uint32_t size) {
    if (idx >= CS_MAX_REGIONS || !addr || size == 0) return false;

    regions[idx].addr       = addr;
    regions[idx].size       = size;
    regions[idx].baseline   = crc16_ccitt(addr, (uint16_t)size);
    regions[idx].enabled    = true;
    regions[idx].registered = true;
    return true;
}

/**
 * @brief  Privileged implementation of cs_enable().
 * @param[in] idx      Region index.
 * @param[in] enabled  Enable/disable flag.
 */
ITCM_FUNC bool __cs_enable(uint8_t idx, bool enabled) {
    if (idx >= CS_MAX_REGIONS || !regions[idx].registered) return false;
    regions[idx].enabled = enabled;
    return true;
}

/**
 * @brief  Privileged implementation of cs_rebaseline().
 * @param[in] idx  Region index.
 * @details Recomputes the CRC from current memory and stores it as
 *          the new baseline.  Call after a legitimate write to a
 *          monitored region.
 */
ITCM_FUNC bool __cs_rebaseline(uint8_t idx) {
    if (idx >= CS_MAX_REGIONS || !regions[idx].registered) return false;
    regions[idx].baseline = crc16_ccitt(regions[idx].addr,
                                        (uint16_t)regions[idx].size);
    return true;
}

/**
 * @brief  Privileged implementation of cs_check_all().
 * @return Number of regions that failed the CRC check.
 *
 * @details First checks cs_hw_ok; if the CRC engine itself is suspect,
 *          invokes the callback with region_idx = 0xFF (sentinel) and
 *          returns 1 without scanning any regions.
 *
 *          Otherwise iterates all enabled regions, recomputes CRC, and
 *          invokes the callback for each mismatch.
 */
ITCM_FUNC uint8_t __cs_check_all(void) {
    /* CRC engine self-test failure — report immediately */
    if (!cs_hw_ok) {
        if (mismatch_cb) {
            mismatch_cb(0xFF, 0, 0);
        }
        return 1;
    }

    uint8_t failures = 0;
    for (uint8_t i = 0; i < CS_MAX_REGIONS; i++) {
        if (!regions[i].registered || !regions[i].enabled) continue;

        uint16_t actual = crc16_ccitt(regions[i].addr,
                                       (uint16_t)regions[i].size);
        if (actual != regions[i].baseline) {
            failures++;
            if (mismatch_cb) {
                mismatch_cb(i, regions[i].baseline, actual);
            }
        }
    }
    return failures;
}

/**
 * @brief  Privileged implementation of cs_get_region().
 * @param[in]  idx  Region index.
 * @param[out] out  Destination descriptor.
 */
ITCM_FUNC bool __cs_get_region(uint8_t idx, cs_region_t *out) {
    if (idx >= CS_MAX_REGIONS || !out) return false;
    if (!regions[idx].registered) return false;
    out->addr     = regions[idx].addr;
    out->size     = regions[idx].size;
    out->baseline = regions[idx].baseline;
    out->enabled  = regions[idx].enabled;
    return true;
}

/**
 * @brief  Privileged implementation of cs_region_count().
 * @return Number of registered (non-empty) region slots.
 */
ITCM_FUNC uint8_t __cs_region_count(void) {
    uint8_t n = 0;
    for (uint8_t i = 0; i < CS_MAX_REGIONS; i++) {
        if (regions[i].registered) n++;
    }
    return n;
}

/* ============================================================================
 * PUBLIC API (critical-section wrappers)
 * ========================================================================= */

/** @copydoc cs_init */
void cs_init(void) {
    enter_critical();
    __cs_init();
    exit_critical();
}

/** @copydoc cs_set_callback */
void cs_set_callback(cs_mismatch_fn fn) {
    enter_critical();
    __cs_set_callback(fn);
    exit_critical();
}

/** @copydoc cs_add_region */
bool cs_add_region(uint8_t idx, const uint8_t *addr, uint32_t size) {
    enter_critical();
    bool ok = __cs_add_region(idx, addr, size);
    exit_critical();
    return ok;
}

/** @copydoc cs_enable */
bool cs_enable(uint8_t idx, bool enabled) {
    enter_critical();
    bool ok = __cs_enable(idx, enabled);
    exit_critical();
    return ok;
}

/** @copydoc cs_rebaseline */
bool cs_rebaseline(uint8_t idx) {
    enter_critical();
    bool ok = __cs_rebaseline(idx);
    exit_critical();
    return ok;
}

/** @copydoc cs_check_all */
uint8_t cs_check_all(void) {
    enter_critical();
    uint8_t n = __cs_check_all();
    exit_critical();
    return n;
}

/** @copydoc cs_get_region */
bool cs_get_region(uint8_t idx, cs_region_t *out) {
    enter_critical();
    bool ok = __cs_get_region(idx, out);
    exit_critical();
    return ok;
}

/** @copydoc cs_region_count */
uint8_t cs_region_count(void) {
    enter_critical();
    uint8_t n = __cs_region_count();
    exit_critical();
    return n;
}
