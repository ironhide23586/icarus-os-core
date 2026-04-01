/**
 * @file    game_state.c
 * @brief   Game State Management
 */

#include "game/game.h"
#include "game/game_internal.h"
#include "bsp/rtc.h"
#include <string.h>
#include <stdio.h>

static void clear_player(volatile player_t *player)
{
    player->y = 0.0f;
    player->velocity = 0.0f;
    player->is_grounded = false;
    player->is_jumping = false;
}

static void clear_obstacles(volatile obstacle_t *obstacles, uint8_t count)
{
    for (uint8_t i = 0; i < count; i++) {
        obstacles[i].x = 0;
        obstacles[i].width = 0;
        obstacles[i].height = 0;
        obstacles[i].active = false;
        obstacles[i].scored = false;
    }
}

static void clear_score(volatile score_t *score)
{
    score->current_score = 0;
    score->high_score = 0;
    score->distance_traveled = 0;
    score->obstacles_cleared = 0;
    score->last_score_tick = 0;
    score->speed_multiplier = 0.0f;
}

static void clear_input(volatile input_t *input)
{
    input->button_pressed = false;
    input->button_prev = false;
    input->last_debounce = 0;
}

void game_state_init(volatile game_state_t *game)
{
    /* Clear all fields explicitly instead of memset */
    game->state = GAME_STATE_INIT;
    game->frame_count = 0;
    game->last_obstacle_spawn = 0;
    game->obstacle_spawn_interval = 0;
    
    clear_player(&game->player);
    clear_obstacles(game->obstacles, 4);  /* GAME_MAX_OBSTACLES */
    clear_score(&game->score);
    clear_input(&game->input);
    
    /* Now set the actual initial values */
    game->player.y = GAME_PLAYER_GROUND_Y;
    game->player.is_grounded = true;
    game->score.speed_multiplier = 1.0f;
    game->obstacle_spawn_interval = 2000;
    
    /* Load high score */
    game->score.high_score = HAL_RTCEx_BKUPRead(&hrtc, GAME_RTC_BACKUP_REG);
    if (game->score.high_score > 999999) {
        game->score.high_score = 0;
    }
    
    /* Data Synchronization Barrier - ensures all writes complete before returning
     * Critical for unprivileged tasks writing to cacheable/bufferable RAM_D1
     * Forces write buffer to flush so subsequent reads see updated values */
    __DSB();
    __ISB();  /* Instruction barrier to ensure pipeline is flushed */

    // if (print_state) {
    //     /* Force a read-back to verify writes took effect */
    //     volatile float y_readback = game->player.y;
    //     volatile float vel_readback = game->player.velocity;
    //     volatile bool grnd_readback = game->player.is_grounded;
    //     volatile game_state_e state_readback = game->state;
        
    //     printf("[STATE_INIT] Setting y=%.2f vel=%.2f grnd=%d state=%d\r\n",
    //         y_readback, vel_readback, grnd_readback, state_readback);
    //     while(1);
    // }
}
