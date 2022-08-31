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
    HV_SETUP_INIT,
    HV_PRECHG_START,
    HV_PRECHG_FAIL,
    HV_PRECHG_FINISH,
    HV_SETUP_FINISH,
    HV_SETUP_FAULT,
} HV_SETUP_STATUS_e;

typedef enum {
    HV_OFF_INIT,
    HV_OFF_PREWORK,
    HV_OFF_FAIL,
    HV_OFF_FINISH,
    HV_OFF_FORCE,
} HV_OFF_STATUS_e;

typedef enum {
    MODE_OFF,
    MODE_ON,
    MODE_PRECHG,
    MODE_EMRG,
} HV_OPERATION_MODE_e;

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

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Global variables ----------------------------------------------------------*/
/* USER CODE BEGIN GV */

/* USER CODE END GV */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Function prototypes -------------------------------------------------------*/
/* USER CODE BEGIN FP */
HV_SETUP_STATUS_e HV_SetupStatusGet(void);
HV_OFF_STATUS_e HV_OffStatusGet(void);
void HV_ModeCommand(HV_OPERATION_MODE_e opMode);
void HV_Initialize(void);
void HV_1ms_Tasks(void);
/* USER CODE END FP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */
// DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
// DOM-IGNORE-END
/*******************************************************************************
 End of File
 */
