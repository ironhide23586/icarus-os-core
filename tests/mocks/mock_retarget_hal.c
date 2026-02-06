/*
 * mock_retarget_hal.c
 * Mock retarget_hal implementation
 */

#include "mock_retarget_hal.h"

void hal_init(void) {
	// Mock: do nothing
}

void LED_On(void) {
	// Mock: do nothing
}

void LED_Off(void) {
	// Mock: do nothing
}

void LED_Blink(uint32_t Hdelay, uint32_t Ldelay) {
	(void)Hdelay;
	(void)Ldelay;
	// Mock: do nothing
}

int32_t platform_write(void* handle, uint8_t reg, const uint8_t *bufp, uint16_t len) {
	(void)handle;
	(void)reg;
	(void)bufp;
	(void)len;
	// Mock: return success
	return 0;
}

int32_t platform_read(void* handle, uint8_t reg, uint8_t *bufp, uint16_t len) {
	(void)handle;
	(void)reg;
	(void)bufp;
	(void)len;
	// Mock: return success
	return 0;
}
