/**
 * @file    crc.c
 * @brief   CRC16-CCITT implementation
 *
 * @details On STM32H7 target, drives the on-chip CRC peripheral with the
 *          CCITT polynomial (0x1021), 16-bit polynomial size, no input
 *          reversal, no output reversal, and an initial value of 0xFFFF.
 *          The peripheral is lazy-initialised on first call: the CRC
 *          clock is enabled on the AHB4 bus and the registers are
 *          configured once.
 *
 *          Hardware CRC consumes one AHB cycle per byte and frees the
 *          CPU for the duration of the transfer; for the buffer sizes
 *          used by the kvstore and table services this is roughly 4×
 *          faster than the bytewise software loop.
 *
 *          The peripheral has internal state (the running CRC value),
 *          so every public call is wrapped in a critical section to
 *          prevent two tasks corrupting each other's computation.
 *
 *          Under HOST_TEST a portable bytewise loop is used so unit
 *          tests run unchanged off-target.
 *
 * @author  Souham Biswas
 * @date    2026
 *
 * @copyright Copyright 2025-2026 Souham Biswas
 *            https://github.com/ironhide23586/icarus-os-core
 *            Licensed under the Apache License, Version 2.0
 */

#include "icarus/crc.h"
#include "icarus/config.h"
#include <stddef.h>

#ifndef HOST_TEST

#include "icarus/kernel.h"   /* enter_critical / exit_critical */
#include "stm32h7xx.h"

/* CRC->CR field encodings (from RM0433):
 *   bit 0      RESET     — write 1 to reload INIT into the data register
 *   bits 4:3   POLYSIZE  — 00=32, 01=16, 10=8, 11=7
 *   bits 6:5   REV_IN    — 00 = no input bit/byte reversal
 *   bit 7      REV_OUT   — 0  = no output bit reversal
 */
#define CRC_CR_POLYSIZE_16   (0x1u << CRC_CR_POLYSIZE_Pos)

static volatile uint8_t crc_initialised;

ITCM_FUNC static void crc_hw_init(void) {
    /* Enable the CRC peripheral clock on AHB4. The dummy read settles
     * the bus per the H7 reference manual recommendation. */
    RCC->AHB4ENR |= RCC_AHB4ENR_CRCEN;
    (void)RCC->AHB4ENR;

    /* Configure poly = 0x1021, init = 0xFFFF, 16-bit polysize,
     * no reversal. */
    CRC->POL  = 0x1021u;
    CRC->INIT = 0xFFFFu;
    CRC->CR   = CRC_CR_POLYSIZE_16;

    crc_initialised = 1u;
}

ITCM_FUNC uint16_t crc16_ccitt(const uint8_t *data, uint16_t len) {
    if (data == NULL || len == 0) {
        return 0xFFFFu;
    }

    __enter_critical();

    if (!crc_initialised) {
        crc_hw_init();
    }

    /* Reset the running value back to INIT (0xFFFF) before this run. */
    CRC->CR |= CRC_CR_RESET;

    /* Feed bytes one at a time. The peripheral allows 8/16/32-bit
     * accesses to DR; an 8-bit write consumes one byte regardless of
     * word alignment. */
    volatile uint8_t *const dr8 = (volatile uint8_t *)&CRC->DR;
    for (uint16_t i = 0; i < len; i++) {
        *dr8 = data[i];
    }

    uint16_t crc = (uint16_t)(CRC->DR & 0xFFFFu);

    __exit_critical();
    return crc;
}

#else /* HOST_TEST — portable bytewise loop */

uint16_t crc16_ccitt(const uint8_t *data, uint16_t len) {
    if (data == NULL || len == 0) {
        return 0xFFFFu;
    }
    uint16_t crc = 0xFFFFu;
    for (uint16_t i = 0; i < len; i++) {
        crc ^= (uint16_t)data[i] << 8;
        for (uint8_t bit = 0; bit < 8; bit++) {
            if (crc & 0x8000u) {
                crc = (uint16_t)((crc << 1) ^ 0x1021u);
            } else {
                crc = (uint16_t)(crc << 1);
            }
        }
    }
    return crc;
}

#endif /* HOST_TEST */
