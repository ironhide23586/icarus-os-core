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
#include "st7735.h"
#include <stdio.h>

/* ============================================================================
 * LCD GLOBALS (from lcd.c)
 * ========================================================================= */

extern ST7735_Object_t st7735_pObj;
extern ST7735_Ctx_t ST7735Ctx;
extern uint32_t st7735_id;

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
    uint8_t text[40];
    uint16_t lcd_width = (uint16_t)ST7735Ctx.Width;
    
    sprintf((char*)text, "ICARUS GAME DEMO");
    LCD_ShowString(10, 10, lcd_width, 16, 16, text);
    
    sprintf((char*)text, "Press K1 button");
    LCD_ShowString(10, 30, lcd_width, 12, 12, text);
    
    while (1) {
        /* Clear stats area */
        ST7735_LCD_Driver.FillRect(&st7735_pObj, 10, 60, 140, 60, BLACK);
        
        /* Display frame count */
        sprintf((char*)text, "Frames: %lu", g_game_state.frame_count);
        LCD_ShowString(10, 60, lcd_width, 12, 12, text);
        
        /* Display button press count */
        sprintf((char*)text, "Presses: %lu", g_game_state.button_presses);
        LCD_ShowString(10, 75, lcd_width, 12, 12, text);
        
        /* Display button state indicator */
        if (g_game_state.button_pressed) {
            ST7735_LCD_Driver.FillRect(&st7735_pObj, 10, 95, 20, 20, GREEN);
            sprintf((char*)text, "PRESSED");
            LCD_ShowString(35, 100, lcd_width, 12, 12, text);
        } else {
            ST7735_LCD_Driver.FillRect(&st7735_pObj, 10, 95, 20, 20, RED);
            sprintf((char*)text, "RELEASED");
            LCD_ShowString(35, 100, lcd_width, 12, 12, text);
        }
        
        /* Increment frame counter */
        g_game_state.frame_count++;
        
        /* Sleep for frame period (30 FPS = ~33ms) */
        task_active_sleep(GAME_FRAME_PERIOD_MS);
    }
}

/* ============================================================================
 * LCD INITIALIZATION
 * ========================================================================= */

/**
 * @brief   Initialize LCD display
 *
 * @details Configures ST7735 LCD in landscape mode and clears screen.
 */
static void init_lcd(void)
{
    /* Configure LCD context */
    ST7735Ctx.Orientation = ST7735_ORIENTATION_LANDSCAPE_ROT180;
    ST7735Ctx.Panel = HannStar_Panel;
    ST7735Ctx.Type = ST7735_0_9_inch_screen;
    
    /* Initialize LCD driver */
    extern ST7735_IO_t st7735_pIO;
    ST7735_RegisterBusIO(&st7735_pObj, &st7735_pIO);
    ST7735_LCD_Driver.Init(&st7735_pObj, ST7735_FORMAT_RBG565, &ST7735Ctx);
    ST7735_LCD_Driver.ReadID(&st7735_pObj, &st7735_id);
    
    /* Set brightness to 100% */
    LCD_SetBrightness(100);
    
    /* Clear screen to black */
    ST7735_LCD_Driver.FillRect(&st7735_pObj, 0, 0, ST7735Ctx.Width, ST7735Ctx.Height, BLACK);
}

/* ============================================================================
 * INITIALIZATION
 * ========================================================================= */

/**
 * @brief   Initialize and register game demo tasks
 */
void game_tasks_init(void)
{
    /* Initialize LCD display */
    init_lcd();
    
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
