/*
 * mock_usb.h
 * Mock for USB CDC dependencies
 */

#ifndef MOCK_USB_H
#define MOCK_USB_H

#include <stdint.h>

// Mock USB types
typedef enum {
	USBD_OK = 0,
	USBD_BUSY = 1,
	USBD_FAIL = 2
} USBD_StatusTypeDef;

// Mock USB functions
USBD_StatusTypeDef CDC_Transmit_FS(uint8_t* Buf, uint16_t Len);

#endif // MOCK_USB_H
