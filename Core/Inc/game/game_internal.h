/**
 * @file    game_internal.h
 * @brief   Internal Game Function Prototypes
 * @version 0.1.0
 *
 * @author  Souham Biswas
 * @date    2026
 *
 * @copyright Copyright (c) 2026 ICARUS Project
 */

#ifndef GAME_INTERNAL_H
#define GAME_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "game/game.h"

/* State management */
void game_state_init(volatile game_state_t *game) __attribute__((noinline, optimize("O0")));

/* Input handling */
void game_input_update(volatile game_state_t *game);

/* Physics */
void game_physics_update(volatile game_state_t *game);

/* Obstacles */
void game_obstacles_update(volatile game_state_t *game);

/* Scoring */
void game_score_update(volatile game_state_t *game);

/* Collision */
void game_collision_check(volatile game_state_t *game);

/* Rendering */
void game_render_frame(volatile game_state_t *game);
void game_render_title(void);
void game_render_game_over(volatile game_state_t *game);

#ifdef __cplusplus
}
#endif

#endif /* GAME_INTERNAL_H */
