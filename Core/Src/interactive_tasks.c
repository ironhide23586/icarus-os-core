/**
 * @file    interactive_tasks.c
 * @brief   Interactive Input Demo Tasks Implementation
 * @version 0.1.0
 *
 * @details Simple demo: LED glows when K1 button is pressed.
 *
 * @author  Souham Biswas
 * @date    2026
 *
 * @copyright Copyright (c) 2026 ICARUS Project
 */

#include "interactive_tasks.h"
#include "icarus/icarus.h"
#include "bsp/config.h"
#include "bsp/gpio.h"
#include <stdio.h>

/* ============================================================================
 * DEMO STATE
 * ========================================================================= */

/**
 * @brief Interactive demo state structure
 */
typedef struct {
    bool button_pressed;
    bool button_prev;
    uint32_t last_debounce;
    uint32_t button_press_count;
} interactive_state_t;

/** @brief Global demo state */
static interactive_state_t g_state = {0};

/* ============================================================================
 * HELPER FUNCTIONS
 * ========================================================================= */

/**
 * @brief   Read button state with debouncing
 */
static bool read_button_debounced(void)
{
    /* Read raw button state (active low, so invert) */
    bool button_raw = (HAL_GPIO_ReadPin(BSP_KEY_PORT, BSP_KEY_PIN) == GPIO_PIN_RESET);
    uint32_t current_tick = os_get_tick_count();
    
    if ((current_tick - g_state.last_debounce) > INTERACTIVE_BUTTON_DEBOUNCE_MS) {
        if (button_raw != g_state.button_pressed) {
            g_state.last_debounce = current_tick;
            g_state.button_pressed = button_raw;
            
            if (button_raw && !g_state.button_prev) {
                g_state.button_press_count++;
                printf("Button pressed! Count: %lu\r\n", g_state.button_press_count);
            }
            
            g_state.button_prev = button_raw;
        }
    }
    
    return g_state.button_pressed;
}

/* ============================================================================
 * DEMO TASKS
 * ========================================================================= */

/**
 * @brief   Button monitoring task - LED glows when button pressed
 */
static void interactive_button_task(void)
{
    printf("\r\n");
    printf("╔════════════════════════════════════════════════════════════╗\r\n");
    printf("║           ICARUS OS - Button LED Demo                     ║\r\n");
    printf("╠════════════════════════════════════════════════════════════╣\r\n");
    printf("║  Press K1 button to light up LED                          ║\r\n");
    printf("║  Release button to turn off LED                           ║\r\n");
    printf("╚════════════════════════════════════════════════════════════╝\r\n");
    printf("\r\n");
    
    while (1) {
        read_button_debounced();
        
        /* LED follows button state: pressed = ON, released = OFF */
        /* Note: Logic is inverted - button pressed turns LED OFF, so we invert */
        HAL_GPIO_WritePin(BSP_LED_PORT, BSP_LED_PIN, 
                          g_state.button_pressed ? GPIO_PIN_RESET : GPIO_PIN_SET);
        
        task_active_sleep(10);
    }
}

/* ============================================================================
 * INITIALIZATION
 * ========================================================================= */

/**
 * @brief   Initialize and register interactive demo tasks
 */
void interactive_tasks_init(void)
{
    /* Initialize state */
    g_state.button_pressed = false;
    g_state.button_prev = false;
    g_state.last_debounce = 0;
    g_state.button_press_count = 0;
    
    /* Register task */
    os_register_task(interactive_button_task, "btn_led");
}
