/**
 * @file       bms_ctrl.c
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @brief
 * @version    0.1
 * @date       2022-09-01
 *
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 *
 * Abbreviation:
 * BMS Battery Management System
 */

/* Global define -------------------------------------------------------------*/
/* USER CODE BEGIN GD */

/* USER CODE END GD */

/* Includes ------------------------------------------------------------------*/

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "bms_ctrl.h"

#include "can_bms_vs_mbms.h"
#include "debounce.h"
#include "definitions.h"
#include "dtc.h"
#include "hv_setup.h"
#include "sys_parameter.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define FAULT_INDICATOR_ON   // TODO: PDU外殼指示燈開啟
#define FAULT_INDICATOR_OFF  // TODO: PDU外殼指示燈關閉
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Globalvariables -----------------------------------------------------------*/
/* USER CODE BEGIN GV */

/* USER CODE END GV */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
static bool          fProtectionProcess = false;
static unsigned char gProtectionState   = 0;
static bool          fEmrgProcess       = false;
static unsigned char gEmergencyState    = 0;
// TODO :DELETE↓
//static bool          polar = false;
//static unsigned char step  = 0;
// TODO :DELETE↑
/* USER CODE END PV */

/* Function prototypes -------------------------------------------------------*/
/* USER CODE BEGIN FP */

/* USER CODE END FP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/**
 * @brief      BMS software reset handler
 *
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-09-01
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
static void BMS_SoftwareReset(void) {
    HV_ModeCommand(MODE_OFF);
    if (HV_StatusGet() == HV_OFF) {
        __builtin_disable_interrupts();

        /* Unlock System */
        SYSKEY = 0x00000000;
        SYSKEY = 0xAA996655;
        SYSKEY = 0x556699AA;

        RSWRSTSET = _RSWRST_SWRST_MASK;

        /* Read RSWRST register to trigger reset */
        (void)RSWRST;

        /* Prevent any unwanted code execution until reset occurs */
        while (1) {
        };
    }
}

/**
 * @brief      BMS protection handler
 *
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-09-01
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
static void BMS_Protection(void) {
    switch (gProtectionState) {
        case 0:
            fProtectionProcess = true;
            HV_ModeCommand(MODE_OFF);
            if (HV_StatusGet() == HV_OFF) {
                gProtectionState++;
            }
            break;
        case 1:
            if (DIN_StateGet(DIN_4) == true) {
                gProtectionState++;
            } else {
                FAULT_INDICATOR_ON; /*TODO: Set GPIO*/
            }
            break;
        case 2:
            BMS_SoftwareReset();
            gProtectionState = 0;
            break;
        default:
            gProtectionState = 0;
            break;
    }
}

/**
 * @brief      BMS emergency handler
 *
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-09-01
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
static void BMS_Emergency(void) {
    switch (gEmergencyState) {
        case 0:
            fEmrgProcess = true;
            HV_ModeCommand(MODE_OFF);
            if (HV_StatusGet() == HV_OFF) {
                gEmergencyState++;
            }
            break;
        case 1:
            if (DIN_StateGet(DIN_4) == false) {
                gEmergencyState++;
            }
            break;
        case 2:
            /*TODO: set_emergency_2_eeprom();*/
            gEmergencyState++;
            break;
        case 3:
            fEmrgProcess = false;
            break;
        default:
            fEmrgProcess    = false;
            gEmergencyState = 0;
            break;
    }
}

/**
 * @brief      BMS operation command detection
 *
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-09-01
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
static void BMS_CommandDetect(void) {
    // TODO: Engineer mode

    if (MBMS_EngrModeStatusGet() == true) {
        BMS_ModeCommand(MBMS_RelayCommandGet());
    }                                                           /*-----------------------------------------------*/
    else if (((DIN_StateGet(DIN_4) == true) &&                  // When the EMS is pressed
              (fProtectionProcess == false)) ||                 // and the BMS_Protection() is not executing
             (fEmrgProcess == true)) {                          // or the BMS_Emergency() is executing
        BMS_ModeCommand(BMS_OCCUR_EMRG);                        // ,it execute the BMS_OCCUR_EMRG command.
        DTC_FaultOccurSet(DTC_EMERGENCY);                       // Set the error code.
    }                                                           /*-----------------------------------------------*/
    else if (((DTC_WorstLevelGet() == ERR_LEVEL_PROTECTION) &&  // When the fault occurs
              (fEmrgProcess == false)) ||                       // and the BMS_Emergency() is not executing
             (fProtectionProcess == true)) {                    // or the BMS_Protection() is executing
        BMS_ModeCommand(BMS_OCCUR_FAULT);                       // ,it execute the BMS_OCCUR_FAULT command.
    }                                                           /*-----------------------------------------------*/
    else if (DTC_WorstLevelGet() == ERR_LEVEL_FAULT) {          // When the fault occur
        BMS_ModeCommand(BMS_OFF);                               // ,it execute BMS_OFF command.
    }                                                           /*-----------------------------------------------*/
    else {                                                      // User part
        BMS_ModeCommand(MBMS_RelayCommandGet());
        // TODO :DELETE Test Function ↓
        // switch (step) {
        //     case 0:
        //         if (DIN_StateGet(DIN_2) == true) {
        //             step++;
        //         }
        //         break;
        //     case 1:
        //         if (DIN_StateGet(DIN_2) == false) {
        //             step++;
        //         }
        //         break;
        //     case 2:
        //         polar = !polar;
        //         if (polar) {
        //             BMS_ModeCommand(BMS_DISCHG_ON);
        //         } else {
        //             BMS_ModeCommand(BMS_OFF);
        //         }
        //         step = 0;
        //         break;
        //     default:
        //         step = 0;
        //         break;
        // }
        // TODO : DELETE ↑
    }
    /*Clear the error code and reset BMS_Emergency() task state
     when the BMS_Emergency() has been executed */
    if (fEmrgProcess == false) {
        DTC_FaultOccurClear(DTC_EMERGENCY);
        gEmergencyState = 0;
    }
    if (fProtectionProcess == false) {
        gProtectionState = 0;
    }
}

/**
 * @brief      BMS operation mode command
 *
 * @param      opMode
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-09-01
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
void BMS_ModeCommand(BMS_WORK_MODE_e opMode) {
    bmsData.WorkModeCmd = opMode;
}

/**
 * @brief     BMS control task flow
 *
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-09-01
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
void BMS_Crtl_1ms_Tasks(void) {
    BMS_CommandDetect();

    switch (bmsData.WorkModeCmd) {
        case BMS_RESET:
            bmsData.Status = SYS_GOTO_RESET;
            BMS_SoftwareReset();
            break;
        case BMS_OCCUR_FAULT:
            bmsData.Status = SYS_FAULT;
            BMS_Protection();
            break;
        case BMS_OCCUR_EMRG:
            bmsData.Status = SYS_EMERGENCY;
            BMS_Emergency();
            break;
        case BMS_OFF:
            bmsData.Status = SYS_TURN_OFF;
            HV_ModeCommand(MODE_OFF);
            break;
        case BMS_CHG_ON:
            // Jump to next case
        case BMS_DISCHG_ON:
            if (bmsData.BusCurrent == 0) {
                bmsData.Status = SYS_TURN_ON;
            } else if (bmsData.BusCurrent > 0) {
                bmsData.Status = SYS_CHARGING;
            } else if (bmsData.BusCurrent < 0) {
                bmsData.Status = SYS_DISCHARGING;
            }
            HV_ModeCommand(MODE_ON);
            break;
        case BMS_CHG_PRE_ON:
            // Jump to next case
        case BMS_DISCHG_PRE_ON:
            bmsData.Status = SYS_PRE_ON;
            HV_ModeCommand(MODE_PRECHG);
            break;
        default:
            break;
    }
}
/* USER CODE END 0 */
/*******************************************************************************
 End of File
 */
