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

#include "config/api/coulomb_gauge.h"
#include "config/api/sys_parameter.h"
#include "definitions.h"  // SYS function prototypes

#define CALCULTAE_TIME_MS(A) ((A < 1) ? 10 - 1 : 10 * A - 1)

// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
// *****************************************************************************

typedef struct {
    unsigned int cnt;
    bool         flag;
} TMR_DATA_t;

struct {
    TMR_DATA_t _1ms;
    TMR_DATA_t _5ms;
    TMR_DATA_t _10ms;
    TMR_DATA_t _20ms;
    TMR_DATA_t _500ms;
} tmrData;

typedef enum {
    APP_EEPROM_READ = 0,
    APP_STATE_INIT,
    APP_STATE_SERVICE_TASKS,
    APP_STATE_PWROFF_TASKS,
    APP_STATE_SYSTEM_OFF,
} APP_STATUS_e;

struct {
    APP_STATUS_e state;
    unsigned     mainPWR : 1;
} appData;
/* ************************************************************************** */
/* ************************************************************************** */
/* Section: File Scope or Global Data                                         */
/* ************************************************************************** */
/* ************************************************************************** */
struct {
    unsigned int duty;
    unsigned int period;
} ledPWM;
// *****************************************************************************
// *****************************************************************************
// Section: Interrupt Handler
// *****************************************************************************
// *****************************************************************************

void TMR4_EvnetHandler(uint32_t status, uintptr_t context) {
    if (tmrData._1ms.cnt++ >= CALCULTAE_TIME_MS(1)) {
        tmrData._1ms.cnt  = 0;
        tmrData._1ms.flag = true;
    }
    if (tmrData._5ms.cnt++ >= CALCULTAE_TIME_MS(5)) {
        tmrData._5ms.cnt  = 0;
        tmrData._5ms.flag = true;
    }
    if (tmrData._10ms.cnt++ >= CALCULTAE_TIME_MS(10)) {
        tmrData._10ms.cnt  = 0;
        tmrData._10ms.flag = true;
    }
    if (tmrData._20ms.cnt++ >= CALCULTAE_TIME_MS(20)) {
        tmrData._20ms.cnt  = 0;
        tmrData._20ms.flag = true;
    }
    if (tmrData._500ms.cnt++ >= CALCULTAE_TIME_MS(500)) {
        tmrData._500ms.cnt  = 0;
        tmrData._500ms.flag = true;
    }
    if (ledPWM.period++ >= 100) {
        ledPWM.period = 0;
    }
    if (ledPWM.duty > ledPWM.period) {
        YLED_Set();
    } else {
        YLED_Clear();
    }
    X2CScope_Update();
}

void LVD_EvnetHandler(EXTERNAL_INT_PIN pin, uintptr_t context) {
    GLED_Toggle();
    EVIC_ExternalInterruptDisable(EXTERNAL_INT_2);
}
// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************

void CAN_XferTest() {
    can_msg_t canTxMsg;
    canTxMsg.id             = 0x18FF4520;
    canTxMsg.dlc            = 8;
    static unsigned char rc = 0;
    if (++rc > 0xff) {
        rc = 0;
    }
    for (unsigned i = 0; i < 8; i++) {
        canTxMsg.data[i] = 0;
    }
    canTxMsg.data[0] = rc;
    CAN_PushTxQueue(CAN_2, &canTxMsg);
}
void CAN_RecvTest() {
    can_msg_t canRxMsg;
    if (CAN_GetRxQueueCount(CAN_2)) {
        CAN_PullRxQueue(CAN_2, &canRxMsg);
        if (canRxMsg.id == 0x18FF4510) {
            ledPWM.duty = canRxMsg.data[0];
        }
    }
}

int main(void) {
    /* Initialize all modules */
    SYS_Initialize(NULL);
    while (true) {
        SYS_Tasks();
        X2CScope_Communicate();

        switch ((APP_STATUS_e)appData.state) {
            case APP_EEPROM_READ:
                eepBms        = eepBmsDef;
                eepSpe        = eepSpeDef;
                appData.state = APP_STATE_INIT;
                break;
            case APP_STATE_INIT:
                /* APP low voltage detect start */
                EVIC_ExternalInterruptCallbackRegister(EXTERNAL_INT_2, LVD_EvnetHandler, (uintptr_t)NULL);
                EVIC_ExternalInterruptEnable(EXTERNAL_INT_2);
                /* APP Timer Start */
                TMR4_CallbackRegister(TMR4_EvnetHandler, (uintptr_t)NULL);
                TMR4_Start();
                /* APP Initialize */
                HV_Initialize();
                BMU_Initialize();
                CAN_Initialize();
                MCP3421_Initialize();
                CoulombGauge_Initialize(&bmsData);
                CurrentSensor_Intialize();
                DIN_ParameterInitialize();

                appData.state = APP_STATE_SERVICE_TASKS;
                break;
            case APP_STATE_SERVICE_TASKS:
                if (tmrData._1ms.flag) {
                    tmrData._1ms.flag = false;
                    HV_1ms_Tasks();
                    // DTC_1ms_Tasks();

                    BMU_1ms_Tasks();
                    BMS_1ms_Tasks();
                    CAN_QueueDataXfer(CAN_1);
                }
                if (tmrData._5ms.flag) {
                    tmrData._5ms.flag = false;
                    DIN_5ms_Tasks();
                }
                if (tmrData._10ms.flag) {
                    tmrData._10ms.flag = false;
                    CurrentSensor_10ms_Tasks();
                    CAN_XferTest();
                    CAN_RecvTest();
                    CAN_QueueDataXfer(CAN_2);
                }
                if (tmrData._20ms.flag) {
                    tmrData._20ms.flag = false;
                    CoulombGauge_Tasks(&bmsData);
                }
                if (tmrData._500ms.flag) {
                    tmrData._500ms.flag = false;
                    RLED_Toggle();
                }
                break;
            case APP_STATE_PWROFF_TASKS:
                /* code */
                break;
            case APP_STATE_SYSTEM_OFF:
                /* code */
                break;
            default:
                break;
        }
    }

    /* Execution should not come here during normal operation */

    return (EXIT_FAILURE);
}
/*******************************************************************************
 End of File
 */
