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
#define ADC_COEFFICIENT      19966L
#define ADC_MOVING_AVG_TIMES 12

static short         adcBuffer[ADC_MOVING_AVG_TIMES];
static unsigned char adcIndex      = 0;
static short         adcZeroOffset = 0;  // convert error value
static short         adcGainOffset = 0;  // caculate error value

/* ************************************************************************** */
/* ************************************************************************** */
// Section: Interface Functions                                               */
/* ************************************************************************** */
/* ************************************************************************** */
/**
 * @brief      Transfer the ADC value to milli Ampere
 * 
 * @param      adcValue 
 * @return     int 
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-09-06
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
static int CurrentSensor_AdcValue2mAmpere(int adcValue) {
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
 * @brief      Get the moving average current value
 * 
 * @param      arr 
 * @param      arrSize 
 * @return     int 
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-09-06
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
static int CurrentSensor_mAmpereGet(short *arr, unsigned int arrSize) {
    int ret = 0;
    ret     = (int)Arr_Average(arr, arrSize);
    ret     = CurrentSensor_AdcValue2mAmpere(ret);
    ret     = ret;
    return ret;
}
/**
 * @brief      Set the current sensor's parameter 
 * 
 * @param      gainOffset 
 * @param      zeroOffset 
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-09-06
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
void CurrentSensor_ParamSet(short gainOffset, short zeroOffset) {
    adcGainOffset = SATURATION(gainOffset, 1000, -1000);
    adcZeroOffset = SATURATION(zeroOffset, 1000, -1000);
}
/**
 * @brief      Initialize the current sensor's parameter after EEPROM read done
 *
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-09-06
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
void CurrentSensor_Intialize(void) {
    CurrentSensor_ParamSet(eepSpe.AdcGainOffset, eepSpe.AdcZeroOffset);
}
/**
 * @brief      Get the MCP3421 ADC value and then transfer the value to current value
 *             The polling period is 10ms
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-09-06
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
void CurrentSensor_10ms_Tasks(void) {
    adcBuffer[adcIndex] = MCP3421_AdcValueGet();
    if (++adcIndex > ADC_MOVING_AVG_TIMES - 1) {
        adcIndex = 0;
    }
    bmsData.BusCurrent = CurrentSensor_mAmpereGet(adcBuffer, ADC_MOVING_AVG_TIMES);
}
