#include <avr/interrupt.h>
#include "Main.h"

/**
 * @brief Timer/Counter1 interrupt handling (once per second) 
 */
ISR(TIMER1_COMPA_vect)
{
    /*  LCD backlight time  */
    if (timeValues.lcdBacklightS > 0)
        timeValues.lcdBacklightS--;
    else
        CLEAR_BIT(LCD_BACKLIGHT_PORT, LCD_BACKLIGHT_PIN);

    /*  LCD operation time  */
    if (timeValues.lcdOnS > 0)
    {
        if (timeValues.lcdOnS == 1)
            mainState.screenOffCmd = 1;

        timeValues.lcdOnS--;
    }

    /*  Alarm sound time  */
    if (timeValues.alarmSoundS > 0)
        timeValues.alarmSoundS--;
    else
        mainControl.alarmSound = 0;

    /*  Valves power supply time  */
    if (timeValues.valvePwrS > 0)
        timeValues.valvePwrS-- ;
    else
        CLEAR_BIT(SUPPL_12V_PORT, SUPPL_12V_PIN);

    /*  Valves auto rotation time  */
    if (timeValues.autoRotationInS >= 1)
        timeValues.autoRotationInS--;
    else
    {
        mainState.autoRotateCmd = 1;
        timeValues.autoRotationInS = AUTOROTATION_PERIOD_S;
    }
        
    /*  Auto rotation timer (days) update in EEPROM  */
    if (timeValues.eepromUpdateS > 0)
    {
        timeValues.eepromUpdateS--;
    }        
    else
    {
        timeValues.eepromUpdateS = 86400;
        mainState.eepromUpdCmd = 1;
    }
        
    /*  Other timers decrementing  */
    if (timeValues.resetCheckDelayS > 0)
        timeValues.resetCheckDelayS--;
    if (timeValues.reserveResetS > 0)
        timeValues.reserveResetS--;
    if (timeValues.remoteDebounceS > 0)
        timeValues.remoteDebounceS--;

    /*  Valves moving time counting  */
    volatile static uint8_t valvesMoveTime = 0;
    if (mainControl.valvesInMotion)
    {
        valvesMoveTime++;
    }
    else if (valvesMoveTime > 0)
    {
        UDR = valvesMoveTime;  // To UART (Debug)
        timeValues.valvesMoveTimeS = valvesMoveTime;
        valvesMoveTime = 0;
    }
}

/**
 * @brief Timer/Counter2 interrupt handling (once per millisecond) 
 */
ISR(TIMER2_COMP_vect)
{
    /*  Single beep signal  */
    if ((mainControl.alarm == 0) && (timeValues.beepMs > 0))
        timeValues.beepMs--;
    
    if ((mainControl.alarm == 0) && (timeValues.beepMs == 0))
        CLEAR_BIT(BEEPER_PORT, BEEPER_PIN);
    
    /*  Alarm sound and LED blinking signalization  */
    static uint16_t alarmIntervalMs = 0;
    if (mainControl.alarm)
    {
        if (alarmIntervalMs == 0)
        {
            alarmIntervalMs = 400;
            mainState.alarmSwitcher = ~(mainState.alarmSwitcher);
        }
        else
        {
            alarmIntervalMs--;
        }
    }
    else
    {
        alarmIntervalMs = 0;
        mainState.alarmSwitcher = 0;
    }
}

/**
 * @brief USART data receiving interrupt handling
 */
ISR(USART_RXC_vect)
{
    uint8_t Input = 0;
    Input = UDR;    // Not used now..
}