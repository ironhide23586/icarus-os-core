/*
 * mock_main.h
 * Minimal mock for main.h
 */

#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>

// Minimal definitions from main.h needed for testing
#define TICKS_PER_TASK 50

// Error handler (mock)
void Error_Handler(void);

#endif /* MAIN_H */
