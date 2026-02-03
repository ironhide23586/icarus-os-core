/*
 * lcd.h mock
 * Mock for LCD functionality during testing
 */

#ifndef __LCD_H
#define __LCD_H

#include <stdint.h>

// Mock LCD color constants
#define WHITE         	 0xFFFF
#define BLACK         	 0x0000	  
#define BLUE         	 0x001F  
#define RED           	 0xF800
#define GREEN         	 0x07E0
#define YELLOW        	 0xFFE0

// Mock LCD types (simplified)
typedef struct {
    uint32_t dummy;
} ST7735_Object_t;

typedef struct {
    uint32_t dummy;
} ST7735_Ctx_t;

// Mock external variables
extern ST7735_Object_t st7735_pObj;
extern uint32_t st7735_id;
extern uint16_t POINT_COLOR;
extern uint16_t BACK_COLOR;
extern ST7735_Ctx_t ST7735Ctx;

// Mock LCD functions (no-ops for testing)
void LCD_Test(void);
void LCD_SetBrightness(uint32_t Brightness);
uint32_t LCD_GetBrightness(void);
void LCD_Light(uint32_t Brightness_Dis, uint32_t time);
void LCD_ShowChar(uint16_t x, uint16_t y, uint8_t num, uint8_t size, uint8_t mode);
void LCD_ShowString(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t size, uint8_t *p);
void LCD_ClearLine(uint16_t y, uint16_t height);

#endif /* __LCD_H */