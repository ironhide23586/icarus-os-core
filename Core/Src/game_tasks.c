/**
 * @file    game_tasks.c
 * @brief   LCD Game Demo Tasks Implementation
 * @version 0.1.0
 *
 * @details Interactive game demo running on ST7735 LCD with button input.
 *
 * @author  Souham Biswas
 * @date    2026
 *
 * @copyright Copyright (c) 2026 ICARUS Project
 */

#include "game_tasks.h"
#include "icarus/icarus.h"
#include "bsp/display.h"
#include "bsp/config.h"
#include "bsp/gpio.h"
#include "lcd.h"

/* ============================================================================
 * GAME STATE
 * ========================================================================= */

/**
 * @brief Game state structure
 */
typedef struct {
    bool button_pressed;        /**< Current button state */
    bool button_prev;           /**< Previous button state (for edge detection) */
    uint32_t last_debounce;     /**< Last debounce timestamp (ticks) */
    uint32_t frame_count;       /**< Total frames rendered */
    uint32_t button_presses;    /**< Total button press count */
} game_state_t;

/** @brief Global game state */
static game_state_t g_game_state = {0};

/* ============================================================================
 * BUTTON INPUT HANDLING
 * ========================================================================= */

/**
 * @brief   Read button state with debouncing
 *
 * @return  true if button is pressed (with debounce), false otherwise
 *
 * @details Reads K1 button (PC13, active low) and applies debounce logic.
 *          Button press is only registered after GAME_BUTTON_DEBOUNCE_MS
 *          milliseconds of stable state.
 */
static bool read_button_debounced(void)
{
    /* Read raw button state (active low, so invert) */
    bool button_raw = !HAL_GPIO_ReadPin(BSP_KEY_PORT, BSP_KEY_PIN);
    
    uint32_t current_tick = os_get_tick_count();
    
    /* Check if enough time has passed since last state change */
    if ((current_tick - g_game_state.last_debounce) > GAME_BUTTON_DEBOUNCE_MS) {
        if (button_raw != g_game_state.button_pressed) {
            g_game_state.last_debounce = current_tick;
            g_game_state.button_pressed = button_raw;
            
            /* Detect rising edge (button press) */
            if (button_raw && !g_game_state.button_prev) {
                g_game_state.button_presses++;
            }
            
            g_game_state.button_prev = button_raw;
        }
    }
    
    return g_game_state.button_pressed;
}

/* ============================================================================
 * GAME TASKS
 * ========================================================================= */

/**
 * @brief   Game input task
 *
 * @details Polls K1 button at high frequency and updates game state.
 *          Runs every 10ms for responsive input handling.
 */
static void game_input_task(void)
{
    while (1) {
        /* Read button with debouncing */
        read_button_debounced();
        
        /* Sleep for 10ms (100 Hz polling rate) */
        task_active_sleep(10);
    }
}

/**
 * @brief   Game render task
 *
 * @details Renders game graphics to LCD at target frame rate (30 FPS).
 *          Currently displays a simple status screen showing:
 *          - Frame count
 *          - Button press count
 *          - Button state indicator
 */
static void game_render_task(void)
{
    /* Display header */
    LCD_ShowString(10, 10, (uint8_t*)"ICARUS GAME DEMO", BLUE, WHITE, 16, 0);
    LCD_ShowString(10, 30, (uint8_t*)"Press K1 button", BLACK, WHITE, 12, 0);
    
    while (1) {
        /* Clear stats area */
        LCD_Fill(10, 60, 118, 100, WHITE);
        
        /* Display frame count */
        char frame_str[32];
        snprintf(frame_str, sizeof(frame_str), "Frames: %lu", g_game_state.frame_count);
        LCD_ShowString(10, 60, (uint8_t*)frame_str, BLACK, WHITE, 12, 0);
        
        /* Display button press count */
        char press_str[32];
        snprintf(press_str, sizeof(press_str), "Presses: %lu", g_game_state.button_presses);
        LCD_ShowString(10, 75, (uint8_t*)press_str, BLACK, WHITE, 12, 0);
        
        /* Display button state indicator */
        if (g_game_state.button_pressed) {
            LCD_Fill(10, 95, 30, 115, GREEN);  /* Green square when pressed */
            LCD_ShowString(35, 100, (uint8_t*)"PRESSED", GREEN, WHITE, 12, 0);
        } else {
            LCD_Fill(10, 95, 30, 115, RED);    /* Red square when released */
            LCD_ShowString(35, 100, (uint8_t*)"RELEASED", RED, WHITE, 12, 0);
        }
        
        /* Increment frame counter */
        g_game_state.frame_count++;
        
        /* Sleep for frame period (30 FPS = ~33ms) */
        task_active_sleep(GAME_FRAME_PERIOD_MS);
    }
}

/* ============================================================================
 * INITIALIZATION
 * ========================================================================= */

/**
 * @brief   Initialize and register game demo tasks
 */
void game_tasks_init(void)
{
    /* Initialize game state */
    g_game_state.button_pressed = false;
    g_game_state.button_prev = false;
    g_game_state.last_debounce = 0;
    g_game_state.frame_count = 0;
    g_game_state.button_presses = 0;
    
    /* Register game tasks */
    os_register_task(game_input_task, "game_input");
    os_register_task(game_render_task, "game_render");
}
