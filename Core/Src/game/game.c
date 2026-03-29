/**
 * @file    game.c
 * @brief   ICARUS Runner - Multi-Task Game Implementation
 * @version 0.2.0
 *
 * @details Demonstrates Icarus OS multi-tasking capabilities:
 *          - 4 concurrent tasks with different periods
 *          - Pipe-based inter-task communication
 *          - Semaphore-protected shared state
 *          - Kernel protected data for game state
 *
 * @author  Souham Biswas
 * @date    2026
 *
 * @copyright Copyright (c) 2026 ICARUS Project
 */

#include "game/game.h"
#include "game/game_internal.h"
#include "icarus/icarus.h"
#include <stdio.h>
#include <string.h>

/* ============================================================================
 * GAME RESOURCES
 * ========================================================================= */

#define GAME_PIPE_JUMP_CMD      0   /**< Pipe for jump commands */
#define GAME_SEM_STATE          0   /**< Semaphore protecting game state */

#define JUMP_CMD_PRESS          1   /**< Jump button pressed */

/* ============================================================================
 * SHARED GAME STATE (static = .bss = RAM_D1 by default)
 * ========================================================================= */

static game_state_t g_game_state;

/* ============================================================================
 * TASK 1: INPUT HANDLER (10ms period)
 * ========================================================================= */

static void game_input_task(void)
{
    uint8_t jump_cmd = JUMP_CMD_PRESS;
    bool button_pressed = false;
    bool button_prev = false;
    uint32_t last_debounce = 0;
    
    printf("[INPUT] Task started\r\n");

    semaphore_consume(GAME_SEM_STATE);
    printf("[INPUT] Initial state: y=%.2f vel=%.2f grnd=%d state=%d\r\n",
           g_game_state.player.y, g_game_state.player.velocity,
           g_game_state.player.is_grounded, g_game_state.state);
    game_state_init(&g_game_state);
    printf("[INPUT] Reset state: y=%.2f vel=%.2f grnd=%d state=%d\r\n",
           g_game_state.player.y, g_game_state.player.velocity,
           g_game_state.player.is_grounded, g_game_state.state);
    while(1);
    semaphore_feed(GAME_SEM_STATE);
    while (1) {
        uint32_t current_tick = os_get_tick_count();
        
        /* Read button (active low) */
        bool button_raw = (HAL_GPIO_ReadPin(BSP_KEY_PORT, BSP_KEY_PIN) == GPIO_PIN_RESET);
        
        /* Debounce and detect state change */
        if ((current_tick - last_debounce) > GAME_BUTTON_DEBOUNCE_MS) {
            if (button_raw != button_pressed) {
                last_debounce = current_tick;
                button_pressed = button_raw;
                
                if (button_raw && !button_prev) {
                    /* Button pressed - send jump command via pipe */
                    pipe_enqueue(GAME_PIPE_JUMP_CMD, &jump_cmd, 1);
                    
                    /* LED feedback */
                    HAL_GPIO_WritePin(BSP_LED_PORT, BSP_LED_PIN, GPIO_PIN_RESET);
                } else if (!button_raw && button_prev) {
                    /* Button released */
                    HAL_GPIO_WritePin(BSP_LED_PORT, BSP_LED_PIN, GPIO_PIN_SET);
                }
                
                button_prev = button_raw;
            }
        }
        
        task_active_sleep(10);  /* 10ms period - fast input response */
    }
}

/* ============================================================================
 * TASK 2: PHYSICS ENGINE (20ms period - 50 FPS)
 * ========================================================================= */

static void game_physics_task(void)
{
    uint8_t jump_cmd;
    
    printf("[PHYSICS] Task started\r\n");
    printf("[PHYSICS] g_game_state addr=0x%08lx size=%u\r\n",
           (unsigned long)&g_game_state, (unsigned)sizeof(g_game_state));
    
    /* Print initial game state */
    // semaphore_consume(GAME_SEM_STATE);
    // printf("[PHYSICS] Initial state: y=%.2f vel=%.2f grnd=%d state=%d\r\n",
    //        g_game_state.player.y, g_game_state.player.velocity,
    //        g_game_state.player.is_grounded, g_game_state.state);
    // printf("[PHYSICS] player offset=%u\r\n",
    //        (unsigned)((char*)&g_game_state.player - (char*)&g_game_state));
    // while(1);
    // semaphore_feed(GAME_SEM_STATE);
    
    while (1) {
        /* Check for jump commands from input task */
        // semaphore_consume(GAME_SEM_STATE);
        if (pipe_dequeue(GAME_PIPE_JUMP_CMD, &jump_cmd, 1)) {
            semaphore_consume(GAME_SEM_STATE);
            
            if (g_game_state.state == GAME_STATE_RUNNING) {
                /* Apply jump ONLY if grounded (no double jumps) */
                if (g_game_state.player.is_grounded) {
                    g_game_state.player.velocity = -GAME_JUMP_VELOCITY;  /* Negative = upward */
                    g_game_state.player.is_jumping = true;
                    g_game_state.player.is_grounded = false;
                    
                    // /* Drain any additional jump commands in the pipe to prevent queuing */
                    // uint8_t dummy;
                    // while (pipe_dequeue(GAME_PIPE_JUMP_CMD, &dummy, 1)) {
                    //     /* Discard */
                    // }
                }
                /* If not grounded, ignore the jump command (it's discarded) */
            } else if (g_game_state.state == GAME_STATE_READY || 
                      g_game_state.state == GAME_STATE_GAME_OVER) {
                /* Start/restart game on button press */
                if (g_game_state.state == GAME_STATE_READY) {
                    game_state_init(&g_game_state);
                    g_game_state.state = GAME_STATE_RUNNING;
                    g_game_state.score.last_score_tick = os_get_tick_count();
                } else {
                    g_game_state.state = GAME_STATE_READY;
                }
            }
            
            semaphore_feed(GAME_SEM_STATE);
        }

        /* Update physics */
        semaphore_consume(GAME_SEM_STATE);
        
        if (g_game_state.state == GAME_STATE_RUNNING) {
            if (!g_game_state.player.is_grounded) {
                /* Print BEFORE update */
                printf("[BEFORE] y=%.2f vel=%.2f grnd=%d\r\n", 
                       g_game_state.player.y, g_game_state.player.velocity, 
                       g_game_state.player.is_grounded);
                
                /* Apply gravity */
                g_game_state.player.velocity += GAME_GRAVITY;
                
                /* Update position */
                g_game_state.player.y += g_game_state.player.velocity;
                
                /* Print AFTER update */
                printf("[AFTER]  y=%.2f vel=%.2f grnd=%d\r\n", 
                       g_game_state.player.y, g_game_state.player.velocity, 
                       g_game_state.player.is_grounded);
                
                /* HALT for debugging - REMOVED */
                // while(1);
            }
            
            /* Ground collision */
            if (g_game_state.player.y >= GAME_PLAYER_GROUND_Y) {
                g_game_state.player.y = GAME_PLAYER_GROUND_Y;
                g_game_state.player.velocity = 0.0f;
                g_game_state.player.is_grounded = true;
                g_game_state.player.is_jumping = false;
            } else {
                /* In air */
                g_game_state.player.is_grounded = false;
            }
        }
        
        semaphore_feed(GAME_SEM_STATE);
        
        task_active_sleep(20);  /* 20ms period - 50 FPS physics */
    }
}

/* ============================================================================
 * TASK 3: GAME LOGIC (50ms period - 20 FPS)
 * ========================================================================= */

static void game_logic_task(void)
{
    printf("[LOGIC] Task started\r\n");
    
    while (1) {
        semaphore_consume(GAME_SEM_STATE);
        
        if (g_game_state.state == GAME_STATE_RUNNING) {
            /* Update obstacles */
            game_obstacles_update(&g_game_state);
            
            /* Update score */
            game_score_update(&g_game_state);
            
            /* Check collisions */
            game_collision_check(&g_game_state);
            
            g_game_state.frame_count++;
        }
        
        semaphore_feed(GAME_SEM_STATE);
        
        task_active_sleep(50);  /* 50ms period - 20 FPS game logic */
    }
}

/* ============================================================================
 * TASK 4: RENDERER (50ms period - 20 FPS)
 * ========================================================================= */

static void game_render_task(void)
{
    printf("[RENDER] Task started\r\n");
    
    /* Hide cursor */
    printf("\033[?25l");
    
    /* Show title screen */
    game_render_title();
    
    while (1) {
        semaphore_consume(GAME_SEM_STATE);
        
        switch (g_game_state.state) {
            case GAME_STATE_RUNNING:
                game_render_frame(&g_game_state);
                break;
                
            case GAME_STATE_GAME_OVER:
                game_render_game_over(&g_game_state);
                break;
                
            case GAME_STATE_READY:
                /* Title screen already shown */
                break;
                
            default:
                break;
        }
        
        semaphore_feed(GAME_SEM_STATE);
        
        task_active_sleep(50);  /* 50ms period - 20 FPS rendering */
    }
}

/* ============================================================================
 * GAME INITIALIZATION
 * ========================================================================= */

void game_init(void)
{
    printf("\r\n");
    printf("╔════════════════════════════════════════════════════════════╗\r\n");
    printf("║           ICARUS RUNNER - Multi-Task Demo                 ║\r\n");
    printf("╠════════════════════════════════════════════════════════════╣\r\n");
    printf("║  Demonstrating Icarus OS capabilities:                    ║\r\n");
    printf("║  • 4 concurrent tasks (Input, Physics, Logic, Render)     ║\r\n");
    printf("║  • Pipe-based inter-task communication                    ║\r\n");
    printf("║  • Semaphore-protected shared state (static → RAM_D1)    ║\r\n");
    printf("║  • Round-robin cooperative scheduling                     ║\r\n");
    printf("╚════════════════════════════════════════════════════════════╝\r\n");
    printf("\r\n");
    
    /* Initialize pipe for jump commands (1 byte messages, 4 message capacity) */
    if (!pipe_init(GAME_PIPE_JUMP_CMD, 4)) {
        printf("ERROR: Failed to initialize jump command pipe\r\n");
        return;
    }
    
    /* Initialize semaphore for state protection (binary semaphore) */
    if (!semaphore_init(GAME_SEM_STATE, 1)) {
        printf("ERROR: Failed to initialize state semaphore\r\n");
        return;
    }
    
    printf("[INIT] Resources initialized\r\n");
    printf("[INIT] Pipe 0: Jump commands (Input → Physics)\r\n");
    printf("[INIT] Semaphore 0: State protection\r\n");
    printf("[INIT] Shared state: %u bytes (static .bss → RAM_D1)\r\n", (unsigned int)sizeof(game_state_t));
    printf("\r\n");
    
    /* Initialize game state */
    game_state_init(&g_game_state);

    g_game_state.state = GAME_STATE_READY;

    /* Register tasks */
    os_register_task(game_input_task, "input");
    os_register_task(game_physics_task, "physics");
    os_register_task(game_logic_task, "logic");
    os_register_task(game_render_task, "render");

    // printf("[INIT] Setting y=%.2f vel=%.2f grnd=%d state=%d\r\n",
    //        g_game_state.player.y, g_game_state.player.velocity,
    //        g_game_state.player.is_grounded, g_game_state.state);
    // while(1);
    
    printf("[INIT] 4 tasks registered\r\n");
    printf("\r\n");
}
