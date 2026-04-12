/**
 * @file    cdc_rx.c
 * @brief   USB CDC receive ring buffer implementation
 *
 * @details SPSC (single-producer single-consumer) ring buffer.
 *          Producer: CDC_Receive_FS callback (USB interrupt context).
 *          Consumer: any RTOS task draining bytes.
 *
 *          The buffer and indices live in DTCM_DATA_PRIV (privileged-only
 *          tightly-coupled memory). Public read accessors go through SVC
 *          gates (\c SVC_CDC_RX_READ_BYTE / \c SVC_CDC_RX_AVAILABLE) so
 *          unprivileged thread-mode tasks can use them once the MPU is
 *          locked. The producer call site is the USB CDC ISR, which is
 *          already in privileged handler context — it calls the
 *          \c __cdc_rx_push privileged variant directly to avoid issuing
 *          an SVC from a high-priority interrupt.
 *
 * @author  Souham Biswas
 * @date    2026
 *
 * @copyright Copyright 2025-2026 Souham Biswas
 *            https://github.com/ironhide23586/icarus-os-core
 *            Licensed under the Apache License, Version 2.0
 */

#include "icarus/cdc_rx.h"
#include "icarus/config.h"
#include <stddef.h>

/* ---- Backing storage in DTCM_PRIV --------------------------------------- */

DTCM_DATA_PRIV static volatile uint8_t  rx_buf[CDC_RX_BUF_SIZE];
DTCM_DATA_PRIV static volatile uint32_t rx_head;
DTCM_DATA_PRIV static volatile uint32_t rx_tail;

/* ---- Privileged implementations (run in priv mode, ITCM hot path) ------ */

ITCM_FUNC void __cdc_rx_init(void) {
    rx_head = 0;
    rx_tail = 0;
}

ITCM_FUNC void __cdc_rx_push(const uint8_t *data, uint32_t len) {
    if ((data == NULL) || (len == 0u)) {
        return;
    }
    for (uint32_t i = 0u; i < len; i++) {
        uint32_t next = (rx_head + 1u) % (uint32_t)CDC_RX_BUF_SIZE;
        if (next == rx_tail) {
            break;
        }
        rx_buf[rx_head] = data[i];
        rx_head = next;
    }
}

ITCM_FUNC bool __cdc_rx_read_byte(uint8_t *out) {
    if ((out == NULL) || (rx_head == rx_tail)) {
        return false;
    }
    *out = rx_buf[rx_tail];
    rx_tail = (rx_tail + 1u) % (uint32_t)CDC_RX_BUF_SIZE;
    return true;
}

ITCM_FUNC uint32_t __cdc_rx_available(void) {
    uint32_t h = rx_head;
    uint32_t t = rx_tail;
    if (h >= t) {
        return h - t;
    }
    return ((uint32_t)CDC_RX_BUF_SIZE - t) + h;
}
