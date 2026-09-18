

#ifndef LCD_H_
#define LCD_H_

#include <avr/io.h>
#include <stddef.h>

/* ---------------- Display commands codes ---------------- */
typedef enum
{
    LCD_CMD_SCREEN_CLEAR = 1,    // Clear the screen, cursor to the beginning
    LCD_CMD_CURSOR_RESET = 2,    // Cursor to the beginning
    LCD_CMD_DISPLAY_ON   = 12,   // Display activation
    LCD_CMD_DISPLAY_OFF  = 8,    // Display shutdown
    LCD_CMD_SCROLL_ON    = 5,    // Enabling screen auto scrolling
    LCD_CMD_SCROLL_OFF   = 4,    // Disabling screen auto scrolling
    LCD_CMD_CURSOR_ON    = 14,   // Enabling the cursor (to disable use the LCD_CMD_DISPLAY_ON command)
    LCD_CMD_CURSOR_FLASH = 15,   // Enabling cursor blinking (to disable use the LCD_CMD_DISPLAY_ON command)
    LCD_CMD_SCREEN_RIGHT = 28,   // Screen shift to the right (by 1 position)
    LCD_CMD_SCREEN_LEFT  = 24,   // Screen shift to the left (by 1 position)
    LCD_CMD_CURSOR_RIGHT = 20,   // Cursor move to the right (by 1 position)
    LCD_CMD_CURSOR_LEFT  = 16    // Cursor move to the left (by 1 position)
    
} LCD_CMD;

typedef void (*msDelayCallback)(uint16_t);

/* ---------------- Inputs Pins and Ports definitions: ---------------- */

#define LCD_DB4_PORT PORTD  /* Data bus pins (DB4..DB7) */
#define LCD_DB4_PIN 4

#define LCD_DB5_PORT PORTD
#define LCD_DB5_PIN 5

#define LCD_DB6_PORT PORTD
#define LCD_DB6_PIN 6

#define LCD_DB7_PORT PORTD
#define LCD_DB7_PIN 7

#define LCD_EN_PORT PORTD   /* Enable pin */
#define LCD_EN_PIN 3

#define LCD_RS_PORT PORTD   /* RS pin */
#define LCD_RS_PIN 2

/* ---------------- User API Functional: ---------------- */

/**
 * @brief Set milliseconds delay callback
 * @param h function pointer void(uint16_t)
 */
void LCD_SetMsDelayCallback(msDelayCallback h);

/**
 * @brief Display initialization
 */
void LCD_init();

/**
 * @brief Print character
 * @param ch character
 */
void LCD_printChar(uint8_t valChar);

/**
 * @brief Print string
 * @param str string
 */
void LCD_PrintString(const uint8_t* str);

/**
 * @brief Execute command
 * @param cmd Command code
 */
void LCD_CommandExec (LCD_CMD cmd);

/**
 * @brief Set cursor to position
 * @param col column
 * @param row 
 */
void LCD_setCursor(uint8_t col, uint8_t row);

#endif /* LCD_H_ */
