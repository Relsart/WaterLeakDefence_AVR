

#ifndef MAIN_HEADER_H_
#define MAIN_HEADER_H_

#include <avr/io.h>
#include <stdio.h>

/**
 * @brief Main system control & state struct
 */
typedef struct
{
    uint8_t alarm           : 1;    // Leakage Alarm
    uint8_t manualClosed    : 1;    // Valves manual closed (by HMI command or external signal)
    uint8_t reset           : 1;    // System reset command (from HMI)
    uint8_t hmiClosed       : 1;    // Valves close command (from HMI)
    uint8_t sensIgnore      : 1;    // Sensor disabling mode
    uint8_t noSound         : 1;    // Sound disabling mode
    uint8_t alarmSound      : 1;    // Alarm sound signalization is on 
    uint8_t valvesInMotion  : 1;    // Valves in movement

} __attribute__((packed)) sysControl_t;

/**
 * @brief States & flags for deferred handling (from ISR)
 */
typedef struct
{
    uint8_t sensLineState;  // State of sensors lines (bits 0..6)
    uint8_t alarmSwitcher;  // Switcher flag for alarm led blinking and beeping
    uint8_t screenOffCmd;   // Flag: switch LCD screen off
    uint8_t autoRotateCmd;  // Flag: valves auto rotation command
    uint8_t eepromUpdCmd;   // Flag: update Autorotation day counter in EEPROM
    
} sysState_t;

/**
 * @brief Time values
 */
typedef struct
{
    uint16_t beepMs;            // Time of single (info) beep sound in milliseconds
    uint8_t alarmSoundS;        // Time of alarm sound in seconds
    uint8_t valvePwrS;          // Time of valves power supply (12V) in seconds
    uint8_t lcdBacklightS;      // Time of LCD backlight in seconds
    uint8_t lcdOnS;             // Time of LCD operation in seconds
    uint32_t autoRotationInS;   // Time in seconds before valves auto-rotation
    uint8_t resetCheckDelayS;   // Checking delay before system resetting in seconds. It should be no alarms during this time
    uint8_t reserveResetS;      // Alternate reset delay (time of pressing UP and DOWN at the same time in seconds)
    uint8_t remoteDebounceS;    // Debounce timer (in seconds) for remote valves open
    uint8_t valvesMoveTimeS;    // Time of last valves movement in seconds
    uint32_t eepromUpdateS;     // Timer for EEPROM updating (once per day) in seconds
   
} timeValues_t;

#define AUTOROTATION_PERIOD_S 1296000;  // 15 days

/**
 * @brief Events counters
 */
typedef struct 
{
    uint8_t alarms;        // Count of leakage alarms
    uint8_t externMoves;   // Count of valves moves (external signal command)
    uint8_t hmiMoves;      // Count of valves moves (HMI command)
    uint8_t autoMoves;     // Count of auto valves rotations (once per 2 weeks)
    uint8_t resets;        // Count of reset commands
    
} counters_t;    


/* * * * * * * *   H M I   * * * * * * * */

/**
 * @brief Buttons codes
 */
typedef enum
{
    HMI_BTN_NONE    = 0,
    HMI_BTN_UP      = 1,
    HMI_BTN_ENTER   = 2,
    HMI_BTN_DOWN    = 3

} HMI_BTN_CODE;


/* * * * * * * *   G L O B A L S   * * * * * * * */

extern volatile sysControl_t mainControl;
extern volatile sysState_t  mainState;
extern volatile timeValues_t timeValues;
extern volatile counters_t counters;

/* * * * * * * *   M A C R O   * * * * * * * */

/*  Output pins  */

#define SET_BIT(REG, BIT)    ((REG) |= (BIT))
#define CLEAR_BIT(REG, BIT)  ((REG) &= ~(BIT))
#define READ_BIT(REG, BIT)   ((REG) & (BIT))

#define LCD_BACKLIGHT_PORT PORTA    /* LCD display backlight control pin */
#define LCD_BACKLIGHT_PIN  (1 << 7)

#define SUPPL_12V_PORT PORTB        /* Set 12V supply to valves pin */
#define SUPPL_12V_PIN  (1 << 4)

#define ALARM_LEDS_PORT PORTC       /* Channel operations LEDs 7..1 (in inverse order!) */

#define BEEPER_PORT PORTC           /* Sound signal (beeper) pin */
#define BEEPER_PIN  (1 << 7)

#define ALARM_OUT_PORT PORTB        /* Alarm output signal to the upper level (optocoupler) */
#define ALARM_OUT_PIN  (1 << 2)

#define VALV_CLOSE_PORT PORTB       /* Valves control pin (1-Close, 0-Open) */
#define VALV_CLOSE_PIN  (1 << 0)

/*  Input pins  */

#define SENSOR_PORT PINA            /* Sensors lines (7) */
#define SENSOR_MASK 0x7F

#define BUTTON_PORT PINB            /* Buttons port */
#define BTN_UP_PIN  (1 << 5)        /* Button "UP" */
#define BTN_ENTER_PIN  (1 << 6)     /* Button "ENTER" */
#define BTN_DOWN_PIN  (1 << 7)      /* Button "DOWN" */

#define REMOTE_CMD_PORT PINB        /* Input signal for valves closing (switch or relay) */
#define REMOTE_CMD_PIN  (1 << 3)

#define VALV_MOV_PORT PINB          /* Input signal "valves in movement" */
#define VALV_MOV_PIN  (1 << 1)

#define DELAY_TCNT TCNT0            // Delays timer/counter

#define BOUNCE 250
//#define MOVE_TIME 60

#endif // MAIN_HEADER_H_