/**
 * @file    crc.h
 * @brief   CRC16-CCITT helper for the ICARUS OS kernel
 *
 * @details Polynomial 0x1021, initial value 0xFFFF, no reflection,
 *          no final XOR. Suitable for CCSDS Space Packet integrity
 *          checks and small flash record protection.
 *
 * @author  Souham Biswas
 * @date    2026
 *
 * @copyright Copyright 2025-2026 Souham Biswas
 *            https://github.com/ironhide23586/icarus-os-core
 *            Licensed under the Apache License, Version 2.0
 */

#ifndef ICARUS_CRC_H
#define ICARUS_CRC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
 * @brief  Compute CRC16-CCITT (poly 0x1021, init 0xFFFF) over a byte buffer.
 * @param  data Buffer to checksum (may be NULL only if @p len is 0).
 * @param  len  Number of bytes in @p data.
 * @return The 16-bit CRC value.
 */
uint16_t crc16_ccitt(const uint8_t *data, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /* ICARUS_CRC_H */
