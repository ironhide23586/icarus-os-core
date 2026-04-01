/**
 * @file    game_render.c
 * @brief   Game Rendering
 */

#include "game/game.h"
#include "game/game_internal.h"
#include <stdio.h>

#define ANSI_CLEAR "\033[2J\033[H"
#define ANSI_GOTO(r,c) printf("\033[%d;%dH", (r), (c))

void game_render_frame(volatile game_state_t *game)
{
    /* Clear screen */
    printf(ANSI_CLEAR);
    
    /* Score bar */
    ANSI_GOTO(1, 1);
    printf("ICARUS RUNNER          HI: %05lu    SCORE: %05lu", 
           game->score.high_score, game->score.current_score);

    game->player.y += game->player.velocity;
    if (game->player.y < GAME_PLAYER_GROUND_Y) {
        game->player.velocity += GAME_GRAVITY;
        ANSI_GOTO(3, 1);
        printf("Player Jumping");
    }
    else {
        game->player.velocity = 0;
        game->player.y = GAME_PLAYER_GROUND_Y;
        game->player.is_grounded = true;
        ANSI_GOTO(4, 1);
        printf("Player Grounded");
    }

    // if (game->player.y >= GAME_PLAYER_GROUND_Y) {
    //     game->player.y = GAME_PLAYER_GROUND_Y;
    //     game->player.velocity = 0.0f;
    //     game->player.is_grounded = true;
    //     game->player.is_jumping = false;
    // } else {
    //     /* In air */
    //     game->player.is_grounded = false;
    // }

    
    /* Debug info - raw hex values (since %.2f is broken in unprivileged mode) */
    volatile uint32_t *raw_y = (volatile uint32_t *)&game->player.y;
    volatile uint32_t *raw_vel = (volatile uint32_t *)&game->player.velocity;
    ANSI_GOTO(2, 1);
    printf("DEBUG: y=0x%08lx vel=0x%08lx grnd=%d", 
           *raw_y, *raw_vel, game->player.is_grounded);
    
    /* Ground line */
    ANSI_GOTO(GAME_GROUND_ROW, 1);
    for (int i = 0; i < GAME_SCREEN_WIDTH; i++) {
        printf("=");
    }
    
    /* Player */
    int player_row = (int)game->player.y;
    for (int i = 0; i < GAME_PLAYER_HEIGHT; i++) {
        ANSI_GOTO(player_row + i, GAME_PLAYER_X);
        printf("###");
    }
    
    /* Obstacles */
    for (uint8_t i = 0; i < GAME_MAX_OBSTACLES; i++) {
        if (game->obstacles[i].active && game->obstacles[i].x >= 0) {
            for (uint8_t h = 0; h < game->obstacles[i].height; h++) {
                ANSI_GOTO(GAME_GROUND_ROW - game->obstacles[i].height + h, 
                         game->obstacles[i].x);
                for (uint8_t w = 0; w < game->obstacles[i].width; w++) {
                    printf("X");
                }
            }
        }
    }
    
    fflush(stdout);
}

void game_render_title(void)
{
    printf(ANSI_CLEAR);
    ANSI_GOTO(10, 25);
    printf("ICARUS RUNNER");
    ANSI_GOTO(12, 20);
    printf("Press K1 to Start");
    fflush(stdout);
}

void game_render_game_over(volatile game_state_t *game)
{
    ANSI_GOTO(10, 28);
    printf("GAME OVER!");
    ANSI_GOTO(12, 22);
    printf("Score: %05lu", game->score.current_score);
    ANSI_GOTO(13, 20);
    printf("High Score: %05lu", game->score.high_score);
    ANSI_GOTO(15, 18);
    printf("Press K1 to Restart");
    fflush(stdout);
}
