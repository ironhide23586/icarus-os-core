/**
 * @file    bsp_usb.h
 * @brief   Board Support Package - USB CDC Driver
 * @version 0.1.0
 *
 * @details USB Full-Speed CDC (Virtual COM Port) interface.
 *          Used for printf() output and debug console.
 *
 * @par USB Configuration:
 *      - Mode: Device
 *      - Class: CDC (Communications Device Class)
 *      - Speed: Full-Speed (12 Mbps)
 *      - VID/PID: 0x0483/0x5740 (STMicroelectronics)
 *
 * @see     USB 2.0 Specification
 * @see     USB CDC Class Specification
 *
 * @author  Souham Biswas
 * @date    2026
 *
 * @copyright Copyright (c) 2026 ICARUS Project
 */

#ifndef BSP_USB_H
#define BSP_USB_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
 * @brief Initialize USB Device
 *
 * @details Initializes USB OTG FS peripheral in device mode
 *          with CDC class driver.
 */
void MX_USB_DEVICE_Init(void);

/**
 * @brief Transmit data over USB CDC
 *
 * @param[in] buf   Data buffer
 * @param[in] len   Number of bytes
 *
 * @return USB status code
 */
uint8_t CDC_Transmit_FS(uint8_t* buf, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /* BSP_USB_H */
