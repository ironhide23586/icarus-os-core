/*
 * usb_device.h mock
 * Mock for USB device functionality during testing
 */

#ifndef __USB_DEVICE__H__
#define __USB_DEVICE__H__

#ifdef __cplusplus
extern "C" {
#endif

// Mock USB Device initialization function (implemented in mock_hal.c)
void MX_USB_DEVICE_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* __USB_DEVICE__H__ */