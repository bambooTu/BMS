/**
 * @file       sys_parameter.h
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @brief
 * @version    0.1
 * @date       2022-10-04
 *
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 *
 * Abbreviation:
 * None
 */
#ifndef _SYS_PARAMETER_H
#define _SYS_PARAMETER_H
// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif
// DOM-IGNORE-END

/* Global define -------------------------------------------------------------*/

/* USER CODE BEGIN GD */
//#define NVM_MODE
#define EEPROM_START_ADDR 0x000

/****** EERPOM Address 0x040 ~ 0x7FF Storage BMS Parameter ******/
/* section 1 */
#define EEPROM_BMS_START_ADDR (EEPROM_START_ADDR + 0x000)
#define EEPROM_BMS_END_ADDR   (EEPROM_START_ADDR + 0x7FF)

/****** EEPROM Address 0x800 ~ 0x8FF Storage Emergency Parameter ******/
/* section 2 */
#define EEPROM_EMG_START_ADDR (EEPROM_START_ADDR + 0x800)
#define EEPROM_EMG_END_ADDR   (EEPROM_START_ADDR + 0x8FF)
/****** EEPROM Address 0x900 ~ 0xFFF Storage Special Parameter ******/
/* section 3 */
#define EEPROM_SPE_START_ADDR (EEPROM_START_ADDR + 0x900)
#define EEPROM_SPE_END        (EEPROM_START_ADDR + 0xFFF)

/*** EEPROM Initial Parameter ***/
#define EEP_KEY_ID       0x55 /* BMS:0x55, BMU:0x65 */
#define SVN_NUMBER       202
#define BMS_COMM_ADDR    0x00 /* Communication Address */
#define HARDWARE_VER_0   1    /* Hardware Versiom */
#define HARDWARE_VER_1   0
#define HARDWARE_VER_2   0
#define FIRMWARE_VER_0   0 /* Firmware Version */
#define FIRMWARE_VER_1   0
#define FIRMWARE_VER_2   0 /* LF-01 V000 */
#define COMM_VER_MAJOR_0 0 /* Protocol Version  */
#define COMM_VER_MAJOR_1 1
#define COMM_VER_SUB_0   0
#define COMM_VER_SUB_1   0
#define COMM_VER_SUB_2   1

#define DTC_LOG_LENGTH 4

#define CELL_DESIGN_CAP      78000UL /*unit:Ah*/
#define CELL_DESIGN_MAX_VOLT 4100UL
/* USER CODE END GD */

/* Includes ------------------------------------------------------------------*/

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "bms_ctrl.h"
#include "dtc.h"
#include "hv_setup.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum {
    BAL_OFF = 0,       // Balance Off
    BAL_CHG,           // Balance @ Charging
    BAL_DISCHG,        // Balance @ Dischging
    BAL_CHG_N_DISCHG,  // Balance @ Discharging & Charging
    BAL_FORCE,         // Force Balance
    BAL_MAX
} BALANCE_MODE_e;

typedef struct {
    BALANCE_MODE_e Mode;      // Balance Mode
    unsigned short Volt;      // unit:m,Balance Target Voltage
    unsigned char  VoltDiff;  // unit:mV,Voltage Different
} BALANCE_PARA_t;

typedef struct {
    unsigned char  EepromKey;  // Eeprom Identifier
    unsigned short EepromSVN;  // Eeprom Sub Version

    /***BMS Parameter Setting***/
    /**Over/Under Temperature**/
    FAULT_PARAM_t OTP;   // Over Temperature Protection
    FAULT_PARAM_t OTW;   // Over Temperature Warning
    FAULT_PARAM_t UTP;   // Under Temperature Protection
    FAULT_PARAM_t UTW;   // Under Temperature Warning
    FAULT_PARAM_t UBTW;  // Unbalance Temperature Warning

    /**Over/Under Voltage**/
    FAULT_PARAM_t BusOVP;   // Bus OverVoltage Protection
    FAULT_PARAM_t BusOVW;   // Bus OverVoltage Warning
    FAULT_PARAM_t BusUVP;   // Bus UnderVoltage Protection
    FAULT_PARAM_t BusUVW;   // Bus UnderVoltage Warning
    FAULT_PARAM_t CellUBP;  // Cell Unbalance VoltageWarning
    FAULT_PARAM_t CellUBW;  // Cell Unbalance VoltageWarning
    FAULT_PARAM_t CellOVP;  // Cell OverVoltage Warning
    FAULT_PARAM_t CellUVP;  // Cell OverVoltage Warning

    /**OverCurrent**/
    FAULT_PARAM_t ODCP;  // Over Discharge Current Protection*
    FAULT_PARAM_t ODCW;  // Over Discharge Current Warning
    FAULT_PARAM_t OCCP;  // Over Charge Current Protection
    FAULT_PARAM_t OCCW;  // Over Charge Current Warning

    unsigned short LockTimeOCP; /*unit:100mS*/

    /*SOH Decay Coefficient*/
    float          FullCap; /* unit: Ah */
    unsigned short DecayCoefficient;

    /***BMSFactoryDate***/
    unsigned char  FactoryDay;
    unsigned char  FactoryMonth;
    unsigned short FactoryYear;
    unsigned char  SerialNum[13 + 1];

    /***Balance***/
    BALANCE_PARA_t BalanceDef;

    /***Dummy***/
    unsigned char Dummy1;
    unsigned char Dummy2;
    unsigned char Dummy3;

    unsigned char CheckSum;
} EEPROM_BMS_t;

typedef struct {
    float          ChgCap;                     // unit:Ah  Fixed Eeprom Locations
    float          DisChgCap;                  // unit:Ah Fixed Eeprom Locations
    unsigned short CycleLife;                  // Fixed Eeprom Locations
    unsigned short ErrorCode[DTC_LOG_LENGTH];  // Diagnostic Trouble Code
} EEPROM_EMERGENCY_t;

typedef struct {
    unsigned char BmsAddr;  // BMS Module Address Fixed Eeprom Locations
    /* MCP3421 */
    unsigned short AdcZeroOffset;
    unsigned short AdcGainOffset;
    unsigned short Dummy;
} EEPROM_SPECIAL_t;

typedef struct {
    BMS_WORK_MODE_e WorkModeCmd;  // BMS Receive Command
    SYS_STATUS_e    Status;       // BMS Status

    HV_STATUS_e    HvStatus;  // Positive Relay Status
    BALANCE_PARA_t Balance;   // BMU Balance Parameter

    int           BusCurrent;     // unit:mA RawData
    int           BusVolt_mV;     // unit:mV
    int           BusVolt;        // unit:V
    int           MinVcell;       // Unit:mV
    int           MaxVcell;       // Unit:mV
    int           DeltaVolt;      // Unit:mV
    int           MinTcell;       // unit:0.1 degC
    int           MaxTcell;       // unit:0.1 degC
    int           DeltaTemp;      // unit:0.1 degC
    unsigned char MinVcellBmuID;  // BMU ID For Min. Volt
    unsigned char MaxVcellBmuID;  // BMU ID For Max. Volt
    unsigned char BmuMinVcellID;  // Min.Volt Cell ID In BMU
    unsigned char BmuMaxVcellID;  // Max.Volt Cell ID In BMU
    unsigned char MinTcellBmuID;  // BMU ID for Min.Temp Cell
    unsigned char MaxTcellBmuID;  // BMU ID for Max.Temp Cell
    unsigned char BmuMinTcellID;  // Min.Temp Cell ID In BMU
    unsigned char BmuMaxTcellID;  // Max.Temp Cell ID In BMU

    unsigned char  SOC;        // unit: %
    unsigned char  SOH;        // unit: %,According to the Cycle Life Calculation
    unsigned int   RemCap;     // unit: mAh
    unsigned int   ChgCap;     // unit: mAh
    unsigned int   DischgCap;  // unit: mAh
    unsigned int   FullCap;    // unit: mAh
    unsigned int   DesingCap;  // unit: mAh
    unsigned short CycleLife;  // Fixed Eeprom Locations
    unsigned short DecayCoefficient;
    /* Protection Flag */
    unsigned char FaultBitArray[(DTC_EVENT_MAX_NUM / 8) + 1];
    unsigned char FaultBitArrayHold[(DTC_EVENT_MAX_NUM / 8) + 1];
} BMS_DATA_t;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Global variables -----------------------------------------------------------*/
/* USER CODE BEGIN GV */
extern volatile const EEPROM_BMS_t       eepBmsDef;  // EEPROM default data
extern volatile const EEPROM_SPECIAL_t   eepSpeDef;  // EEPROM default data
extern volatile const EEPROM_EMERGENCY_t eepEmgDef;  // EEPROM default data
extern BMS_DATA_t                        bmsData;
extern EEPROM_BMS_t                      eepBms;
extern EEPROM_EMERGENCY_t                eepEmg;
extern EEPROM_SPECIAL_t                  eepSpe;
/* USER CODE END GV */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Function prototypes -------------------------------------------------------*/
/* USER CODE BEGIN FP */

/* USER CODE END FP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */
// DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
// DOM-IGNORE-END
#endif /* _SYS_PARAMETER_H */
/*******************************************************************************
 End of File
 */
