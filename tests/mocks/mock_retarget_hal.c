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
