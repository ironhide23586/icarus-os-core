/*
 * mock_main.h
 * Minimal mock for main.h
 */

#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>

// Include icarus config for ICARUS_TICKS_PER_TASK
#include "icarus/icarus_config.h"

// Error handler (mock)
void Error_Handler(void);

#endif /* MAIN_H */
