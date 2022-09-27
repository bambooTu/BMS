/**
 * @file       commonly_used.h
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

#ifndef _COMMONLY_USED_H /* Guard against multiple inclusion */
#define _COMMONLY_USED_NAME_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

/* Provide C++ Compatibility */
#ifdef __cplusplus
extern "C" {
#endif

#define MOD(A, M)              (A % M)
#define DIVIDE(A, M)           (A / M)
#define MAX(A, B)              ((A < B) ? B : A)
#define MIN(A, B)              ((B < A) ? B : A)
#define ABS(A)                 (((A) >= 0) ? (A) : -(A))          /* absolute value */
#define DELTA(A, B)            (((A) >= (B)) ? (A - B) : (B - A)) /* defferrence */
#define LOW_BYTE(A)            (*((unsigned char *)(&A)))
#define HIGH_BYTE(A)           (*((unsigned char *)(&A) + 1))
#define LOW_WORD(A)            (*((unsigned short *)(&A)))
#define HIGH_WORD(A)           (*((unsigned short *)(&A) + 1))
#define BIT_SET(VAR, BIT_NUM)  ((VAR) |= 1UL << (BIT_NUM))
#define BIT_CLR(VAR, BIT_NUM)  ((VAR) &= ~(1UL << (BIT_NUM)))
#define BIT_READ(VAR, BIT_NUM) ((VAR) &= (1UL << (BIT_NUM)))
#define SATURATION(VAR, MAX, MIN)   \
    ({                              \
        double __ret = 0;           \
        do {                        \
            if (MIN > MAX) {        \
                __ret = 0;          \
            } else if (VAR > MAX) { \
                __ret = MAX;        \
            } else if (VAR < MIN) { \
                __ret = MIN;        \
            } else {                \
                __ret = VAR;        \
            }                       \
        } while (0);                \
        __ret;                      \
    })

// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
// *****************************************************************************
// Section: Interface Functions
// *****************************************************************************
// *****************************************************************************
double Lookup_Table(double input, volatile const double *bp0, volatile const double *table, unsigned int tableSize);
float  Filter_Deadzone(float data, double threshold);
float  Filter_ArrayAverage(short *arr, unsigned int arrSize);

/* Provide C++ Compatibility */
#ifdef __cplusplus
}
#endif

#endif /* _EXAMPLE_FILE_NAME_H */

/* *****************************************************************************
 End of File
 */
