/**
 * @file       dtc.c
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @brief
 * @version    0.1
 * @date       2022-10-24
 *
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 *
 * Abbreviation:
 * DTC      Diagnostic code
 */
/* Global define -------------------------------------------------------------*/
/* USER CODE BEGIN GD */

/* USER CODE END GD */

/* Includes ------------------------------------------------------------------*/

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "dtc.h"

#include "sys_parameter.h"
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
DTC_FAULT_t dtc      = {.l = 0};
DTC_FAULT_t dtcLatch = {.l = 0};
DTC_FAULT_t dtcMask  = {.l = 0};

DTC_FAULT_CHECK_TABLE_t FaultCheckTaskTable[] = {
    {        DTC_VCELL_OVP,   &bmsData.MaxVcell, &eepBms.CellOVP}, // Cell Over Voltage Protection
    {        DTC_VCELL_UVP,   &bmsData.MinVcell, &eepBms.CellUVP}, // Cell Under Voltage Protection
    {DTC_VCELL_UNBALANCE_W,  &bmsData.DeltaVolt, &eepBms.CellUBW}, // Cell Unbalance Warning
    {DTC_VCELL_UNBALANCE_P,  &bmsData.DeltaVolt, &eepBms.CellUBP}, // Cell Unbalance Protection
    {          DTC_BUS_OVW, &bmsData.BusVolt_mV,  &eepBms.BusOVW}, // (BUS Voltage) Over Voltage Warning
    {          DTC_BUS_OVP, &bmsData.BusVolt_mV,  &eepBms.BusOVP}, // (BUS Voltage) Over Voltage Protection
    {          DTC_BUS_UVW, &bmsData.BusVolt_mV,  &eepBms.BusUVW}, // (BUS Voltage) Under Voltage Warning
    {          DTC_BUS_UVP, &bmsData.BusVolt_mV,  &eepBms.BusUVP}, // (BUS Voltage) Under Voltage Protection
    {             DTC_OCCW, &bmsData.BusCurrent,    &eepBms.OCCW}, // Over Charge Current Warning
    {             DTC_OCCP, &bmsData.BusCurrent,    &eepBms.OCCP}, // Charge Over Current Protection
    {             DTC_ODCW, &bmsData.BusCurrent,    &eepBms.ODCW}, // Over Discharge Current Warning
    {             DTC_ODCP, &bmsData.BusCurrent,    &eepBms.ODCP}, // Discharge OverCurrent Protection
    {              DTC_OTW,   &bmsData.MaxTcell,     &eepBms.OTW}, // Cell Over Temperature Warning
    {              DTC_OTP,   &bmsData.MaxTcell,     &eepBms.OTP}, // Cell Over Temperature Protection
    {              DTC_UTW,   &bmsData.MinTcell,     &eepBms.UTW}, // Cell Under Temperature Warning
    {              DTC_UTP,   &bmsData.MinTcell,     &eepBms.UTP}  // Cell Under Temperature Protection
};

volatile const DTC_MESSAGE_TABLE_t DTC_BMS_Message_Table[DTC_EVENT_MAX_NUM] = {
    {0x0123,      ERR_LEVEL_FAULT, 2, 1}, // Charge Over Current Protection
    {0x0223,      ERR_LEVEL_FAULT, 2, 2}, // Discharge OverCurrent Protection
    {0x0133,      ERR_LEVEL_FAULT, 3, 1}, // (BUS Voltage) Over Voltage Protection
    {0x0233,      ERR_LEVEL_FAULT, 3, 2}, // (BUS Voltage) Under Voltage Protection
    {0x0143,      ERR_LEVEL_FAULT, 4, 1}, // Cell Over Temperature Protection
    {0x0243,      ERR_LEVEL_FAULT, 4, 2}, // Cell Under Temperature Protection
    {0x0334, ERR_LEVEL_PROTECTION, 3, 3}, // Cell Unbalance Protection
    {0x0122,    ERR_LEVEL_WARNING, 2, 1}, // Over Charge Current Warning
    {0x0222,    ERR_LEVEL_WARNING, 2, 2}, // Over Discharge Current Warning
    {0x0132,    ERR_LEVEL_WARNING, 3, 2}, // (BUS Voltage) Over Voltage Warning
    {0x0232,    ERR_LEVEL_WARNING, 3, 2}, // (BUS Voltage) Under Voltage Warning
    {0x0142,    ERR_LEVEL_WARNING, 4, 1}, // Cell Over Temperature Warning
    {0x0242,    ERR_LEVEL_WARNING, 4, 2}, // Cell Under Temperature Warning
    {0x0332,    ERR_LEVEL_WARNING, 3, 3}, // Cell Unbalance Warning
    {0x0433,      ERR_LEVEL_FAULT, 3, 4}, // Cell Over Voltage Protection
    {0x0533,      ERR_LEVEL_FAULT, 3, 5}, // Cell Under Voltage Protection
    {0x0342,    ERR_LEVEL_WARNING, 4, 3}, // Temperature Unbalance Warning
    {0x0114, ERR_LEVEL_PROTECTION, 1, 1}, // Master BCU Communication Error
    {0x0214, ERR_LEVEL_PROTECTION, 1, 2}, // Safety BCU Communication Error
    {0x0314, ERR_LEVEL_PROTECTION, 1, 3}, // Shunt Communication Error
    {0x0414, ERR_LEVEL_PROTECTION, 1, 4}, // BMU Communication Error
    {0x0514, ERR_LEVEL_PROTECTION, 1, 5}, // PCS Communication Error
    {0x0614,      ERR_LEVEL_FAULT, 1, 6}, // MBMS Communication Error
    {0x0714,    ERR_LEVEL_MESSAGE, 1, 7}, // AFE Communication Error
    {0x0154, ERR_LEVEL_PROTECTION, 5, 1}, // NTC Disconnected
    {0x0254, ERR_LEVEL_PROTECTION, 5, 2}, // NTC Short
    {0x0351,    ERR_LEVEL_MESSAGE, 0, 0}, // BMS Shutdown?RecordData
    {0x0454, ERR_LEVEL_PROTECTION, 5, 4}, // Power Loss(Occur Relay Cut off)
    {0x0551,    ERR_LEVEL_MESSAGE, 0, 0}, // Emergency or EMS do not Install
    {0x0654, ERR_LEVEL_PROTECTION, 5, 6}, // EEPROM CheckSum Error
    {0x0754, ERR_LEVEL_PROTECTION, 5, 7}, // Relay Sticking
    {0x0851,    ERR_LEVEL_MESSAGE, 0, 0}, // Current Direction Error
    {0x0953,      ERR_LEVEL_FAULT, 5, 9}  // PCS Command Error(OVP or UVP)
};
/* USER CODE END GV */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
static unsigned short gDtcTimeCount[(DTC_TCELL_UNBALANCE_W + 1)];
/* USER CODE END PV */

/* Function prototypes -------------------------------------------------------*/
/* USER CODE BEGIN FP */

/* USER CODE END FP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/**
 * @brief      Store the error event to eeprom
 *
 * @param      event Error event
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-10-24
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
static void DTC_Store2Eeprom(DTC_EVENT_e event) {
    static DTC_EVENT_e LatestDTC = DTC_EVENT_MAX_NUM;

    if (DTC_EVENT_MAX_NUM == LatestDTC) {     // If System Reset
        if (eepEmg.ErrorCode[0] == 0x0000) {  // If Latest Error Code(EEPROM Data) equal Zero,Set Latest DTC to Zero
            LatestDTC = 0;
        } else {  // If Latest Error Code(EEPROM Data) unequal Zero,Set Latest DTC
            for (unsigned char i = 0; i < DTC_EVENT_MAX_NUM; i++) {  // Read Latest Error Code from Enum
                if (DTC_BMS_Message_Table[i].errorCode == eepEmg.ErrorCode[0]) {
                    LatestDTC = i;
                    break;
                }
            }
        }
    }

    if (event != LatestDTC) {  // If Latest Error Code(EEPROM Data) unequal New Event Value,Set Latest DTC Into Array
        LatestDTC = event;
        for (unsigned char i = (DTC_LOG_LENGTH - 1); i != 0; i--) {
            eepEmg.ErrorCode[i] = eepEmg.ErrorCode[i - 1];
        }
        eepEmg.ErrorCode[0] = DTC_BMS_Message_Table[event].errorCode;
        /*TODO: APP_EEPROM_EmergencyWrite();*/
    }
}
/**
 * @brief      Clear the bit of the error event that occurred
 *
 * @param      event Error event
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-10-24
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
void DTC_FaultOccurClear(DTC_EVENT_e event) {
    dtc.l &= ~(1 << event);
}
/**
 * @brief      Set the bit of the error event that occurred
 *
 * @param      event Error event
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-10-24
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
void DTC_FaultOccurSet(DTC_EVENT_e event) {
    dtc.l |= 1 << event;
    DTC_Store2Eeprom(event);
}
/**
 * @brief      Clear the bit of error event mask
 *
 * @param      event Error event
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-10-24
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
void DTC_FaultMaskClear(DTC_EVENT_e event) {
    dtcMask.l &= ~(1 << event);
}
/**
 * @brief      Set the bit of error event mask
 *
 * @param      event Error event
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-10-24
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
void DTC_FaultMaskSet(DTC_EVENT_e event) {
    dtcMask.l |= 1 << event;
}
/**
 * @brief      Get error event status
 *
 * @param      event Error event
 * @return     true  Occur
 * @return     false Did not occur
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-10-24
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
bool DTC_FaultEventGet(DTC_EVENT_e event) {
    bool ret = true;
    ret      = (dtc.l >> event) & 0x01;
    return ret;
}
/**
 * @brief      Get the DTC bitmap
 *
 * @return     unsigned long long
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-10-24
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
unsigned long long DTC_FaultMapGet(void) {
    unsigned long long ret = 0;
    ret                    = dtc.l;
    return ret;
}
/**
 * @brief      Clear the DTC latch bitmap in byte
 *
 * @param      byteNum The number of bytes
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-10-24
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
void DTC_LatchMapByteClear(unsigned char byteNum) {
    dtcLatch.l &= ~((unsigned long long)0xff << 8 * byteNum);
}
/**
 * @brief      Get the DTC latch bitmap
 *
 * @return     unsigned long long
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-10-24
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
unsigned long long DTC_LatchMapGet(void) {
    unsigned long long ret = 0;
    ret                    = dtcLatch.l;
    return ret;
}
/**
 * @brief      Update the DTC latch bitmap
 *
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-10-24
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
static void DTC_LatchMapUpdate(void) {
    dtcLatch.l |= dtc.l;
}
/**
 * @brief      Error checking lower limit condition
 *
 * @param      event Error event
 * @param      source Source to be check
 * @param      ptrObj Fault parameter
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-10-24
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
static void DTC_LowerLimitCheck(DTC_EVENT_e event, int source, FAULT_PARAM_t* ptrObj) {
    if (DTC_FaultEventGet(event) == false) {
        if (source < ptrObj->Limit) {
            if (gDtcTimeCount[event]-- == 0) {
                DTC_FaultOccurSet(event);
                gDtcTimeCount[event] = ptrObj->ReleaseTime;
            }
        } else {
            gDtcTimeCount[event] = ptrObj->LimitTime;
        }
    } else {
        if (source > ptrObj->Release) {
            if (gDtcTimeCount[event]-- == 0) {
                DTC_FaultOccurClear(event);
                gDtcTimeCount[event] = ptrObj->LimitTime;
            }
        } else {
            gDtcTimeCount[event] = ptrObj->ReleaseTime;
        }
    }
}
/**
 * @brief      Error checking upper limit condition
 *
 * @param      event Error event
 * @param      source Source to be check
 * @param      ptrObj Fault parameter
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-10-24
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
static void DTC_HigherLimitCheck(DTC_EVENT_e event, int source, FAULT_PARAM_t* ptrObj) {
    if (DTC_FaultEventGet(event) == false) {
        if (source > ptrObj->Limit) {
            if (gDtcTimeCount[event]-- == 0) {
                DTC_FaultOccurSet(event);
                gDtcTimeCount[event] = ptrObj->ReleaseTime;
            }
        } else {
            gDtcTimeCount[event] = ptrObj->LimitTime;
        }
    } else {
        if (source < ptrObj->Release) {
            if (gDtcTimeCount[event]-- == 0) {
                DTC_FaultOccurClear(event);
                gDtcTimeCount[event] = ptrObj->LimitTime;
            }
        } else {
            gDtcTimeCount[event] = ptrObj->ReleaseTime;
        }
    }
}
/**
 * @brief      Check the fault occur
 *
 * @param      table Event table
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-10-24
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
static void DTC_FaultOccurCheck(DTC_FAULT_CHECK_TABLE_t* table) {
    if (table->ptrObject->Limit > table->ptrObject->Release) {
        DTC_HigherLimitCheck(table->event, (int)*table->source, table->ptrObject);
    } else {
        DTC_LowerLimitCheck(table->event, (int)*table->source, table->ptrObject);
    }
}
/**
 * @brief      Get error level
 *
 * @param      event Error event
 * @return     ERROR_LEVEL_e
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-10-24
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
ERROR_LEVEL_e DTC_ErrorLevelGet(DTC_EVENT_e event) {
    return (DTC_BMS_Message_Table[event].errorLevel);
}
/**
 * @brief      Get error code
 *
 * @param      event Error event
 * @return     unsigned short
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-10-24
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
unsigned short DTC_ErrorCodeGet(DTC_EVENT_e event) {
    return (DTC_BMS_Message_Table[event].errorCode);
}
/**
 * @brief      Get current worst error level
 *
 * @return     ERROR_LEVEL_e
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-10-24
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
ERROR_LEVEL_e DTC_WorstLevelGet(void) {
    ERROR_LEVEL_e ret            = ERR_LEVEL_NONE;
    ERROR_LEVEL_e currErrorLevel = ERR_LEVEL_NONE;
    for (unsigned char event = 0; event < DTC_EVENT_MAX_NUM; event++) {
        if (DTC_FaultEventGet(event)) {
            currErrorLevel = DTC_BMS_Message_Table[event].errorLevel;
            if (currErrorLevel > ret) {
                ret = currErrorLevel;
            }
        }
        if (ret == ERR_LEVEL_PROTECTION) break;
    }
    return ret;
}
/**
 * @brief      Initialize the DTC parameter
 *
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-10-24
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
void DTC_Initialize(void) {
    dtc.l     = 0;
    dtcMask.l = 0xFFFFFFFFFFFFFFFF;
    // DTC_FaultMaskClear(DTC_SHUT_DOWN);
    // DTC_FaultMaskClear(DTC_EMERGENCY);
    // DTC_FaultMaskClear(DTC_CURR_DIR_ERR);
}
/**
 * @brief      DTC 1ms polling tasks
 *
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-10-24
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
void DTC_1ms_Tasks(void) {
    for (unsigned char event = 0; event < sizeof(FaultCheckTaskTable) / sizeof(DTC_FAULT_CHECK_TABLE_t); event++) {
        DTC_FaultOccurCheck(&FaultCheckTaskTable[event]);
    }
    dtc.l = dtc.l & dtcMask.l;
    DTC_LatchMapUpdate();
}
/* USER CODE END 0 */

/*******************************************************************************
 End of File
 */
