/**
 * @file       indicator.h
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
#ifndef _INDICATOR_H
#define _INDICATOR_H
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
    INVTERVAL_RESET,
    INVTERVAL_LONG_DELAY,
    INVTERVAL_LONG,
    INVTERVAL_SHORT_DELAY,
    INVTERVAL_SHORT,
} INTERVAL_e;
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
void IND_Initialize(void);
void IND_1ms_Tasks(void);
/* USER CODE END FP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */
// DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
// DOM-IGNORE-END
#endif /* _INDICATOR_H */
/*******************************************************************************
 End of File
 */
