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
// TODO :DELETE
static bool          polar = false;
static unsigned char step  = 0;
// TODO :DELETE
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
static void BMS_SoftWareReset(void) {
    HV_ModeCommand(MODE_OFF);
    if ((HV_OffStatusGet() == HV_OFF_FINISH) || (HV_OffStatusGet() == HV_OFF_FORCE)) {
        __builtin_disable_interrupts();

        /* Unlock System */
        SYSKEY = 0x00000000;
        SYSKEY = 0xAA996655;
        SYSKEY = 0x556699AA;

        RSWRSTSET = _RSWRST_SWRST_MASK;

        /* Read RSWRST register to trigger reset */
        (void)RSWRST;

        /* Prevent any unwanted code execution until reset occurs */
        while (1)
            ;
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
            HV_ModeCommand(MODE_OFF);
            if ((HV_OffStatusGet() == HV_OFF_FINISH) || (HV_OffStatusGet() == HV_OFF_FORCE)) {
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
            BMS_SoftWareReset();
            gProtectionState = 0;
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
            HV_ModeCommand(MODE_OFF);
            if ((HV_OffStatusGet() == HV_OFF_FINISH) || (HV_OffStatusGet() == HV_OFF_FORCE)) {
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
    /*
        bool fProtectionProcess = false;

        if(!ENGR){
            fProtectionProcess = false;
        }

        if(ENGR){
            if ((HV_OffStatusGet() == HV_OFF_FINISH) || (HV_OffStatusGet() == HV_OFF_FORCE) || fProtectionProcess) {
                BMS_EngineerMode();
            }
            esle{
                BMS_ModeCommand(BMS_OFF);
                fProtectionProcess = ture;
            }
        }
    else
    */
    /*Execute the emergency stop command when the EMS is pressed
     and the BMS_Protection() has not been executed*/
    if (((DIN_StateGet(DIN_4) == true) && (fProtectionProcess == false)) ||
        (fEmrgProcess == true)) {  // While the BMS_Emergency() is executing
        BMS_ModeCommand(BMS_OCCUR_EMRG);
        fEmrgProcess = true;
    }
    /*Execute the fault protection command when the fault is occur
        and the BMS_Emergency() has not been executed*/
    else if (((DTC_WorstLevelGet() == ERR_LEVEL_PROTECTION) && (fEmrgProcess == false)) ||
             (fProtectionProcess == true)) {  // While the BMS_Protection() is executing
        BMS_ModeCommand(BMS_OCCUR_FAULT);
        fProtectionProcess = true;
    }
    /*If the fault occur,command BMS_OFF */
    else if (DTC_WorstLevelGet() == ERR_LEVEL_FAULT) {
        BMS_ModeCommand(BMS_OFF);
    }

    /*Clear the error code and reset BMS_Emergency() task state
     when the BMS_Emergency() has been executed */
    if (fEmrgProcess == false) {
        DTC_FaultOccurClear(DIN_4);
        gEmergencyState = 0;
    }

    // User part
    // TODO :DELETE Test Function ↓
    switch (step) {
        case 0:
            if (DIN_StateGet(DIN_2) == true) {
                step++;
            }
            break;
        case 1:
            if (DIN_StateGet(DIN_2) == false) {
                step++;
            }
            break;
        case 2:
            polar = !polar;
            if (polar) {
                BMS_ModeCommand(BMS_DISCHG_ON);
            } else {
                BMS_ModeCommand(BMS_OFF);
            }
            step = 0;
        default:
            step = 0;
            break;
    }
    // TODO : DELETE ↑
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
            BMS_SoftWareReset();
            break;
        case BMS_OCCUR_FAULT:
            BMS_Protection();
            break;
        case BMS_OCCUR_EMRG:
            BMS_Emergency();
            break;
        case BMS_OFF:
            HV_ModeCommand(MODE_OFF);
            break;
        case BMS_CHG_ON:
        case BMS_DISCHG_ON:
            HV_ModeCommand(MODE_ON);
            break;
        case BMS_CHG_PRE_ON:
        case BMS_DISCHG_PRE_ON:
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
