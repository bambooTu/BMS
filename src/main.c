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

#include "bms_ctrl.h"
#include "can.h"
#include "can_bms_vs_bmu.h"
#include "can_bms_vs_mbms.h"
#include "coulomb_gauge.h"
#include "current_sensor.h"
#include "debounce.h"
#include "definitions.h"  // SYS function prototypes
#include "indicator.h"
#include "mcp3421.h"
#include "sys_parameter.h"

#define CALCULTAE_TIME_MS(A) ((A < 1) ? 10 - 1 : 10 * A - 1)

#define CSNVC500_CAN_ID 0x3C2
#define TEST_GUI_CAN_ID 0x18FF4510
// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
// *****************************************************************************

typedef struct {
    unsigned int cnt;
    bool flag;
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
    unsigned mainPWR : 1;
    unsigned short bootTimeCount;
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

void TMR4_EvnetHandler(uint32_t status, uintptr_t context) { // 0.1ms
    if (tmrData._1ms.cnt++ >= CALCULTAE_TIME_MS(1)) {
        tmrData._1ms.cnt = 0;
        tmrData._1ms.flag = true;
    }
    if (tmrData._5ms.cnt++ >= CALCULTAE_TIME_MS(5)) {
        tmrData._5ms.cnt = 0;
        tmrData._5ms.flag = true;
    }
    if (tmrData._10ms.cnt++ >= CALCULTAE_TIME_MS(10)) {
        tmrData._10ms.cnt = 0;
        tmrData._10ms.flag = true;
    }
    if (tmrData._20ms.cnt++ >= CALCULTAE_TIME_MS(20)) {
        tmrData._20ms.cnt = 0;
        tmrData._20ms.flag = true;
    }
    if (tmrData._500ms.cnt++ >= CALCULTAE_TIME_MS(500)) {
        tmrData._500ms.cnt = 0;
        tmrData._500ms.flag = true;
    }
    if (ledPWM.period++ >= 100) {
        ledPWM.period = 0;
    }
    if (appData.bootTimeCount) {
        appData.bootTimeCount--;
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
    // EVIC_ExternalInterruptDisable(EXTERNAL_INT_2);
}
// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************
// TODO : Delete ↓

void CAN_XferTest() {
    CAN_MSG_t canTxMsg;
    canTxMsg.id = 0x18FF4520;
    canTxMsg.dlc = 8;
    static unsigned char rc = 0;
    if (++rc > 0xff) {
        rc = 0;
    }
    for (unsigned i = 0; i < 8; i++) {
        canTxMsg.data[i] = 0;
    }
    canTxMsg.data[0] = rc;
    CAN_PushTxQueue(CAN_1, &canTxMsg);
}

static void CAN_QueueDateRecv(void) {
    CAN_MSG_t canRxMsg;
    if (CAN_GetRxQueueCount(CAN_1)) {
        CAN_PullRxQueue(CAN_1, &canRxMsg);
        MBMS_CheckQueueTasks(&canRxMsg);
    }
    if (CAN_GetRxQueueCount(CAN_2)) {
        CAN_PullRxQueue(CAN_1, &canRxMsg);
    }
    if (CAN_GetRxQueueCount(CAN_3)) {
        CAN_PullRxQueue(CAN_1, &canRxMsg);
    }
    if (CAN_GetRxQueueCount(CAN_4)) {
        CAN_PullRxQueue(CAN_4, &canRxMsg);
        if (canRxMsg.id == CSNVC500_CAN_ID) {
            CURRSNSR_CheckQueueTasks(canRxMsg);
        } else if (canRxMsg.id == TEST_GUI_CAN_ID) {
            ledPWM.duty = canRxMsg.data[0];
        } else {
            BMU_CheckQueueTasks(canRxMsg);
        }
    }
}
// TODO : Delete ↑

int main(void) {
    /* Initialize all modules */
    WDT_Clear();
    SYS_Initialize(NULL);
    while (true) {
        SYS_Tasks();
        X2CScope_Communicate();
        WDT_Clear();
        switch ((APP_STATUS_e) appData.state) {
            case APP_EEPROM_READ:
                eepBms = eepBmsDef;
                eepSpe = eepSpeDef;
                appData.bootTimeCount = CALCULTAE_TIME_MS(100);
                appData.state = APP_STATE_INIT;
                break;
            case APP_STATE_INIT:
                /* APP low voltage detect start */
                EVIC_ExternalInterruptCallbackRegister(EXTERNAL_INT_2, LVD_EvnetHandler, (uintptr_t) NULL);
                EVIC_ExternalInterruptEnable(EXTERNAL_INT_2);
                /* APP Timer Start */
                TMR4_CallbackRegister(TMR4_EvnetHandler, (uintptr_t) NULL);
                TMR4_Start();
                /* APP Initialize */
                HV_Initialize();
                BMU_Initialize();
                CAN_Initialize();
                IND_Initialize();
                MCP3421_Initialize();
                CURRSNSR_Intialize();
                CG_Initialize(&bmsData);
                DIN_ParameterInitialize();
                if (!appData.bootTimeCount) {
                    appData.state = APP_STATE_SERVICE_TASKS;
                }
                break;
            case APP_STATE_SERVICE_TASKS:

                CAN_QueueDateRecv();
                if (tmrData._1ms.flag) {
                    tmrData._1ms.flag = false;
                    HV_1ms_Tasks();
                    IND_1ms_Tasks();
                    BMU_1ms_Tasks();
                    // DTC_1ms_Tasks();
                    MBMS_1ms_tasks();
                    BMS_Crtl_1ms_Tasks();
                }
                if (tmrData._5ms.flag) {
                    tmrData._5ms.flag = false;
                    DIN_5ms_Tasks();
                }
                if (tmrData._10ms.flag) {
                    tmrData._10ms.flag = false;
                    CURRSNSR_10ms_Tasks();
                    // TODO : Delete ↓
                    CAN_XferTest();
                }
                if (tmrData._20ms.flag) {
                    tmrData._20ms.flag = false;
                    CG_20ms_Tasks(&bmsData);
                }
                if (tmrData._500ms.flag) {
                    tmrData._500ms.flag = false;
                    RLED_Toggle();
                }
                CAN_QueueDataXfer(CAN_1);
                CAN_QueueDataXfer(CAN_4);
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
