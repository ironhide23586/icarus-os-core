/*
 * error.c
 *
 *  Created on: Jan 23, 2026
 *      Author: Souham Biswas
 *      GitHub: https://github.com/ironhide23586/icarus-os-core
 */

#ifndef SRC_BSP_ERROR_C_
#define SRC_BSP_ERROR_C_

#include "error.h"



void Error_Handler(void) {
	__disable_irq();
	while (1) {
		__NOP();
	}
}




#endif /* SRC_BSP_ERROR_C_ */
