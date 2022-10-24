#ifndef _DIGITAL_INPUT_H /* Guard against multiple inclusion */
#define _DIGITAL_INPUT_H
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
#include <stdbool.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum {
    DIN_1,            // Button release
    DIN_2,            // Button turn on
    DIN_3,            // Button turn off
    DIN_4,            // EMS cutoff
    DIN_MAPPING_MAX,  // max
} DIN_MAPPING_e;

typedef struct {
    bool         status;
    unsigned int debounceTime[2];
} DIN_PARAM_t;

typedef struct {
    DIN_MAPPING_e dinNum;
    DIN_PARAM_t*  dinParam;
} DIN_TASK_TABLE_t;
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
void DIN_ParameterInitialize(void);
void DIN_5ms_Tasks(void);
bool DIN_StateGet(DIN_MAPPING_e dinNum);
/* USER CODE END FP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */
// DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
// DOM-IGNORE-END
#endif /* _DIGITAL_INPUT_H */
/*******************************************************************************
 End of File
 */
