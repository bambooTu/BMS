#ifndef _CAN_BMS_VS_MBMS_H
#define _CAN_BMS_VS_MBMS_H
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
#include "can.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct {
    unsigned short subCmd;
    unsigned char  length;
    void          *ptrVariable;
} PARAM_POINT_t;
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
void MBMS_1ms_tasks(void);
void MBMS_CheckQueueTasks(CAN_MSG_t *canRxMsg);
/* USER CODE END FP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */
// DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
// DOM-IGNORE-END
#endif /* _CAN_BMS_VS_MBMS_H */
/*******************************************************************************
 End of File
 */
