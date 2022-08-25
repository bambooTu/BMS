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

#include "current_sensor.h"


#include "commonly_used.h"

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: File Scope or Global Data                                         */
/* ************************************************************************** */
/* ************************************************************************** */
#define ADC_COEFFICIENT 19966L
#define ADC_OFFSET      0  // convert error value
#define ADC_GAIN        0  // caculate error value

/* ************************************************************************** */
/* ************************************************************************** */
// Section: Interface Functions                                               */
/* ************************************************************************** */
/* ************************************************************************** */

static float CurrentSensor_AdcValue2mAmpere(float adcValue) {
    float ret;
    int   gain;
    /* Adjustment */
    adcValue += ADC_OFFSET;
    /*                           EEPROM  */
    gain = ADC_COEFFICIENT + ADC_GAIN;
    ret  = (adcValue * gain) / 1000.0;

    return ret;
}

float CurrentSensor_AmpereGet(short *arr, unsigned int arrSize) {
    float ret = 0;
    ret       = Arr_Average(arr, arrSize);
    ret       = CurrentSensor_AdcValue2mAmpere(ret);
    ret       = ret / 1000.0;
    return ret;
}
