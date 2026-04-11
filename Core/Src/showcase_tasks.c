/**
 * @file    showcase_tasks.c
 * @brief   End-to-end demo of crc / cdc_rx / event / fs / tables
 *
 * @details Three lightweight tasks share a small amount of state through
 *          the new kernel modules:
 *
 *            sensor_task   — fakes a sensor producing 64-byte frames once
 *                            per second, computes CRC16 with the HW CRC
 *                            peripheral, files each frame in the internal
 *                            filesystem under "snapXXX", and emits an
 *                            INFO event recording the (frame_no, crc).
 *
 *            ground_task   — every 5 s pretends ground has uplinked a new
 *                            cadence table, calls tbl_load + tbl_activate,
 *                            and emits a CRITICAL event when the activate
 *                            callback fires.
 *
 *            shell_task    — drains queued events into a stack buffer and
 *                            polls cdc_rx for any byte the operator typed,
 *                            echoing each byte back as a sanity check that
 *                            the ring buffer survives the SVC gates.
 *
 *          Together they touch every public entry point of the new
 *          modules: crc16_ccitt, fs_create / fs_write, os_event /
 *          event_drain, tbl_register / tbl_load / tbl_activate / tbl_dump,
 *          and cdc_rx_available / cdc_rx_read_byte. The intent is a
 *          deliberate smoke test rather than a benchmark — when the
 *          firmware boots with this demo enabled and the operator sees
 *          the event stream advance, every layer of the recent refactor
 *          (move into kernel, HW CRC, ITCM/DTCM hardening, SVC gates) is
 *          exercised end-to-end.
 *
 * @author  Souham Biswas
 * @date    2026
 *
 * @copyright Copyright 2025-2026 Souham Biswas
 *            https://github.com/ironhide23586/icarus-os-core
 *            Licensed under the Apache License, Version 2.0
 */

#include "showcase_tasks.h"
#include "icarus/icarus.h"

#include <string.h>
#include <stdio.h>

/* ---- Module IDs for the squelch table (kernel uses uint8_t) ------------ */

#define MOD_SENSOR   8
#define MOD_GROUND   9
#define MOD_SHELL   10

/* ---- Event IDs ---------------------------------------------------------- */

#define EV_FRAME_FILED      0x0100
#define EV_FRAME_FILE_FULL  0x0101
#define EV_TABLE_LOADED     0x0200
#define EV_TABLE_ACTIVATED  0x0201
#define EV_SHELL_BYTE       0x0300

/* ---- Demo cadence table -------------------------------------------------- */

#define DEMO_TBL_ID         0x70
#define DEMO_TBL_SIZE       16
#define DEMO_TBL_SCHEMA_CRC 0xC0DEu

/** Last activated cadence value, written by the activate callback. */
static volatile uint8_t g_active_cadence_ms = 50;

static bool cadence_activate(const void *data, uint16_t len) {
    if (!data || len != DEMO_TBL_SIZE) {
        return false;
    }
    /* Take the first byte as the new cadence (clamped to 10..250 ms). */
    uint8_t v = ((const uint8_t *)data)[0];
    if (v < 10)  v = 10;
    if (v > 250) v = 250;
    g_active_cadence_ms = v;
    return true;
}

static void register_cadence_table(void) {
    tbl_descriptor_t desc = {
        .id         = DEMO_TBL_ID,
        .size       = DEMO_TBL_SIZE,
        .schema_crc = DEMO_TBL_SCHEMA_CRC,
        .activate   = cadence_activate,
    };
    /* name field — keep it inside TBL_NAME_LEN. */
    const char *n = "cadence";
    for (uint8_t i = 0; i < TBL_NAME_LEN - 1 && n[i]; i++) {
        desc.name[i] = n[i];
    }
    (void)tbl_register(&desc);
}

/* ---- Sensor task -------------------------------------------------------- */

#define SENSOR_FRAME_BYTES   64

static void sensor_task(void) {
    uint16_t frame_no = 0;
    uint8_t  frame[SENSOR_FRAME_BYTES];

    while (1) {
        /* Fake a sensor reading: deterministic ramp + frame counter. */
        for (uint16_t i = 0; i < SENSOR_FRAME_BYTES; i++) {
            frame[i] = (uint8_t)(i + (frame_no & 0xFF));
        }

        /* Hardware CRC16-CCITT — runs on the CRC peripheral. */
        uint16_t crc = crc16_ccitt(frame, SENSOR_FRAME_BYTES);

        /* File it. The fs has 16 slots; we wrap names every 16 frames. */
        char name[FS_MAX_NAME_LEN];
        snprintf(name, sizeof(name), "snap%03u", (unsigned)(frame_no % 16));

        /* Replace the slot if it already exists, then create + write. */
        (void)fs_delete(name);

        fs_file_t fh;
        if (fs_create(name, &fh)) {
            (void)fs_write(&fh, frame, SENSOR_FRAME_BYTES);

            uint8_t payload[12];
            payload[0]  = (uint8_t)(frame_no >> 8);
            payload[1]  = (uint8_t)(frame_no & 0xFF);
            payload[2]  = (uint8_t)(crc >> 8);
            payload[3]  = (uint8_t)(crc & 0xFF);
            for (uint8_t i = 4; i < 12; i++) payload[i] = 0;
            os_event(MOD_SENSOR, EVENT_INFO, EV_FRAME_FILED, payload, 12);
        } else {
            os_event(MOD_SENSOR, EVENT_WARN, EV_FRAME_FILE_FULL, NULL, 0);
        }

        frame_no++;
        task_active_sleep(g_active_cadence_ms);
    }
}

/* ---- Ground task: pretend ground uplinks a new cadence table every 5 s -- */

static void ground_task(void) {
    register_cadence_table();

    uint8_t cadence_choices[] = { 25, 50, 100, 200, 75 };
    uint8_t which = 0;

    while (1) {
        task_active_sleep(5000);

        uint8_t blob[DEMO_TBL_SIZE];
        memset(blob, 0, sizeof(blob));
        blob[0] = cadence_choices[which];
        which   = (uint8_t)((which + 1) % (sizeof(cadence_choices) / sizeof(cadence_choices[0])));

        if (!tbl_load(DEMO_TBL_ID, blob, DEMO_TBL_SIZE, DEMO_TBL_SCHEMA_CRC)) {
            continue;
        }

        uint8_t loaded_payload[2] = { blob[0], 0 };
        os_event(MOD_GROUND, EVENT_INFO, EV_TABLE_LOADED, loaded_payload, 2);

        if (tbl_activate(DEMO_TBL_ID)) {
            uint8_t a_payload[2] = { (uint8_t)g_active_cadence_ms, 0 };
            os_event(MOD_GROUND, EVENT_CRITICAL, EV_TABLE_ACTIVATED,
                     a_payload, 2);
        }
    }
}

/* ---- Shell echo + event drain task -------------------------------------- */

#define SHELL_DRAIN_BUDGET   8

static void shell_echo_task(void) {
    /* Drop ourselves out of EVENT_DEBUG noise from any other module while
     * still letting INFO/WARN/CRITICAL through unchanged. */
    event_set_squelch(MOD_SHELL, EVENT_INFO);

    while (1) {
        /* Drain queued events. The kernel buffer is in DTCM_PRIV; the
         * SVC gate copies entries into our stack-resident scratch. */
        event_entry_t scratch[SHELL_DRAIN_BUDGET];
        uint8_t       drained = 0;
        if (event_drain(scratch, SHELL_DRAIN_BUDGET, &drained) && drained) {
            /* Render to printf so retarget_stdio sends them to the host. */
            for (uint8_t i = 0; i < drained; i++) {
                printf("[ev mod=%u sev=%u id=0x%04X]\r\n",
                       (unsigned)scratch[i].module_id,
                       (unsigned)scratch[i].severity,
                       (unsigned)scratch[i].event_id);
            }
        }

        /* Echo any operator key presses pushed by the USB CDC ISR into
         * the cdc_rx ring. The ring lives in DTCM_PRIV; reads route
         * through the SVC gate. */
        while (cdc_rx_available() > 0) {
            uint8_t b;
            if (!cdc_rx_read_byte(&b)) {
                break;
            }
            uint8_t payload[1] = { b };
            os_event(MOD_SHELL, EVENT_INFO, EV_SHELL_BYTE, payload, 1);
            printf("[shell] rx 0x%02X '%c'\r\n",
                   (unsigned)b,
                   (b >= 0x20 && b < 0x7F) ? (char)b : '.');
        }

        task_active_sleep(100);
    }
}

/* ---- Public init -------------------------------------------------------- */

void showcase_tasks_init(void) {
    /* Bring up the shared modules. The OBC's obc_init() also calls
     * these — when this demo is built into the standalone kernel image
     * it must do the equivalent itself. Calling them here is idempotent
     * for fs / tables / event under the new init contracts. */
    cdc_rx_init();
    fs_init();
    event_init();
    tbl_init();

    os_register_task(sensor_task,     "showcs_s");
    os_register_task(ground_task,     "showcs_g");
    os_register_task(shell_echo_task, "showcs_e");
}
