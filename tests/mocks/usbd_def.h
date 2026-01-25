/*
 * mock_usbd_def.h
 * Minimal mock for usbd_def.h
 */

#ifndef __USBD_DEF_H
#define __USBD_DEF_H

#ifdef __cplusplus
extern "C" {
#endif

// Minimal definitions needed for testing
#ifndef NULL
#define NULL 0U
#endif

// Status types
typedef enum {
	USBD_OK = 0,
	USBD_BUSY = 1,
	USBD_FAIL = 2
} USBD_StatusTypeDef;

#ifdef __cplusplus
}
#endif

#endif /* __USBD_DEF_H */
