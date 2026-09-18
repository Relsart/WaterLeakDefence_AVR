
#include "Main.h"

#ifndef HMI_H_
#define HMI_H_

/**
 * @brief HMI screens & selected points codes
 */
typedef enum
{
    HMI_STATE_OFF             = 0,    // Display disabled
    HMI_STATE_S0_AUTOROTAT    = 1,    // Autorotation info screen (first)

    HMI_STATE_MAIN_S1_RESET   = 2,    // Main menu, screen 1, point "1-System RESET" selected
    HMI_STATE_MAIN_S1_MANUAL  = 3,    // Main menu, screen 1, point "2-Manual OPEN (CLOSE)" selected
    HMI_STATE_MAIN_S2_SENS    = 4,    // Main menu, screen 2, point "3-Sensor OFF: (YES/NO)" selected
    HMI_STATE_MAIN_S2_SOUND   = 5,    // Main menu, screen 2, point "4-Sound OFF: (YES/NO)" selected
    HMI_STATE_MAIN_S3_SYSINFO = 6,    // Main menu, screen 3, point "6-SYSTEM INFO" selected
    HMI_STATE_MAIN_S3_EXIT    = 7,    // Main menu, screen 3, point "7-EXIT" selected

    HMI_STATE_SYS_S1_ALARMS   = 8,    // System info menu, screen 1: Last rotation time & Alarms counter
    HMI_STATE_SYS_S2_MANUALS  = 9,    // System info menu, screen 2: Manual rotations counters
    HMI_STATE_SYS_S3_RESETS   = 10,   // System info menu, screen 3: Autorotations & Resets counter
    HMI_STATE_SYS_S4_MEMCLEAR = 11,   // System info menu, screen 4, point "1-EEPROM Clear" selected
    HMI_STATE_SYS_S4_RETURN   = 12    // System info menu, screen 4, point "2-Return to Main" selected

} HMI_STATE;

/**
 * @brief Handle HMI data: signals from buttons and update display data
 * @param btnCode code of pressed button
 * @param hmiState current HMI state (selected screen and point)
 */
void handleHMI(HMI_BTN_CODE btnCode, HMI_STATE* hmiState);

#endif /* HMI_H_ */