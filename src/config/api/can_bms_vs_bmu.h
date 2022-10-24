/**
 * @file       can_bms_vs_bmu.h
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @brief
 * @version    0.1
 * @date       2022-09-02
 *
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 *
 * Abbreviation:
 * None
 */

#ifndef _CAN_BMS_VS_BMU_H
#define _CAN_BMS_VS_BMU_H
// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif
// DOM-IGNORE-END

/* Global define -------------------------------------------------------------*/
/* USER CODE BEGIN GD */
#define BMU_MAX_NUM               32U      /* The number of bmu */
#define BMU_VCELL_MAX_NUM         7U       /* The number of Vcell in the BMU */
#define BMU_TCELL_MAX_NUM         2U       /* The number of Tcell in the BMU */
#define BMU_ID_OFFSET             1U       /* BMU start ID */
#define GUI_ID_OFFSET             1U       /* PC GUI軟體，從1開始編號 */
#define BMU_RESPONSE_TIMEOUT_TIME 10       /* unit:ms */
#define BMU_TASK_CYCLE_TIME       1000     /* Unit:ms */
#define BMU_RECV_MSG_FLAG         0x07     /* BMU recieve all packet */
#define BMU_NTC_OPEN              (-1000L) /* NTC Open Value  -100.0 Deg. C */
#define BMU_NTC_SHOTR             5000L    /* NTC Short Value  500.0 Deg. C */
#define BMU_AFE_COMM_ERR          9999L    /* AFE I2C communication fault value */
#define BMU_CAN_COMM_ERR          0xFFFF   /* BMU communication fault value */
#define BMU_FAULT_LED_DELAY       2000     /* Unit:ms */
/* USER CODE END GD */

/* Includes ------------------------------------------------------------------*/

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "can.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Global variables -----------------------------------------------------------*/
/* USER CODE BEGIN GV */

/* USER CODE END GV */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Function prototypes -------------------------------------------------------*/
/* USER CODE BEGIN FP */
unsigned short BMU_CellTempGet(unsigned char BMU_ID, unsigned char Cell_ID);
unsigned short BMU_CellVoltGet(unsigned char BMU_ID, unsigned char Cell_ID);
void           BMU_CheckQueueTasks(CAN_MSG_t canRxMsg);
void           BMU_Initialize(void);
void           BMU_1ms_Tasks(void);
/* USER CODE END FP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */
// DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif

// DOM-IGNORE-END
#endif /* _CAN_BMS_VS_BMU_H */
/*******************************************************************************
 End of File
 */
