/**
 * @file       debounce.c
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @brief
 * @version    0.1
 * @date       2022-10-24
 *
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 *
 * Abbreviation:
 * None
 */
/* Global define -------------------------------------------------------------*/
/* USER CODE BEGIN GD */

/* USER CODE END GD */

/* Includes ------------------------------------------------------------------*/

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "debounce.h"

#include "definitions.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define RISING  0
#define FALLING 1
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Global variables -----------------------------------------------------------*/
/* USER CODE BEGIN GV */
unsigned int     gDebounceTimeCount[2][DIN_MAPPING_MAX];
DIN_PARAM_t      gDinParamTable[DIN_MAPPING_MAX];
DIN_TASK_TABLE_t gDinTaskTable[] = {
    {          DIN_1,           &gDinParamTable[DIN_1]}, // Button release
    {          DIN_2,           &gDinParamTable[DIN_2]}, // Button turn on
    {          DIN_3,           &gDinParamTable[DIN_3]}, // Button turn off
    {          DIN_4,           &gDinParamTable[DIN_4]}, // EMS cutoff
    {DIN_MAPPING_MAX, &gDinParamTable[DIN_MAPPING_MAX]}, // max
};
/* USER CODE END GV */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Function prototypes -------------------------------------------------------*/
/* USER CODE BEGIN FP */

/* USER CODE END FP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static void DIN_SetState(DIN_MAPPING_e dinNum) {
    gDinParamTable[dinNum].status = true;
}

static void DIN_ClearState(DIN_MAPPING_e dinNum) {
    gDinParamTable[dinNum].status = false;
}
/**
 * @brief      Get digital input status
 *             (Reference DIN_MAPPING_e)
 * @param      dinNum Digital input number
 * @return     true
 * @return     false
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-09-13
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
static bool DIN_PinGet(DIN_MAPPING_e dinNum) {
    bool ret = false;
    switch (dinNum) {
        case DIN_1:
            ret = !SW1_Get();
            break;
        case DIN_2:
            ret = !SW2_Get();
            break;
        case DIN_3:
            ret = false;
            break;
        case DIN_4:
            ret = !SW3_Get();
            break;
        case DIN_MAPPING_MAX:
            // jump to default
        default:
            ret = false;
            break;
    }
    return ret;
}
/**
 * @brief      Digital input debounce
 *
 * @param      ptrObj Digital input task table (User establish Ref. gDinTaskTable[])
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-09-13
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
static void DIN_Debounce(DIN_TASK_TABLE_t* ptrObj) {
    if (DIN_PinGet(ptrObj->dinNum) == true) {
        if (gDebounceTimeCount[RISING][ptrObj->dinNum]-- == 0) {
            DIN_SetState(ptrObj->dinNum);
            gDebounceTimeCount[RISING][ptrObj->dinNum] = ptrObj->dinParam->debounceTime[RISING];
        }
        gDebounceTimeCount[FALLING][ptrObj->dinNum] = ptrObj->dinParam->debounceTime[FALLING];
    } else {
        if (gDebounceTimeCount[FALLING][ptrObj->dinNum]-- == 0) {
            DIN_ClearState(ptrObj->dinNum);
            gDebounceTimeCount[FALLING][ptrObj->dinNum] = ptrObj->dinParam->debounceTime[FALLING];
        }
        gDebounceTimeCount[RISING][ptrObj->dinNum] = ptrObj->dinParam->debounceTime[RISING];
    }
}
/**
 * @brief      Initialize digital input parameter
 *
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-09-13
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
void DIN_Initialize(void) {
    // Set the falling and rising edge debounce time 
    for (DIN_MAPPING_e i = DIN_1; i < DIN_MAPPING_MAX; i++) {
        gDinParamTable[i].debounceTime[RISING]  = 4;
        gDinParamTable[i].debounceTime[FALLING] = 4;
    }
}
/**
 * @brief      Digital input polling tasks
 *
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-09-13
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
void DIN_5ms_Tasks(void) {
    for (DIN_MAPPING_e i = DIN_1; i < DIN_MAPPING_MAX; i++) {
        DIN_Debounce(&gDinTaskTable[i]);
    }
}
/**
 * @brief      Get digital input to debounce result
 *
 * @param      dinNum Digital input number
 * @return     true
 * @return     false
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-09-13
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
bool DIN_StateGet(DIN_MAPPING_e dinNum) {
    return gDinParamTable[dinNum].status;
}
/* USER CODE END 0 */

/*******************************************************************************
 End of File
 */