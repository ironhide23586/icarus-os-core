/*
 * mock_usb.c
 * Mock USB CDC implementation for unit testing
 */

#include "mock_usb.h"

USBD_StatusTypeDef CDC_Transmit_FS(uint8_t* Buf, uint16_t Len) {
	(void)Buf;
	(void)Len;
	// Mock: always succeed
	return USBD_OK;
}
