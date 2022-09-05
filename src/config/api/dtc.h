/* ************************************************************************** */
/** Descriptive File Name

  @Company
    Company Name

  @File Name
    filename.h

  @Summary
    Brief description of the file.

  @Description
    Describe the purpose of this file.
 */
/* ************************************************************************** */

#ifndef _DTC_H /* Guard against multiple inclusion */
#define _DTC_H

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */

/* Provide C++ Compatibility */
#ifdef __cplusplus
extern "C" {
#endif

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Constants                                                         */
/* ************************************************************************** */
/* ************************************************************************** */

// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
// *****************************************************************************

typedef enum {
    DTC_OCCP,              /**Charge Over Current Protection*/
    DTC_ODCP,              /**Discharge Over Current Protection*/
    DTC_BUS_OVP,           /**Bus Over Voltage Protection*/
    DTC_BUS_UVP,           /**Bus Under Voltage Protection*/
    DTC_OTP,               /**Cell Over Temperature Protection*/
    DTC_UTP,               /**Cell Under Temperature Protection*/
    DTC_VCELL_UNBALANCE,   /**Cell Unbalance Protection*/
    DTC_OCCW,              /**Over Charge Current Warning*/
    DTC_ODCW,              /**Over Discharge Current Warning*/
    DTC_BUS_OVW,           /**Bus Over Voltage Warning*/
    DTC_BUS_UVW,           /**Bus Under Voltage Warning*/
    DTC_OTW,               /**Cell Over Temperature Warning*/
    DTC_UTW,               /**Cell Under Temperature Warning*/
    DTC_VCELL_UNBALANCE_W, /**Cell Unbalance Warning*/
    DTC_VCELL_OVP,         /**Cell Over Voltage Protection*/
    DTC_VCELL_UVP,         /**Cell Under Voltage Protection*/
    DTC_TCELL_UNBALANCE_W, /**Temperature Unbalance Warning*/
    DTC_BCU_COMM,          /**Master BCU Communication Error*/
    DTC_SAFETY_BCU_COMM,   /**Safety BCU Communication Error*/
    DTC_CURR_SEN_COMM,     /***Shunt Communication Error*/
    DTC_BMU_COMM,          /**BMU Communication Error*/
    DTC_PCS_COMM,          /**PCS CommunicationError*/
    DTC_MBMS_COMM,         /**MBMS CommunicationError*/
    DTC_AFE_COMM,          /**AFE CommunicationError*/
    DTC_NTC_OPEN,          /**NTC Disconnected*/
    DTC_NTC_SHORT,         /**NTC Short*/
    DTC_SHUT_DOWN,         /***BMS Shutdown?RecordData*/
    DTC_POWER_LOSS,        /***Power Loss(Occure Relay Cutoff)*/
    DTC_EMERGENCY,         /***Emergency or EMS do not Install*/
    DTC_EEPROM_CHECKSUM,   /**EEPROM CheckSum Error*/
    DTC_BMS_RELAY,         /**Relay Sticking*/
    DTC_CURR_DIR_ERR,      /**Current Direction Error*/
    DTC_PCS_CMD,           /***PCS Command Error(OVP?UVP)*/
    DTC_EVENT_MAX_NUM
} DTC_EVENT_e;

typedef struct {
    int            Limit;       /*Protection Conditions Limit*/
    int            Release;     /*Protection Release Conditions*/
    unsigned short LimitTime;   /*Protection Timing,mSec*/
    unsigned short ReleaseTime; /*Release Timing,mSec*/
} FAULT_PARAM_t;                 /*Protection/Warning Parameter*/

typedef enum {
    ERR_LEVEL_NONE,
    ERR_LEVEL_MESSAGE,     //????
    ERR_LEVEL_WARNING,     //??
    ERR_LEVEL_FAULT,       //??
    ERR_LEVEL_PROTECTION,  //????
} ERROR_LEVEL_e;

typedef struct {
    unsigned short errorCode;
    ERROR_LEVEL_e  errorLevel;
    unsigned char  longCount; /* Fault LED Flashing Times */
    unsigned char  shortCount;
} DTC_MESSAGE_TABLE_t;

typedef struct {
    DTC_EVENT_e   event;
    int*          source;
    FAULT_PARAM_t* ptrObject;
} DTC_FAULT_CHECK_TABLE_t;

// *****************************************************************************
// *****************************************************************************
// Section: Interface Functions
// *****************************************************************************
// *****************************************************************************

void           DTC_1ms_Tasks(void);
void           DTC_FaultOccurSet(DTC_EVENT_e event);
void           DTC_FaultOccurClear(DTC_EVENT_e event);
unsigned short DTC_ErrorCodeGet(DTC_EVENT_e DTC);
ERROR_LEVEL_e  DTC_ErrorLevelGet(DTC_EVENT_e DTC);
ERROR_LEVEL_e  DTC_WorstLevelGet(void);

/* Provide C++ Compatibility */
#ifdef __cplusplus
}
#endif

#endif /* _DTC_H  */

/* *****************************************************************************
 End of File
 */
