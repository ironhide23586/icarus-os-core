/**
 * @file    game.h
 * @brief   ICARUS Runner - Main Game API
 * @version 0.1.0
 *
 * @details Chrome dino-style endless runner. Runs unprivileged on STM32H7.
 *
 * @author  Souham Biswas
 * @date    2026
 *
 * @copyright Copyright (c) 2026 ICARUS Project
 */

#ifndef GAME_H
#define GAME_H

#ifdef __cplusplus
extern "C" {
#endif

#include "game/game_config.h"
#include "game/game_types.h"

/* ============================================================================
 * PUBLIC API
 * ========================================================================= */

/**
 * @brief Initialize and register game task
 */
void game_init(void);

#ifdef __cplusplus
}
#endif

#endif /* GAME_H */
