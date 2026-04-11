/**
 * @file    cdc_rx.c
 * @brief   USB CDC receive ring buffer implementation
 *
 * @details SPSC (single-producer single-consumer) ring buffer.
 *          Producer: CDC_Receive_FS callback (USB interrupt context).
 *          Consumer: any RTOS task draining bytes.
 *          No locking needed — aligned 32-bit index writes are atomic
 *          on Cortex-M7.
 *
 * @author  Souham Biswas
 * @date    2026
 *
 * @copyright Copyright 2025-2026 Souham Biswas
 *            https://github.com/ironhide23586/icarus-os-core
 *            Licensed under the Apache License, Version 2.0
 */

#include "icarus/cdc_rx.h"

static volatile uint8_t  rx_buf[CDC_RX_BUF_SIZE];
static volatile uint32_t rx_head;
static volatile uint32_t rx_tail;

void cdc_rx_init(void) {
    rx_head = 0;
    rx_tail = 0;
}

void cdc_rx_push(const uint8_t *data, uint32_t len) {
    for (uint32_t i = 0; i < len; i++) {
        uint32_t next = (rx_head + 1) % CDC_RX_BUF_SIZE;
        if (next == rx_tail) {
            break;
        }
        rx_buf[rx_head] = data[i];
        rx_head = next;
    }
}

bool cdc_rx_read_byte(uint8_t *out) {
    if (rx_head == rx_tail) {
        return false;
    }
    *out = rx_buf[rx_tail];
    rx_tail = (rx_tail + 1) % CDC_RX_BUF_SIZE;
    return true;
}

uint32_t cdc_rx_available(void) {
    uint32_t h = rx_head;
    uint32_t t = rx_tail;
    if (h >= t) return h - t;
    return CDC_RX_BUF_SIZE - t + h;
}
