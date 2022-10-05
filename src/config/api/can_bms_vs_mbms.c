/**
 * @file       can_bms_vs_mbms.c
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @brief
 * @version    0.1
 * @date       2022-10-04
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
#include "can_bms_vs_mbms.h"

#include "bms_ctrl.h"
#include "can_bms_vs_bmu.h"
#include "commonly_used.h"
#include "current_sensor.h"
#include "dtc.h"
#include "hv_setup.h"
#include "sys_parameter.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/

/* USER CODE BEGIN PTD */
typedef struct {
    unsigned short subCmd;
    unsigned char length;
    void *ptrVariable;
} PARAM_POINT_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define CAN_ENGR_MODE_TIMEOUT (5000UL)
#define MBMS_COMM_TIMEOUT     (5000UL)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Global variables -----------------------------------------------------------*/
/* USER CODE BEGIN GV */
unsigned short gMbmsTimeoutCount = 0;
unsigned short gEngrTimeoutCount = 0;
unsigned char gEngrMode = 0;
BMS_WORK_MODE_e gMbmsRelayCmd = 0;
/* USER CODE END GV */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
static unsigned short seedRandom;
static unsigned short seed = 0;

static const PARAM_POINT_t VoltParamGroup[] = {
    {0x0101, sizeof (eepBms.BusOVP.Limit), &eepBms.BusOVP.Limit},
    {0x0102, sizeof (eepBms.BusOVP.LimitTime), &eepBms.BusOVP.LimitTime},
    {0x0103, sizeof (eepBms.BusOVP.Release), &eepBms.BusOVP.Release},
    {0x0104, sizeof (eepBms.BusOVP.ReleaseTime), &eepBms.BusOVP.ReleaseTime},
    {0x0201, sizeof (eepBms.BusOVW.Limit), &eepBms.BusOVW.Limit},
    {0x0202, sizeof (eepBms.BusOVW.LimitTime), &eepBms.BusOVW.LimitTime},
    {0x0203, sizeof (eepBms.BusOVW.Release), &eepBms.BusOVW.Release},
    {0x0204, sizeof (eepBms.BusOVW.ReleaseTime), &eepBms.BusOVW.ReleaseTime},
    {0x0301, sizeof (eepBms.BusUVP.Limit), &eepBms.BusUVP.Limit},
    {0x0302, sizeof (eepBms.BusUVP.LimitTime), &eepBms.BusUVP.LimitTime},
    {0x0303, sizeof (eepBms.BusUVP.Release), &eepBms.BusUVP.Release},
    {0x0304, sizeof (eepBms.BusUVP.ReleaseTime), &eepBms.BusUVP.ReleaseTime},
    {0x0401, sizeof (eepBms.BusUVW.Limit), &eepBms.BusUVW.Limit},
    {0x0402, sizeof (eepBms.BusUVW.LimitTime), &eepBms.BusUVW.LimitTime},
    {0x0403, sizeof (eepBms.BusUVW.Release), &eepBms.BusUVW.Release},
    {0x0404, sizeof (eepBms.BusUVW.ReleaseTime), &eepBms.BusUVW.ReleaseTime},
    {0x1101, sizeof (eepBms.CellUBP.Limit), &eepBms.CellUBP.Limit},
    {0x1102, sizeof (eepBms.CellUBP.LimitTime), &eepBms.CellUBP.LimitTime},
    {0x1103, sizeof (eepBms.CellUBP.Release), &eepBms.CellUBP.Release},
    {0x1104, sizeof (eepBms.CellUBP.ReleaseTime), &eepBms.CellUBP.ReleaseTime},
    {0x1201, sizeof (eepBms.CellUBW.Limit), &eepBms.CellUBW.Limit},
    {0x1202, sizeof (eepBms.CellUBW.LimitTime), &eepBms.CellUBW.LimitTime},
    {0x1203, sizeof (eepBms.CellUBW.Release), &eepBms.CellUBW.Release},
    {0x1204, sizeof (eepBms.CellUBW.ReleaseTime), &eepBms.CellUBW.ReleaseTime},
    {0x1301, sizeof (eepBms.CellOVP.Limit), &eepBms.CellOVP.Limit},
    {0x1302, sizeof (eepBms.CellOVP.LimitTime), &eepBms.CellOVP.LimitTime},
    {0x1303, sizeof (eepBms.CellOVP.Release), &eepBms.CellOVP.Release},
    {0x1304, sizeof (eepBms.CellOVP.ReleaseTime), &eepBms.CellOVP.ReleaseTime},
    {0x1401, sizeof (eepBms.CellUVP.Limit), &eepBms.CellUVP.Limit},
    {0x1402, sizeof (eepBms.CellUVP.LimitTime), &eepBms.CellUVP.LimitTime},
    {0x1403, sizeof (eepBms.CellUVP.Release), &eepBms.CellUVP.Release},
    {0x1404, sizeof (eepBms.CellUVP.ReleaseTime), &eepBms.CellUVP.ReleaseTime}
};

static const PARAM_POINT_t CurrParamGroup[] = {
    {0x0101, sizeof (eepBms.OCCP.Limit), &eepBms.OCCP.Limit},
    {0x0102, sizeof (eepBms.OCCP.LimitTime), &eepBms.OCCP.LimitTime},
    {0x0103, sizeof (eepBms.OCCP.Release), &eepBms.OCCP.Release},
    {0x0104, sizeof (eepBms.OCCP.ReleaseTime), &eepBms.OCCP.ReleaseTime},
    {0x0201, sizeof (eepBms.OCCW.Limit), &eepBms.OCCW.Limit},
    {0x0202, sizeof (eepBms.OCCW.LimitTime), &eepBms.OCCW.LimitTime},
    {0x0203, sizeof (eepBms.OCCW.Release), &eepBms.OCCW.Release},
    {0x0204, sizeof (eepBms.OCCW.ReleaseTime), &eepBms.OCCW.ReleaseTime},
    {0x0301, sizeof (eepBms.ODCP.Limit), &eepBms.ODCP.Limit},
    {0x0302, sizeof (eepBms.ODCP.LimitTime), &eepBms.ODCP.LimitTime},
    {0x0303, sizeof (eepBms.ODCP.Release), &eepBms.ODCP.Release},
    {0x0304, sizeof (eepBms.ODCP.ReleaseTime), &eepBms.ODCP.ReleaseTime},
    {0x0401, sizeof (eepBms.ODCW.Limit), &eepBms.ODCW.Limit},
    {0x0402, sizeof (eepBms.ODCW.LimitTime), &eepBms.ODCW.LimitTime},
    {0x0403, sizeof (eepBms.ODCW.Release), &eepBms.ODCW.Release},
    {0x0404, sizeof (eepBms.ODCW.ReleaseTime), &eepBms.ODCW.ReleaseTime}
};

static const PARAM_POINT_t TempParamGroup[] = {
    {0x0101, sizeof (eepBms.OTP.Limit), &eepBms.OTP.Limit},
    {0x0102, sizeof (eepBms.OTP.LimitTime), &eepBms.OTP.LimitTime},
    {0x0103, sizeof (eepBms.OTP.Release), &eepBms.OTP.Release},
    {0x0104, sizeof (eepBms.OTP.ReleaseTime), &eepBms.OTP.ReleaseTime},
    {0x0201, sizeof (eepBms.OTW.Limit), &eepBms.OTW.Limit},
    {0x0202, sizeof (eepBms.OTW.LimitTime), &eepBms.OTW.LimitTime},
    {0x0203, sizeof (eepBms.OTW.Release), &eepBms.OTW.Release},
    {0x0204, sizeof (eepBms.OTW.ReleaseTime), &eepBms.OTW.ReleaseTime},
    {0x0301, sizeof (eepBms.UTP.Limit), &eepBms.UTP.Limit},
    {0x0302, sizeof (eepBms.UTP.LimitTime), &eepBms.UTP.LimitTime},
    {0x0303, sizeof (eepBms.UTP.Release), &eepBms.UTP.Release},
    {0x0304, sizeof (eepBms.UTP.ReleaseTime), &eepBms.UTP.ReleaseTime},
    {0x0401, sizeof (eepBms.UTW.Limit), &eepBms.UTW.Limit},
    {0x0402, sizeof (eepBms.UTW.LimitTime), &eepBms.UTW.LimitTime},
    {0x0403, sizeof (eepBms.UTW.Release), &eepBms.UTW.Release},
    {0x0404, sizeof (eepBms.UTW.ReleaseTime), &eepBms.UTW.ReleaseTime},
    {0x0501, sizeof (eepBms.UBTW.Limit), &eepBms.UBTW.Limit},
    {0x0502, sizeof (eepBms.UBTW.LimitTime), &eepBms.UBTW.LimitTime},
    {0x0503, sizeof (eepBms.UBTW.Release), &eepBms.UBTW.Release},
    {0x0504, sizeof (eepBms.UBTW.ReleaseTime), &eepBms.UBTW.ReleaseTime}
};

static const PARAM_POINT_t OtherParamGroup[] = {
    {0xA001, sizeof (eepBms.FullCap), &eepBms.FullCap},
    {0xA002, sizeof (bmsData.DesingCap), &bmsData.DesingCap},
    {0xA003, sizeof (bmsData.RemCap), &bmsData.RemCap},
    {0xA004, sizeof (eepEmg.CycleLife), &eepEmg.CycleLife}
};

const unsigned short Security_uint16_Table[] = {0x6C27u, 0xB93Fu, 0x9813u, 0x8A21u, 0x4F5Du, 0x0C12u, 0xF230u, 0x64E5u,
    0x3F0Cu, 0xB071u, 0xDA51u, 0x406Cu, 0xB542u, 0x905Au, 0x3CF3u, 0xA789u};
/* USER CODE END PV */

/* Function prototypes -------------------------------------------------------*/
/* USER CODE BEGIN FP */

/* USER CODE END FP */

/* Private user code ---------------------------------------------------------*/

/* USER CODE BEGIN 0 */
struct {
    unsigned char commad;
    unsigned char subCommad;
    unsigned char pfCommand;
    unsigned char sourceAddr;
} MBMS;

typedef enum {
    SUB_CMD_HV_CTRL = 0x500,
    SUB_CMD_SYS_RESET = 0x510,
    SUB_CMD_CLR_DTC = 0x515,
} MBMS_SUB_CMD_e;

unsigned int MBMS_GetSecurityKey(unsigned short seed) {
    unsigned int ret;
    unsigned char key[4];
    unsigned short table;

    key[0] = 0;
    key[1] = 0;
    key[2] = 0;
    key[3] = 0;
    table = Security_uint16_Table[seed & 0x0F];

    key[0] = ((~table) >> 3) ^ seed;
    key[1] = (key[0] + (~seed)) ^ (table >> 8);
    key[2] = (~key[1]) & (key[1] ^ (~(table) >> 5));
    key[3] = (key[2] ^ ((seed + key[0]) >> 1));

    ret = key[0];
    ret <<= 8;
    ret |= key[1];
    ret <<= 8;
    ret |= key[2];
    ret <<= 8;
    ret |= key[3];

    return ret;
}

static void MBMS_XferMsgInit(CAN_MSG_t *canTxMsg) {
    canTxMsg->J1939.priority = 5;
    canTxMsg->J1939.reserved = 1;
    canTxMsg->J1939.dataPage = 0;
    canTxMsg->J1939.pduFormat = 0x00;
    canTxMsg->J1939.pduSpecific = 0x00; /* Destination Address */
    canTxMsg->J1939.sourceAddress = eepSpe.BmsAddr;
    canTxMsg->dlc = 8;

    for (unsigned i = 0; i < 8; i++) {
        *(canTxMsg->data + i) = 0xAA;
    }
}

static void MBMS_Ack(CAN_MSG_t *canRxMsg) {
    CAN_MSG_t canTxMsg;

    MBMS_XferMsgInit(&canTxMsg);
    canTxMsg.J1939.pduSpecific = canRxMsg->J1939.sourceAddress;
    canTxMsg.J1939.pduFormat = 0xE8; /*  */

    canTxMsg.data[0] = 0; /* ACK */
    canTxMsg.data[5] = canRxMsg->data[0];
    canTxMsg.data[6] = canRxMsg->data[1];
    canTxMsg.data[7] = canRxMsg->data[2];
    CAN_PushTxQueue(CAN_1, &canTxMsg);
}

static void MBMS_Nack(CAN_MSG_t *canRxMsg) {
    CAN_MSG_t canTxMsg;

    MBMS_XferMsgInit(&canTxMsg);
    canTxMsg.J1939.pduSpecific = canRxMsg->J1939.sourceAddress;
    canTxMsg.J1939.pduFormat = 0xE8; /*  */

    canTxMsg.data[0] = 1; /* NACK */
    canTxMsg.data[5] = canRxMsg->data[0];
    canTxMsg.data[6] = canRxMsg->data[1];
    canTxMsg.data[7] = canRxMsg->data[2];
    CAN_PushTxQueue(CAN_1, &canTxMsg);
}

static void MBMS_GetGroupParam(const PARAM_POINT_t *ptrTable, unsigned char tableMax, unsigned short subCmd,
        CAN_MSG_t *canTxMsg) {
    unsigned char i = 0;
    unsigned short Var16 = 0;
    unsigned int Var32 = 0;

    for (i = 0; i < tableMax; i++) {
        if (subCmd == (ptrTable + i)->subCmd) {
            switch ((ptrTable + i)->length) {
                case 1:
                    canTxMsg->data[2] = *((unsigned char *) ((ptrTable + i)->ptrVariable));
                    break;

                case 2:
                    Var16 = *((unsigned short *) ((ptrTable + i)->ptrVariable));
                    canTxMsg->data[2] = (unsigned char) (Var16 & 0xFF);
                    canTxMsg->data[3] = (unsigned char) ((Var16 & 0xFF00) >> 8);
                    break;

                case 4:
                    Var32 = *((unsigned int *) ((ptrTable + i)->ptrVariable));
                    canTxMsg->data[2] = (unsigned char) (Var32 & 0xFF); /* LSB */
                    canTxMsg->data[3] = (unsigned char) ((Var32 & 0xFF00) >> 8);
                    canTxMsg->data[4] = (unsigned char) ((Var32 & 0xFF0000) >> 16);
                    canTxMsg->data[5] = (unsigned char) ((Var32 & 0xFF000000) >> 24); /* MSB */
                    break;

                default:
                    i = tableMax;
                    break;
            }

            break;
        }
    }

    if (i >= tableMax) {
        canTxMsg->J1939.pduFormat = 0xE8; /* NACK */
    }
}

static void MBMS_SetGroupParam(const PARAM_POINT_t *ptrTable, unsigned char tableMax, CAN_MSG_t *canRxMsg) {
    unsigned char i = 0;
    unsigned short subCmd = 0;
    unsigned short Var16 = 0;
    unsigned int Var32 = 0;

    HIGH_BYTE(subCmd) = canRxMsg->data[0];
    LOW_BYTE(subCmd) = canRxMsg->data[1];
    for (i = 0; i < tableMax; i++) {
        if (subCmd == (ptrTable + i)->subCmd) {
            switch ((ptrTable + i)->length) {
                case 1:
                    canRxMsg->data[2] = *((unsigned char *) ((ptrTable + i)->ptrVariable));
                    break;

                case 2:
                    Var16 = *((unsigned short *) ((ptrTable + i)->ptrVariable));
                    canRxMsg->data[2] = (unsigned char) (Var16 & 0xFF);
                    canRxMsg->data[3] = (unsigned char) ((Var16 & 0xFF00) >> 8);
                    break;

                case 4:
                    Var32 = *((unsigned int *) ((ptrTable + i)->ptrVariable));
                    canRxMsg->data[2] = (unsigned char) (Var32 & 0xFF); /* LSB */
                    canRxMsg->data[3] = (unsigned char) ((Var32 & 0xFF00) >> 8);
                    canRxMsg->data[4] = (unsigned char) ((Var32 & 0xFF0000) >> 16);
                    canRxMsg->data[5] = (unsigned char) ((Var32 & 0xFF000000) >> 24); /* MSB */
                    break;

                default:
                    i = tableMax;
                    break;
            }

            break;
        }
    }

    if (i >= tableMax) {
        canRxMsg->J1939.pduFormat = 0xE8; /* NACK */
    }
}

static void PF_GetCommVer(CAN_MSG_t *canRxMsg) {
    CAN_MSG_t canTxMsg;
    MBMS_XferMsgInit(&canTxMsg);
    canTxMsg.J1939.pduSpecific = canRxMsg->J1939.sourceAddress;
    canTxMsg.J1939.pduFormat = PF_GET_COMM_VER; /*  */

    canTxMsg.data[0] = COMM_VER_MAJOR_0;
    canTxMsg.data[1] = COMM_VER_MAJOR_1;
    canTxMsg.data[2] = 0x2E;
    canTxMsg.data[3] = COMM_VER_SUB_0;
    canTxMsg.data[4] = COMM_VER_SUB_1;
    canTxMsg.data[5] = COMM_VER_SUB_2;
    CAN_PushTxQueue(CAN_1, &canTxMsg);
}

static void PF_GetSeed(CAN_MSG_t *canRxMsg) {
    CAN_MSG_t canTxMsg;
    MBMS_XferMsgInit(&canTxMsg);
    canTxMsg.J1939.pduSpecific = canRxMsg->J1939.sourceAddress;
    canTxMsg.J1939.pduFormat = PF_GET_SEED; /*  */

    seed = seedRandom;
    canTxMsg.data[0] = LOW_BYTE(seed);
    canTxMsg.data[1] = HIGH_BYTE(seed);
    CAN_PushTxQueue(CAN_1, &canTxMsg);
}

static void PF_GetMfgDate(CAN_MSG_t *canRxMsg) {
    CAN_MSG_t canTxMsg;
    MBMS_XferMsgInit(&canTxMsg);
    canTxMsg.J1939.pduSpecific = canRxMsg->J1939.sourceAddress;
    canTxMsg.J1939.pduFormat = PF_GET_MFG_DATE; /*  */

    canTxMsg.data[0] = eepBms.FactoryDay;
    canTxMsg.data[1] = eepBms.FactoryMonth;
    canTxMsg.data[2] = LOW_BYTE(eepBms.FactoryYear);
    canTxMsg.data[3] = HIGH_BYTE(eepBms.FactoryYear);
    CAN_PushTxQueue(CAN_1, &canTxMsg);
}

static void PF_GetProdSN(CAN_MSG_t *canRxMsg) {
    CAN_MSG_t canTxMsg;
    MBMS_XferMsgInit(&canTxMsg);

    canTxMsg.J1939.pduSpecific = canRxMsg->J1939.sourceAddress;
    canTxMsg.J1939.pduFormat = PF_GET_PROD_SN;

    for (unsigned char i = 0; i < 8; i++) {
        canTxMsg.data[i] = eepBms.SerialNum[i];
    }
    CAN_PushTxQueue(CAN_1, &canTxMsg);
    canTxMsg.J1939.pduFormat = 0x13;
    for (unsigned char i = 8; (i - 8) < 5; i++) {
        canTxMsg.data[i] = eepBms.SerialNum[i];
    }
    canTxMsg.data[5] = 0x5A;
    canTxMsg.data[6] = 0x5A;
    canTxMsg.data[7] = 0x5A;
    CAN_PushTxQueue(CAN_1, &canTxMsg);
}

static void PF_GetSwHwVer(CAN_MSG_t *canRxMsg) {
    CAN_MSG_t canTxMsg;
    MBMS_XferMsgInit(&canTxMsg);

    canTxMsg.J1939.pduSpecific = canRxMsg->J1939.sourceAddress;
    canTxMsg.J1939.pduFormat = PF_GET_SW_HW_VER; /*  */

    canTxMsg.data[0] = HARDWARE_VER_0;
    canTxMsg.data[1] = HARDWARE_VER_1;
    canTxMsg.data[2] = HARDWARE_VER_2;
    canTxMsg.data[3] = FIRMWARE_VER_0;
    canTxMsg.data[4] = FIRMWARE_VER_1;
    canTxMsg.data[5] = FIRMWARE_VER_2;
    canTxMsg.data[6] = 0x5A;
    canTxMsg.data[7] = 0x5A;
    CAN_PushTxQueue(CAN_1, &canTxMsg);
}

static void PF_GetCurrGrpParam(CAN_MSG_t *canRxMsg) {
    unsigned short subCmd;
    CAN_MSG_t canTxMsg;
    MBMS_XferMsgInit(&canTxMsg);

    canTxMsg.J1939.pduSpecific = canRxMsg->J1939.sourceAddress;
    canTxMsg.J1939.pduFormat = PF_GET_CURR_GRP_PARAM;

    HIGH_BYTE(subCmd) = canRxMsg->data[3];
    LOW_BYTE(subCmd) = canRxMsg->data[4];

    MBMS_GetGroupParam(CurrParamGroup, sizeof (CurrParamGroup) / sizeof (CurrParamGroup[0]), subCmd, &canTxMsg);

    if (canTxMsg.J1939.pduFormat != PF_GET_CURR_GRP_PARAM) {
        MBMS_Nack(canRxMsg);
    } else {
        canTxMsg.J1939.pduSpecific = canRxMsg->J1939.sourceAddress;
        canTxMsg.data[0] = canRxMsg->data[3];
        canTxMsg.data[1] = canRxMsg->data[4];
        CAN_PushTxQueue(CAN_1, &canTxMsg);
    }
}

static void PF_GetVoltGrpParam(CAN_MSG_t *canRxMsg) {
    unsigned short subCmd;
    CAN_MSG_t canTxMsg;
    MBMS_XferMsgInit(&canTxMsg);

    canTxMsg.J1939.pduSpecific = canRxMsg->J1939.sourceAddress;
    canTxMsg.J1939.pduFormat = PF_GET_VOLT_GRP_PARAM;

    HIGH_BYTE(subCmd) = canRxMsg->data[3];
    LOW_BYTE(subCmd) = canRxMsg->data[4];

    MBMS_GetGroupParam(VoltParamGroup, sizeof (VoltParamGroup) / sizeof (VoltParamGroup[0]), subCmd, &canTxMsg);

    if (canTxMsg.J1939.pduFormat != PF_GET_VOLT_GRP_PARAM) {
        MBMS_Nack(canRxMsg);
    } else {
        canTxMsg.J1939.pduSpecific = canRxMsg->J1939.sourceAddress;
        canTxMsg.data[0] = canRxMsg->data[3];
        canTxMsg.data[1] = canRxMsg->data[4];
        CAN_PushTxQueue(CAN_1, &canTxMsg);
    }
}

static void PF_GetTempGrpParam(CAN_MSG_t *canRxMsg) {
    unsigned short subCmd;
    CAN_MSG_t canTxMsg;
    MBMS_XferMsgInit(&canTxMsg);

    canTxMsg.J1939.pduSpecific = canRxMsg->J1939.sourceAddress;
    canTxMsg.J1939.pduFormat = PF_GET_TEMP_GRP_PARAM;

    HIGH_BYTE(subCmd) = canRxMsg->data[3];
    LOW_BYTE(subCmd) = canRxMsg->data[4];

    MBMS_GetGroupParam(TempParamGroup, sizeof (TempParamGroup) / sizeof (TempParamGroup[0]), subCmd, &canTxMsg);

    if (canTxMsg.J1939.pduFormat != PF_GET_VOLT_GRP_PARAM) {
        MBMS_Nack(canRxMsg);
    } else {
        canTxMsg.J1939.pduSpecific = canRxMsg->J1939.sourceAddress;
        canTxMsg.data[0] = canRxMsg->data[3];
        canTxMsg.data[1] = canRxMsg->data[4];
        CAN_PushTxQueue(CAN_1, &canTxMsg);
    }
}

static void PF_GetOtherGrpParam(CAN_MSG_t *canRxMsg) {
    unsigned short subCmd;
    CAN_MSG_t canTxMsg;
    MBMS_XferMsgInit(&canTxMsg);

    canTxMsg.J1939.pduSpecific = canRxMsg->J1939.sourceAddress;
    canTxMsg.J1939.pduFormat = PF_GET_OTHER_GRP_PARAM;

    HIGH_BYTE(subCmd) = canRxMsg->data[3];
    LOW_BYTE(subCmd) = canRxMsg->data[4];

    MBMS_GetGroupParam(OtherParamGroup, sizeof (OtherParamGroup) / sizeof (TempParamGroup[0]), subCmd, &canTxMsg);

    if (canTxMsg.J1939.pduFormat != PF_GET_VOLT_GRP_PARAM) {
        MBMS_Nack(canRxMsg);
    } else {
        canTxMsg.J1939.pduSpecific = canRxMsg->J1939.sourceAddress;
        canTxMsg.data[0] = canRxMsg->data[3];
        canTxMsg.data[1] = canRxMsg->data[4];
        CAN_PushTxQueue(CAN_1, &canTxMsg);
    }
}

static void PF_GetCurrCalibParam(CAN_MSG_t *canRxMsg) {
    CAN_MSG_t canTxMsg;
    MBMS_XferMsgInit(&canTxMsg);

    canTxMsg.J1939.pduSpecific = canRxMsg->J1939.sourceAddress;
    canTxMsg.J1939.pduFormat = PF_GET_CURR_CALIB_PARAM; /*  */

    canTxMsg.data[0] = 0x30; /* Command */
    canTxMsg.data[1] = 0x00; /* Command */
    canTxMsg.data[2] = LOW_BYTE(eepSpe.AdcZeroOffset);
    canTxMsg.data[3] = HIGH_BYTE(eepSpe.AdcZeroOffset);
    CAN_PushTxQueue(CAN_1, &canTxMsg);

    canTxMsg.data[0] = 0x30; /* Command */
    canTxMsg.data[1] = 0x10; /* Command */
    canTxMsg.data[2] = LOW_BYTE(eepSpe.AdcGainOffset);
    canTxMsg.data[3] = HIGH_BYTE(eepSpe.AdcGainOffset);
    CAN_PushTxQueue(CAN_1, &canTxMsg);

    canTxMsg.data[0] = 0x30; /* Command */
    canTxMsg.data[1] = 0x20; /* Command */
    canTxMsg.data[2] = *(((unsigned char *) (&bmsData.BusCurrent)));
    canTxMsg.data[3] = *(((unsigned char *) (&bmsData.BusCurrent)) + 1);
    canTxMsg.data[4] = *(((unsigned char *) (&bmsData.BusCurrent)) + 2);
    canTxMsg.data[5] = *(((unsigned char *) (&bmsData.BusCurrent)) + 3);
    CAN_PushTxQueue(CAN_1, &canTxMsg);
}

static void PF_GetSystemParam(CAN_MSG_t *canRxMsg) {
    CAN_MSG_t canTxMsg;
    MBMS_XferMsgInit(&canTxMsg);

    int Current;
    Current = bmsData.BusCurrent;
    Current = Current / 1000;

    canTxMsg.J1939.pduSpecific = canRxMsg->J1939.sourceAddress;
    canTxMsg.J1939.pduFormat = PF_GET_SYS_PARAM_G1; /*  */

    canTxMsg.data[0] = LOW_BYTE(bmsData.BusVolt);
    canTxMsg.data[1] = HIGH_BYTE(bmsData.BusVolt);
    canTxMsg.data[2] = LOW_BYTE(Current); // bus current(unit:A) low byte
    canTxMsg.data[3] = HIGH_BYTE(Current); // bus current(unit:A) high byte
    canTxMsg.data[4] = bmsData.SOC;
    canTxMsg.data[5] = (unsigned char) bmsData.HvStatus;
    canTxMsg.data[6] = bmsData.SOH;
    canTxMsg.data[7] = bmsData.Status;
    CAN_PushTxQueue(CAN_1, &canTxMsg);

    canTxMsg.J1939.pduFormat = PF_GET_SYS_PARAM_G2; /*  */

    canTxMsg.data[0] = LOW_BYTE(bmsData.MaxVcell);
    canTxMsg.data[1] = HIGH_BYTE(bmsData.MaxVcell);
    canTxMsg.data[2] = LOW_BYTE(bmsData.MinVcell);
    canTxMsg.data[3] = HIGH_BYTE(bmsData.MinVcell);
    canTxMsg.data[4] = LOW_BYTE(bmsData.MaxTcell);
    canTxMsg.data[5] = HIGH_BYTE(bmsData.MaxVcell);
    canTxMsg.data[6] = LOW_BYTE(bmsData.MinTcell);
    canTxMsg.data[7] = HIGH_BYTE(bmsData.MinTcell);
    CAN_PushTxQueue(CAN_1, &canTxMsg);

    canTxMsg.J1939.pduFormat = PF_GET_SYS_PARAM_G3; /*  */

    Current = bmsData.BusCurrent;
    canTxMsg.data[0] = (unsigned char) (bmsData.BusVolt_mV & 0xFF);
    canTxMsg.data[1] = (unsigned char) ((bmsData.BusVolt_mV & 0xFF00) >> 8);
    canTxMsg.data[2] = (unsigned char) ((bmsData.BusVolt_mV & 0xFF0000) >> 16);
    canTxMsg.data[3] = (unsigned char) ((bmsData.BusVolt_mV & 0xFF000000) >> 24);
    canTxMsg.data[4] = (unsigned char) (Current & 0xFF);
    canTxMsg.data[5] = (unsigned char) ((Current & 0xFF00) >> 8);
    canTxMsg.data[6] = (unsigned char) ((Current & 0xFF0000) >> 16);
    canTxMsg.data[7] = (unsigned char) ((Current & 0xFF000000) >> 24);
    CAN_PushTxQueue(CAN_1, &canTxMsg);

    canTxMsg.J1939.pduFormat = PF_GET_SYS_PARAM_G4; /*  */

    canTxMsg.data[0] = bmsData.MaxVcellBmuID;
    canTxMsg.data[1] = bmsData.BmuMaxVcellID;
    canTxMsg.data[2] = bmsData.MinVcellBmuID;
    canTxMsg.data[3] = bmsData.BmuMinVcellID;
    canTxMsg.data[4] = bmsData.MaxTcellBmuID;
    canTxMsg.data[5] = bmsData.BmuMaxTcellID;
    canTxMsg.data[6] = bmsData.MinTcellBmuID;
    canTxMsg.data[7] = bmsData.BmuMinTcellID;
    CAN_PushTxQueue(CAN_1, &canTxMsg);

    //
    DTC_FaultOccurClear(DTC_MBMS_COMM);
    gMbmsTimeoutCount = MBMS_COMM_TIMEOUT;
}

static void PF_GetCellData(CAN_MSG_t *canRxMsg) {
    CAN_MSG_t canTxMsg;
    MBMS_XferMsgInit(&canTxMsg);
    short Temperature;
    unsigned short Voltage;

    canTxMsg.J1939.pduSpecific = canRxMsg->J1939.sourceAddress;
    canTxMsg.J1939.pduFormat = PF_GET_CELL_DATA_G1; /*	*/

    canTxMsg.data[0] = canRxMsg->data[3];
    canTxMsg.data[1] = BMU_VCELL_MAX_NUM;
    Temperature = BMU_CellTempGet(canRxMsg->data[3], 0);
    canTxMsg.data[2] = LOW_BYTE(Temperature);
    canTxMsg.data[3] = HIGH_BYTE(Temperature);
    Temperature = BMU_CellTempGet(canRxMsg->data[3], 1);
    canTxMsg.data[4] = LOW_BYTE(Temperature);
    canTxMsg.data[5] = HIGH_BYTE(Temperature);
    canTxMsg.data[6] = 0x00; // Status flag
    canTxMsg.data[7] = 0x00; // Error flag
    CAN_PushTxQueue(CAN_1, &canTxMsg);

    canTxMsg.J1939.pduFormat = PF_GET_CELL_DATA_G2; /*	*/

    Voltage = BMU_CellVoltGet(canRxMsg->data[3], 0);
    canTxMsg.data[0] = LOW_BYTE(Voltage);
    canTxMsg.data[1] = HIGH_BYTE(Voltage);
    Voltage = BMU_CellVoltGet(canRxMsg->data[3], 1);
    canTxMsg.data[2] = LOW_BYTE(Voltage);
    canTxMsg.data[3] = HIGH_BYTE(Voltage);
    Voltage = BMU_CellVoltGet(canRxMsg->data[3], 2);
    canTxMsg.data[4] = LOW_BYTE(Voltage);
    canTxMsg.data[5] = HIGH_BYTE(Voltage);
    Voltage = BMU_CellVoltGet(canRxMsg->data[3], 3);
    canTxMsg.data[6] = LOW_BYTE(Voltage);
    canTxMsg.data[7] = HIGH_BYTE(Voltage);
    CAN_PushTxQueue(CAN_1, &canTxMsg);

    canTxMsg.J1939.pduFormat = PF_GET_CELL_DATA_G3; /*	*/

    Voltage = BMU_CellVoltGet(canRxMsg->data[3], 4);
    canTxMsg.data[0] = LOW_BYTE(Voltage);
    canTxMsg.data[1] = HIGH_BYTE(Voltage);
    Voltage = BMU_CellVoltGet(canRxMsg->data[3], 5);
    canTxMsg.data[2] = LOW_BYTE(Voltage);
    canTxMsg.data[3] = HIGH_BYTE(Voltage);
    Voltage = BMU_CellVoltGet(canRxMsg->data[3], 6);
    canTxMsg.data[4] = LOW_BYTE(Voltage);
    canTxMsg.data[5] = HIGH_BYTE(Voltage);
    canTxMsg.data[6] = 0xAA;
    canTxMsg.data[7] = 0xAA;
    CAN_PushTxQueue(CAN_1, &canTxMsg);
}

static void PF_GetBalanceData(CAN_MSG_t *canRxMsg) {
    CAN_MSG_t canTxMsg;
    MBMS_XferMsgInit(&canTxMsg);

    canTxMsg.J1939.pduSpecific = canRxMsg->J1939.sourceAddress;
    canTxMsg.J1939.pduFormat = PF_GET_BALANCE_DATA; /*	*/

    canTxMsg.data[0] = (unsigned char) bmsData.Balance.Mode;
    canTxMsg.data[1] = bmsData.Balance.VoltDiff;
    canTxMsg.data[2] = LOW_BYTE(bmsData.Balance.Volt);
    canTxMsg.data[3] = HIGH_BYTE(bmsData.Balance.Volt);
    canTxMsg.data[4] = LOW_BYTE(bmsData.MinVcell);
    canTxMsg.data[5] = HIGH_BYTE(bmsData.MinVcell);
    canTxMsg.data[6] = (unsigned char) bmsData.Status;

    CAN_PushTxQueue(CAN_1, &canTxMsg);
}

static void PF_GetCoulombGaugeData(CAN_MSG_t *canRxMsg) {
    CAN_MSG_t canTxMsg;
    MBMS_XferMsgInit(&canTxMsg);

    canTxMsg.J1939.pduSpecific = canRxMsg->J1939.sourceAddress;
    canTxMsg.J1939.pduFormat = PF_GET_BALANCE_DATA; /*	*/

    canTxMsg.data[0] = bmsData.SOH;
    canTxMsg.data[1] = bmsData.SOC;
    canTxMsg.data[2] = LOW_BYTE(eepEmg.CycleLife);
    canTxMsg.data[3] = HIGH_BYTE(eepEmg.CycleLife);
    canTxMsg.data[4] = (unsigned char) (bmsData.RemCap);
    canTxMsg.data[5] = (unsigned char) (bmsData.RemCap >> 8);
    canTxMsg.data[6] = (unsigned char) (bmsData.RemCap >> 16);
    canTxMsg.data[7] = (unsigned char) (bmsData.RemCap >> 24);

    CAN_PushTxQueue(CAN_1, &canTxMsg);
}

static void PF_GetLogData(CAN_MSG_t *canRxMsg) {
    CAN_MSG_t canTxMsg;
    MBMS_XferMsgInit(&canTxMsg);

    canTxMsg.J1939.pduSpecific = canRxMsg->J1939.sourceAddress;
    canTxMsg.J1939.pduFormat = PF_GET_BALANCE_DATA; /*	*/

    for (unsigned char i = 0; i < DTC_LOG_LENGTH; i++) {
        canTxMsg.data[i] = LOW_BYTE(eepEmg.ErrorCode[i]);
        canTxMsg.data[i + 1] = HIGH_BYTE(eepEmg.ErrorCode[i]);
    }

    CAN_PushTxQueue(CAN_1, &canTxMsg);
}

static void PF_GetDtcFlag(CAN_MSG_t *canRxMsg) {
    CAN_MSG_t canTxMsg;
    MBMS_XferMsgInit(&canTxMsg);

    canTxMsg.J1939.pduSpecific = canRxMsg->J1939.sourceAddress;
    canTxMsg.J1939.pduFormat = PF_GET_BALANCE_DATA; /*	*/
    // TODO:
    CAN_PushTxQueue(CAN_1, &canTxMsg);
}

static void CMD_SetCommAddr(CAN_MSG_t *canRxMsg) {
    bool sendNack = true;
    unsigned char NewBmsAddress = 0;

    if (gEngrMode == true) {
        gEngrTimeoutCount = CAN_ENGR_MODE_TIMEOUT;

        if ((canRxMsg->data[0] == 0x55) && (canRxMsg->data[1] == 0x5A)) {
            /** 成功 **/
            NewBmsAddress = canRxMsg->data[2];

            canRxMsg->data[0] = 0x55;
            canRxMsg->data[1] = 0xEF;
            canRxMsg->data[2] = 0x5A;
            MBMS_Ack(canRxMsg); /* 用舊的通訊位址回應 */
            sendNack = false;

            /* 變更位址 */
            eepSpe.BmsAddr = NewBmsAddress;
            // TODO:SetAddressFilter(eepSpe.BmsAddr, false);

            /* 寫入EEPROM */
            // TODO:set_fixed_para_2_eeprom();

            MBMS_Ack(canRxMsg); /* 用新的通訊位址再回應一次 */
        }
    }

    if (sendNack) {
        canRxMsg->data[0] = 0x55;
        canRxMsg->data[1] = 0xEF;
        canRxMsg->data[2] = 0x5A;
        MBMS_Nack(canRxMsg);
    }
}

static void CMD_SetMfgDate(CAN_MSG_t *canRxMsg) {
    if (gEngrMode == true) {
        gEngrTimeoutCount = CAN_ENGR_MODE_TIMEOUT;

        eepBms.FactoryDay = canRxMsg->data[0];
        eepBms.FactoryMonth = canRxMsg->data[1];
        LOW_BYTE(eepBms.FactoryYear) = canRxMsg->data[2];
        HIGH_BYTE(eepBms.FactoryYear) = canRxMsg->data[3];
    }
}

static void CMD_SetSN1(CAN_MSG_t *canRxMsg) {
    if (gEngrMode == true) {
        gEngrTimeoutCount = CAN_ENGR_MODE_TIMEOUT;
        eepBms.SerialNum[0] = canRxMsg->data[0];
        eepBms.SerialNum[1] = canRxMsg->data[1];
        eepBms.SerialNum[2] = canRxMsg->data[2];
        eepBms.SerialNum[3] = canRxMsg->data[3];
        eepBms.SerialNum[4] = canRxMsg->data[4];
        eepBms.SerialNum[5] = canRxMsg->data[5];
        eepBms.SerialNum[6] = canRxMsg->data[6];
        eepBms.SerialNum[7] = canRxMsg->data[7];
    }
}

static void CMD_SetSN2(CAN_MSG_t *canRxMsg) {
    if (gEngrMode == true) {
        gEngrTimeoutCount = CAN_ENGR_MODE_TIMEOUT;
        eepBms.SerialNum[8] = canRxMsg->data[0];
        eepBms.SerialNum[9] = canRxMsg->data[1];
        eepBms.SerialNum[10] = canRxMsg->data[2];
        eepBms.SerialNum[11] = canRxMsg->data[3];
        eepBms.SerialNum[12] = canRxMsg->data[4];
    }
}

static void CMD_WriteParam2Eeprom(CAN_MSG_t *canRxMsg) {
    bool sendNack = true;

    if (gEngrMode == true) {
        gEngrTimeoutCount = CAN_ENGR_MODE_TIMEOUT;
        if ((canRxMsg->data[0] == 0x55) && (canRxMsg->data[1] == 0xA5)) {
            sendNack = false;

            // TODO: set_fixed_para_2_eeprom();
            // TODO: set_parameter_2_eeprom();
            // TODO: set_emergency_2_eeprom();

            MBMS_Ack(canRxMsg);
        }
    }

    if (sendNack) {
        MBMS_Nack(canRxMsg);
    }
}

static void CMD_ExcuteCtrlCmd(CAN_MSG_t *canRxMsg) {
    unsigned short subCmd;
    bool sendNack = true;

    HIGH_BYTE(subCmd) = canRxMsg->data[0];
    LOW_BYTE(subCmd) = canRxMsg->data[1];

    switch (subCmd) {
        case SUB_CMD_HV_CTRL:
            gMbmsRelayCmd = (BMS_WORK_MODE_e) canRxMsg->data[2];
            // remote_work_cmd_check();
            sendNack = false;
            break;

        case SUB_CMD_SYS_RESET:
            if (0x47 == canRxMsg->data[2]) {
                gMbmsRelayCmd = BMS_RESET;
                // Reset() ;
            }
            break;

        case SUB_CMD_CLR_DTC:
            if (0x53 == canRxMsg->data[2]) {
                for (unsigned i = 0; i < DTC_LOG_LENGTH; i++) {
                    eepEmg.ErrorCode[i] = 0x0000;
                }
                // TODO:set_emergency_2_eeprom();
                sendNack = false;
            }
            break;

        default:
            break;
    }

    if (sendNack) {
        MBMS_Nack(canRxMsg);
    } else {
        MBMS_Ack(canRxMsg);
    }
}

static void CMD_EnterEngrMode(CAN_MSG_t *canRxMsg) {
    unsigned short key;
    bool sendNack = true;

    gEngrMode = false;

    if ((0xAA == canRxMsg->data[2]) && (0x55 == canRxMsg->data[3])) {
        LOW_BYTE(key) = canRxMsg->data[4]; // 取出驗証Key碼
        HIGH_BYTE(key) = canRxMsg->data[5];

        if (key == MBMS_GetSecurityKey(seed)) {
            gEngrMode = true;
            sendNack = false;
            seed = key; // 清除Seed記錄
            gEngrTimeoutCount = CAN_ENGR_MODE_TIMEOUT;

            MBMS_Ack(canRxMsg);
        }
    }

    if (sendNack) {
        MBMS_Nack(canRxMsg);
    }
}

static void CMD_ExtensionTime(CAN_MSG_t *canRxMsg) {
    unsigned short key;
    bool sendNack = true;

    gEngrMode = false;

    if ((0xAA == canRxMsg->data[2]) && (0x55 == canRxMsg->data[3])) {
        LOW_BYTE(key) = canRxMsg->data[4]; // 取出驗証Key碼
        HIGH_BYTE(key) = canRxMsg->data[5];

        if (key == MBMS_GetSecurityKey(seed)) {
            gEngrMode = true;
            sendNack = false;
            seed = key; // 清除Seed記錄
            gEngrTimeoutCount = CAN_ENGR_MODE_TIMEOUT;

            MBMS_Ack(canRxMsg);
        }
    }

    if (sendNack) {
        MBMS_Nack(canRxMsg);
    }
}

static void CMD_SetCurrParam(CAN_MSG_t *canRxMsg) {
    if (gEngrMode == true) {
        gEngrTimeoutCount = CAN_ENGR_MODE_TIMEOUT;
        MBMS_SetGroupParam(CurrParamGroup, sizeof (CurrParamGroup) / sizeof (CurrParamGroup[0]), canRxMsg);
    } else {
        MBMS_Nack(canRxMsg);
    }
}

static void CMD_SetVoltParam(CAN_MSG_t *canRxMsg) {
    if (gEngrMode == true) {
        gEngrTimeoutCount = CAN_ENGR_MODE_TIMEOUT;
        MBMS_SetGroupParam(VoltParamGroup, sizeof (VoltParamGroup) / sizeof (VoltParamGroup[0]), canRxMsg);
    } else {
        MBMS_Nack(canRxMsg);
    }
}

static void CMD_SetTempParam(CAN_MSG_t *canRxMsg) {
    if (gEngrMode == true) {
        gEngrTimeoutCount = CAN_ENGR_MODE_TIMEOUT;
        MBMS_SetGroupParam(TempParamGroup, sizeof (TempParamGroup) / sizeof (TempParamGroup[0]), canRxMsg);
    } else {
        MBMS_Nack(canRxMsg);
    }
}

static void CMD_SetOtherParam(CAN_MSG_t *canRxMsg) {
    if (gEngrMode == true) {
        gEngrTimeoutCount = CAN_ENGR_MODE_TIMEOUT;
        MBMS_SetGroupParam(OtherParamGroup, sizeof (OtherParamGroup) / sizeof (OtherParamGroup[0]), canRxMsg);
    } else {
        MBMS_Nack(canRxMsg);
    }
}

static void CMD_SetCurrCalibParam(CAN_MSG_t *canRxMsg) {
    unsigned short cmd;

    if (gEngrMode == true) {
        gEngrTimeoutCount = CAN_ENGR_MODE_TIMEOUT;

        HIGH_BYTE(cmd) = canRxMsg->data[0];
        LOW_BYTE(cmd) = canRxMsg->data[1];

        switch (cmd) {
            case 0x3000:
                LOW_BYTE(eepSpe.AdcZeroOffset) = canRxMsg->data[2];
                HIGH_BYTE(eepSpe.AdcZeroOffset) = canRxMsg->data[3];

                CURRSNSR_ParamSet(eepSpe.AdcZeroOffset, eepSpe.AdcGainOffset);
                break;

            case 0x3010:
                LOW_BYTE(eepSpe.AdcGainOffset) = canRxMsg->data[2];
                HIGH_BYTE(eepSpe.AdcGainOffset) = canRxMsg->data[3];

                CURRSNSR_ParamSet(eepSpe.AdcZeroOffset, eepSpe.AdcGainOffset);
                break;

            default:
                // MBMS_Nack(canRxMsg) ;
                break;
        }
    }
}

static void CMD_RemoteCtrlRealy(CAN_MSG_t *canRxMsg) {
    if ((canRxMsg->data[4] == 0x12) && (canRxMsg->data[5] == 0x34)) {
        gMbmsRelayCmd = (BMS_WORK_MODE_e) canRxMsg->data[2];
    }

    gMbmsTimeoutCount = MBMS_COMM_TIMEOUT;
}

static void CMD_SetBalanceParam(CAN_MSG_t *canRxMsg) {
    bmsData.Balance.Mode = (BALANCE_MODE_e) canRxMsg->data[0];
    bmsData.Balance.VoltDiff = canRxMsg->data[1];
    LOW_BYTE(bmsData.Balance.Volt) = canRxMsg->data[2];
    HIGH_BYTE(bmsData.Balance.Volt) = canRxMsg->data[3];
}

static void CMD_EnterBootloader(CAN_MSG_t *canRxMsg) {
    if (gEngrMode == true) {
        gEngrMode = false;

        if ((canRxMsg->data[0] == 0x0A) && (canRxMsg->data[1] == 0x55)) {
            if (HV_StatusGet() == HV_OFF) {
                /* 填入0xAA，進入bootload程式段 */
                /*
                write_int_eep(EEP_BOOTLOAD, 0xAA) ;
                Reset() ;
                 */
                // set_bootload_2_eeprom();
                gMbmsRelayCmd = BMS_RESET;
            }
        }
    }
    canRxMsg->data[0] = 0x01;
    canRxMsg->data[1] = 0x1C;
    canRxMsg->data[2] = 0x00;
    MBMS_Nack(canRxMsg);
}

static void MBMS_DataGet(CAN_MSG_t *canRxMsg) {
    MBMS.pfCommand = canRxMsg->data[1];
    switch (MBMS.pfCommand) {
        case PF_GET_COMM_VER:
            PF_GetCommVer(canRxMsg);
            break;
        case PF_GET_SEED:
            PF_GetSeed(canRxMsg);
            break;
        case PF_GET_MFG_DATE:
            PF_GetMfgDate(canRxMsg);
            break;
        case PF_GET_PROD_SN:
            PF_GetProdSN(canRxMsg);
            break;
        case PF_GET_SW_HW_VER:
            PF_GetSwHwVer(canRxMsg);
            break;
        case PF_GET_CURR_GRP_PARAM:
            PF_GetCurrGrpParam(canRxMsg);
            break;
        case PF_GET_VOLT_GRP_PARAM:
            PF_GetVoltGrpParam(canRxMsg);
            break;
        case PF_GET_TEMP_GRP_PARAM:
            PF_GetTempGrpParam(canRxMsg);
            break;
        case PF_GET_OTHER_GRP_PARAM:
            PF_GetOtherGrpParam(canRxMsg);
            break;
        case PF_GET_CURR_CALIB_PARAM:
            PF_GetCurrCalibParam(canRxMsg);
            break;
        case PF_GET_SYS_PARAM_G1:
            PF_GetSystemParam(canRxMsg);
            break;
        case PF_GET_CELL_DATA_G1:
            PF_GetCellData(canRxMsg);
            break;
        case PF_GET_BALANCE_DATA:
            PF_GetBalanceData(canRxMsg);
            break;
        case PF_GET_COULOMB_GAUGE_DATA:
            PF_GetCoulombGaugeData(canRxMsg);
            break;
        case PF_GET_DTC_LOG_DATA:
            PF_GetLogData(canRxMsg);
            break;
        case PF_GET_DTC_FLAG:
            PF_GetDtcFlag(canRxMsg);
            break;
        default:
            MBMS_Nack(canRxMsg);
            break;
    }
}

void MBMS_CheckQueueTasks(CAN_MSG_t *canRxMsg) {
    if ((canRxMsg->dlc == J1939_DATA_LENGTH) && (canRxMsg->J1939.sourceAddress != J1939_GLOBAL_ADDRESS)) {
        MBMS.commad = canRxMsg->J1939.pduFormat;
    }
    switch (MBMS.commad) {
        case CMD_DATA_GET:
            MBMS_DataGet(canRxMsg);
            break;
        case CMD_SET_COMM_ADDR:
            CMD_SetCommAddr(canRxMsg);
            break;
        case CMD_SET_PROD_MFG_DATE:
            CMD_SetMfgDate(canRxMsg);
            break;
        case CMD_SET_PROD_SN1:
            CMD_SetSN1(canRxMsg);
            break;
        case CMD_SET_PROD_SN2:
            CMD_SetSN2(canRxMsg);
            break;
        case CMD_WR_PARAM_TO_EEPROM:
            CMD_WriteParam2Eeprom(canRxMsg);
            break;
        case CMD_EX_CTRL_CMD:
            CMD_ExcuteCtrlCmd(canRxMsg);
            break;
        case CMD_ENTER_ENGR_MODE:
            CMD_EnterEngrMode(canRxMsg);
            break;
        case CMD_EXT_TIME:
            CMD_ExtensionTime(canRxMsg);
            break;
        case CMD_SET_CURR_PARAM:
            CMD_SetCurrParam(canRxMsg);
            break;
        case CMD_SET_VOLT_PARAM:
            CMD_SetVoltParam(canRxMsg);
            break;
        case CMD_SET_TEMP_PARAM:
            CMD_SetTempParam(canRxMsg);
            break;
        case CMD_SET_OTHERS_PARAM:
            CMD_SetOtherParam(canRxMsg);
            break;
        case CMD_SET_CURR_CALIB_PARAM:
            CMD_SetCurrCalibParam(canRxMsg);
            break;
        case CMD_RMT_CTRL_RLY:
            CMD_RemoteCtrlRealy(canRxMsg);
            break;
        case CMD_SET_BAL_DATA_PARAM:
            CMD_SetBalanceParam(canRxMsg);
            break;
        case CMD_ENTER_BOOTLOADER:
            CMD_EnterBootloader(canRxMsg);
            break;
        default:
            MBMS_Nack(canRxMsg);
            break;
    }
}

BMS_WORK_MODE_e MBMS_RelayCommandGet(void) {
    BMS_WORK_MODE_e ret = BMS_OFF;
    if (gMbmsRelayCmd <= BMS_DISCHG_PRE_ON) {
        ret = gMbmsRelayCmd;
    }
    return ret;
}

bool MBMS_EngrModeStatusGet(void) {
    bool ret = false;
    ret = gEngrMode;
    return ret;
}

void MBMS_1ms_tasks(void) {
    if (gMbmsTimeoutCount) {
        gMbmsTimeoutCount--;
    } else {
        gMbmsTimeoutCount = 30000;
        DTC_FaultOccurSet(DTC_MBMS_COMM);
    }
    if (gEngrTimeoutCount) {
        gEngrTimeoutCount--;
    }
    seedRandom++;
}
/* USER CODE END 0 */
/*******************************************************************************
 End of File
 */
