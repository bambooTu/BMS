/**
 * @file       main.c
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @brief
 * @version    0.1
 * @date       2022-10-24
 *
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 *
 * Abbreviation:
 * None
 */
/* Global define -------------------------------------------------------------*/
/* USER CODE BEGIN GD */

/* USER CODE END GD */

/* Includes ------------------------------------------------------------------*/

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdbool.h>  // Defines true
#include <stddef.h>   // Defines NULL
#include <stdlib.h>   // Defines EXIT_FAILURE

#include "bms_ctrl.h"
#include "can.h"
#include "can_bms_vs_bmu.h"
#include "can_bms_vs_mbms.h"
#include "classb.h"
#include "coulomb_gauge.h"
#include "csnv500.h"
#include "current_sensor.h"
#include "debounce.h"
#include "definitions.h"  // SYS function prototypes
#include "indicator.h"
#include "mcp3421.h"
#include "sys_parameter.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
// Timer parameter
typedef struct {
    unsigned int cnt;
    bool         flag;
} TMR_DATA_t;

typedef enum {
    APP_STATE_EEPROM_READ = 0,
    APP_STATE_INIT,
    APP_STATE_SERVICE_TASKS,
    APP_STATE_PWROFF_TASKS,
    APP_STATE_SYSTEM_OFF,
} APP_STATUS_e;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define CALCULTAE_TIME_MS(A)            ((A < 1) ? 10 - 1 : 10 * A - 1)
// Classb
#define APP_FLASH_ADDRESS               (NVM_FLASH_START_ADDRESS + (NVM_FLASH_SIZE / 2))
#define FLASH_START_ADDR                (0x9d000000)  // Program Flash 0x9D000000 ~ 0x9D07FFFF(Virtual Memory Map)
#define FLASH_SIZE                      (0x80000)     // Prpgram Flash Size
#define CLASSB_FLASH_RUNTIME_TEST_SIZE  (4096U)       // Prpgram Flash Test Size
#define CLASSB_SRAM_RUNTIME_TEST_SIZE   (4096U)
#define FLASH_CRC32_ADDR                (0x9d0fffef)  // Program Flash Panel 2
#define CLASSB_CLOCK_TEST_TMR1_RATIO_NS (30517U)
#define CLASSB_CLOCK_TEST_RATIO_NS_MS   (1000000U)
#define CLASSB_CLOCK_DEFAULT_CLOCK_FREQ (120000000U)
#define CLASSB_CLOCK_ERROR_PERCENT      (5U)
// CAN Device address
#define CSNV500_CAN_ID                  0x3C2
#define TEST_GUI_CAN_ID                 0x18FF4510

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Global variables -----------------------------------------------------------*/
/* USER CODE BEGIN GV */
struct {
    TMR_DATA_t _1ms;
    TMR_DATA_t _5ms;
    TMR_DATA_t _10ms;
    TMR_DATA_t _20ms;
    TMR_DATA_t _100ms;
    TMR_DATA_t _500ms;
} tmrData;

struct {
    APP_STATUS_e   state;
    unsigned       mainPWR : 1;
    unsigned short bootTimeCount;
} appData;

unsigned int crc_val[1] = {0};

/* USER CODE END GV */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Function prototypes -------------------------------------------------------*/
/* USER CODE BEGIN FP */
void MAIN_CheckCanQueue(void);
void MAIN_XferCanQueue(void);
bool MAIN_CLASSB_RuntimeChecks(void);
/* USER CODE END FP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
    if (tmrData._100ms.cnt++ >= CALCULTAE_TIME_MS(100)) {
        tmrData._100ms.cnt  = 0;
        tmrData._100ms.flag = true;
    }
    if (tmrData._500ms.cnt++ >= CALCULTAE_TIME_MS(500)) {
        tmrData._500ms.cnt  = 0;
        tmrData._500ms.flag = true;
    }

    if (appData.bootTimeCount) {
        appData.bootTimeCount--;
    }
    X2CScope_Update();
}

void LVD_EvnetHandler(EXTERNAL_INT_PIN pin, uintptr_t context) {
    GLED_Toggle();
    // EVIC_ExternalInterruptDisable(EXTERNAL_INT_2);
}

int main(void) {
    WDT_Clear();           // Clear watch dog
    SYS_Initialize(NULL);  // Initialize all modules
    crc_val[0] = CLASSB_FlashCRCGenerate(FLASH_START_ADDR, CLASSB_FLASH_RUNTIME_TEST_SIZE);

    while (true) {
        SYS_Tasks();
        X2CScope_Communicate();
        WDT_Clear();  // Clear watch dog
        switch ((APP_STATUS_e)appData.state) {
            case APP_STATE_EEPROM_READ:
                eepBms                = eepBmsDef;
                eepSpe                = eepSpeDef;
                appData.bootTimeCount = CALCULTAE_TIME_MS(100);
                appData.state         = APP_STATE_INIT;
                break;
            case APP_STATE_INIT:

                // APP low voltage detection start
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
                CURRSNSR_Intialize();
                CG_Initialize(&bmsData);
                DIN_Initialize();
                DTC_Initialize();
                // Delay to APP_STATE_SERVICE_TASKS
                if (!appData.bootTimeCount) {
                    appData.state = APP_STATE_SERVICE_TASKS;
                }
                break;
            case APP_STATE_SERVICE_TASKS:
                MAIN_XferCanQueue();
                MAIN_CheckCanQueue();  // All CAN-bus put receive data to the queue

                if (tmrData._1ms.flag) {
                    tmrData._1ms.flag = false;
                    HV_1ms_Tasks();        // HV relay control
                    IND_1ms_Tasks();       // Indicator control
                    BMU_1ms_Tasks();       // Bmu communcation and data processing
                    DTC_1ms_Tasks();       // Error code processing
                    MBMS_1ms_tasks();      // MBMS communication
                    BMS_Crtl_1ms_Tasks();  // Bms system flow control
                }

                if (tmrData._5ms.flag) {
                    tmrData._5ms.flag = false;
                    DIN_5ms_Tasks();  // Digital input signal processing
                }

                if (tmrData._10ms.flag) {
                    tmrData._10ms.flag = false;
                    CSNV500_10ms_tasks();   // CSNV500 timeout count
                    CURRSNSR_10ms_Tasks();  // Current sensor communication and data processing
                }

                if (tmrData._20ms.flag) {
                    tmrData._20ms.flag = false;
                    CG_20ms_Tasks(&bmsData);  // Coulomb gauge data process
                }

                if (tmrData._100ms.flag) {
                    tmrData._100ms.flag = false;
                    MAIN_CLASSB_RuntimeChecks();  // Runtime classb check
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

/**
 * @brief
 *
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-10-28
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
void MAIN_XferCanQueue(void) {
    CAN_QueueDataXfer(CAN_1);  // CAN-bus 1 sent queue data
    CAN_QueueDataXfer(CAN_2);  // CAN-bus 2 sent queue data
    CAN_QueueDataXfer(CAN_3);  // CAN-bus 3 sent queue data
    CAN_QueueDataXfer(CAN_4);  // CAN-bus 4 sent queue data
}
/**
 * @brief
 *
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-10-28
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
void MAIN_CheckCanQueue(void) {
    CAN_MSG_t canRxMsg;
    if (CAN_GetRxQueueCount(CAN_1)) {
        CAN_PullRxQueue(CAN_1, &canRxMsg);
        MBMS_CheckQueueTasks(&canRxMsg);
    }
    if (CAN_GetRxQueueCount(CAN_2)) {
        CAN_PullRxQueue(CAN_2, &canRxMsg);
    }
    if (CAN_GetRxQueueCount(CAN_3)) {
        CAN_PullRxQueue(CAN_3, &canRxMsg);
    }
    if (CAN_GetRxQueueCount(CAN_4)) {
        CAN_PullRxQueue(CAN_4, &canRxMsg);
        if (canRxMsg.id == CSNV500_CAN_ID) {
            CSNV500_CheckQueueTasks(canRxMsg);
        } else {
            BMU_CheckQueueTasks(canRxMsg);
        }
    }
}
/**
 * @brief
 *
 * @return     true
 * @return     false
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-10-28
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
bool MAIN_CLASSB_RuntimeChecks(void) {
    bool                 ret_val                = false;
    static unsigned char runtimeTestState       = CLASSB_TEST_CPU;
    uint16_t             clock_test_tmr1_cycles = 0;
    CLASSB_TEST_STATUS   classb_rst_status      = CLASSB_TEST_NOT_EXECUTED;

    switch (runtimeTestState) {
        case CLASSB_TEST_CPU:
            __builtin_disable_interrupts();
            classb_rst_status = CLASSB_CPU_RegistersTest(true);
            __builtin_enable_interrupts();

            runtimeTestState = CLASSB_TEST_FLASH;
            break;
        case CLASSB_TEST_FLASH:
            classb_rst_status = CLASSB_FlashCRCTest(FLASH_START_ADDR, CLASSB_FLASH_RUNTIME_TEST_SIZE, crc_val[0], true);

            runtimeTestState = CLASSB_TEST_RAM;
            break;
        case CLASSB_TEST_RAM:
            __builtin_disable_interrupts();
            classb_rst_status = CLASSB_SRAM_MarchTestInit((uint32_t *)CLASSB_SRAM_APP_AREA_START,
                                                          CLASSB_SRAM_RUNTIME_TEST_SIZE, CLASSB_SRAM_MARCH_C, true);
            __builtin_enable_interrupts();

            runtimeTestState = CLASSB_TEST_CLOCK;
            break;
        case CLASSB_TEST_CLOCK:
            clock_test_tmr1_cycles = ((5 * CLASSB_CLOCK_TEST_RATIO_NS_MS) / CLASSB_CLOCK_TEST_TMR1_RATIO_NS);
            classb_rst_status      = CLASSB_ClockTest(CLASSB_CLOCK_DEFAULT_CLOCK_FREQ, CLASSB_CLOCK_ERROR_PERCENT,
                                                      clock_test_tmr1_cycles, true);

            runtimeTestState = CLASSB_TEST_CPU;
            break;
        default:
            runtimeTestState = CLASSB_TEST_CPU;
            break;
    }

    if (classb_rst_status == CLASSB_TEST_PASSED) {
        ret_val = true;
    }
    return ret_val;
}
/* USER CODE END 0 */

/*******************************************************************************
 End of File
 */
