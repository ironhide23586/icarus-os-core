/**
 * @file    bsp_iwdg.h
 * @brief   Board Support Package — Independent Watchdog (IWDG1)
 *
 * @details Thin wrapper around the STM32H7 Independent Watchdog peripheral
 *          (IWDG1). Provides init / refresh / reset-reason helpers without
 *          pulling in `HAL_IWDG_*` (the HAL IWDG module is intentionally
 *          left disabled to keep the BSP footprint small — direct register
 *          access through CMSIS is sufficient).
 *
 *          The IWDG runs from the internal LSI (≈ 32 kHz on H7); the
 *          watchdog timeout is `(reload + 1) × prescaler / LSI`. Typical
 *          flight settings: prescaler = IWDG_PRESCALER_64 (6), reload =
 *          1000 → timeout ≈ 2 s.
 *
 * @par Sequence to start the watchdog:
 *      1. `IWDG_Init(prescaler, reload)`  — unlocks PR/RLR, programs them,
 *                                            kicks the start key
 *      2. `IWDG_Refresh()`                — call from a high-priority
 *                                            "feed" task before timeout
 *
 * @par Reset-reason inspection:
 *      The kernel BSP also exposes `IWDG_WasReset()` and
 *      `IWDG_ClearResetFlag()` so application code can detect a watchdog
 *      reset on the previous boot without poking RCC->RSR directly.
 *
 * @author  Souham Biswas
 * @date    2026
 *
 * @copyright Copyright 2025-2026 Souham Biswas
 *            https://github.com/ironhide23586/icarus-os-core
 *            Licensed under the Apache License, Version 2.0
 */

#ifndef BSP_IWDG_H
#define BSP_IWDG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/* ---- Prescaler encodings (PR register field) ---------------------------- */

#define IWDG_PRESCALER_4    0u
#define IWDG_PRESCALER_8    1u
#define IWDG_PRESCALER_16   2u
#define IWDG_PRESCALER_32   3u
#define IWDG_PRESCALER_64   4u
#define IWDG_PRESCALER_128  5u
#define IWDG_PRESCALER_256  6u

/**
 * @brief  Configure and start the independent watchdog.
 *
 * @param  prescaler  One of the `IWDG_PRESCALER_*` values (3-bit PR field).
 * @param  reload     12-bit reload value (1..4095). Total timeout is
 *                    `(reload + 1) × prescaler_div / LSI`.
 *
 * @note   Once started the IWDG cannot be stopped without a system reset.
 *         The function is idempotent — calling it twice with the same
 *         arguments is safe.
 */
void IWDG_Init(uint8_t prescaler, uint16_t reload);

/**
 * @brief  Reload the watchdog counter ("kick" / "feed").
 *
 * @note   Must be called periodically — at least once per timeout window —
 *         or the chip will reset.
 */
void IWDG_Refresh(void);

/**
 * @brief  Was the most recent reset caused by the IWDG?
 *
 * @return `true` if the IWDGRST flag in RCC->RSR was set on this boot,
 *         `false` otherwise. The flag is sticky until cleared with
 *         `IWDG_ClearResetFlag()`, so this query is safe to call any
 *         number of times.
 */
bool IWDG_WasReset(void);

/**
 * @brief  Clear the IWDG reset flag in RCC->RSR.
 *
 * @note   Safe to call unconditionally; clears the flag even if it was
 *         not set. Typically called once at boot after the application
 *         has logged the reset reason.
 */
void IWDG_ClearResetFlag(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_IWDG_H */
