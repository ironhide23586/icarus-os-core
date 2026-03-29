/**
 * @file    game_obstacles.c
 * @brief   Obstacle Management
 */

#include "game/game.h"
#include "game/game_internal.h"
#include "icarus/icarus.h"
#include <stdlib.h>

void game_obstacles_update(game_state_t *game)
{
    uint32_t current_tick = os_get_tick_count();
    
    /* Move existing obstacles */
    for (uint8_t i = 0; i < GAME_MAX_OBSTACLES; i++) {
        if (game->obstacles[i].active) {
            game->obstacles[i].x -= (int16_t)game->score.speed_multiplier;
            
            /* Deactivate if off screen */
            if (game->obstacles[i].x < -5) {
                game->obstacles[i].active = false;
            }
        }
    }
    
    /* Spawn new obstacle */
    if ((current_tick - game->last_obstacle_spawn) > game->obstacle_spawn_interval) {
        for (uint8_t i = 0; i < GAME_MAX_OBSTACLES; i++) {
            if (!game->obstacles[i].active) {
                game->obstacles[i].x = GAME_OBSTACLE_SPAWN_X;
                game->obstacles[i].width = (uint8_t)(GAME_OBSTACLE_MIN_WIDTH + (uint8_t)(rand() % 2));
                game->obstacles[i].height = (uint8_t)(GAME_OBSTACLE_MIN_HEIGHT + (uint8_t)(rand() % 2));
                game->obstacles[i].active = true;
                game->obstacles[i].scored = false;
                game->last_obstacle_spawn = current_tick;
                break;
            }
        }
    }
}
