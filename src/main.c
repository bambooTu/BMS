/*******************************************************************************
  Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This file contains the "main" function for a project.

  Description:
    This file contains the "main" function for a project.  The
    "main" function calls the "SYS_Initialize" function to initialize the state
    machines of all modules in the system
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdbool.h>  // Defines true
#include <stddef.h>   // Defines NULL
#include <stdlib.h>   // Defines EXIT_FAILURE

#include "config/api/sys_parameter.h"
#include "definitions.h"  // SYS function prototypes

#define PRECHG_TIMEOUT_MS         (1000)
#define PRECHG_NUM_OF_ERROR       (3)
#define TURN_ON_PRECHG_DELAY_MS   (500)
#define TURN_OFF_PRECHG_DELAY_MS  (500)
#define TURN_ON_POS_DELAY_MS      (500)
#define TURN_OFF_POS_DELAY_MS     (500)
#define TURN_OFF_NEG_DELAY_MS     (500)
#define BACK_END_VOLT             (100)
#define PRECHG_PERCENTAGE_SETTING (90)
#define RELAY_NEG_OPEN            RELAY_NEG_Clear();
#define RELAY_NEG_CLOSE           RELAY_NEG_Set();
#define RELAY_POS_OPEN            RELAY_POS_Clear();
#define RELAY_POS_CLOSE           RELAY_POS_Set();
#define RELAY_PRECHG_OPEN         RELAY_PRECHG_Clear();
#define RELAY_PRECHG_CLOSE        RELAY_PRECHG_Set();
#define FAULT_INDICATOR_ON
#define FAULT_INDICATOR_OFF
#define CURRENT               25
#define PRECHG_CURRENT_OFFSET 20
#define FAULT_CONDITION
#define HV_CMD
#define CALCULTAE_TIME_MS(A) ((A < 10) ? 10 - 1 : 10 * A - 1)

typedef enum {
    HV_SETUP_INIT,
    HV_PRECHG_START,
    HV_PRECHG_FAIL,
    HV_PRECHG_FINISH,
    HV_SETUP_FINISH,
    HV_SETUP_FAULT,
} HV_SETUP_STATUS_e;

typedef enum {
    HV_OFF_INIT,
    HV_OFF_PREWORK,
    HV_OFF_FAIL,
    HV_OFF_FINISH,
    HV_OFF_FORCE,
} HV_OFF_STATUS_e;

typedef enum {
    MODE_OFF,
    MODE_ON,
    MODE_PRECHG,
    MODE_EMRG,
} HV_OPERATION_MODE_e;

typedef struct {
    HV_SETUP_STATUS_e   setupStatus;
    HV_OFF_STATUS_e     offStatus;
    HV_OPERATION_MODE_e opMode;
    unsigned int        errorCount;
    unsigned int        delayTimeCount;
} HV_DATA_t;

HV_DATA_t HV = {
    .offStatus = HV_SETUP_INIT, .setupStatus = HV_OFF_INIT, .opMode = false, .delayTimeCount = 0, .errorCount = 0};

static void HV_SeqTurnOn(void) {
    HV.offStatus = HV_OFF_INIT;
    switch (HV.setupStatus) {
        default:
        case HV_SETUP_INIT:
            HV.errorCount = 0;
            RELAY_NEG_CLOSE;
            RELAY_POS_OPEN;
            RELAY_PRECHG_OPEN;

            if (HV.delayTimeCount++ >= TURN_ON_PRECHG_DELAY_MS) {
                HV.delayTimeCount = 0;
                HV.setupStatus    = HV_PRECHG_START;
            }

            break;
        case HV_PRECHG_START:
            RELAY_PRECHG_CLOSE;
            RELAY_POS_OPEN;
            if (HV.opMode == MODE_ON) {
                if (CURRENT < PRECHG_CURRENT_OFFSET) {
                    if (HV.delayTimeCount++ >= TURN_ON_POS_DELAY_MS) {
                        HV.delayTimeCount = 0;
                        HV.setupStatus    = HV_PRECHG_FINISH;
                    };
                } else {
                    HV.delayTimeCount = 0;
                    HV.setupStatus    = HV_PRECHG_FAIL;
                }
            }
            break;
        case HV_PRECHG_FINISH:
            RELAY_POS_CLOSE;
            if (HV.delayTimeCount++ >= TURN_OFF_PRECHG_DELAY_MS) {
                HV.delayTimeCount = 0;
                HV.setupStatus    = HV_SETUP_FINISH;
            }
            break;
        case HV_PRECHG_FAIL:
            if (HV.delayTimeCount++ >= PRECHG_TIMEOUT_MS) {
                if (HV.errorCount++ >= PRECHG_NUM_OF_ERROR) {
                    RELAY_PRECHG_OPEN;
                    HV.errorCount     = 0;
                    HV.delayTimeCount = 0;
                    HV.setupStatus    = HV_SETUP_FAULT;
                } else {
                    HV.errorCount     = 0;
                    HV.delayTimeCount = 0;
                    HV.setupStatus    = HV_PRECHG_START;
                }
            };

            break;
        case HV_SETUP_FINISH:
            HV.delayTimeCount = 0;
            RELAY_PRECHG_OPEN;
            if (HV.opMode == MODE_PRECHG) {
                RELAY_PRECHG_CLOSE;
                if (HV.delayTimeCount++ >= TURN_OFF_PRECHG_DELAY_MS) {
                    HV.delayTimeCount = 0;
                    HV.opMode         = HV_PRECHG_START;
                }
            }
            GLED_Set();
            break;
        case HV_SETUP_FAULT:
            HV.delayTimeCount = 0;
            RELAY_NEG_OPEN;
            break;
    }
}

static void HV_SeqTurnOff(void) {
    HV.setupStatus = HV_SETUP_INIT;
    switch (HV.offStatus) {
        default:
        case HV_OFF_INIT:
            RELAY_PRECHG_OPEN;
            HV.errorCount = 0;
            if (HV.opMode == MODE_EMRG) {
                HV.offStatus = HV_OFF_FORCE;
            } else {
                HV.offStatus = HV_OFF_PREWORK;
            }
            break;
        case HV_OFF_PREWORK:
            if (CURRENT < PRECHG_CURRENT_OFFSET) { /*TODO: change  CURRENT to the Real var  */
                if (HV.delayTimeCount++ >= TURN_OFF_POS_DELAY_MS) {
                    HV.delayTimeCount = 0;
                    RELAY_POS_OPEN;
                    HV.offStatus = HV_OFF_FINISH;
                }
            } else {
                HV.delayTimeCount = 0;
                HV.offStatus      = HV_OFF_FAIL;
            }
            break;
        case HV_OFF_FAIL:
            if (HV.delayTimeCount++ >= PRECHG_TIMEOUT_MS) {
                if (HV.errorCount++ >= PRECHG_NUM_OF_ERROR) {
                    HV.errorCount     = 0;
                    HV.delayTimeCount = 0;
                    HV.offStatus      = HV_OFF_FORCE;
                } else {
                    HV.errorCount     = 0;
                    HV.delayTimeCount = 0;
                    HV.offStatus      = HV_OFF_PREWORK;
                }
            }
            break;
        case HV_OFF_FINISH:
            if (HV.delayTimeCount++ >= TURN_OFF_NEG_DELAY_MS) {
                HV.delayTimeCount = 0;
                RELAY_NEG_OPEN;
            }
            GLED_Clear();
            break;
        case HV_OFF_FORCE:
            HV.delayTimeCount = 0;
            RELAY_POS_OPEN;
            RELAY_NEG_OPEN;
            Fault_EventSet(DTC_BMS_RELAY);
            break;
    }
}

HV_OFF_STATUS_e HV_SetupStatusGet(void) {
    return HV.setupStatus;
}

HV_OFF_STATUS_e HV_OffStatusGet(void) {
    return HV.offStatus;
}

void HV_ModeCommand(HV_OPERATION_MODE_e opMode) {
    HV.opMode = opMode;
}

void HV_Initialize(void) {
    RELAY_POS_OPEN;
    RELAY_NEG_OPEN;
    RELAY_PRECHG_OPEN;
}

void HV_1ms_Tasks(void) {
    if ((HV.opMode == MODE_ON) || (HV.opMode == MODE_PRECHG)) {
        HV_SeqTurnOn();
    } else {
        HV_SeqTurnOff();
    }
}

void BMS_SoftWareReset(void) {
    HV_ModeCommand(MODE_OFF);
    if ((HV_OffStatusGet() == HV_OFF_FINISH) || (HV_OffStatusGet() == HV_OFF_FORCE)) {
        __builtin_disable_interrupts();

        /* Unlock System */
        SYSKEY = 0x00000000;
        SYSKEY = 0xAA996655;
        SYSKEY = 0x556699AA;

        RSWRSTSET = _RSWRST_SWRST_MASK;

        /* Read RSWRST register to trigger reset */
        (void)RSWRST;

        /* Prevent any unwanted code execution until reset occurs */
        while (1)
            ;
    }
}
bool fProtectionProcess = false;

void BMS_Protection(void) {
    static unsigned char protectionState = 0;
    switch (protectionState) {
        case 0:
            HV_ModeCommand(MODE_OFF);
            if ((HV_OffStatusGet() == HV_OFF_FINISH) || (HV_OffStatusGet() == HV_OFF_FORCE)) {
                protectionState++;
            }
            break;
        case 1:
            if (DIN_StateGet(DIN_4) == true) {
                protectionState++;
            } else {
                FAULT_INDICATOR_ON; /*TODO: Set GPIO*/
            }
            break;
        case 2:
            BMS_SoftWareReset();
            protectionState = 0;
        default:
            protectionState = 0;
            break;
    }
}
bool                 fEmrgProcess    = false;
static unsigned char gEmergencyState = 0;

void BMS_Emergency(void) {
    switch (gEmergencyState) {
        case 0:
            HV_ModeCommand(MODE_OFF);
            if ((HV_OffStatusGet() == HV_OFF_FINISH) || (HV_OffStatusGet() == HV_OFF_FORCE)) {
                gEmergencyState++;
                Nop();
            }
            break;
        case 1:
            if (DIN_StateGet(DIN_4) == false) {
                gEmergencyState++;
                Nop();
            }
            break;
        case 2:
            /*TODO: set_emergency_2_eeprom();*/
            Nop();
            gEmergencyState++;
            break;
        case 3:
            Nop();
            fEmrgProcess = false;
            break;
        default:
            fEmrgProcess    = false;
            gEmergencyState = 0;
            break;
    }
}

void BMS_ModeCommand(BMS_WORK_MODE_e opMode) {
    bmsData.WorkModeCmd = opMode;
}

static void BMS_CommandDetect(void) {
    /*TODO: Engineer mode*/
    /*
        bool fProtectionProcess = false;

        if(!ENGR){
            fProtectionProcess = false;
        }

        if(ENGR){
            if ((HV_OffStatusGet() == HV_OFF_FINISH) || (HV_OffStatusGet() == HV_OFF_FORCE) || fProtectionProcess) {
                BMS_EngineerMode();
            }
            esle{
                BMS_ModeCommand(BMS_OFF);
                fProtectionProcess = ture;
            }
        }
    else
    */
    /*Execute the emergency stop command when the EMS is pressed
     and the BMS_Protection() has not been executed*/
    if (((DIN_StateGet(DIN_4) == true) && (fProtectionProcess == false)) ||
        (fEmrgProcess == true)) {  // While the BMS_Emergency() is executing
        BMS_ModeCommand(BMS_OCCUR_EMRG);
        fEmrgProcess = true;
    } /*Execute the fault protection command when the fault is occur
        and the BMS_Emergency() has not been executed*/
    else if (((DTC_WorstLevelGet() == ERR_LEVEL_PROTECTION) && (fEmrgProcess == false)) ||
             (fProtectionProcess == true)) {  // While the BMS_Protection() is executing
        BMS_ModeCommand(BMS_OCCUR_FAULT);
        fProtectionProcess = true;
    } /*If the fault occur,command BMS_OFF */
    else if (DTC_WorstLevelGet() == ERR_LEVEL_FAULT) {
        BMS_ModeCommand(BMS_OFF);
    }

    /*Clear the error code and reset BMS_Emergency() task state
     when the BMS_Emergency() has been executed */
    if (fEmrgProcess == false) {
        DTC_FaultOccurClear(DIN_4);
        gEmergencyState = 0;
    }
}

void BMS_1ms_Tasks(void) {
    BMS_CommandDetect();
    switch (bmsData.WorkModeCmd) {
        case BMS_RESET:
            BMS_SoftWareReset();
            break;
        case BMS_OCCUR_FAULT:
            BMS_Protection();
            break;
        case BMS_OCCUR_EMRG:
            BMS_Emergency();
            break;
        case BMS_OFF:
            HV_ModeCommand(MODE_OFF);
            break;
        case BMS_CHG_ON:
        case BMS_DISCHG_ON:
            HV_ModeCommand(MODE_ON);
            break;
        case BMS_CHG_PRE_ON:
        case BMS_DISCHG_PRE_ON:
            HV_ModeCommand(MODE_PRECHG);
            break;
        default:
            break;
    }
}

struct {
    unsigned _1ms   : 1;
    unsigned _5ms   : 1;
    unsigned _500ms : 1;
} tmrData;

unsigned int cnt_1ms, cnt_5ms, cnt_500ms = 0;

void TMR4_EvnetHandler(uint32_t status, uintptr_t context) {
    if (cnt_1ms++ >= CALCULTAE_TIME_MS(1)) {
        cnt_1ms      = 0;
        tmrData._1ms = true;
    }
    if (cnt_5ms++ >= CALCULTAE_TIME_MS(5)) {
        cnt_5ms      = 0;
        tmrData._5ms = true;
    }
    if (cnt_500ms++ >= CALCULTAE_TIME_MS(500)) {
        cnt_500ms      = 0;
        tmrData._500ms = true;
    }
    X2CScope_Update();
}
// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************
bool          polar = false;
unsigned char step  = 0;

int main(void) {
    /* Initialize all modules */
    SYS_Initialize(NULL);
    TMR4_CallbackRegister(TMR4_EvnetHandler, (uintptr_t)NULL);
    TMR4_Start();
    DIN_ParameterInitialize();
    HV_Initialize();
    eepBms = eepBmsDef;

    while (true) {
        /* Maintain state machines of all polled MPLAB Harmony modules. */
        SYS_Tasks();
        X2CScope_Communicate();
        if (tmrData._1ms) {
            tmrData._1ms = false;
            HV_1ms_Tasks();
            DTC_1ms_Tasks();
            BMS_1ms_Tasks();

            switch (step) {
                case 0:
                    if (DIN_StateGet(DIN_2) == true) {
                        step++;
                    }
                    break;
                case 1:
                    if (DIN_StateGet(DIN_2) == false) {
                        step++;
                    }
                    break;
                case 2:
                    polar = !polar;
                    if (polar) {
                        BMS_ModeCommand(BMS_DISCHG_ON);
                        GLED_Set();
                    } else {
                        BMS_ModeCommand(BMS_OFF);
                    }
                    step = 0;
                default:
                    step = 0;
                    break;
            }
        }

        if (tmrData._5ms) {
            tmrData._5ms = false;
            DIN_5ms_Tasks();
        }
        if (tmrData._500ms) {
            tmrData._500ms = false;
            /*TODO: Delete Test Function*/
            static unsigned char testi = 0;
            if (testi++ > 20) {
                // DTC_FaultOccurSet(DTC_BMU_COMM);
            }
            /*TODO: Delete Test Function*/
            RLED_Toggle();
        }
    }

    /* Execution should not come here during normal operation */

    return (EXIT_FAILURE);
}

/*******************************************************************************
 End of File
 */
