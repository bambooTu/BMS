/**
 * @file       bms_ctrl.h
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @brief
 * @version    0.1
 * @date       2022-10-04
 *
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 *
 * Abbreviation:
 * None
 */
#ifndef _BMS_CTRL_H
#define _BMS_CTRL_H
// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif
// DOM-IGNORE-END

/* Global define -------------------------------------------------------------*/
/* USER CODE BEGIN GD */

/* USER CODE END GD */

/* Includes ------------------------------------------------------------------*/

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum {
    SYS_INIT = 0,     // system initialization
    SYS_TURN_OFF,     // Relay open
    SYS_TURN_ON,      // Relay close @ 0 Ampere
    SYS_CHARGING,     // System is charging
    SYS_DISCHARGING,  // System is discharging
    SYS_FAULT,        // System fault cccur and waiting for reset
    SYS_GOTO_RESET,   // Reset mode
    SYS_EMERGENCY,    // Emergency relay off
    SYS_PRE_ON,       // PreChg relay close
    SYS_BMS_MAX
} SYS_STATUS_e;  // BMS System Status

typedef enum {
    /* Remote Control */
    BMS_OFF = 0,        // Power off
    BMS_CHG_ON,         // Remote control power on @ Charge mode
    BMS_DISCHG_ON,      // Remote control power on @ Discharge mode
    BMS_CHG_PRE_ON,     // Remote control pre-Power on @ Charge mode @ Branch parrellel
    BMS_DISCHG_PRE_ON,  // Remote control pre-Power on @ Discharge mode @ Branch parrellel
    /* Internal Control */
    BMS_HAND_ON,      // Manual control
    BMS_RESET,        // BMS software reset
    BMS_OCCUR_FAULT,  // BMS occur fault
    BMS_OCCUR_EMRG,   // EMS is pressed occur
    BMS_CONTROL_MAX
} BMS_WORK_MODE_e;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Globalvariables -----------------------------------------------------------*/
/* USER CODE BEGIN GV */

/* USER CODE END GV */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Function prototypes -------------------------------------------------------*/
/* USER CODE BEGIN FP */
void BMS_ModeCommand(BMS_WORK_MODE_e opMode);
void BMS_Crtl_1ms_Tasks(void);
/* USER CODE END FP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */
// DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
// DOM-IGNORE-END
#endif /* _BMS_CTRL_H */
/*******************************************************************************
 End of File
 */
