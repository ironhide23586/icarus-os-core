/**
 * @file    game_types.h
 * @brief   ICARUS Runner - Type Definitions
 * @version 0.1.0
 *
 * @author  Souham Biswas
 * @date    2026
 *
 * @copyright Copyright (c) 2026 ICARUS Project
 */

#ifndef GAME_TYPES_H
#define GAME_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/* ============================================================================
 * GAME STATE ENUMERATION
 * ========================================================================= */

typedef enum {
    GAME_STATE_INIT,
    GAME_STATE_READY,
    GAME_STATE_RUNNING,
    GAME_STATE_GAME_OVER,
} game_state_e;

/* ============================================================================
 * GAME STRUCTURES
 * ========================================================================= */

typedef struct {
    int16_t x;
    uint8_t width;
    uint8_t height;
    bool active;
    bool scored;
} obstacle_t;

typedef struct {
    float y;
    float velocity;
    bool is_grounded;
    bool is_jumping;
} player_t;

typedef struct {
    uint32_t current_score;
    uint32_t high_score;
    uint32_t distance_traveled;
    uint32_t obstacles_cleared;
    uint32_t last_score_tick;
    float speed_multiplier;
} score_t;

typedef struct {
    bool button_pressed;
    bool button_prev;
    uint32_t last_debounce;
} input_t;

typedef struct {
    game_state_e state;
    player_t player;
    obstacle_t obstacles[4];  /* GAME_MAX_OBSTACLES */
    score_t score;
    input_t input;
    uint32_t frame_count;
    uint32_t last_obstacle_spawn;
    uint32_t obstacle_spawn_interval;
} game_state_t;

#ifdef __cplusplus
}
#endif

#endif /* GAME_TYPES_H */
