#ifndef _INDICATOR_H
#define _INDICATOR_H

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

/*Indicator*/
typedef enum {
    INVTERVAL_RESET,
    INVTERVAL_LONG_DELAY,
    INVTERVAL_LONG,
    INVTERVAL_SHORT_DELAY,
    INVTERVAL_SHORT,
} INTERVAL_e;

void Indicator_1ms_Tasks(void);
void Indicator_WithoutFault(void);
void Indicator_WithFault(unsigned char longCount, unsigned char shortCount, INTERVAL_e* IntervalStep,
                         unsigned char* OneRound);

// DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
// DOM-IGNORE-END

#endif /* _APP_H */

/*******************************************************************************
 End of File
 */
