/**
 * @file       can_bms2bmu.c
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @brief
 * @version    0.1
 * @date       2022-08-29
 *
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 *
 * Abbreviation:
 * BMS    Battery Manager Systeum
 *
 * BR     Branch                      _____________________[Branch]_______________________
 *                                   |                                                    |
 * BMU    Battery Monitor Unit       ----[BMU]----[BMU]----[BMU]----[BMU]----[BMU]-- * N --
 *                                         |        |        |        |        |
 * Vcell  Cell Voltage                  [Cell*N] [Cell*N] [Cell*N] [Cell*N] [Cell*N]
 * Tcell  Cell Temperature
 */

/* Global define -------------------------------------------------------------*/
/* USER CODE BEGIN GD */

/* USER CODE END GD */

/* Includes ------------------------------------------------------------------*/

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "can_bms_vs_bmu.h"

#include "commonly_used.h"
#include "sys_parameter.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/

/* USER CODE BEGIN PTD */

typedef struct {
    /*BMU*/
    unsigned char BmuConnected[(BMU_MAX_NUM >> 3) + 1];     /* BMU comm status 1: Connected 0: Disconnected*/
    unsigned char BmuTimeout[(BMU_MAX_NUM >> 3) + 1];       /* BMU comm status，1: Timeout 0: Normal */
    int           BmuVcell[BMU_MAX_NUM][BMU_VCELL_MAX_NUM]; /* Unit:mV,Volt. of all cell in the BMU */
    int           BmuVoltage[BMU_MAX_NUM];                  /* Unit:mV,Sum of Vcell voltage */
    int           BmuMaxVcell[BMU_MAX_NUM];                 /* Max. Volt. in the BMU */
    int           BmuMinVcell[BMU_MAX_NUM];                 /* Max. Volt. in the BMU */
    unsigned char BmuMaxVcellID[BMU_MAX_NUM];               /* ID of the Max. Vcell in the BMU */
    unsigned char BmuMinVcellID[BMU_MAX_NUM];               /* ID of the Min. Vcell in the BMU */
    int           BmuTcell[BMU_MAX_NUM][BMU_TCELL_MAX_NUM]; /* Unit:0.1 Deg. C,Temp. of all cell in the BMU */
    int           BmuMaxTcell[BMU_MAX_NUM];                 /* Max. Temp. in the BMU */
    int           BmuMinTcell[BMU_MAX_NUM];                 /* Max. Temp. in the BMU */
    unsigned char BmuMaxTcellID[BMU_MAX_NUM];               /* ID of the Max. Tcell in the BMU */
    unsigned char BmuMinTcellID[BMU_MAX_NUM];               /* ID of the Min. Tcell in the BMU */
    unsigned char BmuMsgFlag[BMU_MAX_NUM];                  /* unit: bitmap,The number of packets is recieved  */
    unsigned char BmuFaultLed[BMU_MAX_NUM];                 /* Fault indicator number of the BMU */

    /*Branch*/
    int Voltage; /* Unit:mV, Branch Voltage = Sum of Bmu Voltage */

    int           MaxVcell;      /* Max. Volt. in the branch */
    unsigned char MaxVcellBmuID; /* BMU ID of the Max. Vcell in the branch */
    unsigned char MaxVcellID;    /* ID of the Max. Vcell in the branch */

    int           MinVcell;      /* Min. Volt. in the branch */
    unsigned char MinVcellBmuID; /* BMU ID of the Min. Vcell in the branch */
    unsigned char MinVcellID;    /* ID of the Min. Vcell in the branch   */

    int           MaxTcell;      /* Max. Temp. in the branch  */
    unsigned char MaxTcellBmuID; /* BMU ID of the Max. Tcell in the branch */
    unsigned char MaxTcellID;    /* ID of the Max. Tcell in the branch   */

    int           MinTcell;      /* Min. Temp. in the branch */
    unsigned char MinTcellBmuID; /* BMU ID of the Min. Tcell in the branch */
    unsigned char MinTcellID;    /* ID of the Min. Tcell in the branch */

} BRANCH_INFO_t;

typedef enum {
    BMS_BMU_REQ_CMD,
    BMS_BMU_CHK_RSP,
    BMS_BMU_SRCH_DATA,
    BMS_BMU_STAT_MAX
} BMS_BMU_TASK_STATE_e;

typedef enum {
    PNG_TEMP_GET_G1 = 0xB9,
    PNG_VOLT_GET_G1 = 0xBB,
    PNG_VOLT_GET_G2 = 0xBA,
} AFE_PNG_e;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Globalvariables -----------------------------------------------------------*/
/* USER CODE BEGIN GV */
BMS_BMU_TASK_STATE_e BMU_taskState            = BMS_BMU_REQ_CMD;
unsigned int         BMU_ID                   = 0;
unsigned int         BMU_retryCount           = 0;
unsigned int         BMU_tasksTimeCount       = 0;
unsigned int         gBmuFaultLed1msDEC       = 0;
unsigned int         BMU_responseTimeoutCount = 0;
unsigned char        BMU_rxMessageFlag        = 0;
unsigned char        BMS_SourceAddrBackup     = 0;

unsigned int testBusVolt    = 0;
unsigned int testBusCurrent = 0;
unsigned int testMaxTcell   = 0;
unsigned int testMinTcell   = 0;
unsigned int testMaxVcell   = 0;
unsigned int testMinVcell   = 0;
/* USER CODE END GV */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Function prototypes -------------------------------------------------------*/
/* USER CODE BEGIN FP */

/* USER CODE END FP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
BRANCH_INFO_t BR = {0};

static void BMU_ResponseCheck(unsigned int BMU_ID, CAN_MSG_t canRxMsg) {
    if ((BMU_ID + 1) == canRxMsg.J1939.sourceAddress) {
        switch (canRxMsg.J1939.pduFormat) {
            case PNG_TEMP_GET_G1:
                BMU_responseTimeoutCount = (BMU_RESPONSE_TIMEOUT_TIME / 2);
                BMU_rxMessageFlag |= (1 << 0);
                for (unsigned char i = 0; i < BMU_TCELL_MAX_NUM; i++) {
                    BR.BmuTcell[BMU_ID][i] = (canRxMsg.data[(i * 2 + 1)] << 8) + (canRxMsg.data[i]);
                }
                break;
            case PNG_VOLT_GET_G1:
                BMU_responseTimeoutCount = (BMU_RESPONSE_TIMEOUT_TIME / 2);
                BMU_rxMessageFlag |= (1 << 1);
                for (unsigned char i = 0; i < 4; i++) {
                    BR.BmuVcell[BMU_ID][i] = (canRxMsg.data[(i * 2 + 1)] << 8) + (canRxMsg.data[i]);
                }
                break;
            case PNG_VOLT_GET_G2:
                BMU_responseTimeoutCount = (BMU_RESPONSE_TIMEOUT_TIME / 2);
                BMU_rxMessageFlag |= (1 << 2);
                for (unsigned char i = 0; i < (BMU_TCELL_MAX_NUM - 4); i++) {
                    BR.BmuVcell[BMU_ID][i + 4] = (canRxMsg.data[(i * 2 + 1)] << 8) + (canRxMsg.data[i]);
                }
                break;
            default:
                break;
        }
    }
}

bool BMU_XtrmVcellSearch(unsigned int BMU_ID) {
    /* Search BMU Max. Min. Voltage Value */
    bool          ret             = false;
    unsigned char fCommOkAFE      = 0;
    BR.BmuMaxVcell[BMU_ID]        = BR.BmuVcell[BMU_ID][0];
    BR.BmuMinVcell[BMU_ID]        = BR.BmuVcell[BMU_ID][0];
    unsigned short pastBmuVoltage = BR.BmuVoltage[BMU_ID];
    BR.BmuVoltage[BMU_ID]         = 0;

    for (unsigned char i = 0; i < BMU_VCELL_MAX_NUM; i++) {
        if (BR.BmuVcell[BMU_ID][i] == BMU_AFE_COMM_ERR) {
            BR.BmuVoltage[BMU_ID] = pastBmuVoltage;
            break;
        } else if (BR.BmuVcell[BMU_ID][i] > BR.BmuMaxVcell[BMU_ID]) {
            BR.BmuMaxVcell[BMU_ID]   = BR.BmuVcell[BMU_ID][i];
            BR.BmuMaxVcellID[BMU_ID] = i;
        } else if (BR.BmuVcell[BMU_ID][i] < BR.BmuMinVcell[BMU_ID]) {
            BR.BmuMinVcell[BMU_ID]   = BR.BmuVcell[BMU_ID][i];
            BR.BmuMinVcellID[BMU_ID] = i;
        }
        BR.BmuVoltage[BMU_ID] += BR.BmuVcell[BMU_ID][i];
        fCommOkAFE = i;
    }

    if (fCommOkAFE == BMU_VCELL_MAX_NUM) {
        ret = true;
    }
    return ret;
}

static void BMU_XtrmTcellSearch(unsigned int BMU_ID) {
    /* Search BMU Max. Min. Temperature Value */
    BR.BmuMaxTcell[BMU_ID] = BR.BmuTcell[BMU_ID][0];
    BR.BmuMinTcell[BMU_ID] = BR.BmuTcell[BMU_ID][0];
    for (unsigned char i = 0; i < BMU_TCELL_MAX_NUM; i++) {
        if (BR.BmuTcell[BMU_ID][i] > BR.BmuMaxTcell[BMU_ID]) {
            BR.BmuMaxTcell[BMU_ID]   = BR.BmuTcell[BMU_ID][i];
            BR.BmuMaxTcellID[BMU_ID] = i;
        } else if (BR.BmuTcell[BMU_ID][i] < BR.BmuMinTcell[BMU_ID]) {
            BR.BmuMinTcell[BMU_ID]   = BR.BmuTcell[BMU_ID][i];
            BR.BmuMinTcellID[BMU_ID] = i;
        }
    }
}

static void BMU_XferMsgInit(CAN_MSG_t *canTxMsg) {
    canTxMsg->J1939.priority      = 6; /* 0~7(3bits), 6:0x18, 7:0x1C */
    canTxMsg->J1939.reserved      = 1; /* EDP?X?ibit */
    canTxMsg->J1939.dataPage      = 0;
    canTxMsg->J1939.pduFormat     = 0x00;
    canTxMsg->J1939.pduSpecific   = 0x00; /* DestinationAddress */
    canTxMsg->J1939.sourceAddress = BMS_SourceAddrBackup;
    canTxMsg->dlc                 = 8;

    for (unsigned i = 0; i < 8; i++) {
        *(canTxMsg->data + i) = 0xAA;
    }
}

static void BMU_RequestDataMsg(unsigned int BMU_ID) {
    CAN_MSG_t canTxMsg;
    BMU_XferMsgInit(&canTxMsg);
    canTxMsg.J1939.pduFormat   = 0xEA;
    canTxMsg.J1939.pduSpecific = BMU_ID;

    canTxMsg.data[0] = 0x00;
    canTxMsg.data[1] = 0xBB;
    canTxMsg.data[2] = 0x00;
    CAN_PushTxQueue(CAN_4, &canTxMsg);
}

static void BMU_BalanceParamMsg(unsigned int BMU_ID) {
    CAN_MSG_t canTxMsg;
    BMU_XferMsgInit(&canTxMsg);
    canTxMsg.J1939.pduFormat   = 0xBC;
    canTxMsg.J1939.pduSpecific = BMU_ID;

    canTxMsg.data[0] = (unsigned char)(eepBms.BalanceDef.Mode);
    canTxMsg.data[1] = (unsigned char)(eepBms.BalanceDef.VoltDiff);
    canTxMsg.data[2] = (unsigned char)(eepBms.BalanceDef.Volt & 0xFF);
    canTxMsg.data[3] = (unsigned char)((eepBms.BalanceDef.Volt & 0xFF00) >> 8);
    canTxMsg.data[4] = (unsigned char)(bmsData.MinVcell & 0xFF);
    canTxMsg.data[5] = (unsigned char)((bmsData.MinVcell & 0xFF00) >> 8);
    canTxMsg.data[6] = (unsigned char)(bmsData.SysStatus);
    canTxMsg.data[7] = BR.BmuFaultLed[(BMU_ID)];
    CAN_PushTxQueue(CAN_4, &canTxMsg);
}

static void BMU_ResponseTimeout(unsigned char BMU_ID) {
    BR.BmuTimeout[(BMU_ID >> 3)] |= (1 << (BMU_ID & 0x07));
    BR.BmuMsgFlag[BMU_ID] = BMU_rxMessageFlag;

    for (unsigned char i = 0; i < BMU_VCELL_MAX_NUM; i++) {
        BR.BmuVcell[BMU_ID][i] = BMU_CAN_COMM_ERR;
    }
    BR.BmuVcell[BMU_ID][(BMU_VCELL_MAX_NUM - 1)] = BMU_rxMessageFlag;
}

static void BMU_ResponseComplete(unsigned char BMU_ID) {
    BR.BmuConnected[(BMU_ID >> 3)] |= (1 << (BMU_ID & 0x07));
    BR.BmuTimeout[(BMU_ID >> 3)] &= ~(1 << (BMU_ID & 0x07));

    BR.BmuMsgFlag[BMU_ID] = BMU_rxMessageFlag;
}

static inline void BRANCH_XtrmVoltSearch(void) {
    unsigned char Bit;
    unsigned int  BmuVoltSum = 0;

    /* Search  Max. Min. voltage value in Branch */

    BR.MaxVcell      = BR.BmuMaxVcell[0];
    BR.MaxVcellBmuID = 0;

    BR.MinVcell      = BR.BmuMinVcell[0];
    BR.MinVcellBmuID = 0;

    for (unsigned char i = 0; i < BMU_MAX_NUM; i++) {
        Bit = 1 << (i & 0x07);
        if ((BR.BmuTimeout[(i >> 3)] & Bit) == 0) {
            if (BR.BmuMaxVcell[i] > BR.MaxVcell) {
                BR.MaxVcell      = BR.BmuMaxVcell[i];
                BR.MaxVcellBmuID = i;
            }
            if (BR.BmuMinVcell[i] < BR.MinVcell) {
                BR.MinVcell      = BR.BmuMinVcell[i];
                BR.MinVcellBmuID = i;
            }
            /* BMU fault LED indicator */
            // if (is_fault_event(DTC_CELL_OVP) == true) {
            //     if (BR.BmuMaxVcell[i] > eepBMS.CellOVP.Limit) {
            //         BR.BmuFaultLed[i] = 1; /* LED???G?A?q?T??? */
            //         gBmuFaultLed1msDEC = BMU_FAULT_LED_DELAY;
            //     }
            // }
            // if (is_fault_event(DTC_CELL_UVP) == true) {
            //     if (BR.BmuMinVcell[i] < eepBMS.CellUVP.Limit) {
            //         BR.BmuFaultLed[i] = 1;
            //         gBmuFaultLed1msDEC = BMU_FAULT_LED_DELAY;
            //     }
            // }

            BmuVoltSum += BR.BmuVoltage[i];  // Calculate Total Voltage
        }
    }
    BR.Voltage = BmuVoltSum;
}

static inline void BRANCH_XtrmTempSearch(void) {
    unsigned char Bit;
    unsigned char fErrNTC = 0;
    /* Search  Max. Min. voltage value in Branch*/
    BR.MaxTcell      = BR.BmuMaxTcell[0];
    BR.MaxTcellBmuID = 0;

    BR.MinTcell      = BR.BmuMinTcell[0];
    BR.MinTcellBmuID = 0;

    for (unsigned char i = 0; i < BMU_MAX_NUM; i++) {
        Bit = 1 << (i & 0x07);
        if ((BR.BmuTimeout[(i >> 3)] & Bit) == 0) {
            if (BR.BmuMaxTcell[i] == BMU_NTC_SHOTR) {
                // TODO:  set_fault_occur(DTC_NTC_SHORT);
                fErrNTC++;
            } else if (BR.BmuMinTcell[i] == BMU_NTC_OPEN) {
                // TODO:  set_fault_occur(DTC_NTC_OPEN);
                fErrNTC++;
            } else {
                if (BR.BmuMaxTcell[i] > BR.MaxTcell) {
                    BR.MaxTcell      = BR.BmuMaxTcell[i];
                    BR.MaxTcellBmuID = i;
                } else if (BR.BmuMinTcell[i] < BR.MinTcell) {
                    BR.MinTcell      = BR.BmuMinTcell[i];
                    BR.MinTcellBmuID = i;
                }
            }

            /* BMU fault LED indicator */
            if (DTC_FaultEventGet(DTC_OTP) == true) {
                if (BR.BmuMaxTcell[i] > eepBms.OTP.Limit) {
                    BR.BmuFaultLed[i]  = 1;
                    gBmuFaultLed1msDEC = BMU_FAULT_LED_DELAY;
                }
            }
            if (DTC_FaultEventGet(DTC_UTP) == true) {
                if (BR.BmuMinTcell[i] < eepBms.UTP.Limit) {
                    BR.BmuFaultLed[i]  = 1;
                    gBmuFaultLed1msDEC = BMU_FAULT_LED_DELAY;
                }
            }
        }
    }
    if (fErrNTC == 0) {
        // TODO: clr_fault_occur(DTC_NTC_OPEN);
        // TODO: clr_fault_occur(DTC_NTC_SHORT);
    }
}

static void BRANCH_XtrmTcellIDSearch(void) {
    BR.MinTcellID = BR.BmuMinTcellID[BR.MinTcellBmuID];
    BR.MaxTcellID = BR.BmuMaxTcellID[BR.MaxTcellBmuID];
}

static void BRANCH_XtrmVcellIDSearch(void) {
    BR.MinVcellID = BR.BmuMinVcellID[BR.MinVcellBmuID];
    BR.MaxVcellID = BR.BmuMaxVcellID[BR.MaxVcellBmuID];
}

static void BMU_CommStatusCheck(void) {
    static unsigned char ErrCnt = 0;
    unsigned char        Err    = 0;

    for (unsigned i = 0; i < BMU_MAX_NUM; i++) {
        if (BR.BmuMsgFlag[i] != BMU_RECV_MSG_FLAG) {
            Err++;
        }
        BR.BmuMsgFlag[i] = 0;
    }
    if (0 == Err) {
        ErrCnt = 0;
        // TODO: clr_fault_event(DTC_BMU_COMM);
    } else {
        if (ErrCnt++ > 2) {
            // TODO: set_fault_occur(DTC_BMU_COMM);
        }
    }
}

/**
 * @brief      Get the cell temperature value at the specified ID
 *
 * @param      BMU_ID Start ID is 1 , End ID is BMU_MAX_NUM
 * @param      Cell_ID Start ID is 0 , End ID is BMU_TCELL_MAX_NUM
 * @return     unsigned short
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-08-29
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
unsigned short BMU_CellTempGet(unsigned char BMU_ID, unsigned char Cell_ID) {
    unsigned short ret;
    if ((BMU_ID < (BMU_MAX_NUM + BMU_ID_OFFSET)) && (BMU_ID >= BMU_ID_OFFSET) && (BMU_ID < BMU_TCELL_MAX_NUM)) {
        BMU_ID -= BMU_ID_OFFSET;
        ret = BR.BmuVcell[BMU_ID][Cell_ID];
    } else {
        ret = 0xFF;
    }
    return ret;
}

/**
 * @brief      Get the cell Voltage value at the specified ID
 *
 * @param      BMU_ID Start ID is 1 , End ID is BMU_MAX_NUM
 * @param      Cell_ID Start ID is 0 , End ID is BMU_VCELL_MAX_NUM
 * @return     unsigned short
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-08-29
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
unsigned short BMU_CellVoltGet(unsigned char BMU_ID, unsigned char Cell_ID) {
    unsigned short ret;
    if ((BMU_ID < (BMU_MAX_NUM + BMU_ID_OFFSET)) && (BMU_ID >= BMU_ID_OFFSET) && (BMU_ID < BMU_VCELL_MAX_NUM)) {
        BMU_ID -= BMU_ID_OFFSET;
        ret = BR.BmuVcell[BMU_ID][Cell_ID];
    } else {
        ret = 0xFF;
    }
    return ret;
}

static inline void BMU_DataInterchange(void) {
    bmsData.BmuMaxTcellID = BR.MaxTcellID;
    bmsData.BmuMaxVcellID = BR.MaxVcellID;
    bmsData.BmuMinTcellID = BR.MinTcellID;
    bmsData.BmuMinVcellID = BR.MinVcellID;
    bmsData.BusVolt       = BR.Voltage;
    bmsData.MaxTcell      = BR.MaxTcell;
    bmsData.MaxTcellBmuID = BR.MaxTcellBmuID;
    bmsData.MaxVcell      = BR.MaxVcell;
    bmsData.MaxVcellBmuID = BR.MaxVcellBmuID;
    bmsData.MinTcell      = BR.MinTcell;
    bmsData.MinTcellBmuID = BR.MinTcellBmuID;
    bmsData.MinVcell      = BR.MinVcell;
    bmsData.MinVcellBmuID = BR.MinVcellBmuID;
    bmsData.DeltaVolt     = bmsData.MaxVcell - bmsData.MinVcell;
    bmsData.DeltaTemp     = bmsData.MaxTcell - bmsData.MinTcell;

    bmsData.BusVolt_mV = testBusVolt;
    bmsData.BusCurrent = testBusCurrent;
    bmsData.MaxTcell   = testMaxTcell;
    bmsData.MinTcell   = testMinTcell;
    bmsData.MaxVcell   = testMaxVcell;
    bmsData.MinVcell   = testMinVcell;

    if (bmsData.MinTcell > bmsData.MaxTcell) {
        SWAP(bmsData.MinTcell, bmsData.MaxTcell);
    }
    if (bmsData.MinVcell > bmsData.MaxVcell) {
        SWAP(bmsData.MinVcell, bmsData.MaxVcell);
    }
    static unsigned char cnt = 0;
    CAN_MSG_t            canTxMsg;
    if (++cnt > 9) {
        cnt = 0;

        canTxMsg.id      = 0x15000000;
        canTxMsg.dlc     = 8;
        canTxMsg.data[0] = DTC_FaultEventGet(DTC_BUS_OVP) + (DTC_FaultEventGet(DTC_BUS_UVP) << 1) +
                           (DTC_FaultEventGet(DTC_OCCP) << 2) + (DTC_FaultEventGet(DTC_ODCP) << 3) +
                           (DTC_FaultEventGet(DTC_OTP) << 4) + (DTC_FaultEventGet(DTC_UTP) << 5) +
                           (DTC_FaultEventGet(DTC_VCELL_OVP) << 6) + (DTC_FaultEventGet(DTC_VCELL_UVP) << 7);

        canTxMsg.data[1] = 0;
        canTxMsg.data[2] = bmsData.SysStatus;
        canTxMsg.data[3] = 0;
        canTxMsg.data[4] = 0;
        canTxMsg.data[5] = 0;
        canTxMsg.data[6] = 0;
        canTxMsg.data[7] = 0;
        CAN_PushTxQueue(CAN_4, &canTxMsg);
    }
}

static void BMU_CtrlSM(void) {
    if (BMU_tasksTimeCount == 0) {
        BMU_taskState      = BMS_BMU_SRCH_DATA;
        BMU_tasksTimeCount = BMU_TASK_CYCLE_TIME;
    }
    switch (BMU_taskState) {
        case BMS_BMU_REQ_CMD:
            BMU_rxMessageFlag        = 0;
            BMU_responseTimeoutCount = BMU_RESPONSE_TIMEOUT_TIME;
            BMU_BalanceParamMsg(BMU_ID);
            BMU_RequestDataMsg(BMU_ID);
            BMU_taskState = BMS_BMU_CHK_RSP;
            break;
        case BMS_BMU_CHK_RSP:
            if (BMU_ID < BMU_MAX_NUM) {
                // BMU_rxMessageFlag = 0x07;  // TODO: Delete
                if (BMU_rxMessageFlag == BMU_RECV_MSG_FLAG) {
                    BMU_ResponseComplete(BMU_ID);
                    if (BMU_XtrmVcellSearch(BMU_ID)) {
                        /*TODO:DTC_FaultOccurClear(DTC_AFE_COMM);*/
                    } else {
                        /*TODO:DTC_FaultOccurSet(DTC_AFE_COMM);*/
                    }
                    BMU_XtrmTcellSearch(BMU_ID);
                    BMU_ID++;
                    BMU_taskState = BMS_BMU_REQ_CMD;
                } else if (BMU_responseTimeoutCount == 0) {
                    if (++BMU_retryCount > 2) {
                        BMU_ResponseTimeout(BMU_ID);
                        BMU_retryCount = 0;
                        BMU_ID++;
                    }
                    BMU_taskState = BMS_BMU_REQ_CMD;
                }
            }
            break;
        case BMS_BMU_SRCH_DATA:
            BMU_ID        = 0;
            BMU_taskState = BMS_BMU_REQ_CMD;

            BMU_CommStatusCheck();
            BRANCH_XtrmVoltSearch();
            BRANCH_XtrmTempSearch();
            BRANCH_XtrmVcellIDSearch();
            BRANCH_XtrmTcellIDSearch();

            break;
        default:
            BMU_taskState = BMS_BMU_REQ_CMD;
            break;
    }
}

void BMU_Initialize(void) {
    BMU_tasksTimeCount = BMU_TASK_CYCLE_TIME;
    BMU_taskState      = BMS_BMU_REQ_CMD;
    CAN_MSG_t canRxMsg;
    if (0x00 == eepSpe.BmsAddr) {  // TODO:
        /* 測試時如果沒有變更位址，各BMU回覆0xC0(BCU)的位址，
            避免造成Slave BCU的負擔，Slave BCU預設位址0xC0 */
        BMS_SourceAddrBackup = 0xD0;
    } else {
        // Avoid changing BMS Address when application executing
        BMS_SourceAddrBackup = (eepSpe.BmsAddr | 0xC0);  // Intranet communication address
    }

    /* Clear Queue */
    while (CAN_GetRxQueueCount(CAN_4) != 0) {
        CAN_PullRxQueue(CAN_4, &canRxMsg);
    }
}

void BMU_CheckQueueTasks(CAN_MSG_t canRxMsg) {
    BMU_ResponseCheck(BMU_ID, canRxMsg);
    if (canRxMsg.id == 0x15000001) {
        testBusVolt = canRxMsg.data[0] + (canRxMsg.data[1] << 8) + (canRxMsg.data[2] << 16) + (canRxMsg.data[3] << 24);
        testBusCurrent =
            canRxMsg.data[4] + (canRxMsg.data[5] << 8) + (canRxMsg.data[6] << 16) + (canRxMsg.data[7] << 24);
    } else if (canRxMsg.id == 0x15000002) {
        testMaxTcell = canRxMsg.data[0] + (canRxMsg.data[1] << 8) + (canRxMsg.data[2] << 16) + (canRxMsg.data[3] << 24);
        testMinTcell = canRxMsg.data[4] + (canRxMsg.data[5] << 8) + (canRxMsg.data[6] << 16) + (canRxMsg.data[7] << 24);
    } else if (canRxMsg.id == 0x15000003) {
        testMaxVcell = canRxMsg.data[0] + (canRxMsg.data[1] << 8) + (canRxMsg.data[2] << 16) + (canRxMsg.data[3] << 24);
        testMinVcell = canRxMsg.data[4] + (canRxMsg.data[5] << 8) + (canRxMsg.data[6] << 16) + (canRxMsg.data[7] << 24);
    }
}

/*TODO: 1ms TimeCount Handle*/
void BMU_1ms_Tasks(void) {
    BMU_CtrlSM();
    BMU_DataInterchange();
    if (BMU_responseTimeoutCount) {
        BMU_responseTimeoutCount--;
    }

    if (BMU_tasksTimeCount) {
        BMU_tasksTimeCount--;
    }

    if (gBmuFaultLed1msDEC) {
        gBmuFaultLed1msDEC--;
    } else {
        for (unsigned i = 0; i < BMU_MAX_NUM; i++) {
            BR.BmuFaultLed[i] = 0;
        }
    }
}

/* USER CODE END 0 */
/*******************************************************************************
 End of File
 */
