/**
 * @file    game_physics.c
 * @brief   Physics & Player Movement (Legacy - now in physics task)
 * @note    This file kept for compatibility but physics is now in game.c
 */

#include "game/game.h"
#include "game/game_internal.h"

/* Physics update is now handled by the physics task in game.c */
void game_physics_update(volatile game_state_t *game)
{
    /* This function is no longer used in multi-task version */
    /* Physics is handled by game_physics_task() in game.c */
    (void)game;
}
