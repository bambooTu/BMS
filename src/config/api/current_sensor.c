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

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: File Scope or Global Data                                         */
/* ************************************************************************** */
/* ************************************************************************** */
#define CURR_SNSR_TYPE          1             // 0:MCP3421 1:CSNVC500
#define CURR_SNSR_TIMEOUT_TIMES 10            // unit:10ms
#define IP_VALUE_OFFSET         0x80000000UL  // ref. CSNVC500 datasheet
#define ADC_COEFFICIENT         19966L
#define MOVING_AVG_TIMES        12L
#define ZERO_THRESHOLD          0L

// CRC-8 POLY: 8+X2+X+1
unsigned char crc8Table[] = {
    0x00, 0x07, 0x0e, 0x09, 0x1c, 0x1b, 0x12, 0x15, 0x38, 0x3f, 0x36, 0x31, 0x24, 0x23, 0x2a, 0x2d, 0x70, 0x77, 0x7e,
    0x79, 0x6c, 0x6b, 0x62, 0x65, 0x48, 0x4f, 0x46, 0x41, 0x54, 0x53, 0x5a, 0x5d, 0xe0, 0xe7, 0xee, 0xe9, 0xfc, 0xfb,
    0xf2, 0xf5, 0xd8, 0xdf, 0xd6, 0xd1, 0xc4, 0xc3, 0xca, 0xcd, 0x90, 0x97, 0x9e, 0x99, 0x8c, 0x8b, 0x82, 0x85, 0xa8,
    0xaf, 0xa6, 0xa1, 0xb4, 0xb3, 0xba, 0xbd, 0xc7, 0xc0, 0xc9, 0xce, 0xdb, 0xdc, 0xd5, 0xd2, 0xff, 0xf8, 0xf1, 0xf6,
    0xe3, 0xe4, 0xed, 0xea, 0xb7, 0xb0, 0xb9, 0xbe, 0xab, 0xac, 0xa5, 0xa2, 0x8f, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9d,
    0x9a, 0x27, 0x20, 0x29, 0x2e, 0x3b, 0x3c, 0x35, 0x32, 0x1f, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0d, 0x0a, 0x57, 0x50,
    0x59, 0x5e, 0x4b, 0x4c, 0x45, 0x42, 0x6f, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7d, 0x7a, 0x89, 0x8e, 0x87, 0x80, 0x95,
    0x92, 0x9b, 0x9c, 0xb1, 0xb6, 0xbf, 0xb8, 0xad, 0xaa, 0xa3, 0xa4, 0xf9, 0xfe, 0xf7, 0xf0, 0xe5, 0xe2, 0xeb, 0xec,
    0xc1, 0xc6, 0xcf, 0xc8, 0xdd, 0xda, 0xd3, 0xd4, 0x69, 0x6e, 0x67, 0x60, 0x75, 0x72, 0x7b, 0x7c, 0x51, 0x56, 0x5f,
    0x58, 0x4d, 0x4a, 0x43, 0x44, 0x19, 0x1e, 0x17, 0x10, 0x05, 0x02, 0x0b, 0x0c, 0x21, 0x26, 0x2f, 0x28, 0x3d, 0x3a,
    0x33, 0x34, 0x4e, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5c, 0x5b, 0x76, 0x71, 0x78, 0x7f, 0x6a, 0x6d, 0x64, 0x63, 0x3e,
    0x39, 0x30, 0x37, 0x22, 0x25, 0x2c, 0x2b, 0x06, 0x01, 0x08, 0x0f, 0x1a, 0x1d, 0x14, 0x13, 0xae, 0xa9, 0xa0, 0xa7,
    0xb2, 0xb5, 0xbc, 0xbb, 0x96, 0x91, 0x98, 0x9f, 0x8a, 0x8d, 0x84, 0x83, 0xde, 0xd9, 0xd0, 0xd7, 0xc2, 0xc5, 0xcc,
    0xcb, 0xe6, 0xe1, 0xe8, 0xef, 0xfa, 0xfd, 0xf4, 0xf3,
};

static struct {
    int            ipValue;
    bool           errorIndication;
    unsigned       errorInfo;
    unsigned short vacantData;
    unsigned char  crc8;
} CSNVC500;

static short adcZeroOffset = 0;  // convert error value
static short adcGainOffset = 0;  // caculate error value
// TODO : static

unsigned int gCurrSnsrTimeoutCount = 0;
int          gCurrentSensorGain    = 1;

/* ************************************************************************** */
/* ************************************************************************** */
// Section: Interface Functions                                               */
/* ************************************************************************** */
/* ************************************************************************** */
/**
 * @brief      Calculate the crc 8 value.
 *
 * @param      ptr data array
 * @param      len size size
 * @return     unsigned char
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-09-28
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
static unsigned char CURRSNSR_CalculateCRC8(unsigned char* ptr, unsigned char len) {
    unsigned char crc = 0x00;

    while (len--) {
        crc = crc8Table[crc ^ *ptr++];
    }
    return (crc);
}
/**
 * @brief      Decode the CSNVC500 current value(mA)
 *             The polling period is 10ms
 * @param      ipValue CSNVC500 current value(encode)
 * @return     int
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-09-26
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
static int CSNVC500_CurrGet(unsigned int ipValue) {
    int ret = 0;
    ret     = ipValue - IP_VALUE_OFFSET;
    return ret;
}
/**
 * @brief      Transfer the MCP3421's ADC value to milli Ampere
 *
 * @param      adcValue
 * @return     int
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-09-06
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
static int CURRSNSR_AdcValueToCurr(int adcValue) {
    int ret;
    int gain;
    /* Adjustment */
    adcValue += adcZeroOffset;
    /*                        EEPROM  */
    gain = ADC_COEFFICIENT + adcGainOffset;
    ret  = (adcValue * gain) / 1000;

    return ret;
}
/**
 * @brief      Sensor Type 0: Get the MCP3421 ADC value and do a moving average, then convert the value to the current value (mA).
 *             Sensor Type 1: Get the CSNVC500 current value(mA) after doing the moving average.
 *            
 * @param      arr
 * @param      arrSize
 * @return     int
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-09-06
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
static inline int CURRSNSR_ProcessedCurrGet(void) {
    int                  ret   = 0;
    static unsigned char Index = 0;
    static double        Buffer[MOVING_AVG_TIMES];

#if (CURR_SNSR_TYPE == 0)
    Buffer[adcIndex] = MCP3421_AdcValueGet();
    if (++Index > MOVING_AVG_TIMES - 1) {
        Index = 0;
    }
    ret = (int)Filter_ArrayAverage(Buffer, MOVING_AVG_TIMES);
    ret = CURRSNSR_AdcValueToCurr(ret);
#elif (CURR_SNSR_TYPE == 1)

    Buffer[Index] = CSNVC500_CurrGet(CSNVC500.ipValue);
    if (++Index > MOVING_AVG_TIMES) {
        Index = 0;
    }
    ret = (int)Filter_ArrayAverage(Buffer, MOVING_AVG_TIMES);
#endif
    Filter_Deadzone(ret, ZERO_THRESHOLD);
    return ret;
}

/**
 * @brief      Set the current sensor's parameter.
 *
 * @param      gainOffset
 * @param      zeroOffset
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-09-06
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
void CURRSNSR_ParamSet(short gainOffset, short zeroOffset) {
    adcGainOffset = SATURATION(gainOffset, 1000, -1000);
    adcZeroOffset = SATURATION(zeroOffset, 1000, -1000);
}
/**
 * @brief      When there is a current sensor ID in the queue, decode the current value(mA).
 *
 * @param      canRxMsg
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-09-28
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
void CURRSNSR_CheckQueueTasks(can_msg_t canRxMsg) {
    gCurrSnsrTimeoutCount = CURR_SNSR_TIMEOUT_TIMES;

    if (CURRSNSR_CalculateCRC8(canRxMsg.data, 7) == canRxMsg.data[7]) {
        CSNVC500.ipValue = (canRxMsg.data[3])           //
                           + (canRxMsg.data[2] << 8)    //
                           + (canRxMsg.data[1] << 16)   //
                           + (canRxMsg.data[0] << 24);  //
        CSNVC500.errorInfo       = (canRxMsg.data[4] & 0xFE) >> 1;
        CSNVC500.errorIndication = (canRxMsg.data[4] & 0x01);
    }
}
/**
 * @brief      Initialize the current sensor's parameter after EEPROM read done.
 *
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-09-06
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
void CURRSNSR_Intialize(void) {
    CURRSNSR_ParamSet(eepSpe.AdcGainOffset, eepSpe.AdcZeroOffset);
    CSNVC500.ipValue         = IP_VALUE_OFFSET;
    CSNVC500.errorIndication = 0;
    CSNVC500.vacantData      = 0;
    CSNVC500.errorInfo       = 0;
    CSNVC500.crc8            = 0;
}
/**
 * @brief      Sensor Type 0: Get the MCP3421 ADC value and then transfer the value to current value(mA).
 *             Sensor Type 1: Get the CSNVC500 current value(mA).
 *             The polling period is 10ms.
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-09-06
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
void CURRSNSR_10ms_Tasks(void) {
    
    // Current sensor timeout check
    if (gCurrSnsrTimeoutCount) {
        gCurrSnsrTimeoutCount--;
    } else {
        CSNVC500.ipValue = IP_VALUE_OFFSET;
    }

    // Current sensor error check
    if (CSNVC500.errorIndication == true) {
        CSNVC500.ipValue = IP_VALUE_OFFSET;
    }

    bmsData.BusCurrent = CURRSNSR_ProcessedCurrGet() * gCurrentSensorGain;
}

