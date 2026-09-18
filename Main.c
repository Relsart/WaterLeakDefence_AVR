

#include <avr/eeprom.h>
#include <avr/interrupt.h>

#include "Hmi.h"
#include "Functional.h"
#include "LCD_Display/LCD.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
                                       *  A  B  O  U  T  *
Author: RelsArt
Date: September 2022 
MCU model: ATmega32 (TQFP)
 
FUSE: Low = 0E, High = C9:
    Power monitor enabled, threshold is 4V
    External quartz 4 MHz, starting after 258 cycles + 4 ms
    JTAG disabled
    Bootloader disabled
 
Used timers:
    TIM0 (8 bit)  - for delay() function (no interruptions handling)
    TIM1 (16 bit) - seconds timer general purpose 
    TIM2 (8 bit)  - beeper
 
EEPROM:  0 byte not used
         1 byte - control byte (not used now)
         2 byte - days before the valves valvesAutoRotation
         3 byte - alarms counter
         4 byte - manual rotations counter (high level to the RMT terminal)
         5 byte - manual rotations counter (commands from HMI)
         6 byte - automotive rotations counter (once in 2 weeks)
         7 byte - resets counter
 
GPIO Pins assignment:
    PA0..PA6- Sensors lines 7..1
    PA7- LCD display backlight control
    
    PB0- Valves control (1-Close, 0-Open)
    PB1- Input signal "Valves in movement"
    PB2- Alarm output signal to the upper level (optocoupler)
    PB3- Input signal for valves closing (toggle switch or upper level relay)
    PB4- Set 12V supply to valves
    PB5- Button UP
    PB6- Button ENTER
    PB7- Button DOWN
    
    PC0..PC6- Channel operations LEDs 7..1 (in inverse order!)
    PC7- Sound signal (beeper)
    
    PD0- UART (RXD)
    PD1- UART (TXD)
    PD2- LCD  (RS) 0 command / 1 data
    PD3- LCD  (EN) command strobe
    PD4- LCD  (DB4)
    PD5- LCD  (DB5)
    PD6- LCD  (DB6)
    PD7- LCD  (DB7)

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/**
 * @brief MCU periphery initialization
 */
void initMcu();

/**
 * @brief Globals
 */
volatile sysControl_t mainControl = {0};
volatile sysState_t mainState = {0};
volatile timeValues_t timeValues = {0};
volatile counters_t counters = {0};
volatile HMI_STATE hmiState = HMI_STATE_OFF;


int main(void)
{
    volatile uint8_t thisScanSensState = 0;     // Current scan sensors lines state
    volatile uint8_t buttonState = 0;           // HMI buttons state
    volatile uint8_t remoteCloseCmd = 0;        // External signal state (1 == close command)
    
    /* * * * Initialization * * * */
    initMcu();
    delay(20);
    LCD_SetMsDelayCallback(delay);  // Set the delay callback for the LCD display driver
    LCD_init();
    
    while(1)    // Main Routine
    {
        /* * * *  Inputs reading  * * * */
        thisScanSensState = (mainControl.sensIgnore == 0) ? sensorScan(SCAN) : 0;
        buttonState = buttonsScan();
        remoteCloseCmd = remoteCmdScan();
        mainControl.valvesInMotion = valvesMovingScan();    // valves are moving now (filtered signal)
    
        /* * * *  First sensor alarmed  * * * */
        if ((mainControl.alarm == 0) && (thisScanSensState != 0))
        {
            cli();
            mainState.sensLineState = thisScanSensState;
            mainControl.alarm = 1;
            mainControl.alarmSound = 1;
            timeValues.alarmSoundS = 180;   // Alarm sound is on for 3 minutes
            SET_BIT(ALARM_OUT_PORT, ALARM_OUT_PIN);
            UDR = 171;   // Alarm code to UART
            valveControl(CLOSE_CMD);
            timeValues.autoRotationInS = AUTOROTATION_PERIOD_S;
            sensorScan(RESET);
            counters.alarms++;
            eeprom_write_byte((uint8_t *)3, counters.alarms);    
            sei();
        }
        
        /* * * *  Additional sensor alarmed  * * * */
        if ((mainControl.alarm != 0) && (thisScanSensState != 0) && (mainState.sensLineState != thisScanSensState))
        {
            mainState.sensLineState |= thisScanSensState;
            sensorScan(RESET);
        }

        /* * * *  LED and sound alarm indication  * * * */
        if (mainControl.alarm != 0)
        {
            if ((mainControl.noSound == 0) && (mainControl.alarmSound))
                ALARM_LEDS_PORT = (mainState.sensLineState | 0b10000000) & mainState.alarmSwitcher;
            else
                ALARM_LEDS_PORT = mainState.sensLineState & mainState.alarmSwitcher;
        }

        /* * * *  System reset  * * * */
        if ((mainControl.alarm != 0) && (mainControl.reset != 0))
        {
            mainControl.reset = 0;
            CLEAR_BIT(BEEPER_PORT, BEEPER_PIN);
            timeValues.resetCheckDelayS = 3;      // 3 seconds to delay
            uint8_t tempState = 0;
            while (timeValues.resetCheckDelayS)   // There should be no alarms within 3 seconds
            {
                tempState = sensorScan(SCAN);
                if (tempState != 0)
                    break;
            }
            
            if (tempState == 0)
            {
                mainControl.alarm = 0;
                if ((mainControl.manualClosed == 0) && (mainControl.hmiClosed == 0))
                {
                    valveControl(OPEN_CMD);
                    timeValues.autoRotationInS = AUTOROTATION_PERIOD_S;
                }
                
                CLEAR_BIT(ALARM_OUT_PORT, ALARM_OUT_PIN);
                ALARM_LEDS_PORT = 0;
                mainState.sensLineState = 0;
                thisScanSensState = 0;
                counters.resets++;
                eeprom_write_byte ((uint8_t *)7, counters.resets);
                beep(1000);
            }
        }
 
        /* * * *  Valves manual operation commands (CLOSE)  * * * */
        if ((mainControl.alarm == 0) && (mainControl.manualClosed == 0) && ((remoteCloseCmd != 0) || (mainControl.hmiClosed)))
        {
            beep(500);
            mainControl.manualClosed = 1;
            valveControl(CLOSE_CMD);
            timeValues.autoRotationInS = AUTOROTATION_PERIOD_S;
            
            if (remoteCloseCmd != 0)
            {
                counters.externMoves++;
                eeprom_write_byte ((uint8_t*)4, counters.externMoves);
            }

            if (mainControl.hmiClosed)
            {
                counters.hmiMoves++;
                eeprom_write_byte ((uint8_t*)5, counters.hmiMoves);
            }
        }

        /* * * *  Valves manual operation commands (OPEN)  * * * */
        if ((mainControl.alarm == 0) && (mainControl.manualClosed != 0) &&  ((remoteCloseCmd == 0) && (mainControl.hmiClosed == 0)))
        {
            timeValues.remoteDebounceS = 2;
            uint8_t tempScan = 0;
            while (timeValues.remoteDebounceS > 0)
            {
                tempScan = remoteCmdScan();
                if (tempScan != 0)
                    break;
            }
            
            if (tempScan == 0)
            {
                valveControl(OPEN_CMD);
                timeValues.autoRotationInS = AUTOROTATION_PERIOD_S;
                mainControl.manualClosed = 0;
                beep(500);
            }
        }
   
        /* * * * *  Deferred signals handling  * * * *  */
        if (mainState.screenOffCmd)
        {
            mainState.screenOffCmd = 0; 
            LCD_CommandExec(LCD_CMD_SCREEN_CLEAR);
            LCD_CommandExec(LCD_CMD_DISPLAY_OFF);
            hmiState = HMI_STATE_OFF;
        }
    
        if (mainState.autoRotateCmd)
        {
            mainState.autoRotateCmd = 0;
            if ((mainControl.alarm == 0) && (mainControl.manualClosed == 0))
            {
                valvesAutoRotation();
                counters.autoMoves++;
                eeprom_write_byte ((uint8_t*)6, counters.autoMoves);
                timeValues.autoRotationInS = AUTOROTATION_PERIOD_S;
            }
        }
        
        if (mainState.eepromUpdCmd)
        {
            mainState.eepromUpdCmd = 0;
            uint8_t days = timeValues.autoRotationInS / 86400;
            eeprom_write_byte ((uint8_t*)2, days);
            timeValues.eepromUpdateS = 86400;
        }    

        /* * * * *  HMI (buttons and display) handling  * * * *  */
        if (buttonState != HMI_BTN_NONE)
        {
            handleHMI(buttonState, &hmiState);
            buttonState = HMI_BTN_NONE;
        }
    
        /* * * * *  ALTERNATIVE Reset (by pressing "Up" and "Down" buttons for 3 seconds)  * * * *  */
        if ((READ_BIT(BUTTON_PORT, BTN_UP_PIN) == 0) && (READ_BIT(BUTTON_PORT, BTN_DOWN_PIN) == 0))
        {
            timeValues.reserveResetS = 3;
            uint8_t resetCondition = 0;    
            while (timeValues.reserveResetS)
            {
                resetCondition = ((READ_BIT(BUTTON_PORT, BTN_UP_PIN) == 0) && (READ_BIT(BUTTON_PORT, BTN_DOWN_PIN) == 0)) ? 1 : 0;
                if (resetCondition == 0)
                    break;
            }
            if (resetCondition)
                mainControl.reset = 1;
        }
    }
}


void initMcu()
{
    timeValues.eepromUpdateS = 86400;
    
    // ------------------ EEPROM -----------------

    uint8_t temp = eeprom_read_byte((const uint8_t*)2);
    timeValues.autoRotationInS = temp * 86400;

    counters.alarms =       eeprom_read_byte((const uint8_t *)3);
    counters.externMoves =  eeprom_read_byte((const uint8_t *)4);
    counters.hmiMoves =     eeprom_read_byte((const uint8_t *)5);
    counters.autoMoves =    eeprom_read_byte((const uint8_t *)6);
    counters.resets =       eeprom_read_byte((const uint8_t *)7);
    
    mainControl.sensIgnore = 0;
    
    // ----------------- GPIO PORTS -----------------

    DDRA =  0b10000000;   // PA0..6- inputs (sensor lines), PA7- output (display backlight)
    PORTA = 0b01111111;   // Pull-up to sensors, output off
    DDRB =  0b00010101;   // Control and buttons (info in "about" section above)
    PORTB = 0b11101010;
    DDRC = 0xFF;          // Outputs (LEDs and beeper)
    PORTC = 0;            // All disabled at startup
    DDRD = 0xFF;          // Outputs: LCD control, PD0, PD1- UART ( TODO: Check UART !!! ) 
    PORTD = 0;

    // ------------------- TIMERS -------------------
    
    TCCR0 = (1<< CS01)|(1<<CS00);               // TC0 clock = 1/64CK
    TCCR1B = (1<< WGM12)|(1<<CS11)|(1<<CS10);   // TC1 clock = 1/64CK, "reset on match" mode
    OCR1A = 62500;                              // 62500 clock cycles for 1 second
    TCCR2 = (1<<WGM21)|(1<<CS21)|(1<<CS20);     // TC2 clock = 1/64CK, "reset on match" mode
    OCR2 = 62;                                  // 62 clock cycles is nearly 1 millisecond
    SET_BIT(TIMSK, ((1<<OCIE1A)|(1<<OCIE2)));   // Enabling interruptions "Compare Match" for TC1 and TC2
    
    // -------------------- UART -------------------
    
    UBRRL = 25;                                 // Baud rate 9600
    UCSRB = (1<<RXEN)|(1<<TXEN)|(1<<RXCIE);     // Enable UART transceiver and Rx interruptions
    UCSRC = (1<<URSEL)|(3<<UCSZ0);              // Frame format 8n1

    // ------------------- OTHER ------------------        
    
    ACSR |= (1<<ACD);    // Comparator disable
    sei();               // Global interrupts enabling
}
