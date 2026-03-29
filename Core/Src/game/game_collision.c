/**
 * @file    game_collision.c
 * @brief   Collision Detection
 */

#include "game/game.h"
#include "game/game_internal.h"
#include "bsp/rtc.h"

void game_collision_check(game_state_t *game)
{
    int16_t player_bottom = (int16_t)game->player.y + GAME_PLAYER_HEIGHT;
    
    for (uint8_t i = 0; i < GAME_MAX_OBSTACLES; i++) {
        if (game->obstacles[i].active) {
            int16_t obs_left = game->obstacles[i].x;
            int16_t obs_right = obs_left + game->obstacles[i].width;
            int16_t obs_top = GAME_GROUND_ROW - game->obstacles[i].height;
            
            /* Check X overlap */
            bool x_overlap = (GAME_PLAYER_X < obs_right) && 
                           ((GAME_PLAYER_X + GAME_PLAYER_WIDTH) > obs_left);
            
            /* Check Y overlap */
            bool y_overlap = (player_bottom > obs_top);
            
            if (x_overlap && y_overlap) {
                /* Collision! */
                game->state = GAME_STATE_GAME_OVER;
                
                /* Save high score */
                if (game->score.current_score > game->score.high_score) {
                    game->score.high_score = game->score.current_score;
                    HAL_RTCEx_BKUPWrite(&hrtc, GAME_RTC_BACKUP_REG, game->score.high_score);
                }
                return;
            }
        }
    }
}
