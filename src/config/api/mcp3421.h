/**
 * @file       mcp3421.h
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
#ifndef _MCP3421_H
#define _MCP3421_H
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
#include "definitions.h" 
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
    APP_STATE_SENSOR_STATUS_VERIFY,
    APP_STATE_CHECK_SENSOR_READY,
    APP_STATE_READ_ADC_VALUE,
    APP_STATE_XFER_ERROR,
    APP_STATE_IDLE
} APP_STATES_e;

typedef enum {
    APP_TRANSFER_STATUS_IN_PROGRESS,
    APP_TRANSFER_STATUS_SUCCESS,
    APP_TRANSFER_STATUS_ERROR,
    APP_TRANSFER_STATUS_IDLE,
} APP_TRANSFER_STATUS_e;

typedef struct {
    /* The application's current state */
    APP_STATES_e taskState;
    APP_STATES_e pastState;
    I2C_ERROR    errorState;
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
void           MCP3421_Initialize(void);
unsigned short MCP3421_AdcValueGet(void);
/* USER CODE END FP */
/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */
// DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
// DOM-IGNORE-END

#endif /* _MCP3421_H */
/*******************************************************************************
 End of File
 */
