/**
 * @file       app_eeprom.c
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
/* Global define -------------------------------------------------------------*/
/* USER CODE BEGIN GD */

/* USER CODE END GD */

/* Includes ------------------------------------------------------------------*/

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "sys_parameter.h"
#include "definitions.h"  // SYS function prototypes
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum {
    EEPROM_READY = 0,
    EEPROM_READING,
    EEPROM_WRITING,
    EEPROM_MAX
} EEPROM_OPERATION_STATUS_e; /*Operation Status*/
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define EEPROM_EMG_SIZE sizeof(EEPROM_EMERGENCY_t)
#define EEPROM_SPE_SIZE sizeof(EEPROM_SPECIAL_t)
#define EEPROM_BMS_SIZE sizeof(EEPROM_BMS_t)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Global variables -----------------------------------------------------------*/
/* USER CODE BEGIN GV */

/* USER CODE END GV */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
static EEPROM_OPERATION_STATUS_e eepOpStatus = 0;

struct {
    unsigned Bms     : 1;
    unsigned Emg     : 1;
    unsigned Spe     : 1;
    unsigned Reserve : 5;
} eepWriteCmd;

/* USER CODE END PV */

/* Function prototypes -------------------------------------------------------*/
/* USER CODE BEGIN FP */

/* USER CODE END FP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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

/* USER CODE END 0 */

/*******************************************************************************
 End of File
 */
