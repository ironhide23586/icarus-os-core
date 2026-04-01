/**
 * @file    game_config.h
 * @brief   ICARUS Runner - Configuration Constants
 * @version 0.1.0
 *
 * @author  Souham Biswas
 * @date    2026
 *
 * @copyright Copyright (c) 2026 ICARUS Project
 */

#ifndef GAME_CONFIG_H
#define GAME_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/* Display */
#define GAME_SCREEN_WIDTH       80
#define GAME_SCREEN_HEIGHT      24
#define GAME_GROUND_ROW         20

/* Timing */
#define GAME_FRAME_TIME_MS      50
#define GAME_SCORE_INTERVAL_MS  100

/* Player */
#define GAME_PLAYER_X           10
#define GAME_PLAYER_WIDTH       3
#define GAME_PLAYER_HEIGHT      3
#define GAME_PLAYER_GROUND_Y    (GAME_GROUND_ROW - GAME_PLAYER_HEIGHT)

/* Physics */
#define GAME_GRAVITY            0.5f
#define GAME_JUMP_VELOCITY      3.0f
#define GAME_MAX_FALL_VELOCITY  2.0f
#define GAME_JUMP_HEIGHT        6

/* Obstacles */
#define GAME_MAX_OBSTACLES      4
#define GAME_OBSTACLE_MIN_WIDTH 2U
#define GAME_OBSTACLE_MAX_WIDTH 4U
#define GAME_OBSTACLE_MIN_HEIGHT 2U
#define GAME_OBSTACLE_MAX_HEIGHT 4U
#define GAME_OBSTACLE_SPAWN_X   (GAME_SCREEN_WIDTH - 1)

/* Difficulty */
#define GAME_SPEED_LEVEL_1      100
#define GAME_SPEED_LEVEL_2      300
#define GAME_SPEED_LEVEL_3      600
#define GAME_SPEED_LEVEL_4      1000

/* Scoring */
#define GAME_OBSTACLE_BONUS     10

/* Input */
#define GAME_BUTTON_DEBOUNCE_MS 50

/* Persistence */
#define GAME_RTC_BACKUP_REG     RTC_BKP_DR0

#ifdef __cplusplus
}
#endif

#endif /* GAME_CONFIG_H */
