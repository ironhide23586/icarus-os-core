/*
 * retarget_Hal.h
 *
 *  Created on: Jan 23, 2026
 *      Author: souhamb
 */

#ifndef INC_BSP_RETARGET_HAL_H_
#define INC_BSP_RETARGET_HAL_H_

#include "main.h"
#include "i2c.h"
#include "rtc.h"
#include "spi.h"
#include "tim.h"
#include "usb_device.h"
//#include "usb_cdc.h"
#include "gpio.h"

#include "lcd.h"
#include "lsm9ds1_reg.h"

// #include "kernel/task.h"

#define IMU_ADDRESS 0x6B

void hal_init();
void LED_Blink(uint32_t Hdelay, uint32_t Ldelay);
void LED_On(void);
void LED_Off(void);


#endif /* INC_BSP_RETARGET_HAL_H_ */
