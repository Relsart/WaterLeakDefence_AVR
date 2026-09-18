
#ifndef FUNCTIONAL_H_
#define FUNCTIONAL_H_

/**
 * @brief Time delay function
 * @param time delay in milliseconds
 */
void delay(uint16_t timeMs);

/**
 * @brief Single sound (beep) signal
 * @param sound time in milliseconds
 */
void beep (uint16_t time);

/**
 * @brief Valves control
 * @param 1: open command, 0: close
 */
void valveControl(uint8_t open);

#define OPEN_CMD 1
#define CLOSE_CMD 0

/**
 * @brief Valves auto rotation (against valves souring)
 */
void valvesAutoRotation();


/**
 * @brief Sensors lines scan and handling (debounce)
 * @param reset: if 1- reset state and debounce counter
 */
uint8_t sensorScan(uint8_t reset);

#define SCAN 0
#define RESET 1

/**
 * @brief HMI buttons scan and handling (debounce)
 * @return code of pressed button
 */
HMI_BTN_CODE buttonsScan();

/**
 * @brief Remote signal (switch or relay) scan for valves closing (+ debounce)
 * @return 1: close command, 0: open
 */
uint8_t remoteCmdScan();

/**
 * @brief Valves movement detection (+ debounce)
 * @return 1: valves move
 */
uint8_t valvesMovingScan();

#endif /* FUNCTIONAL_H_ */
