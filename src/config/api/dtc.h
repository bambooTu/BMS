/**
 * @file       dtc.h
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
#ifndef _DTC_H /* Guard against multiple inclusion */
#define _DTC_H
// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif
// DOM-IGNORE-END

/* Global define -------------------------------------------------------------*/
/* USER CODE BEGIN GD */
#include <stdbool.h>  // Defines true
#include <stddef.h>   // Defines NULL
#include <stdlib.h>   // Defines EXIT_FAILURE
/* USER CODE END GD */

/* Includes ------------------------------------------------------------------*/

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef union {
    struct {
        unsigned OCCP              : 1;  // Charge Over Current Protection
        unsigned ODCP              : 1;  // Discharge OverCurrent Protection
        unsigned OVP               : 1;  // (BUS Voltage) Over Voltage Protection
        unsigned UVP               : 1;  // (BUS Voltage) Under Voltage Protection
        unsigned OTP               : 1;  // Cell Over Temperature Protection
        unsigned UTP               : 1;  // Cell Under Temperature Protection
        unsigned VCELL_UNBALANCE_P : 1;  // Cell Unbalance Protection
        unsigned OCCW              : 1;  // Over Charge Current Warning

        unsigned ODCW              : 1;  // Over Discharge Current Warning
        unsigned BUS_OVW           : 1;  // (BUS Voltage) Over Voltage Warning
        unsigned BUS_UVW           : 1;  // (BUS Voltage) Under Voltage Warning
        unsigned OTW               : 1;  // Cell Over Temperature Warning
        unsigned UTW               : 1;  // Cell Under Temperature Warning
        unsigned VCELL_UNBALANCE_W : 1;  // Cell Unbalance Warning
        unsigned CELL_OVP          : 1;  // Cell Over Voltage Protection
        unsigned CELL_UVP          : 1;  // Cell Under Voltage Protection

        unsigned TCELL_UNBALANCE_W : 1;  // Temperature Unbalance Warning
        unsigned BCU_COMM          : 1;  // Master BCU Communication Error
        unsigned SAFETY_BCU_COMM   : 1;  // Safety BCU Communication Error
        unsigned CURR_SEN_COMM     : 1;  // Shunt Communication Error
        unsigned BMU_COMM          : 1;  // BMU Communication Error
        unsigned PCS_COMM          : 1;  // PCS Communication Error
        unsigned MBMS_COMM         : 1;  // MBMS Communication Error
        unsigned AFE_COMM          : 1;  // AFE Communication Error

        unsigned NTC_OPEN        : 1;  // NTC Disconnected
        unsigned NTC_SHORT       : 1;  // NTC Short
        unsigned SHUT_DOWN       : 1;  // BMS Shutdown?RecordData
        unsigned POWER_LOSS      : 1;  // Power Loss(Occur Relay Cut off)
        unsigned EMERGENCY       : 1;  // Emergency or EMS do not Install
        unsigned EEPROM_CHECKSUM : 1;  // EEPROM CheckSum Error
        unsigned BMS_RELAY       : 1;  // Relay Sticking
        unsigned CURR_DIR_ERR    : 1;  // Current Direction Error

        unsigned PCS_CMD : 1;  // CS Command Error(OVP or UVP)
    } b;
    unsigned long long l;
} DTC_FAULT_t;

typedef enum {
    DTC_OCCP,               // Charge Over Current Protection
    DTC_ODCP,               // Discharge Over Current Protection
    DTC_BUS_OVP,            // Bus Over Voltage Protection
    DTC_BUS_UVP,            // Bus Under Voltage Protection
    DTC_OTP,                // Cell Over Temperature Protection
    DTC_UTP,                // Cell Under Temperature Protection
    DTC_VCELL_UNBALANCE_P,    // Cell Unbalance Protection
    DTC_OCCW,               // Over Charge Current Warning
    DTC_ODCW,               // Over Discharge Current Warning
    DTC_BUS_OVW,            // Bus Over Voltage Warning
    DTC_BUS_UVW,            // Bus Under Voltage Warning
    DTC_OTW,                // Cell Over Temperature Warning
    DTC_UTW,                // Cell Under Temperature Warning
    DTC_VCELL_UNBALANCE_W,  // Cell Unbalance Warning
    DTC_VCELL_OVP,          // Cell Over Voltage Protection
    DTC_VCELL_UVP,          // Cell Under Voltage Protection
    DTC_TCELL_UNBALANCE_W,  // Temperature Unbalance Warning
    DTC_BCU_COMM,           // Master BCU Communication Error
    DTC_SAFETY_BCU_COMM,    // Safety BCU Communication Error
    DTC_CURR_SEN_COMM,      // *Shunt Communication Error
    DTC_BMU_COMM,           // BMU Communication Error
    DTC_PCS_COMM,           // PCS CommunicationError
    DTC_MBMS_COMM,          // MBMS CommunicationError
    DTC_AFE_COMM,           // AFE CommunicationError
    DTC_NTC_OPEN,           // NTC Disconnected
    DTC_NTC_SHORT,          // NTC Short
    DTC_SHUT_DOWN,          // BMS Shutdown?RecordData
    DTC_POWER_LOSS,         // Power Loss(Occur Relay Cutoff)
    DTC_EMERGENCY,          // Emergency or EMS do not Install
    DTC_EEPROM_CHECKSUM,    // EEPROM CheckSum Error
    DTC_BMS_RELAY,          // Relay Sticking
    DTC_CURR_DIR_ERR,       // Current Direction Error
    DTC_PCS_CMD,            // PCS Command Error(OVP?UVP)
    DTC_EVENT_MAX_NUM
} DTC_EVENT_e;

typedef struct {
    int            Limit;        // Protection Limit Conditions 
    int            Release;      // Protection Release Conditions
    unsigned short LimitTime;    // Protection Timing,mSec
    unsigned short ReleaseTime;  // Release Timing,mSec
} FAULT_PARAM_t;                 // Protection/Warning Parameter

typedef enum {
    ERR_LEVEL_NONE,        // Error level 0 : None error
    ERR_LEVEL_MESSAGE,     // Error level 1 : Message to user
    ERR_LEVEL_WARNING,     // Error level 2 : Warning to user
    ERR_LEVEL_FAULT,       // Error level 3 : Recoverable error
    ERR_LEVEL_PROTECTION,  // Error level 4 : Unrecoverable error (Manual system revcoery)
} ERROR_LEVEL_e;

typedef struct {
    unsigned short errorCode;   
    ERROR_LEVEL_e  errorLevel;  
    unsigned char  longCount;   // Indicator long flashing times
    unsigned char  shortCount;  // Indicator short flashing times
} DTC_MESSAGE_TABLE_t;

typedef struct {
    DTC_EVENT_e    event;
    int*           source;
    FAULT_PARAM_t* ptrObject;
} DTC_FAULT_CHECK_TABLE_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Global variables -----------------------------------------------------------*/
/* USER CODE BEGIN GV */
extern volatile const DTC_MESSAGE_TABLE_t DTC_BMS_Message_Table[DTC_EVENT_MAX_NUM];
/* USER CODE END GV */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Function prototypes -------------------------------------------------------*/
/* USER CODE BEGIN FP */
void               DTC_Initialize(void);
void               DTC_1ms_Tasks(void);
unsigned long long DTC_FaultMapGet(void);
unsigned long long DTC_LatchMapGet(void);
void               DTC_LatchMapByteClear(unsigned char byteNum);
bool               DTC_FaultEventGet(DTC_EVENT_e event);
void               DTC_FaultMaskSet(DTC_EVENT_e event);
void               DTC_FaultMaskClear(DTC_EVENT_e event);
void               DTC_FaultOccurSet(DTC_EVENT_e event);
void               DTC_FaultOccurClear(DTC_EVENT_e event);
unsigned short     DTC_ErrorCodeGet(DTC_EVENT_e DTC);
ERROR_LEVEL_e      DTC_ErrorLevelGet(DTC_EVENT_e DTC);
ERROR_LEVEL_e      DTC_WorstLevelGet(void);
/* USER CODE END FP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */
// DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
// DOM-IGNORE-END
#endif /* _DTC_H */
/*******************************************************************************
 End of File
 */
