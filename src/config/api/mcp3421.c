/**
 * @file       mcp3421.c
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @brief
 * @version    0.1
 * @date       2022-09-05
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
#include "mcp3421.h"
#include "commonly_used.h"
/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */
#define I2C_ERROR_TIMES   100
#define I2C_TIMEOUT_TIMES 100
/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Global variables -----------------------------------------------------------*/
/* USER CODE BEGIN GV */
/* USER CODE END GV */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
static volatile APP_TRANSFER_STATUS_e I2C1_XferStatus = APP_TRANSFER_STATUS_IN_PROGRESS;
APP_I2C_DATA_t                        I2C1_data       = {};  // TODO : Static
static MCP3421_ADDR_t                 mcp3421_addr    = {};
static MCP3421_CONFIG_t               mcp3421_config  = {};
/* USER CODE END PV */

/* Function prototypes -------------------------------------------------------*/
/* USER CODE BEGIN FP */

/* USER CODE END FP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/**
 * @brief      I2C interrupt callback function
 *
 * @param      context I2C transfer status
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-09-05
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
static void APP_I2CCallback(uintptr_t context) {
    APP_TRANSFER_STATUS_e* pXferStatus = (APP_TRANSFER_STATUS_e*)context;
    I2C1_data.timeoutCount             = 0;
    if (I2C1_ErrorGet() == I2C_ERROR_NONE) {
        if (pXferStatus) {
            *pXferStatus = APP_TRANSFER_STATUS_SUCCESS;
        }
    } else {
        if (pXferStatus) {
            *pXferStatus = APP_TRANSFER_STATUS_ERROR;
        }
    }
}
/**
 * @brief      Using I2C Command to set the MCP3421's configuration
 *
 * @param      addr   MCP3421's address
 * @param      config MPC3421's configuration (ref. Datasheet)
 * @return     true   Command Successful
 * @return     false  Command fail
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-09-05
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
static bool MCP3421_WriteConfigCmd(unsigned char addr, unsigned char config) {
    bool ret              = false;
    I2C1_data.txBuffer[0] = config;
    ret                   = I2C1_Write(addr, I2C1_data.txBuffer, 1);
    return ret;
}
/**
 * @brief      Using I2C Command to get the ADC value
 *
 * @param      addr MCP3421's address
 * @return     true
 * @return     false
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-09-05
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
static bool MCP3421_ReadAdcCmd(unsigned char addr) {
    bool ret = false;
    ret      = I2C1_Read(addr, I2C1_data.rxBuffer, 3);
    return ret;
}
/**
 * @brief      MCP3421 parameter initialize
 *
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-09-05
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
void MCP3421_Initialize(void) {
    mcp3421_addr.device_code = MCP3421_DEVICE_CDOE;
    mcp3421_addr.r_w         = READ;

    mcp3421_config.gain            = GAIN1;
    mcp3421_config.sampling_rate   = RATE16BIT;
    mcp3421_config.conversion_mode = CONTINUOUS_MODE;

    I2C1_data.taskState = APP_STATE_SENSOR_STATUS_VERIFY;
    /* Register the TWIHS Callback with transfer status as context */
    I2C1_XferStatus = APP_TRANSFER_STATUS_IN_PROGRESS;
    I2C1_CallbackRegister(APP_I2CCallback, (uintptr_t)&I2C1_XferStatus);
}
/**
 * @brief      Use I2C get the ADC value from IC MCP3421 @ period 10ms
 *
 * @return     unsigned short
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-09-05
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
unsigned short MCP3421_AdcValueGet(void) {
    unsigned short adc_value = 0;
    if (I2C1_data.timeoutCount++ >= I2C_TIMEOUT_TIMES) {
        I2C1_data.timeoutCount = SATURATION(I2C1_data.timeoutCount, I2C_TIMEOUT_TIMES, 0);  // Saturation the value
        I2C1_data.taskState    = APP_STATE_IDLE;
    }
    switch (I2C1_data.taskState) {
        case APP_STATE_SENSOR_STATUS_VERIFY:
            I2C1_data.pastState = APP_STATE_SENSOR_STATUS_VERIFY;
            /* Verify if Sensor is ready */
            I2C1_XferStatus = APP_TRANSFER_STATUS_IN_PROGRESS;
            if (MCP3421_WriteConfigCmd(mcp3421_addr.byte >> 1, mcp3421_config.byte) == false) {
                I2C1_data.taskState = APP_STATE_XFER_ERROR;
            } else {
                I2C1_data.taskState = APP_STATE_CHECK_SENSOR_READY;
            }
            break;

        case APP_STATE_CHECK_SENSOR_READY:
            I2C1_data.pastState = APP_STATE_CHECK_SENSOR_READY;
            if (I2C1_XferStatus == APP_TRANSFER_STATUS_SUCCESS) {
                /* Sensor is ready. */
                I2C1_data.errorCount = 0;
                I2C1_data.taskState  = APP_STATE_READ_ADC_VALUE;
            } else if (I2C1_XferStatus == APP_TRANSFER_STATUS_ERROR) {
                /* Sensor is not ready. */
                I2C1_data.taskState = APP_STATE_XFER_ERROR;
            }
            break;

        case APP_STATE_READ_ADC_VALUE:
            I2C1_data.pastState = APP_STATE_READ_ADC_VALUE;
            if (I2C1_XferStatus == APP_TRANSFER_STATUS_SUCCESS) {
                I2C1_data.errorCount = 0;
                adc_value            = (I2C1_data.rxBuffer[0] << 8) + I2C1_data.rxBuffer[1];
            } else if (I2C1_XferStatus == APP_TRANSFER_STATUS_ERROR) {
                I2C1_data.taskState = APP_STATE_XFER_ERROR;
            }
            I2C1_XferStatus = APP_TRANSFER_STATUS_IN_PROGRESS;
            if (MCP3421_ReadAdcCmd(mcp3421_addr.byte >> 1) == false) {
                I2C1_data.taskState = APP_STATE_XFER_ERROR;
            }
            break;

        case APP_STATE_XFER_ERROR:
            if (I2C1_data.errorCount++ >= I2C_ERROR_TIMES) {
                I2C1_data.errorCount = SATURATION(I2C1_data.errorCount, I2C_ERROR_TIMES, 0);  // Saturation the value
                I2C1_data.taskState  = APP_STATE_IDLE;
            } else {
                if (I2C1_data.pastState <= APP_STATE_CHECK_SENSOR_READY) { 
                    I2C1_data.taskState = APP_STATE_SENSOR_STATUS_VERIFY;
                } else {
                    I2C1_data.taskState = APP_STATE_READ_ADC_VALUE;
                }
            }
            break;

        case APP_STATE_IDLE:
            break;
            // TODO:Current Sensor Timeout
        default:
            break;
    }
    return adc_value;
}
/* USER CODE END 0 */

/*******************************************************************************
 End of File
 */
