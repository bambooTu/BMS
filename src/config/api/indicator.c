#include "sys_parameter.h"

#define debugLED_On
#define debugLED_Off
#define debugLED_Get false
#define debugLED_Toggle
#define NoFaultInterval_mS    100
#define LongInterval_mS       600
#define LongDelayInterval_mS  2500
#define ShortInterval_mS      250
#define ShortDelayInterval_mS 1500

extern volatile const DTC_MESSAGE_TABLE_t DTC_BMS_Message_Table[DTC_EVENT_MAX_NUM];
extern DTC_FAULT_t                        FaultIndicator, FaultIndicatorMask;

static DTC_EVENT_e   FaultIndicatorIndex = 0;
static INTERVAL_e    IntervalStep        = INVTERVAL_RESET;
static unsigned char fCompleteARound     = true;

void Indicator_1ms_Tasks(void) {
    if (FaultIndicator.l & FaultIndicatorMask.l) {
        if (fCompleteARound) {
            fCompleteARound = false;
            debugLED_Off;
            IntervalStep = INVTERVAL_RESET;
            do {
                FaultIndicatorIndex++;
                if (((FaultIndicatorMask.l >> FaultIndicatorIndex) & 0x01) == false) {
                    FaultIndicatorIndex++;
                }
                if (FaultIndicatorIndex >= DTC_EVENT_MAX_NUM) {
                    FaultIndicatorIndex = 0;
                }
            } while (((FaultIndicator.l >> FaultIndicatorIndex) & 0x01) == false);
        }
        Indicator_WithFault(DTC_BMS_Message_Table[FaultIndicatorIndex].longCount,
                            DTC_BMS_Message_Table[FaultIndicatorIndex].shortCount, &IntervalStep, &fCompleteARound);
    } else {
        fCompleteARound     = true;
        FaultIndicatorIndex = 0;
        Indicator_WithoutFault();
    }
}

void Indicator_WithoutFault(void) {
    static uint16_t IntervalCount = 0;
    if (IntervalCount >= NoFaultInterval_mS) {
        debugLED_Toggle;
        IntervalCount = 0;
    }
    IntervalCount += 1;
}

void Indicator_WithFault(unsigned char longCount, unsigned char shortCount, INTERVAL_e* IntervalStep,
                         unsigned char* OneRound) {
    static unsigned char  ShortIntervalCount = 0;
    static unsigned char  LongIntervalCount  = 0;
    static unsigned short IntervalCount      = 0;

    switch (*IntervalStep) {
        case INVTERVAL_LONG_DELAY:
            if (IntervalCount >= LongDelayInterval_mS) {
                *IntervalStep = INVTERVAL_LONG;
                IntervalCount = 0;
            }
            break;
        case INVTERVAL_LONG:
            if (IntervalCount >= LongInterval_mS) {
                debugLED_Toggle;
                if (debugLED_Get == false) {
                    LongIntervalCount++;
                    if (LongIntervalCount >= longCount) {
                        *IntervalStep     = INVTERVAL_SHORT_DELAY;
                        LongIntervalCount = 0;
                    }
                }
                IntervalCount = 0;
            }
            break;
        case INVTERVAL_SHORT_DELAY:
            if (IntervalCount >= ShortDelayInterval_mS) {
                *IntervalStep = INVTERVAL_SHORT;
                IntervalCount = 0;
            }
            break;
        case INVTERVAL_SHORT:
            if (IntervalCount >= ShortInterval_mS) {
                debugLED_Toggle;
                if (debugLED_Get == false) {
                    ShortIntervalCount++;
                    if (ShortIntervalCount >= shortCount) {
                        *OneRound          = true;
                        *IntervalStep      = INVTERVAL_LONG_DELAY;
                        ShortIntervalCount = 0;
                    }
                }
                IntervalCount = 0;
            }
            break;

        case INVTERVAL_RESET:
        default:
            ShortIntervalCount = 0;
            LongIntervalCount  = 0;
            IntervalCount      = 0;
            *IntervalStep      = INVTERVAL_LONG_DELAY;
            break;
    }
    IntervalCount += 1;
}

/*******************************************************************************
 End of File
 */
