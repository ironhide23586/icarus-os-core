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

void     cdc_rx_init(void);
void     cdc_rx_push(const uint8_t *data, uint32_t len);
bool     cdc_rx_read_byte(uint8_t *out);
uint32_t cdc_rx_available(void);

#ifdef __cplusplus
}
#endif

#endif /* ICARUS_CDC_RX_H */
