/**
 * @file       commonly_used.c
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @brief
 * @version    0.1
 * @date       2022-08-19
 *
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 *
 * Abbreviation:
 * None
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

/* ************************************************************************** */
/* ************************************************************************** */
// Section: Local Functions                                                   */
/* ************************************************************************** */
/* ************************************************************************** */

/* ************************************************************************** */
/* ************************************************************************** */
// Section: Interface Functions                                               */
/* ************************************************************************** */
/* ************************************************************************** */

/**
 * @brief
 * Ascending power 1D Lookup Table
 * @param      input Number of table dimensions
 * @param      bp0   1D-Table breakpoints
 * @param      table 1D-Table data
 * @param      tableSize 1D-Table size
 * @return     double
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-08-19
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
double Lookup_Table(double input, volatile const double *bp0, volatile const double *table, unsigned int tableSize) {
    double       frac;
    double       yL_0d0;
    unsigned int bpIdx;
    unsigned int iLeft;
    unsigned int iRght;
    unsigned int maxIndex = tableSize - 1;

    if (input <= bp0[0U]) {
        iLeft = 0U;
        frac  = (input - *bp0) / (*(bp0 + 1) - *bp0);
    } else if (input < bp0[maxIndex]) {
        /* Binary Search */
        bpIdx = maxIndex >> 1U;
        iLeft = 0U;
        iRght = maxIndex;
        while (iRght - iLeft > 1U) {
            if (input < *(bp0 + bpIdx)) {
                iRght = bpIdx;
            } else {
                iLeft = bpIdx;
            }

            bpIdx = (iRght + iLeft) >> 1U;
        }

        frac = (input - *(bp0 + iLeft)) / (*(bp0 + iLeft + 1U) - *(bp0 + iLeft));
    } else {
        iLeft = maxIndex - 1U;
        frac  = (input - *(bp0 + maxIndex - 1U)) / (*(bp0 + maxIndex) - *(bp0 + maxIndex - 1U));
    }

    yL_0d0 = *(table + iLeft);
    return (*(table + iLeft + 1U) - yL_0d0) * frac + yL_0d0;
}

/**
 * @brief
 * Average is the sum of array elements
 * @param      arr Array input
 * @param      arrSize Array size
 * @return     float
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-08-19
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
float Arr_Average(short *arr, unsigned int arrSize) {
    double ret = 0;

    for (unsigned int index = 0; index < arrSize; index++) {
        ret += arr[index];
    }
    ret /= arrSize;
    return ((float)ret);
}

/**
 * @brief
 * The input data is in positive and negative range ,output is zero
 * @param      data Data Input
 * @param      threshold Threshold(Positive and negative range)
 * @return     float
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-08-19
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
float Deadzone_Filter(float data, double threshold) {
    if ((data < threshold) && (data > ((-1) * threshold))) {
        data = 0;
    }
    return data;
}
