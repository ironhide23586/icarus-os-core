/**
 * @file    iwdg.c
 * @brief   Independent Watchdog (IWDG1) implementation
 *
 * @details Direct CMSIS register access — the HAL IWDG module is left
 *          disabled in `stm32h7xx_hal_conf.h` to avoid pulling in the
 *          full HAL_IWDG state machine for what amounts to four register
 *          writes.
 *
 *          Register cheat-sheet (RM0433 §50):
 *
 *            IWDG1->KR    write key register
 *                         0xCCCC = enable
 *                         0xAAAA = refresh (reload counter from RLR)
 *                         0x5555 = unlock PR/RLR/WINR for write access
 *
 *            IWDG1->PR    3-bit prescaler field [0..6] → div 4..256
 *            IWDG1->RLR   12-bit reload value [1..4095]
 *            IWDG1->SR    status: PVU (PR write pending), RVU (RLR pending),
 *                                 WVU (WINR pending) — must be 0 before
 *                                 we touch PR/RLR
 *
 *          The reset-reason side reads RCC->RSR (System Reset Status &
 *          Control Register on H7) — bit IWDG1RSTF tells us the previous
 *          boot was triggered by IWDG1. Cleared via the RMVF bit.
 *
 * @author  Souham Biswas
 * @date    2026
 *
 * @copyright Copyright 2025-2026 Souham Biswas
 *            https://github.com/ironhide23586/icarus-os-core
 *            Licensed under the Apache License, Version 2.0
 */

#include "bsp/iwdg.h"

#ifndef HOST_TEST
#include "stm32h7xx.h"

#define IWDG_KEY_ENABLE   0xCCCCu
#define IWDG_KEY_REFRESH  0xAAAAu
#define IWDG_KEY_UNLOCK   0x5555u

void IWDG_Init(uint8_t prescaler, uint16_t reload) {
    if (prescaler > IWDG_PRESCALER_256) {
        prescaler = IWDG_PRESCALER_256;
    }
    if (reload == 0u) {
        reload = 1u;
    } else if (reload > 0x0FFFu) {
        reload = 0x0FFFu;
    }

    /* Unlock PR/RLR for writing. */
    IWDG1->KR = IWDG_KEY_UNLOCK;

    /* Wait until any pending write to PR has propagated, then program. */
    while ((IWDG1->SR & IWDG_SR_PVU) != 0u) { }
    IWDG1->PR = prescaler;

    /* Same for RLR. */
    while ((IWDG1->SR & IWDG_SR_RVU) != 0u) { }
    IWDG1->RLR = reload;

    /* Reload + enable. The first KR=0xAAAA also locks PR/RLR back. */
    IWDG1->KR = IWDG_KEY_REFRESH;
    IWDG1->KR = IWDG_KEY_ENABLE;
}

void IWDG_Refresh(void) {
    IWDG1->KR = IWDG_KEY_REFRESH;
}

bool IWDG_WasReset(void) {
    return (RCC->RSR & RCC_RSR_IWDG1RSTF) != 0u;
}

void IWDG_ClearResetFlag(void) {
    /* RMVF: writing 1 clears all reset reason flags in RCC->RSR. */
    RCC->RSR |= RCC_RSR_RMVF;
}

#else /* HOST_TEST — software model so unit tests can exercise the API */

#include <stdint.h>

static uint8_t  g_iwdg_started;
static uint32_t g_iwdg_refresh_count;
static bool     g_iwdg_was_reset;

void IWDG_Init(uint8_t prescaler, uint16_t reload) {
    (void)prescaler;
    (void)reload;
    g_iwdg_started       = 1u;
    g_iwdg_refresh_count = 0u;
}

void IWDG_Refresh(void) {
    if (g_iwdg_started != 0u) {
        g_iwdg_refresh_count++;
    }
}

bool IWDG_WasReset(void) {
    return g_iwdg_was_reset;
}

void IWDG_ClearResetFlag(void) {
    g_iwdg_was_reset = false;
}

/* ---- Test-only host hooks (not in iwdg.h; declared in test files) ----- */

uint32_t __iwdg_host_refresh_count(void) {
    return g_iwdg_refresh_count;
}

void __iwdg_host_set_reset_flag(bool value) {
    g_iwdg_was_reset = value;
}

uint8_t __iwdg_host_started(void) {
    return g_iwdg_started;
}

void __iwdg_host_reset_state(void) {
    g_iwdg_started       = 0u;
    g_iwdg_refresh_count = 0u;
    g_iwdg_was_reset     = false;
}

#endif /* HOST_TEST */
