/*
 * retarget_stdio.c
 *
 *  Created on: Jan 23, 2026
 *      Author: Souham Biswas
 *      GitHub: https://github.com/ironhide23586/icarus-os-core
 */


#include "icarus/kernel.h"
#include "icarus/scheduler.h"
#include "icarus/semaphore.h"

#include "usbd_cdc_if.h"
#include "usbd_def.h"
#include "cmsis_gcc.h"

/* Printf mutex semaphore index (binary semaphore for serialization) */
#define PRINTF_MUTEX_IDX  1

/* Function prototype */
int __io_putchar(int ch);

//int __io_putchar(int ch) {
//	enqueue_print_buffer(ch);
//	return ch;
//}

//
// Change your __io_putchar implementation
int __io_putchar(int ch) {
  static uint8_t buf[64];
  static uint8_t i = 0;
  static uint8_t mutex_initialized = 0;
  
  /* Initialize printf mutex on first call (binary semaphore) */
  if (!mutex_initialized) {
    semaphore_init(PRINTF_MUTEX_IDX, 1);
    mutex_initialized = 1;
  }
  
  /* Acquire mutex before accessing shared buffer */
  semaphore_consume(PRINTF_MUTEX_IDX);

  buf[i++] = (uint8_t)ch;

  // Flush on newline or when buffer is full
  if (ch == '\n' || i == sizeof(buf)) {

    uint8_t result = CDC_Transmit_FS(buf, i);
    uint16_t retry_count = 0;

    while (retry_count < 1000 && result == USBD_BUSY) {
      result = CDC_Transmit_FS(buf, i);
      task_blocking_sleep(1);
      retry_count++;
    }
    i = 0;
    // task_blocking_sleep(10);
  }
  
  /* Release mutex */
  semaphore_feed(PRINTF_MUTEX_IDX);
  
  return ch;
}
