#include "indicator.h"
#include "definitions.h"
#include "dtc.h"

#define INDICATOR_ON         GLED_Set()
#define INDICATOR_OFF        GLED_Clear()
#define INDICATOR_STAT       GLED_Get()
#define INDICATOR_TOGGLE     GLED_Toggle()
#define NO_FAULT_INTVL_MS    100
#define LONG_INTVL_MS        600
#define LONG_DELAY_INTVL_MS  2500
#define SHORT_INTVL_MS       250
#define SHORT_DELAY_INTVL_MS 1500

static DTC_FAULT_t   FaultIndicator      = {.l = 0};
static DTC_FAULT_t   FaultIndicatorMask  = {.l = 0};
static DTC_EVENT_e   FaultIndicatorIndex = 0;
static INTERVAL_e    IntervalStep        = INVTERVAL_RESET;
static unsigned char fCompleteARound     = true;

static void IND_WithoutFault(void) {
    static uint16_t IntervalCount = 0;
    if (IntervalCount >= NO_FAULT_INTVL_MS) {
        INDICATOR_TOGGLE;
        IntervalCount = 0;
    }
    IntervalCount += 1;
}

static void IND_WithFault(unsigned char longCount, unsigned char shortCount, INTERVAL_e* IntervalStep,
                          unsigned char* fCompleteARound) {
    static unsigned char  ShortIntervalCount = 0;
    static unsigned char  LongIntervalCount  = 0;
    static unsigned short IntervalCount      = 0;

    switch (*IntervalStep) {
        case INVTERVAL_LONG_DELAY:
            if (IntervalCount >= LONG_DELAY_INTVL_MS) {
                *IntervalStep = INVTERVAL_LONG;
                IntervalCount = 0;
            }
            break;
        case INVTERVAL_LONG:
            if (IntervalCount >= LONG_INTVL_MS) {
                INDICATOR_TOGGLE;
                if (INDICATOR_STAT == false) {
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
            if (IntervalCount >= SHORT_DELAY_INTVL_MS) {
                *IntervalStep = INVTERVAL_SHORT;
                IntervalCount = 0;
            }
            break;
        case INVTERVAL_SHORT:
            if (IntervalCount >= SHORT_INTVL_MS) {
                INDICATOR_TOGGLE;
                if (INDICATOR_STAT == false) {
                    ShortIntervalCount++;
                    if (ShortIntervalCount >= shortCount) {
                        *fCompleteARound   = true;
                        *IntervalStep      = INVTERVAL_LONG_DELAY;
                        ShortIntervalCount = 0;
                    }
                }
                IntervalCount = 0;
            }
            break;

        case INVTERVAL_RESET:
            // jump to default
        default:
            ShortIntervalCount = 0;
            LongIntervalCount  = 0;
            IntervalCount      = 0;
            *IntervalStep      = INVTERVAL_LONG_DELAY;
            break;
    }
    IntervalCount += 1;
}

void IND_Initialize(void) {
    INDICATOR_OFF;
    FaultIndicatorMask.l              = 0x1FFFFFFFF;
    FaultIndicatorMask.b.SHUT_DOWN    = 0;
    FaultIndicatorMask.b.EMERGENCY    = 0;
    FaultIndicatorMask.b.CURR_DIR_ERR = 0;
}

void IND_1ms_Tasks(void) {
    FaultIndicator.l          = DTC_FaultMapGet();
    FaultIndicator.b.AFE_COMM = 1;  // TODO:DELETE
    FaultIndicator.b.ODCP     = 1;  // TODO:DELETE
    FaultIndicator.l          = FaultIndicator.l & FaultIndicatorMask.l;
    if (FaultIndicator.l) {
        if (fCompleteARound) {
            fCompleteARound = false;
            INDICATOR_OFF;
            IntervalStep = INVTERVAL_RESET;
            if ((FaultIndicator.l >> FaultIndicatorIndex) & 0x01) {
                fCompleteARound = false;
            } else {
                fCompleteARound = true;
            }
            FaultIndicatorIndex++;
            if (FaultIndicatorIndex >= DTC_EVENT_MAX_NUM) {
                FaultIndicatorIndex = 0;
            }
        } else {
            IND_WithFault(DTC_BMS_Message_Table[FaultIndicatorIndex - 1].longCount,
                          DTC_BMS_Message_Table[FaultIndicatorIndex - 1].shortCount, &IntervalStep, &fCompleteARound);
        }
    } else {
        fCompleteARound     = true;
        FaultIndicatorIndex = 0;
        IND_WithoutFault();
    }
}

/*******************************************************************************
 End of File
 */
