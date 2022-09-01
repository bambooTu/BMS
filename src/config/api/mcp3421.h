// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif
// DOM-IGNORE-END

/* Global define -------------------------------------------------------------*/
/* USER CODE BEGIN GD */
#define MCP3421_DEVICE_CDOE 0xD
#define I2C_TX_BUFFER_SIZE  8
#define I2C_RX_BUFFER_SIZE  8
/* USER CODE END GD */

/* Includes ------------------------------------------------------------------*/

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "../config/default/library/i2cbb/i2c_bb_local.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef union {
    struct {
        unsigned r_w         : 1;
        unsigned reserve     : 3;
        unsigned device_code : 4;
    };
    unsigned char byte;
} MCP3421_ADDR_t;

typedef union {
    struct {
        unsigned gain            : 2;
        unsigned sampling_rate   : 2;
        unsigned conversion_mode : 1;
        unsigned reserve         : 2;
        unsigned ready_flag      : 1;
    };
    unsigned char byte;
} MCP3421_CONFIG_t;

typedef enum {
    WRITE,
    READ
} IIC_R_W_e;

typedef enum {
    GAIN1,
    GAIN2,
    GAIN4,
    GAIN8
} MCP3421_GAIN_e;

typedef enum {
    RATE12BIT,
    RATE14BIT,
    RATE16BIT,
    RATE18BIT,
} MCP3421_RATE_e;

typedef enum {
    ONE_SHOT_MODE,
    CONTINUOUS_MODE,
} MCP3421_MODE_e;

typedef enum {
    I2C_TRANSFER_STATUS_IN_PROGRESS,
    I2C_TRANSFER_STATUS_SUCCESS,
    I2C_TRANSFER_STATUS_ERROR,
    I2C_TRANSFER_STATUS_IDLE,
} I2C_TRANSFER_STATUS_e;

typedef enum {
    /* Application's state machine's initial state. */
    I2C_STATE_INIT = 0,
    I2C_STATE_READ_ADC,
    I2C_STATE_READ_ADC_FINISH,
    I2C_STATE_XFER_ERROR,
    I2C_STATE_COMM_ERROR
} APP_I2C_STATES_e;

typedef struct {
    /* The application's current state */
    APP_I2C_STATES_e taskState;
    I2CBB_ERROR      errorState;
    /* TODO: Define any additional data used by the application. */
    unsigned char txBuffer[I2C_TX_BUFFER_SIZE];
    unsigned char rxBuffer[I2C_RX_BUFFER_SIZE];
    unsigned char errorCount;
    unsigned char timeoutCount;
} APP_I2C_DATA_t;
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
void  MCP3421_Initialize(void);
short MCP3421_AdcValueGet(void);
void  MCP3421_InterruptTasks(void);
void  MCP3421_ReadAdc(unsigned char addr);
void  MCP3421_WriteConfig(unsigned char addr, unsigned char config);
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
