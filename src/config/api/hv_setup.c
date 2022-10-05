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
#include "commonly_used.h"
#include "definitions.h"
#include "sys_parameter.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum {
    HV_OFF_INIT,
    HV_OFF_PREWORK,
    HV_OFF_FAIL,
    HV_OFF_FINISH,
    HV_OFF_FORCE,
} HV_OFF_STATUS_e;

typedef enum {
    HV_SETUP_INIT,
    HV_PRECHG_START,
    HV_PRECHG_FAIL,
    HV_PRECHG_FINISH,
    HV_SETUP_FINISH,
    HV_SETUP_FAULT,
} HV_SETUP_STATUS_e;

typedef struct {
    HV_SETUP_STATUS_e   setupStatus;
    HV_OFF_STATUS_e     offStatus;
    HV_OPERATION_MODE_e opMode;
    unsigned int        errorCount;
    unsigned int        delayTimeCount;
} HV_DATA_t;
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
#define RELAY_NEG_OPEN            RELAY_NEG_Clear();     // TODO : Setting GPIO
#define RELAY_NEG_CLOSE           RELAY_NEG_Set();       // TODO : Setting GPIO
#define RELAY_POS_OPEN            RELAY_POS_Clear();     // TODO : Setting GPIO
#define RELAY_POS_CLOSE           RELAY_POS_Set();       // TODO : Setting GPIO
#define RELAY_PRECHG_OPEN         RELAY_PRECHG_Clear();  // TODO : Setting GPIO
#define RELAY_PRECHG_CLOSE        RELAY_PRECHG_Set();    // TODO : Setting GPIO
#define PRECHG_CURRENT_OFFSET     200000                 // unit mA
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
                if (ABS(bmsData.BusCurrent) < PRECHG_CURRENT_OFFSET) {
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
                if (HV.delayTimeCount++ >= TURN_ON_PRECHG_DELAY_MS) {
                    HV.delayTimeCount = 0;
                    HV.opMode         = HV_PRECHG_START;
                }
            }
            YLED_Set();  // TODO:Delete
            break;
        case HV_SETUP_FAULT:
            HV.delayTimeCount = 0;
            RELAY_NEG_OPEN;
            DTC_FaultOccurSet(DTC_BMS_RELAY);
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
            if (ABS(bmsData.BusCurrent) < PRECHG_CURRENT_OFFSET) {
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
            YLED_Clear();  // TODO:Delete
            break;
        case HV_OFF_FORCE:
            HV.delayTimeCount = 0;
            RELAY_POS_OPEN;
            RELAY_NEG_OPEN;
            YLED_Clear();  // TODO:Delete
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
static HV_SETUP_STATUS_e HV_SetupStatusGet(void) {
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
static HV_OFF_STATUS_e HV_OffStatusGet(void) {
    return HV.offStatus;
}

HV_STATUS_e HV_StatusGet(void) {
    static HV_STATUS_e ret;
    if ((HV_OffStatusGet() == HV_OFF_FINISH) || (HV_OffStatusGet() == HV_OFF_FORCE)) {
        ret = HV_OFF;
    } else if (HV_SetupStatusGet() == HV_SETUP_FINISH) {
        ret = HV_ON;
    } else if (HV_SetupStatusGet() == HV_SETUP_FAULT) {
        ret = HV_FAULT;
    } else if (HV_SetupStatusGet() == HV_PRECHG_START) {
        ret = HV_PRECHG;
    }
    return ret;
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
    HV.opMode = MODE_OFF;
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
    bmsData.HvStatus = HV_StatusGet();
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
