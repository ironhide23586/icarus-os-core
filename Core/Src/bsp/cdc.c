/**
 * @file    cdc.c
 * @brief   USB CDC raw write helper implementation
 *
 * @details Calls the ST USB CDC class driver (`CDC_Transmit_FS` from
 *          `usbd_cdc_if.h`) and busy-retries on `USBD_BUSY` using
 *          `task_active_sleep(1)` between attempts. The retry budget
 *          is bounded by @ref CDC_WRITE_MAX_RETRIES so a stuck endpoint
 *          cannot block the caller forever.
 *
 *          Under HOST_TEST the ST middleware is unavailable, so the
 *          implementation is replaced by a small in-memory sink that
 *          stores the most recent payload — exposed to the unit tests
 *          via `__cdc_host_*` hooks declared in `tests/src/test_cdc.c`.
 *
 * @author  Souham Biswas
 * @date    2026
 *
 * @copyright Copyright 2025-2026 Souham Biswas
 *            https://github.com/ironhide23586/icarus-os-core
 *            Licensed under the Apache License, Version 2.0
 */

#include "bsp/cdc.h"

#include <string.h>

#ifndef HOST_TEST

#include "usbd_cdc_if.h"
#include "icarus/icarus.h"   /* task_active_sleep */

bool CDC_Write(const uint8_t *data, uint16_t len) {
    if (len == 0u) {
        return true;
    }
    if (data == NULL) {
        return false;
    }

    /* CDC_Transmit_FS takes a non-const buffer; it does not modify the
     * payload but its prototype is loose. Casting away const here is
     * safe — we own the call site contract. */
    uint8_t *buf = (uint8_t *)(uintptr_t)data;

    uint8_t  result  = CDC_Transmit_FS(buf, len);
    uint16_t retries = 0u;
    while (result == USBD_BUSY && retries < CDC_WRITE_MAX_RETRIES) {
        task_active_sleep(1u);
        result = CDC_Transmit_FS(buf, len);
        retries++;
    }
    return (result == USBD_OK);
}

#else /* HOST_TEST — in-memory sink */

#define CDC_HOST_SINK_SIZE  256u

static uint8_t  g_cdc_sink[CDC_HOST_SINK_SIZE];
static uint16_t g_cdc_sink_len;
static uint32_t g_cdc_call_count;
static bool     g_cdc_force_fail;

bool CDC_Write(const uint8_t *data, uint16_t len) {
    g_cdc_call_count++;
    if (g_cdc_force_fail) {
        return false;
    }
    if (len == 0u) {
        return true;
    }
    if (data == NULL) {
        return false;
    }
    if (len > CDC_HOST_SINK_SIZE) {
        len = CDC_HOST_SINK_SIZE;
    }
    memcpy(g_cdc_sink, data, len);
    g_cdc_sink_len = len;
    return true;
}

/* ---- Test-only host hooks ---------------------------------------------- */

const uint8_t *__cdc_host_sink(void) {
    return g_cdc_sink;
}

uint16_t __cdc_host_sink_len(void) {
    return g_cdc_sink_len;
}

uint32_t __cdc_host_call_count(void) {
    return g_cdc_call_count;
}

void __cdc_host_set_fail(bool fail) {
    g_cdc_force_fail = fail;
}

void __cdc_host_reset_state(void) {
    memset(g_cdc_sink, 0, sizeof(g_cdc_sink));
    g_cdc_sink_len   = 0u;
    g_cdc_call_count = 0u;
    g_cdc_force_fail = false;
}

#endif /* HOST_TEST */

bool CDC_WriteString(const char *s) {
    if (s == NULL) {
        return true;
    }
    return CDC_Write((const uint8_t *)s, (uint16_t)strlen(s));
}
