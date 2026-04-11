/**
 * @file    cdc_rx.h
 * @brief   USB CDC receive ring buffer
 *
 * @details Single-producer (USB ISR) / single-consumer (consumer task)
 *          lock-free ring buffer for incoming serial data over the
 *          kernel's USB CDC class. Producer is the CDC_Receive_FS
 *          callback (ISR context); consumer is any RTOS task that
 *          wants to drain bytes.
 *
 * @author  Souham Biswas
 * @date    2026
 *
 * @copyright Copyright 2025-2026 Souham Biswas
 *            https://github.com/ironhide23586/icarus-os-core
 *            Licensed under the Apache License, Version 2.0
 */

#ifndef ICARUS_CDC_RX_H
#define ICARUS_CDC_RX_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#define CDC_RX_BUF_SIZE 512

/* ---- Public API (issue SVC into priv mode on target) ------------------- */

void     cdc_rx_init(void);
bool     cdc_rx_read_byte(uint8_t *out);
uint32_t cdc_rx_available(void);

/* ---- Producer entry point ---------------------------------------------- */

/**
 * @brief  Append bytes to the ring buffer.
 *
 * @note   Designed for the privileged USB CDC ISR. The wrapper calls the
 *         priv-mode implementation directly (no SVC) because issuing an
 *         SVC from a high-priority interrupt is unsafe. Thread-mode
 *         callers can use it too — the priv-mode access works because
 *         the wrapper itself runs in handler / privileged context when
 *         called from an ISR; on host tests it is just a direct call.
 */
void     cdc_rx_push(const uint8_t *data, uint32_t len);

/* ---- Privileged implementations (internal — do not call directly) ----- */

void     __cdc_rx_init(void);
void     __cdc_rx_push(const uint8_t *data, uint32_t len);
bool     __cdc_rx_read_byte(uint8_t *out);
uint32_t __cdc_rx_available(void);

#ifdef __cplusplus
}
#endif

#endif /* ICARUS_CDC_RX_H */
