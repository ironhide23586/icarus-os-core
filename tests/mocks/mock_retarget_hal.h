/*
 * mock_retarget_hal.h
 * Minimal mock for retarget_hal.h
 */

#ifndef INC_BSP_RETARGET_HAL_H_
#define INC_BSP_RETARGET_HAL_H_

#include <stdint.h>

// Minimal definitions needed
void hal_init(void);
void LED_On(void);
void LED_Off(void);
void LED_Blink(uint32_t Hdelay, uint32_t Ldelay);

#endif /* INC_BSP_RETARGET_HAL_H_ */
