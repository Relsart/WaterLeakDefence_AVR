

#include "LCD.h"
#include "Macro.h"

msDelayCallback delayMs;    // Milliseconds delay callback

void LCD_SetMsDelayCallback(msDelayCallback h)
{
    delayMs = h;
}

/**
 * @brief Writing data to the display data bus (in half-byte, 4-line bus mode)
 * @param dt data byte
 */
void LCD_writeByte(uint8_t dt)
{
    if (delayMs == NULL)
        return;

    // Strobe on EN pin:
    SET_EN;
    delayMs(1);

    // Write the HIGH half?byte:
    if((dt & 0x80) == 0) CLEAR_DB7; else SET_DB7;
    if((dt & 0x40) == 0) CLEAR_DB6; else SET_DB6;
    if((dt & 0x20) == 0) CLEAR_DB5; else SET_DB5;
    if((dt & 0x10) == 0) CLEAR_DB4; else SET_DB4;

    // Strobe on EN pin:
    delayMs(1);
    CLEAR_EN;
    delayMs(1);
    SET_EN;

    // Write the LOW half?byte:
    if((dt & 0x08) == 0) CLEAR_DB7; else SET_DB7;
    if((dt & 0x04) == 0) CLEAR_DB6; else SET_DB6;
    if((dt & 0x02) == 0) CLEAR_DB5; else SET_DB5;
    if((dt & 0x01) == 0) CLEAR_DB4; else SET_DB4;
    
    delayMs(1);
    CLEAR_EN;
    delayMs(1);
}

/**
 * @brief Display initialization
 */
void LCD_init()
{
    if (delayMs == NULL)
        return;

    delayMs(15);
    CLEAR_RS;
    SET_EN;
    SET_DB5;
    SET_DB4;
    delayMs(1);
    CLEAR_EN;
    delayMs(2);
    SET_EN;
    delayMs(1);
    CLEAR_EN;
    delayMs(2);
    SET_EN;
    SET_DB5;
    CLEAR_DB4;
    delayMs(1);
    CLEAR_EN;
    delayMs(2);
    LCD_writeByte(0x28);  // 0b00101000: Display mode: 4 lines bus, 2 lines, 5x8 dot character
    delayMs(1);
    LCD_writeByte(0x02);  // 0b00000010: Counter reset
    delayMs(1);
    LCD_writeByte(0x08);  // 0b00001000: Display off 
    delayMs(1);
    LCD_writeByte(0x06);  // 0b00000110: Incrementing the address counter (printing from left to right), the screen does not move.
    delayMs(1);
    LCD_writeByte(0x0C);  // 0b00001100: The screen is on, cursor disabled
    delayMs(1);
    LCD_writeByte(0x01);  // 0b00000001: Screen clear
    delayMs(3);
}

/**
 * @brief Execute command
 * @param cmd Command code
 */
void LCD_CommandExec(LCD_CMD cmd)
{
    if (delayMs == NULL)
        return;

    CLEAR_RS;
    LCD_writeByte((uint8_t)cmd);
    // More delay necessary for this commands:
    if ((cmd == LCD_CMD_SCREEN_CLEAR) || (cmd == LCD_CMD_CURSOR_RESET))
        delayMs(2);
}

/**
 * @brief Print character
 * @param ch character
 */
void LCD_printChar(uint8_t ch)
{
    SET_RS;
    LCD_writeByte(ch);
}

/**
 * @brief Print string
 * @param str string
 */
void LCD_PrintString(const uint8_t* str)
{
    if (str)
    {
        for(uint8_t i = 0; i < 200; i++)
        {
            if(str[i] == 0)
                break;
            LCD_printChar(str[i]);
        }
    }
}

/**
 * @brief Set cursor to position
 * @param col column
 * @param row 
 */
void LCD_setCursor(uint8_t col, uint8_t row)
{
    if (delayMs == NULL)
        return;

    CLEAR_RS;
    if(row == 1)
        LCD_writeByte(0x80 | (col - 1));
    else
        LCD_writeByte(0x80 | (col - 1 + 0x40));
    delayMs(1);
}
