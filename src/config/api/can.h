/**
 * @file       can.h
 * @author     Chiou (charlie.chiou@amitatech.com)
 * @brief      CAN API
 * @version    0.1
 * @date       2022-08-18
 *
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 *
 * Abbreviation:
 * PCS             Power Control System
 * GC              Grid Control
 * BMU             Battery Management Unit
 * BMS             Battery Management System
 * MBMS            Master BMS
 * PF              PDU Formt
 */

#ifndef _CAN_H
#define _CAN_H

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif
// DOM-IGNORE-END

/* Global define -------------------------------------------------------------*/
/* USER CODE BEGIN GD */
#define SYSPARAM_CAN_DATA_LENGTH 8
/* USER CODE END GD */
/* Includes ------------------------------------------------------------------*/

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "definitions.h"  // SYS function prototypes
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

typedef enum {
    CL_DEVICE_SLAVE = 0x05,  // Controlled devices such as BMS Host, BMS, MBMS,
                             // parallel controller, Slave device etc.
    CL_DEVICE_MASTER,        // PCS, GC, Master device...etc.
} AMITA_CL;

typedef enum {
    CMD_DATA_GET            = 0xEA,  // Request for response information
    CMD_SET_COMMS_ADDR      = 0xEF,  // Change mailing address
    CMD_SET_PROD_MFG_DATE   = 0x11,  // Set Product manufacturing date
    CMD_SET_PROD_SN1        = 0x12,  // Set Product serial number1
    CMD_SET_PROD_SN2        = 0x13,  // Set Product serial number2
    CMD_WR_PARAM_TO_EEPROM  = 0x15,  // Execute write EEPROM(command:0x84 ~ 0x8F)
    CMD_EX_CTL_CMD          = 0x1A,  // Execute control commands
    CMD_ENTER_ENG_MODE      = 0x1E,  // Request to switch to engineering mode
    CMD_EXT_TIME            = 0x1F,  // Requesting "Engineering Mode" extension time
    CMD_SET_CURR_PARAM      = 0x75,  // Set Current protection parameters
    CMD_SET_VOLT_PARAM      = 0x76,  // Set Voltage protection parameters
    CMD_SET_TEMP_PARAM      = 0x77,  // Set Temperature protection parameters
    CMD_SET_OTHERS_PARAM    = 0x7A,  // Set Other types of protection parameters
    CMD_SET_CURR_CORR_PARAM = 0x8F,  // Current Sensor Correction Parameters
    CMD_REM_CTL_RLY         = 0x90,  // Remote control(Relay)
    CMD_SET_BAL_DATA_PARAM  = 0xBC,  // Set Balance Data parameter(BQ76930)
    CMD_ENTER_BOOTLOADER    = 0x1C,  // Enter Bootloader mode
} AMITA_CMD;

typedef enum {
    PF_GET_PROTO_VER      = 0x01,  // Get communication protocol version
    PF_GET_SEED           = 0x02,  // Get the Seed
    PF_GET_PROD_MFG_DATE  = 0x11,  // Get product manufacturing date
    PF_GET_PROD_SN        = 0x12,  // Get product serial number
    PF_GET_HW_SW_VER      = 0x14,  // Get hardware and firmware version
    PF_GET_CURR_PARM      = 0x75,  // Set current protection parameters
    PF_GET_VOLT_PARM      = 0x76,  // Set voltage protection parameters
    PF_GET_TEMP_PARM      = 0x77,  // Set temperature protection parameters
    PF_GET_OTHERS_PARM    = 0x7A,  // Set other types of protection parameters
    PF_GET_CURR_SENS_PARM = 0x8F,  // Get current sensor parameters
    PF_GET_SYS_INFO       = 0x91,  // Get system parameters(PCS Only)
    PF_GET_BMU_TEMP_VCELL = 0xA0,  // Get the BMU temperature reading and the voltage value of each Cell
    PF_GET_BAL_DATA       = 0xBC,  // Get the currently executed Balance Data
    PF_GET_GAS_GA         = 0xCB,  // Read gas Gauge
    PF_GET_FLT_EVENT      = 0xD0,  // Log fault code message log
    PF_GET_FLT_EVENT_FLAG = 0xD8,  // Get fault message status flag
} AMITA_PF;

typedef struct {
    union {
        struct {  // J1939 Protocol
            uint8_t sourceAddress : 8;
            uint8_t pduSpecific   : 8;
            uint8_t pduFormat     : 8;
            uint8_t dataPage      : 1;
            uint8_t reserved      : 1;
            uint8_t priority      : 3;
        } J1939;

        struct {  // Amita Protocol
            uint8_t   sourceAddress : 8;
            uint8_t   destAddress   : 8;
            AMITA_CMD command       : 8;
            uint8_t   dataPage      : 1;
            uint8_t   reserved      : 1;
            AMITA_CL class          : 3;
        } Amita;
        uint32_t id;
    };
    uint8_t                dlc;
    uint8_t                data[SYSPARAM_CAN_DATA_LENGTH];
    uint32_t               timestamp;
    CANFD_MSG_RX_ATTRIBUTE msgAttr;
} can_msg_t;

typedef enum {
    CAN_EVENT_ERR,
    CAN_EVENT_RX,
    CAN_EVENT_TX,
} CAN_EVENT;
typedef enum {
    CAN_1,
    CAN_2,
    CAN_3,
    CAN_4,
} CAN_MODULE;

typedef union {
    struct {
        uint8_t full     : 1;
        uint8_t empty    : 1;
        uint8_t reserved : 6;
    } b;
    uint8_t byte;
} can_queue_status_t;

typedef struct {
    can_msg_t*         pHead;
    can_msg_t*         pTail;
    can_queue_status_t Status;
    uint32_t           Count;
} can_queue_t;

typedef struct {
    can_queue_t txQueue;
    can_queue_t rxQueue;
    uint16_t    errorCount;
    CANFD_ERROR errorStatus;

} can_object_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define CAN_NUMBER_OF_MODULE 4
#define CAN_QUEUE_SIZE       8

#define CAN_FIFONUM_TX0 1
#define CAN_FIFONUM_RX0 2
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
void CAN_Initialize(void);

uint8_t CAN_PushTxQueue(CAN_MODULE Instance, can_msg_t* pMessage);
uint8_t CAN_PushRxQueue(CAN_MODULE Instance, can_msg_t* pMessage);
uint8_t CAN_PullTxQueue(CAN_MODULE Instance, can_msg_t* pMessage);
uint8_t CAN_PullRxQueue(CAN_MODULE Instance, can_msg_t* pMessage);

uint32_t CAN_GetTxQueueCount(CAN_MODULE Instance);
uint32_t CAN_GetRxQueueCount(CAN_MODULE Instance);

bool CAN_QueueDataXfer(CAN_MODULE Instance);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

// DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
// DOM-IGNORE-END

#endif /* _CAN_H */

/*******************************************************************************
 End of File
 */
