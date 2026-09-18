
#include <avr/eeprom.h>
#include <inttypes.h>
#include "LCD_Display/LCD.h"
#include "Hmi.h"

//#define PRINT_AUTOROTATION_IN_SECONDS

void handleHMI(HMI_BTN_CODE btnCode, HMI_STATE* _hmiState)
{
    if (!_hmiState || (btnCode == HMI_BTN_NONE))
        return;
        
    HMI_STATE hmiState = *_hmiState;

    /* * * *  Turn the display and the backlight on  * * *  */
    
    timeValues.lcdBacklightS = 30;  // Half minute backlight on
    timeValues.lcdOnS = 60;         // One minute display on
    SET_BIT(LCD_BACKLIGHT_PORT, LCD_BACKLIGHT_PIN);
  
    /* * * *  Navigation through the main menu (up/down)  * * * */
 
    if ((hmiState >= HMI_STATE_MAIN_S1_RESET) && (hmiState <= HMI_STATE_MAIN_S3_EXIT))
    {
        if ((btnCode == HMI_BTN_UP) && (hmiState > HMI_STATE_MAIN_S1_RESET))
        {
            if ((hmiState == HMI_STATE_MAIN_S2_SENS) || (hmiState == HMI_STATE_MAIN_S3_SYSINFO))
                LCD_CommandExec(LCD_CMD_SCREEN_CLEAR);    // Above the 4th or 6th point - new (clear) screen

            hmiState--;
        }

        if ((btnCode == HMI_BTN_DOWN) && (hmiState < HMI_STATE_MAIN_S3_EXIT))
        {
            if ((hmiState == HMI_STATE_MAIN_S1_MANUAL) || (hmiState == HMI_STATE_MAIN_S2_SOUND))
                LCD_CommandExec(LCD_CMD_SCREEN_CLEAR);   // Below the 3rd or 5th point - new (clear) screen
                
            hmiState++;
        }
    }
    
    /* * * *  Navigation through the system info menu (up/down)  * * * */
    
    if (hmiState >= HMI_STATE_SYS_S1_ALARMS)
    {
        if ((btnCode == HMI_BTN_UP) && (hmiState > HMI_STATE_SYS_S1_ALARMS))
        {
            hmiState--;
        }
        
        if ((btnCode == HMI_BTN_DOWN) && (hmiState < HMI_STATE_SYS_S4_RETURN))
        {
            hmiState++;
        }
    }

    /* * * *  Menu points selection (actions executing)  * * * */
    
    if (btnCode == HMI_BTN_ENTER)
    {
        switch (hmiState)
        {
        case HMI_STATE_OFF:
            hmiState = HMI_STATE_S0_AUTOROTAT;
            LCD_CommandExec(LCD_CMD_SCREEN_CLEAR);
            LCD_CommandExec(LCD_CMD_DISPLAY_ON);
            break;
        case HMI_STATE_S0_AUTOROTAT:
            LCD_CommandExec(LCD_CMD_SCREEN_CLEAR);
            hmiState = HMI_STATE_MAIN_S1_RESET;
            break;
        case HMI_STATE_MAIN_S1_RESET:
            mainControl.reset = 1;
            break;
        case HMI_STATE_MAIN_S1_MANUAL:
            if(mainControl.alarm == 0)
            {
                mainControl.hmiClosed = ((mainControl.hmiClosed > 0) ? 0 : 1);
            }
            break;
        case HMI_STATE_MAIN_S2_SENS:
            mainControl.sensIgnore = ((mainControl.sensIgnore > 0) ? 0 : 1);
            break;
        case HMI_STATE_MAIN_S2_SOUND:
            mainControl.noSound = ((mainControl.noSound > 0) ? 0 : 1);
            break;
        case HMI_STATE_MAIN_S3_SYSINFO:
            hmiState = HMI_STATE_SYS_S1_ALARMS;
            break;
        case HMI_STATE_MAIN_S3_EXIT:
            hmiState = HMI_STATE_OFF;
            btnCode = HMI_BTN_NONE;
            LCD_CommandExec(LCD_CMD_SCREEN_CLEAR);
            LCD_CommandExec(LCD_CMD_DISPLAY_OFF);
            CLEAR_BIT(LCD_BACKLIGHT_PORT, LCD_BACKLIGHT_PIN);
            break;
        case HMI_STATE_SYS_S1_ALARMS:   break;
        case HMI_STATE_SYS_S2_MANUALS:  break;
        case HMI_STATE_SYS_S3_RESETS:   break;
        case HMI_STATE_SYS_S4_MEMCLEAR:
            counters.alarms = 0;
            counters.externMoves = 0;
            counters.hmiMoves = 0;
            counters.autoMoves = 0;
            counters.resets = 0;
            timeValues.valvesMoveTimeS = 0;
            eeprom_write_byte((uint8_t*)2, 15); // Reset days before Autorotation number
            eeprom_write_byte((uint8_t*)3, counters.alarms);
            eeprom_write_byte((uint8_t*)4, counters.externMoves);
            eeprom_write_byte((uint8_t*)5, counters.hmiMoves);
            eeprom_write_byte((uint8_t*)6, counters.autoMoves);
            eeprom_write_byte((uint8_t*)7, counters.resets);
            break;
        case HMI_STATE_SYS_S4_RETURN:
            LCD_CommandExec(LCD_CMD_SCREEN_CLEAR);
            LCD_CommandExec(LCD_CMD_DISPLAY_ON);
            hmiState = HMI_STATE_MAIN_S3_SYSINFO;
            break;
        }
    }

    /* * * *  Print info at the display  * * * */

    uint8_t strbuff[20] = {0};
    switch (hmiState)
    {
        case HMI_STATE_S0_AUTOROTAT:
        {
            volatile uint32_t temp = timeValues.autoRotationInS % 86400;
            volatile uint8_t days = (uint8_t)(timeValues.autoRotationInS / 86400);
            volatile uint8_t hours = (uint8_t)(temp / 3600);
            volatile uint8_t minutes = (uint8_t)((temp % 3600) / 60);
            LCD_setCursor(1, 1);
            LCD_PrintString((uint8_t*)"Auto-rotation in");
            #ifndef PRINT_AUTOROTATION_IN_SECONDS
            // Print in days/hours/minutes format:
            sprintf((char *)strbuff, "%02ud %02uh %02um", days, hours, minutes);
            LCD_setCursor(1, 2);
            LCD_PrintString(strbuff);
            #else
            // Print seconds counter:
            sprintf((char *)strbuff, "%lu", timeValues.autoRotationInS);
            LCD_setCursor(1, 2);
            LCD_PrintString(strbuff);
            #endif
            break;
        }
        case HMI_STATE_MAIN_S1_RESET:
        case HMI_STATE_MAIN_S1_MANUAL:
        {
            LCD_setCursor(1, 1);
            LCD_PrintString((uint8_t*)"1-System RESET");
            LCD_setCursor(1, 2);
            LCD_PrintString((uint8_t*)"2-Manual ");
            
            if (mainControl.hmiClosed)
                LCD_PrintString((uint8_t*)"OPEN ");
            else
                LCD_PrintString((uint8_t*)"CLOSE");
            
            LCD_setCursor(1, ((hmiState == HMI_STATE_MAIN_S1_RESET) ? 1 : 2));
            LCD_CommandExec(LCD_CMD_CURSOR_ON);
            LCD_CommandExec(LCD_CMD_CURSOR_FLASH);
            break;
        }
        case HMI_STATE_MAIN_S2_SENS:
        case HMI_STATE_MAIN_S2_SOUND:
        {
            LCD_setCursor(1, 1);
            LCD_PrintString((uint8_t*)"3-Sensor OFF:");
            
            if(mainControl.sensIgnore)
                LCD_PrintString((uint8_t*)"YES");
            else
                LCD_PrintString((uint8_t*)"NO ");
            
            LCD_setCursor(1, 2);
            LCD_PrintString((uint8_t*)"4-Sound OFF:");
            
            if(mainControl.noSound)
                LCD_PrintString((uint8_t*)"YES");
            else
                LCD_PrintString((uint8_t*)"NO ");
            
            LCD_setCursor(1, ((hmiState == HMI_STATE_MAIN_S2_SENS) ? 1 : 2));
            break;
        }
        case HMI_STATE_MAIN_S3_SYSINFO:
        case HMI_STATE_MAIN_S3_EXIT:
        {
            LCD_setCursor(1, 1);
            LCD_PrintString((uint8_t*)"6-SYSTEM INFO");
            LCD_setCursor(1, 2);
            LCD_PrintString((uint8_t*)"7-EXIT");
            LCD_setCursor(1, ((hmiState == HMI_STATE_MAIN_S3_SYSINFO) ? 1 : 2));
            LCD_CommandExec(LCD_CMD_CURSOR_ON);
            LCD_CommandExec(LCD_CMD_CURSOR_FLASH);
            break;
        }        
        case HMI_STATE_SYS_S1_ALARMS:
        {
            LCD_CommandExec(LCD_CMD_SCREEN_CLEAR);
            LCD_CommandExec(LCD_CMD_DISPLAY_ON);     // Cursor disable
            sprintf((char *)strbuff, "Last move:%02us", timeValues.valvesMoveTimeS);
            LCD_setCursor(1, 1);
            LCD_PrintString(strbuff);
            sprintf((char *)strbuff, "N of alarms:%02u", counters.alarms);
            LCD_setCursor(1, 2);
            LCD_PrintString(strbuff);
            break;
        }        
        case HMI_STATE_SYS_S2_MANUALS:
        {
            LCD_CommandExec(LCD_CMD_SCREEN_CLEAR);
            sprintf((char *)strbuff, "N of ExtRmts:%02u", counters.externMoves);
            LCD_setCursor(1, 1);
            LCD_PrintString(strbuff);
            sprintf((char *)strbuff, "N of HMIRmts:%02u", counters.hmiMoves);
            LCD_setCursor(1, 2);
            LCD_PrintString(strbuff);
            break;
        }        
        case HMI_STATE_SYS_S3_RESETS:
        {
            LCD_CommandExec(LCD_CMD_SCREEN_CLEAR);
            LCD_CommandExec(LCD_CMD_DISPLAY_ON);     // Cursor disable
            sprintf((char *)strbuff, "N of AutoRmt:%02u", counters.autoMoves);
            LCD_setCursor(1, 1);
            LCD_PrintString(strbuff);
            sprintf((char *)strbuff, "N of Resets:%02u", counters.resets);
            LCD_setCursor(1, 2);
            LCD_PrintString(strbuff);
            break;
        }        
        case HMI_STATE_SYS_S4_MEMCLEAR:
        case HMI_STATE_SYS_S4_RETURN:
        {
            LCD_CommandExec(LCD_CMD_SCREEN_CLEAR);
            LCD_setCursor(1, 1);
            LCD_PrintString((uint8_t*)"1-EEPROM Clear");
            LCD_setCursor(1, 2);
            LCD_PrintString((uint8_t*)"2-Return to Main");
            LCD_setCursor(1, ((hmiState == HMI_STATE_SYS_S4_MEMCLEAR) ? 1 : 2));
            LCD_CommandExec(LCD_CMD_CURSOR_ON);
            LCD_CommandExec(LCD_CMD_CURSOR_FLASH);
            break;
        }
    }
    
    *_hmiState = hmiState;
}
