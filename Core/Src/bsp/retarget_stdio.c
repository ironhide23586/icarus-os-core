/*
 * retarget_stdio.c
 *
 *  Created on: Jan 23, 2026
 *      Author: souhamb
 */


#include "kernel/task.h"

#include "usbd_cdc_if.h"
#include "usbd_def.h"
#include "cmsis_gcc.h"



//int __io_putchar(int ch) {
//	enqueue_print_buffer(ch);
//	return ch;
//}

//
// Change your __io_putchar implementation
int __io_putchar(int ch) {
  static uint8_t buf[64];
  static uint8_t i = 0;

  buf[i++] = ch;

  // Flush on newline or when buffer is full
  if (ch == '\n' || i == sizeof(buf)) {

    uint8_t result = CDC_Transmit_FS(buf, i);
    uint8_t retry_count = 0;

    while (retry_count < 1000 && result == USBD_BUSY) {
      result = CDC_Transmit_FS(buf, i);
      task_blocking_sleep(1);
      retry_count++;
    }
    i = 0;
    task_blocking_sleep(10);
  }
  return ch;
}
