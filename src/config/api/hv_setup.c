/**
 * @file       hv_setup.c
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @brief
 * @version    0.1
 * @date       2022-09-01
 *
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 *
 * Abbreviation:
 * HV  High-voltage
 * PDU power distribution unit
 *
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
#define PRECHG_TIMEOUT_MS         (1000)
#define PRECHG_NUM_OF_ERROR       (3)
#define TURN_ON_PRECHG_DELAY_MS   (500)
#define TURN_OFF_PRECHG_DELAY_MS  (500)
#define TURN_ON_POS_DELAY_MS      (500)
#define TURN_OFF_POS_DELAY_MS     (500)
#define TURN_OFF_NEG_DELAY_MS     (500)
/*TODO:*/
#define BACK_END_VOLT             (100)
#define PRECHG_PERCENTAGE_SETTING (90)
#define RELAY_NEG_OPEN            RELAY_NEG_Clear();
#define RELAY_NEG_CLOSE           RELAY_NEG_Set();
#define RELAY_POS_OPEN            RELAY_POS_Clear();
#define RELAY_POS_CLOSE           RELAY_POS_Set();
#define RELAY_PRECHG_OPEN         RELAY_PRECHG_Clear();
#define RELAY_PRECHG_CLOSE        RELAY_PRECHG_Set();
#define CURRENT                   19
#define PRECHG_CURRENT_OFFSET     20
/*TODO:*/
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Global variables -----------------------------------------------------------*/
/* USER CODE BEGIN GV */
HV_DATA_t HV = {
    .offStatus = HV_SETUP_INIT, .setupStatus = HV_OFF_INIT, .opMode = false, .delayTimeCount = 0, .errorCount = 0};
/* USER CODE END GV */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Function prototypes -------------------------------------------------------*/
/* USER CODE BEGIN FP */

/* USER CODE END FP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/**
 * @brief      High-voltage PDU trun on sequence
 *
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-08-31
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
static void HV_SeqTurnOn(void) {
    HV.offStatus = HV_OFF_INIT;
    switch (HV.setupStatus) {
        default:
        case HV_SETUP_INIT:
            HV.errorCount = 0;
            RELAY_NEG_CLOSE;
            RELAY_POS_OPEN;
            RELAY_PRECHG_OPEN;
            if (HV.delayTimeCount++ >= TURN_ON_PRECHG_DELAY_MS) {
                HV.delayTimeCount = 0;
                HV.setupStatus    = HV_PRECHG_START;
            }
            break;
        case HV_PRECHG_START:
            RELAY_PRECHG_CLOSE;
            RELAY_POS_OPEN;
            if (HV.opMode == MODE_ON) {
                if (CURRENT < PRECHG_CURRENT_OFFSET) {
                    if (HV.delayTimeCount++ >= TURN_ON_POS_DELAY_MS) {
                        HV.delayTimeCount = 0;
                        HV.setupStatus    = HV_PRECHG_FINISH;
                    };
                } else {
                    HV.delayTimeCount = 0;
                    HV.setupStatus    = HV_PRECHG_FAIL;
                }
            }
            break;
        case HV_PRECHG_FINISH:
            RELAY_POS_CLOSE;
            if (HV.delayTimeCount++ >= TURN_OFF_PRECHG_DELAY_MS) {
                HV.delayTimeCount = 0;
                HV.setupStatus    = HV_SETUP_FINISH;
            }
            break;
        case HV_PRECHG_FAIL:
            if (HV.delayTimeCount++ >= PRECHG_TIMEOUT_MS) {
                if (HV.errorCount++ >= PRECHG_NUM_OF_ERROR) {
                    RELAY_PRECHG_OPEN;
                    HV.errorCount     = 0;
                    HV.delayTimeCount = 0;
                    HV.setupStatus    = HV_SETUP_FAULT;
                } else {
                    HV.delayTimeCount = 0;
                    HV.setupStatus    = HV_PRECHG_START;
                }
            };

            break;
        case HV_SETUP_FINISH:
            HV.delayTimeCount = 0;
            RELAY_PRECHG_OPEN;
            if (HV.opMode == MODE_PRECHG) {
                RELAY_PRECHG_CLOSE;
                if (HV.delayTimeCount++ >= TURN_OFF_PRECHG_DELAY_MS) {
                    HV.delayTimeCount = 0;
                    HV.opMode         = HV_PRECHG_START;
                }
            }
            YLED_Set();  // TODO:Delete
            break;
        case HV_SETUP_FAULT:
            HV.delayTimeCount = 0;
            RELAY_NEG_OPEN;
            break;
    }
}

/**
 * @brief      High voltage PDU turn off sequence
 *
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-08-31
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
static void HV_SeqTurnOff(void) {
    HV.setupStatus = HV_SETUP_INIT;
    switch (HV.offStatus) {
        default:
        case HV_OFF_INIT:
            RELAY_PRECHG_OPEN;
            HV.errorCount = 0;
            if (HV.opMode == MODE_EMRG) {
                HV.offStatus = HV_OFF_FORCE;
            } else {
                HV.offStatus = HV_OFF_PREWORK;
            }
            break;
        case HV_OFF_PREWORK:
            if (CURRENT < PRECHG_CURRENT_OFFSET) { /*TODO: change  CURRENT to the Real var  */
                if (HV.delayTimeCount++ >= TURN_OFF_POS_DELAY_MS) {
                    HV.delayTimeCount = 0;
                    RELAY_POS_OPEN;
                    HV.offStatus = HV_OFF_FINISH;
                }
            } else {
                HV.delayTimeCount = 0;
                HV.offStatus      = HV_OFF_FAIL;
            }
            break;
        case HV_OFF_FAIL:
            if (HV.delayTimeCount++ >= PRECHG_TIMEOUT_MS) {
                if (HV.errorCount++ >= PRECHG_NUM_OF_ERROR) {
                    HV.errorCount     = 0;
                    HV.delayTimeCount = 0;
                    HV.offStatus      = HV_OFF_FORCE;
                } else {
                    HV.delayTimeCount = 0;
                    HV.offStatus      = HV_OFF_PREWORK;
                }
            }
            break;
        case HV_OFF_FINISH:
            RELAY_NEG_OPEN;
            YLED_Clear();
            break;
        case HV_OFF_FORCE:
            HV.delayTimeCount = 0;
            RELAY_POS_OPEN;
            RELAY_NEG_OPEN;
            Fault_EventSet(DTC_BMS_RELAY);
            YLED_Clear();
            break;
    }
}

/**
 * @brief      Get high-voltage PDU setup status
 *
 * @return     HV_SETUP_STATUS_e
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-08-31
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
HV_SETUP_STATUS_e HV_SetupStatusGet(void) {
    return HV.setupStatus;
}

/**
 * @brief      Get high-voltage PDU off status
 *
 * @return     HV_OFF_STATUS_e
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-08-31
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
HV_OFF_STATUS_e HV_OffStatusGet(void) {
    return HV.offStatus;
}

/**
 * @brief      High-voltage PDU operation command
 *
 * @param      opMode
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-08-31
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
void HV_ModeCommand(HV_OPERATION_MODE_e opMode) {
    HV.opMode = opMode;
}

/**
 * @brief      High-voltage PDU initialization
 *
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-08-31
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
void HV_Initialize(void) {
    RELAY_POS_OPEN;
    RELAY_NEG_OPEN;
    RELAY_PRECHG_OPEN;
}

/**
 * @brief      High-voltage PDU polling tasks
 *
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-08-31
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
void HV_1ms_Tasks(void) {
    if ((HV.opMode == MODE_ON) || (HV.opMode == MODE_PRECHG)) {
        HV_SeqTurnOn();
    } else {
        HV_SeqTurnOff();
    }
}
/* USER CODE END 0 */

/*******************************************************************************
 End of File
 */
