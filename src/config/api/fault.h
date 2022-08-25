#ifndef _FAULT_H
#define _FAULT_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include "definitions.h"  // SYS function prototypes
// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif
// DOM-IGNORE-END

typedef union {
    struct {
        unsigned OCCP             : 1; /**Charge Over Current Protection*/
        unsigned ODCP             : 1; /**Discharge OverCurrent Protection*/
        unsigned OVP              : 1; /**(BUS Voltage) Over Voltage Protection*/
        unsigned UVP              : 1; /**(BUS Voltage) Under Voltage Protection*/
        unsigned OTP              : 1; /**Cell Over Temperature Protection*/
        unsigned UTP              : 1; /**Cell Under Temperature Protection*/
        unsigned CELL_UNBALANCE_P : 1; /**Cell Unbalance Protection*/
        unsigned OCCW             : 1; /**Over Charge Current Warning*/

        unsigned ODCW             : 1; /**Over Discharge Current Warning*/
        unsigned BUS_OVW          : 1; /**(BUS Voltage) Over Voltage Warning*/
        unsigned BUS_UVW          : 1; /**(BUS Voltage) Under Voltage Warning*/
        unsigned OTW              : 1; /**Cell Over Temperature Warning*/
        unsigned UTW              : 1; /**Cell Under Temperature Warning*/
        unsigned CELL_UNBALANCE_W : 1; /**Cell Unbalance Warning*/
        unsigned CELL_OVP         : 1; /**Cell Over Voltage Protection*/
        unsigned CELL_UVP         : 1; /**Cell Under Voltage Protection*/

        unsigned T_UNBALANCE_W   : 1; /**Temperature Unbalance Warning*/
        unsigned BCU_COMM        : 1; /**Master BCU Communication Error*/
        unsigned SAFETY_BCU_COMM : 1; /**Safety BCU Communication Error*/
        unsigned CURR_SEN_COMM   : 1; /***Shunt Communication Error*/
        unsigned BMU_COMM        : 1; /**BMU Communication Error*/
        unsigned PCS_COMM        : 1; /**PCS Communication Error*/
        unsigned MBMS_COMM       : 1; /**MBMS Communication Error*/
        unsigned AFE_COMM        : 1; /**AFE Communication Error*/

        unsigned NTC_OPEN        : 1; /**NTC Disconnected*/
        unsigned NTC_SHORT       : 1; /**NTC Short*/
        unsigned SHUT_DOWN       : 1; /***BMS Shutdown?RecordData*/
        unsigned POWER_LOSS      : 1; /***Power Loss(Occure Relay Cut off)*/
        unsigned EMERGENCY       : 1; /***Emergency or EMS do not Install*/
        unsigned EEPROM_CHECKSUM : 1; /**EEPROM CheckSum Error*/
        unsigned BMS_RELAY       : 1; /**Relay Sticking*/
        unsigned CURR_DIR_ERR    : 1; /**Current Direction Error*/

        unsigned PCS_CMD : 1; /***PCS Command Error(OVP or UVP)*/
    } b;
    uint64_t l;
} DTC_FAULT_t;

void Fault_ParameterInitialize(void);
void Fault_EventSet(DTC_EVENT_e event);
void Fault_EventClear(DTC_EVENT_e event);
bool Fault_EventGet(DTC_EVENT_e event);

// DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
// DOM-IGNORE-END

#endif /* _APP_H */

/*******************************************************************************
 End of File
 */
