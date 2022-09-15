/**
 * @file       sys_parameter.c
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @brief
 * @version    0.1
 * @date       2022-09-01
 *
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 *
 * Abbreviation:
 * BMS  Battery management system
 * EMG  Emergency
 * SPE  Special
 * EEP  EEPROM
 */

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */

#include "sys_parameter.h"

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: File Scope or Global Data                                         */
/* ************************************************************************** */
/* ************************************************************************** */

#define EEPROM_EMG_SIZE sizeof(EEPROM_EMERGENCY_t)
#define EEPROM_SPE_SIZE sizeof(EEPROM_SPECIAL_t)
#define EEPROM_BMS_SIZE sizeof(EEPROM_BMS_t)

struct {
    unsigned Bms     : 1;
    unsigned Emg     : 1;
    unsigned Spe     : 1;
    unsigned Reserve : 5;
} eepWriteCmd;

volatile const EEPROM_BMS_t eepBmsDef = {
    EEP_KEY_ID,
    SVN_NUMBER,
 /***BMSParameterSetting***/
  /**Over Temperature**/
  /*Limit,Release,Limit Time,Release Time*/
    {    450,    430, 1000, 3000}, /* OTP */
    {    400,    380, 1000, 3000}, /* OTW */
    {    100,    120, 1000, 3000}, /* UTP */
    {    150,    170, 1000, 3000}, /* UTW */
    {    100,     50, 5000, 3000}, /* UBTW */

  /**OverVoltage**/
  /*Limit,Release,LimitTime,ReleaseTime*/
    { 806000, 800000, 3000, 3000}, /* OVP */
    { 795000, 790000, 3000, 3000}, /* OVW */
    { 560000, 575000, 3000, 3000}, /* UVP */
    { 570000, 590000, 3000, 3000}, /* UVW */
    {    500,    200, 2000, 5000}, /* CellUBP */
    {    350,    300, 2000, 5000}, /* CellUBW */
    {   3650,   3600, 2000, 5000}, /* CellOVP */
    {   2500,   2600, 2000, 5000}, /* CellUVP */

  /**OverCurrent**/
  /*Limit,Release,LimitTime,ReleaseTime*/
    {-120000,  -1000, 1000, 1000}, /* ODCP */
    { -80000,  -1000, 1000, 1000}, /* ODCW */
    {  80000,   1000, 1000, 1000}, /* OCCP */
    {  75000,   1000, 1000, 1000},

 /*OCCW*/
  /* OCPLockTime */
    (60000 / 100),

 /***BMS Capacity Record***/
    (78000 / 2), /* mAH,Batt FullCap */
    (0), /* SOH Decay Coefficient */
  /***BMS Factort Date***/
    26U, /* Factory Day */
    10U, /* Factory Month */
    2021UL, /* Factory Year */
    "0000000000000",
 // clang-format off
    {BAL_OFF, CELL_DESIGN_MAX_VOLT,   10     },
  // clang-format on
  /***Dummy***/
    0,
    0,
    0,
 /******/
    0, /* CheckSum */
};

volatile const EEPROM_EMERGENCY_t eepEmgDef = {
    .ChgCap = CELL_DESIGN_CAP, .CycleLife = 0, .DisChgCap = 0, .ErrorCode = {0}};
volatile const EEPROM_SPECIAL_t eepSpeDef = {.BmsAddr = BMS_COMM_ADDR, .AdcGainOffset = 0, .AdcZeroOffset = 0};

EEPROM_BMS_t       eepBms  = {};
EEPROM_EMERGENCY_t eepEmg  = {};
EEPROM_SPECIAL_t   eepSpe  = {};
BMS_DATA_t         bmsData = {};

EEPROM_OPERATION_STATUS_e eepOpStatus = 0;

/* ************************************************************************** */
/* ************************************************************************** */
// Section: Local Functions                                                   */
/* ************************************************************************** */
/* ************************************************************************** */

/**
 * @brief      Data written into EEPROM
 *
 * @param      ptrData  The starting address of the data
 * @param      dataSize  Data size
 * @param      startAddr The starting address of the EEPROM
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-08-19
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
static void APP_EepromWordWrite(unsigned int* ptrData, unsigned int dataSize, unsigned int startAddr) {
    for (unsigned int i = 0; i < dataSize; i++) {
        EEPROM_WordWrite(startAddr, *(ptrData + i));
        while (EEPROM_IsBusy() == true)
            ;
        startAddr = startAddr + 4;
    }
}

/**
 * @brief      Read data from EEPROM and Storage into the buffer
 *
 * @param      ptrData The starting address of the data
 * @param      dataSize Data size
 * @param      startAddr The starting address of the EEPROM
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-08-19
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
static void APP_EepromWordRead(unsigned int* ptrData, unsigned int dataSize, unsigned int startAddr) {
    for (unsigned int i = 0; i < dataSize; i++) {
        EEPROM_WordRead(startAddr, ptrData + i);
        while (EEPROM_IsBusy() == true)
            ;
        startAddr = startAddr + 4;
    }
}

/**
 * @brief      Calculate the checksum
 *
 * @param      ptrData The starting address of the calculated data
 * @param      dataSize
 * @return     unsigned char
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-08-19
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
static unsigned char APP_EepromChecksumCalculate(unsigned char* ptrData, unsigned short dataSize) {
    unsigned char checkSum = 0;

    for (unsigned short i = 0; i < dataSize; i++) {
        checkSum += *(ptrData + i);
    }
    return checkSum;
}

/**
 * @brief      Erase the EEPROM page data
 *
 * @param      startAddr EEPROM start address
 * @param      dataSize EEPROM size
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-08-19
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
void APP_EepromPageErase(unsigned int startAddr, unsigned int dataSize) {
    for (unsigned int i = 0; i < dataSize; i++) {
        EEPROM_PageErase(startAddr);
        while (EEPROM_IsBusy() == true)
            ;
        startAddr = startAddr + 4;
    }
}

/* ************************************************************************** */
/* ************************************************************************** */
// Section: Interface Functions                                               */
/* ************************************************************************** */
/* ************************************************************************** */

/**
 * @brief      Read data from EEPROM when APP start
 *
 * @return     unsigned char
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-08-19
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
unsigned char APP_EepromInitialize(void) {
    bool          ret      = false;
    unsigned char checkSum = 0;
    /* Read all EEPROM data*/
    APP_EepromWordRead((unsigned int*)&eepSpe, EEPROM_SPE_SIZE / 4,  // Byte to word
                       EEPROM_SPE_START_ADDR);
    APP_EepromWordRead((unsigned int*)&eepEmg, EEPROM_EMG_SIZE / 4,  // Byte to word
                       EEPROM_EMG_START_ADDR);
    APP_EepromWordRead((unsigned int*)&eepBms, EEPROM_BMS_SIZE / 4,  // Byte to word
                       EEPROM_BMS_START_ADDR);

    /*If Chip is new, write default data into EEPROM*/
    if (0xFF == eepSpe.BmsAddr) {
        eepEmg          = eepEmgDef;
        eepSpe          = eepSpeDef;
        eepBms          = eepBmsDef;
        checkSum        = APP_EepromChecksumCalculate((unsigned char*)&eepBms, EEPROM_BMS_SIZE - 1);
        eepBms.CheckSum = checkSum;
        // NVM_PageEraseTasks();
        APP_EepromWordWrite((unsigned int*)&eepSpe, EEPROM_SPE_SIZE / 4,  // Byte to word
                            EEPROM_SPE_START_ADDR);
        APP_EepromWordWrite((unsigned int*)&eepEmg, EEPROM_EMG_SIZE / 4,  // Byte to word
                            EEPROM_EMG_START_ADDR);
        APP_EepromWordWrite((unsigned int*)&eepBms, EEPROM_BMS_SIZE / 4,  // Byte to word
                            EEPROM_BMS_START_ADDR);
    } /* If key & SVN value is different from default value , write data into EERPOM */
    else if ((EEP_KEY_ID != eepBms.EepromKey) || (SVN_NUMBER != eepBms.EepromSVN)) {
        eepBms          = eepBmsDef;
        checkSum        = APP_EepromChecksumCalculate((unsigned char*)&eepBms, EEPROM_BMS_SIZE - 1);
        eepBms.CheckSum = checkSum;
        APP_EepromWordWrite((unsigned int*)&eepBms, EEPROM_BMS_SIZE / 4, EEPROM_BMS_START_ADDR);
    } /* If checkSum value is different from EEPROM ,Set DTC  */
    else {
        APP_EepromWordRead((unsigned int*)&eepBms, EEPROM_BMS_SIZE / 4,  // Byte to Word
                           EEPROM_BMS_START_ADDR);
        checkSum = APP_EepromChecksumCalculate((unsigned char*)&eepBms, EEPROM_BMS_SIZE - 1);
        if (checkSum != eepBms.CheckSum) {
            DTC_FaultOccurSet(DTC_EEPROM_CHECKSUM);
        }
        ret = true;
    }
    return ret;
}

/**
 * @brief      Write bms data to EEPROM
 *
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-09-01
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
void APP_EepromBmsWrite(void) {
    eepWriteCmd.Bms = true;
}
/**
 * @brief      Write emergency data to EEPROM
 *
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-09-01
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
void APP_EepromEmergencyWrite(void) {
    eepEmg.ChgCap    = bmsData.ChgCap;
    eepEmg.CycleLife = bmsData.CycleLife;
    eepEmg.DisChgCap = bmsData.DischgCap;
    eepWriteCmd.Emg  = true;
}
/**
 * @brief      Write special data to EEPROM
 *
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-09-01
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
void APP_EepromSpecialWrite(void) {
    eepWriteCmd.Spe = true;
}

/**
 * @brief      EEPROM polling tasks
 *
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-09-01
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
void APP_EepromTasks(void) {
    if (eepWriteCmd.Emg == true) {
        eepOpStatus     = EEPROM_WRITING;
        eepWriteCmd.Emg = false;
        APP_EepromWordWrite((unsigned int*)&eepEmg, EEPROM_EMG_SIZE / 4,  // Byte to Word
                            EEPROM_EMG_START_ADDR);
    } else if (eepWriteCmd.Spe == true) {
        eepOpStatus     = EEPROM_WRITING;
        eepWriteCmd.Spe = false;
        APP_EepromWordWrite((unsigned int*)&eepSpe, EEPROM_SPE_SIZE / 4,  // Byte to Word
                            EEPROM_SPE_START_ADDR);
    } else if (eepWriteCmd.Bms == true) {
        unsigned char checkSum = 0;
        eepOpStatus            = EEPROM_WRITING;
        eepWriteCmd.Bms        = false;
        checkSum               = APP_EepromChecksumCalculate((unsigned char*)&eepBms, EEPROM_BMS_SIZE - 1);
        eepBms.CheckSum        = checkSum;
        APP_EepromWordWrite((unsigned int*)&eepBms, EEPROM_BMS_SIZE / 4,  // Byte to Word
                            EEPROM_BMS_START_ADDR);
    } else {
        eepOpStatus = EEPROM_READY;
    }
}

EEPROM_OPERATION_STATUS_e APP_EepromStatusGet(void) {
    return eepOpStatus;
}
