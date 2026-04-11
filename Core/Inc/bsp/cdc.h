/**
 * @file    bsp_cdc.h
 * @brief   Board Support Package — USB CDC raw write helper
 *
 * @details Thin wrapper around the ST USB CDC class driver
 *          (`CDC_Transmit_FS`) that hides the `USBD_BUSY` retry loop
 *          and the `usbd_cdc_if.h` include behind a clean BSP API.
 *
 *          The kernel already retargets `printf()` to USB CDC via
 *          `bsp/retarget_stdio.c`; this header is for callers that
 *          want to push raw bytes (typically a shell that needs to
 *          echo characters or write a prompt without going through
 *          stdio's line buffering).
 *
 *          Producer thread: any RTOS task. The function blocks via
 *          `task_active_sleep(1)` between retry attempts when the CDC
 *          IN endpoint is busy, so it must not be called from an ISR
 *          (use the printf retarget for ISR-context output, or queue
 *          the bytes onto a pipe for a worker task to drain).
 *
 * @author  Souham Biswas
 * @date    2026
 *
 * @copyright Copyright 2025-2026 Souham Biswas
 *            https://github.com/ironhide23586/icarus-os-core
 *            Licensed under the Apache License, Version 2.0
 */

#ifndef BSP_CDC_H
#define BSP_CDC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief  Maximum number of busy-retry attempts inside `CDC_Write`.
 *
 * @details Each retry sleeps one OS tick (`task_active_sleep(1)`), so
 *          the upper bound on a single `CDC_Write` call is roughly
 *          @ref CDC_WRITE_MAX_RETRIES ticks. Default 500 → about
 *          0.5 s at the kernel's 1 ms tick.
 */
#define CDC_WRITE_MAX_RETRIES   500u

/**
 * @brief  Push @p len bytes to the USB CDC IN endpoint.
 *
 * @param  data  Pointer to the bytes to send (must be non-NULL when
 *               @p len > 0).
 * @param  len   Number of bytes to send.
 *
 * @return `true` if the bytes were accepted by the CDC stack within
 *         @ref CDC_WRITE_MAX_RETRIES busy-retries; `false` otherwise
 *         (caller's bytes were *not* sent).
 *
 * @note   Must be called from task context — the busy-retry path uses
 *         `task_active_sleep(1)` and would deadlock from an ISR.
 *
 * @note   The transmit buffer the ST middleware accepts is borrowed,
 *         not copied. The pointer @p data must remain valid until the
 *         function returns.
 */
bool CDC_Write(const uint8_t *data, uint16_t len);

/**
 * @brief  Convenience wrapper that pushes a NUL-terminated C string.
 *
 * @param  s  NUL-terminated string. NULL is treated as a no-op success.
 *
 * @return `true` on success, `false` if the underlying `CDC_Write`
 *         could not flush within the retry budget.
 */
bool CDC_WriteString(const char *s);

#ifdef __cplusplus
}
#endif

#endif /* BSP_CDC_H */
