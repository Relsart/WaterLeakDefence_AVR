
#include "Main.h"
#include "Functional.h"

/**
 * @brief Time delay function
 * @param timeMs delay in milliseconds
 */
void delay(uint16_t timeMs)
{
    DELAY_TCNT = 0;
    const uint8_t MsTickCount = 63;   // 1 sec = 4000000 / 64 = 62500, i.e. 62.5 in 1ms (approximately 63)
    while (timeMs != 0)
    {
        if (DELAY_TCNT >= MsTickCount)
        {
            DELAY_TCNT = 0;
            timeMs--;
        }
    }
}

/**
 * @brief Single sound (beep) signal
 * @param timeMs time in milliseconds
 */
void beep (uint16_t timeMs)
{
    timeValues.beepMs = timeMs;  // Beeping time
    
    if (mainControl.noSound == 0)
        SET_BIT(BEEPER_PORT, BEEPER_PIN);
}

/**
 * @brief Valves control
 * @param true: open command, false: close
 */
void valveControl(uint8_t open)
{
    timeValues.valvePwrS = 60;  // Save power supply time
    SET_BIT(SUPPL_12V_PORT, SUPPL_12V_PIN);
    delay(400);
    
    if (open)
        CLEAR_BIT(VALV_CLOSE_PORT, VALV_CLOSE_PIN);
    else
        SET_BIT(VALV_CLOSE_PORT, VALV_CLOSE_PIN);
}

/**
 * @brief Valves auto rotation (against valves souring)
 */
void valvesAutoRotation()
{
    valveControl(CLOSE_CMD);
    delay(1000);
    valveControl(OPEN_CMD);
}

/**
 * @brief Sensors lines scan and handling (+ debounce)
 * @param reset: if true- reset state and debounce counter
 * @return Sensor lines state
 */
uint8_t sensorScan(uint8_t reset)
{
    static uint8_t prevScan = 0;    // Previous scan saved state
    static uint8_t jitter;          // Debounce counter
    
    if (reset)
    {
        jitter = 0;
        prevScan = 0;
        return 0;
    }
    
    uint8_t thisScan = ~SENSOR_PORT;    // Get lines state (+ convert to normal logic)
    thisScan &= SENSOR_MASK;            // Mask other bits
    
    if ((thisScan != prevScan) || (thisScan == 0))  // No alarm or bounce (current != previous)
    {
        prevScan = thisScan;
        jitter = 0;
    }
    else    // There is a suspicion of stable alarm
    {
        if (jitter < BOUNCE)
            jitter++;
        else
            return thisScan;  // It is alarm: return lines state
    }
    return 0;
}

/**
 * @brief HMI buttons scan and handling (+ debounce)
 * @return code of pressed button
 */
HMI_BTN_CODE buttonsScan()
{
    static uint8_t prevScan = 0;        // Previous scan saved state
    static uint8_t jitter;              // Debounce counter
    HMI_BTN_CODE result = HMI_BTN_NONE; // Result button code
    uint8_t thisScan = ~BUTTON_PORT;    // Get buttons state (+ convert to normal logic)
    thisScan >>= 5;                     // Only the high 3 bits are needed
    
    if ((thisScan != prevScan) || (thisScan == 0))    // No alarm or bounce
    {
        prevScan = thisScan;
        jitter=0;
    }
    else    // There is a suspicion of stable button press
    {
        if (jitter < BOUNCE)
            jitter++;
        else
            if (jitter == BOUNCE)    // Once handling mode!
            {
                beep(200);
                jitter++;
                switch (thisScan)
                {
                case 1: 
                    result = HMI_BTN_DOWN;
                    break;
                case 2:
                    result = HMI_BTN_ENTER;
                    break;
                case 4: 
                    result = HMI_BTN_UP;
                    break;
                }
            }
    }
    return result;
}

/**
 * @brief Remote signal (switch or relay) scan for valves closing (+ debounce)
 * @return 1: close command, 0: open
 */
uint8_t remoteCmdScan()
{
    const uint16_t FilterTimeThreshold = 1000;          
    static uint8_t flagLow = 0;
    static uint16_t filterTimeCount = 500;

    uint8_t thisScan = READ_BIT((~REMOTE_CMD_PORT), REMOTE_CMD_PIN) ? 1 : 0;
    if ((flagLow == 0) == (thisScan == 0))  // Signal state hasn't changed
    {
        if ( filterTimeCount != 0 ) 
            filterTimeCount--;
    }
    else    // Signal state has changed
    {
        filterTimeCount++;
        if (filterTimeCount >= FilterTimeThreshold)
        {
            flagLow = (~flagLow) & 1;
            filterTimeCount = 0;
        }
    }

    return flagLow;

}

/**
 * @brief Valves movement detection (+ debounce)
 * @return 1: valves move
 */
uint8_t valvesMovingScan()
{
    static uint8_t flagLow = 0;
    const uint8_t FilterTimeThreshold = 100;
    static uint8_t filterTimeCount = 50;
    
    uint8_t thisScan = READ_BIT((~VALV_MOV_PORT), VALV_MOV_PIN) ? 1 : 0;
    if ((flagLow == 0) == (thisScan == 0))   // Signal state hasn't changed
    {
        if (filterTimeCount != 0)
            filterTimeCount--;
    }
    else    // Signal state has changed
    {
        filterTimeCount++;
        if (filterTimeCount >= FilterTimeThreshold)
        {
            flagLow = (~flagLow) & 1;
            filterTimeCount = 0;
        }
    }

    return flagLow;
}
