/* ************************************************************************** */
/** Descriptive File Name

  @Company
    Company Name

  @File Name
    filename.c

  @Summary
    Brief description of the file.

  @Description
    Describe the purpose of this file.
 */
/* ************************************************************************** */

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */

#include "sys_parameter.h"

// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
// *****************************************************************************

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: File Scope or Global Data                                         */
/* ************************************************************************** */
/* ************************************************************************** */

static unsigned short   gDtcTimeCount[(DTC_TCELL_UNBALANCE_W + 1)];
DTC_FAULT_CHECK_TABLE_t FaultCheckTaskTable[] = {
    {        DTC_VCELL_OVP,   &bmsData.MaxVcell, &eepBms.CellOVP},
    {        DTC_VCELL_UVP,   &bmsData.MinVcell, &eepBms.CellUVP},
    {DTC_VCELL_UNBALANCE_W,  &bmsData.DeltaVolt, &eepBms.CellUBW},
    {  DTC_VCELL_UNBALANCE,  &bmsData.DeltaVolt, &eepBms.CellUBP},
    {          DTC_BUS_OVW, &bmsData.BusVolt_mV,  &eepBms.BusOVW},
    {          DTC_BUS_OVP, &bmsData.BusVolt_mV,  &eepBms.BusOVP},
    {          DTC_BUS_UVW, &bmsData.BusVolt_mV,  &eepBms.BusUVW},
    {          DTC_BUS_UVP, &bmsData.BusVolt_mV,  &eepBms.BusUVP},
    {             DTC_OCCW, &bmsData.BusCurrent,    &eepBms.OCCW},
    {             DTC_OCCP, &bmsData.BusCurrent,    &eepBms.OCCP},
    {             DTC_ODCW, &bmsData.BusCurrent,    &eepBms.ODCW},
    {             DTC_ODCP, &bmsData.BusCurrent,    &eepBms.ODCP},
    {              DTC_OTW,   &bmsData.MaxTcell,     &eepBms.OTW},
    {              DTC_OTP,   &bmsData.MaxTcell,     &eepBms.OTP},
    {              DTC_UTW,   &bmsData.MinTcell,     &eepBms.UTW},
    {              DTC_UTP,   &bmsData.MinTcell,     &eepBms.UTP}
};

volatile const DTC_MESSAGE_TABLE_t DTC_BMS_Message_Table[DTC_EVENT_MAX_NUM] = {
    {0x0123,      ERR_LEVEL_FAULT, 2, 1}, /* Charge Over Current Protection */
    {0x0223,      ERR_LEVEL_FAULT, 2, 2}, /* Discharge OverCurrent Protection */
    {0x0133,      ERR_LEVEL_FAULT, 3, 1}, /* (BUS Voltage) Over Voltage Protection */
    {0x0233,      ERR_LEVEL_FAULT, 3, 2}, /* (BUS Voltage) Under Voltage Protection */
    {0x0143,      ERR_LEVEL_FAULT, 4, 1}, /* Cell Over Temperature Protection */
    {0x0243,      ERR_LEVEL_FAULT, 4, 2}, /* Cell Under Temperature Protection */
    {0x0334, ERR_LEVEL_PROTECTION, 3, 3}, /* Cell Unbalance Protection */
    {0x0122,    ERR_LEVEL_WARNING, 2, 1}, /* Over Charge Current Warning */
    {0x0222,    ERR_LEVEL_WARNING, 2, 2}, /* Over Discharge Current Warning */
    {0x0132,    ERR_LEVEL_WARNING, 3, 2}, /* (BUS Voltage) Over Voltage Warning */
    {0x0232,    ERR_LEVEL_WARNING, 3, 2}, /* (BUS Voltage) Under Voltage Warning */
    {0x0142,    ERR_LEVEL_WARNING, 4, 1}, /* Cell Over Temperature Warning */
    {0x0242,    ERR_LEVEL_WARNING, 4, 2}, /* Cell Under Temperature Warning */
    {0x0332,    ERR_LEVEL_WARNING, 3, 3}, /* Cell Unbalance Warning */
    {0x0433,      ERR_LEVEL_FAULT, 3, 4}, /* Cell Over Voltage Protection */
    {0x0533,      ERR_LEVEL_FAULT, 3, 5}, /* Cell Under Voltage Protection */
    {0x0342,    ERR_LEVEL_WARNING, 4, 3}, /* Temperature Unbalance Warning */
    {0x0114, ERR_LEVEL_PROTECTION, 1, 1}, /* Master BCU Communication Error */
    {0x0214, ERR_LEVEL_PROTECTION, 1, 2}, /* Safety BCU Communication Error */
    {0x0314, ERR_LEVEL_PROTECTION, 1, 3}, /* Shunt Communication Error */
    {0x0414, ERR_LEVEL_PROTECTION, 1, 4}, /* BMU Communication Error */
    {0x0514, ERR_LEVEL_PROTECTION, 1, 5}, /* PCS Communication Error */
    {0x0614,      ERR_LEVEL_FAULT, 1, 6}, /* MBMS Communication Error */
    {0x0714,    ERR_LEVEL_MESSAGE, 1, 7}, /* AFE Communication Error */
    {0x0154, ERR_LEVEL_PROTECTION, 5, 1}, /* NTC Disconnected */
    {0x0254, ERR_LEVEL_PROTECTION, 5, 2}, /* NTC Short */
    {0x0351,    ERR_LEVEL_MESSAGE, 0, 0}, /* BMS Shutdown?RecordData */
    {0x0454, ERR_LEVEL_PROTECTION, 5, 4}, /* Power Loss(Occure Relay Cut off) */
    {0x0551,    ERR_LEVEL_MESSAGE, 0, 0}, /* Emergency or EMS do not Install */
    {0x0654, ERR_LEVEL_PROTECTION, 5, 6}, /* EEPROM CheckSum Error */
    {0x0754, ERR_LEVEL_PROTECTION, 5, 7}, /* Relay Sticking */
    {0x0851,    ERR_LEVEL_MESSAGE, 0, 0}, /* Current Direction Error */
    {0x0953,      ERR_LEVEL_FAULT, 5, 9}  /* PCS Command Error(OVP or UVP) */
};
/* ************************************************************************** */
/* ************************************************************************** */
// Section: Interface Functions                                               */
/* ************************************************************************** */
/* ************************************************************************** */

void DTC_FaultOccurClear(DTC_EVENT_e event) {
    Fault_EventClear(event);
}

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
        /*TODO: APP_EepromEmergencyWrite();*/
    }
}

void DTC_FaultOccurSet(DTC_EVENT_e event) {
    Fault_EventSet(event);
    DTC_Store2Eeprom(event);
    // for (uint8_t event = 0; event < DTC_EVENT_MAX_NUM; event++) {
    //     switch (DTC_BMS_Message_Table[event].errorLevel) {
    //         case ERR_LEVEL_PROTECTION:
    //             bmsData.WorkModeCmd = BMS_OCCUR_FAULT;
    //             break;
    //         case ERR_LEVEL_FAULT:
    //             bmsData.WorkModeCmd = BMS_OFF;
    //             break;
    //         case ERR_LEVEL_WARNING:
    //         case ERR_LEVEL_MESSAGE:
    //         default:
    //             break;
    //     }
    // }
}

static unsigned char DTC_FaultEventGet(DTC_EVENT_e event) {
    unsigned char ret = true;
    ret               = Fault_EventGet(event);
    return ret;
}

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

static void DTC_FaultOccureCheck(DTC_FAULT_CHECK_TABLE_t* table) {
    if (table->ptrObject->Limit > table->ptrObject->Release) {
        DTC_HigherLimitCheck(table->event, (int)*table->source, table->ptrObject);
    } else {
        DTC_LowerLimitCheck(table->event, (int)*table->source, table->ptrObject);
    }
}

ERROR_LEVEL_e DTC_ErrorLevelGet(DTC_EVENT_e event) {
    return (DTC_BMS_Message_Table[event].errorLevel);
}

unsigned short DTC_ErrorCodeGet(DTC_EVENT_e event) {
    return (DTC_BMS_Message_Table[event].errorCode);
}

ERROR_LEVEL_e DTC_WorstLevelGet(void) {
    ERROR_LEVEL_e ret            = ERR_LEVEL_NONE;
    ERROR_LEVEL_e currErrorLevel = ERR_LEVEL_NONE;
    for (uint8_t event = 0; event < DTC_EVENT_MAX_NUM; event++) {
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

void DTC_1ms_Tasks(void) {
    bmsData.DeltaVolt = bmsData.MaxVcell - bmsData.MinVcell;
    bmsData.DeltaTemp = bmsData.MaxTcell - bmsData.MinTcell;
    for (uint8_t event = 0; event < sizeof(FaultCheckTaskTable) / sizeof(DTC_FAULT_CHECK_TABLE_t); event++) {
        DTC_FaultOccureCheck(&FaultCheckTaskTable[event]);
    }

    //    switch (DTC_WorstLevelGet()) {
    //        case ERR_LEVEL_PROTECTION:
    //            bmsData.WorkModeCmd = BMS_OCCUR_FAULT;
    //            break;
    //        case ERR_LEVEL_FAULT:
    //            bmsData.WorkModeCmd = BMS_OFF;
    //            break;
    //        case ERR_LEVEL_WARNING:
    //        case ERR_LEVEL_MESSAGE:
    //        default:
    //            break;
    //    }
}