
/* Global define -------------------------------------------------------------*/
/* USER CODE BEGIN GD */

/* USER CODE END GD */

/* Includes ------------------------------------------------------------------*/

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "sys_parameter.h"
#include "bms_ctrl.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define FAULT_INDICATOR_ON
#define FAULT_INDICATOR_OFF
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Globalvariables -----------------------------------------------------------*/
/* USER CODE BEGIN GV */
bool          fProtectionProcess = false;
unsigned char protectionState    = 0;
bool          fEmrgProcess       = false;
unsigned char gEmergencyState    = 0;
/* USER CODE END GV */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Function prototypes -------------------------------------------------------*/
/* USER CODE BEGIN FP */

/* USER CODE END FP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
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

static void BMS_Protection(void) {
    switch (protectionState) {
        case 0:
            HV_ModeCommand(MODE_OFF);
            if ((HV_OffStatusGet() == HV_OFF_FINISH) || (HV_OffStatusGet() == HV_OFF_FORCE)) {
                protectionState++;
            }
            break;
        case 1:
            if (DIN_StateGet(DIN_4) == true) {
                protectionState++;
            } else {
                FAULT_INDICATOR_ON; /*TODO: Set GPIO*/
            }
            break;
        case 2:
            BMS_SoftWareReset();
            protectionState = 0;
        default:
            protectionState = 0;
            break;
    }
}

static void BMS_Emergency(void) {
    switch (gEmergencyState) {
        case 0:
            HV_ModeCommand(MODE_OFF);
            if ((HV_OffStatusGet() == HV_OFF_FINISH) || (HV_OffStatusGet() == HV_OFF_FORCE)) {
                gEmergencyState++;
                Nop();
            }
            break;
        case 1:
            if (DIN_StateGet(DIN_4) == false) {
                gEmergencyState++;
                Nop();
            }
            break;
        case 2:
            /*TODO: set_emergency_2_eeprom();*/
            Nop();
            gEmergencyState++;
            break;
        case 3:
            Nop();
            fEmrgProcess = false;
            break;
        default:
            fEmrgProcess    = false;
            gEmergencyState = 0;
            break;
    }
}

static void BMS_CommandDetect(void) {
    /*TODO: Engineer mode*/
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
    } /*Execute the fault protection command when the fault is occur
        and the BMS_Emergency() has not been executed*/
    else if (((DTC_WorstLevelGet() == ERR_LEVEL_PROTECTION) && (fEmrgProcess == false)) ||
             (fProtectionProcess == true)) {  // While the BMS_Protection() is executing
        BMS_ModeCommand(BMS_OCCUR_FAULT);
        fProtectionProcess = true;
    } /*If the fault occur,command BMS_OFF */
    else if (DTC_WorstLevelGet() == ERR_LEVEL_FAULT) {
        BMS_ModeCommand(BMS_OFF);
    }

    /*Clear the error code and reset BMS_Emergency() task state
     when the BMS_Emergency() has been executed */
    if (fEmrgProcess == false) {
        DTC_FaultOccurClear(DIN_4);
        gEmergencyState = 0;
    }
}

void BMS_ModeCommand(BMS_WORK_MODE_e opMode) {
    bmsData.WorkModeCmd = opMode;
}

void BMS_1ms_Tasks(void) {
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
