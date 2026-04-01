/**
 * @file    game_score.c
 * @brief   Scoring System
 */

#include "game/game.h"
#include "game/game_internal.h"
#include "icarus/icarus.h"

void game_score_update(volatile game_state_t *game)
{
    uint32_t current_tick = os_get_tick_count();
    
    /* Distance-based scoring */
    if ((current_tick - game->score.last_score_tick) >= GAME_SCORE_INTERVAL_MS) {
        game->score.current_score += (uint32_t)game->score.speed_multiplier;
        game->score.last_score_tick = current_tick;
        game->score.distance_traveled++;
    }
    
    /* Obstacle bonus */
    for (uint8_t i = 0; i < GAME_MAX_OBSTACLES; i++) {
        if (game->obstacles[i].active && !game->obstacles[i].scored) {
            if (game->obstacles[i].x < GAME_PLAYER_X) {
                game->score.current_score += GAME_OBSTACLE_BONUS;
                game->score.obstacles_cleared++;
                game->obstacles[i].scored = true;
            }
        }
    }
    
    /* Update speed multiplier */
    if (game->score.current_score >= GAME_SPEED_LEVEL_4) {
        game->score.speed_multiplier = 1.8f;
    } else if (game->score.current_score >= GAME_SPEED_LEVEL_3) {
        game->score.speed_multiplier = 1.6f;
    } else if (game->score.current_score >= GAME_SPEED_LEVEL_2) {
        game->score.speed_multiplier = 1.4f;
    } else if (game->score.current_score >= GAME_SPEED_LEVEL_1) {
        game->score.speed_multiplier = 1.2f;
    }
}
