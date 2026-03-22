/*
 * mock_lcd.c
 * Mock LCD implementation for unit testing
 */

#include "lcd.h"

// Mock external variables
ST7735_Object_t st7735_pObj = {0};
uint32_t st7735_id = 0;
uint16_t POINT_COLOR = WHITE;
uint16_t BACK_COLOR = BLACK;
ST7735_Ctx_t ST7735Ctx = {0};

// Mock LCD functions (no-ops for testing)
void LCD_Test(void) {
    // Mock: do nothing
}

void LCD_SetBrightness(uint32_t Brightness) {
    (void)Brightness;
    // Mock: do nothing
}

uint32_t LCD_GetBrightness(void) {
    return 100; // Mock: return fixed brightness
}

void LCD_Light(uint32_t Brightness_Dis, uint32_t time) {
    (void)Brightness_Dis;
    (void)time;
    // Mock: do nothing
}

void LCD_ShowChar(uint16_t x, uint16_t y, uint8_t num, uint8_t size, uint8_t mode) {
    (void)x;
    (void)y;
    (void)num;
    (void)size;
    (void)mode;
    // Mock: do nothing
}

void LCD_ShowString(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t size, uint8_t *p) {
    (void)x;
    (void)y;
    (void)width;
    (void)height;
    (void)size;
    (void)p;
    // Mock: do nothing
}

void LCD_ClearLine(uint16_t y, uint16_t height) {
    (void)y;
    (void)height;
    // Mock: do nothing
}