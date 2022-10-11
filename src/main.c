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
#include "classb.h"
#include "coulomb_gauge.h"
#include "current_sensor.h"
#include "debounce.h"
#include "definitions.h"  // SYS function prototypes
#include "indicator.h"
#include "mcp3421.h"
#include "sys_parameter.h"

#define CALCULTAE_TIME_MS(A) ((A < 1) ? 10 - 1 : 10 * A - 1)

// Classb
#define APP_FLASH_ADDRESS               (NVM_FLASH_START_ADDRESS + (NVM_FLASH_SIZE / 2))
#define FLASH_START_ADDR                (0x9d000000)  // Program Flash 0x9D000000 ~ 0x9D07FFFF(Virtual Memory Map)
#define FLASH_SIZE                      (0x80000)     // Prpgram Flash Size
#define CLASSB_FLASH_TEST_BUFFER_SIZE   (4096U)       // Prpgram Flash Test Size
#define FLASH_CRC32_ADDR                (APP_FLASH_ADDRESS)  // Program Flash Panel 2
#define SRAM_RST_SIZE                   (32768U)
#define CLASSB_CLOCK_TEST_TMR1_RATIO_NS (30517U)
#define CLASSB_CLOCK_TEST_RATIO_NS_MS   (1000000U)
#define CLASSB_CLOCK_DEFAULT_CLOCK_FREQ (200000000U)

// CAN Device address
#define CSNVC500_CAN_ID 0x3C2
#define TEST_GUI_CAN_ID 0x18FF4510
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
    APP_STATUS_e   state;
    unsigned       mainPWR : 1;
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

unsigned int  crc_val[1]   = {0};

bool runtimeClassBChecks(void);
// *****************************************************************************
// *****************************************************************************
// Section: Interrupt Handler
// *****************************************************************************
// *****************************************************************************

void TMR4_EvnetHandler(uint32_t status, uintptr_t context) {  // 0.1ms
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
    CAN_PushTxQueue(CAN_1, &canTxMsg);
}
// TODO : Delete ↑
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
int main(void) {
    /* Initialize all modules */
    WDT_Clear();
    SYS_Initialize(NULL);
    crc_val[0] = CLASSB_FlashCRCGenerate(FLASH_START_ADDR, CLASSB_FLASH_TEST_BUFFER_SIZE);
    // Use NVMCTRL to write the calculated CRC into a Flash location
    while (NVM_IsBusy() == true) {
        ;
    }
    NVM_RowWrite(crc_val, FLASH_CRC32_ADDR);

    while (true) {
        SYS_Tasks();
        X2CScope_Communicate();
        WDT_Clear();
        switch ((APP_STATUS_e)appData.state) {
            case APP_EEPROM_READ:
                eepBms                = eepBmsDef;
                eepSpe                = eepSpeDef;
                appData.bootTimeCount = CALCULTAE_TIME_MS(100);
                appData.state         = APP_STATE_INIT;
                break;
            case APP_STATE_INIT:
                // APP low voltage detect start
                EVIC_ExternalInterruptCallbackRegister(EXTERNAL_INT_2, LVD_EvnetHandler, (uintptr_t)NULL);
                EVIC_ExternalInterruptEnable(EXTERNAL_INT_2);

                // APP Timer Start
                TMR4_CallbackRegister(TMR4_EvnetHandler, (uintptr_t)NULL);
                TMR4_Start();

                // APP Initialize
                HV_Initialize();
                BMU_Initialize();
                CAN_Initialize();
                IND_Initialize();
                MCP3421_Initialize();
                CURRSNSR_Intialize();
                CG_Initialize(&bmsData);
                DIN_ParameterInitialize();

                // Delay to APP_STATE_SERVICE_TASKS
                if (!appData.bootTimeCount) {
                    appData.state = APP_STATE_SERVICE_TASKS;
                }
                break;
            case APP_STATE_SERVICE_TASKS:
                CAN_QueueDataXfer(CAN_1);
                CAN_QueueDataXfer(CAN_4);
                CAN_QueueDateRecv();
                if (tmrData._1ms.flag) {
                    tmrData._1ms.flag = false;
                    HV_1ms_Tasks();   // HV relay control
                    IND_1ms_Tasks();  // Indicator control
                    BMU_1ms_Tasks();  // Bmu communcation and data processing
                    // DTC_1ms_Tasks(); // Error code process
                    MBMS_1ms_tasks();      // MBMS communication
                    BMS_Crtl_1ms_Tasks();  // Bms system flow control
                }
                if (tmrData._5ms.flag) {
                    tmrData._5ms.flag = false;
                    DIN_5ms_Tasks();  // Digital input process
                }
                if (tmrData._10ms.flag) {
                    tmrData._10ms.flag = false;
                    CURRSNSR_10ms_Tasks();  // Current sensor communication and data processing
                    // TODO : Delete ↓
                    CAN_XferTest();
                }
                if (tmrData._20ms.flag) {
                    tmrData._20ms.flag = false;
                    CG_20ms_Tasks(&bmsData);  // Coulomb gauge data process
                }
                if (tmrData._500ms.flag) {
                    tmrData._500ms.flag = false;
                    runtimeClassBChecks();
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

bool runtimeClassBChecks(void) {
    bool               ret_val            = false;
    CLASSB_TEST_STATUS classb_rst1_status = CLASSB_TEST_NOT_EXECUTED;
    CLASSB_TEST_STATUS classb_rst2_status = CLASSB_TEST_NOT_EXECUTED;
    
    __builtin_disable_interrupts();
    classb_rst1_status = CLASSB_CPU_RegistersTest(true);
    __builtin_enable_interrupts();
    classb_rst2_status =
        CLASSB_FlashCRCTest(FLASH_START_ADDR, CLASSB_FLASH_TEST_BUFFER_SIZE, *(uint32_t *)FLASH_CRC32_ADDR, true);

    if ((classb_rst1_status == CLASSB_TEST_PASSED) && (classb_rst2_status == CLASSB_TEST_PASSED)) {
        ret_val = true;
    }

    return ret_val;
}
/*******************************************************************************
 End of File
 */
