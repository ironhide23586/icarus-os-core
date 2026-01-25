/*
 * mock_main.c
 * Mock main.h implementation
 */

#include "mock_main.h"
#include <stdio.h>
#include <stdlib.h>

void Error_Handler(void) {
	printf("ERROR: Error_Handler called in test\n");
	exit(1);
}
