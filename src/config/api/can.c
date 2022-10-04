/**
 * @file       can.c
 * @author     Chiou (charlie.chiou@amitatech.com)
 * @brief
 * @version    0.1
 * @date       2022-08-18
 *
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 *
 * Abbreviation:
 * None
 */

/* Includes ------------------------------------------------------------------*/

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "can.h"

#include "sys_parameter.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef bool (*CAN_MESSAGE_RECEIVE)(uint32_t* id, uint8_t* length, uint8_t* data, uint32_t* timestamp, uint8_t fifoNum,
                                    CANFD_MSG_RX_ATTRIBUTE* msgAttr);

typedef bool (*CAN_MESSAGE_TRANSMIT)(uint32_t id, uint8_t length, uint8_t* data, uint8_t fifoQueueNum, CANFD_MODE mode,
                                     CANFD_MSG_TX_ATTRIBUTE msgAttr);

typedef bool (*CAN_TX_FIFOQUEUE_IS_Full)(uint8_t fifoQueueNum);

typedef CANFD_ERROR (*CAN_ERROR_GET)(void);
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

union {
    struct {
        uint8_t _0;
        uint8_t _1;
        uint8_t _2;
        uint8_t _3;
        uint8_t _4;
        uint8_t _5;
        uint8_t _6;
        uint8_t _7;
    } u8;
    uint64_t u64;
} Data;

const CAN_MESSAGE_RECEIVE CANx_MessageReceive[CAN_NUMBER_OF_MODULE] = {
    CAN1_MessageReceive,
    CAN2_MessageReceive,
    CAN3_MessageReceive,
    CAN4_MessageReceive,
};
const CAN_MESSAGE_TRANSMIT CANx_MessageTransmit[CAN_NUMBER_OF_MODULE] = {
    CAN1_MessageTransmit,
    CAN2_MessageTransmit,
    CAN3_MessageTransmit,
    CAN4_MessageTransmit,
};
const CAN_TX_FIFOQUEUE_IS_Full CANx_TxFIFOQueueIsFull[CAN_NUMBER_OF_MODULE] = {
    CAN1_TxFIFOQueueIsFull,
    CAN2_TxFIFOQueueIsFull,
    CAN3_TxFIFOQueueIsFull,
    CAN4_TxFIFOQueueIsFull,
};

const CAN_ERROR_GET CANx_ErrorGet[CAN_NUMBER_OF_MODULE] = {
    CAN1_ErrorGet,
    CAN2_ErrorGet,
    CAN3_ErrorGet,
    CAN4_ErrorGet,
};

// CAN Variable
static CAN_MSG_t    txQueue[CAN_NUMBER_OF_MODULE][CAN_QUEUE_SIZE], rxQueue[CAN_NUMBER_OF_MODULE][CAN_QUEUE_SIZE];
static CAN_MSG_t    recvMessage[CAN_NUMBER_OF_MODULE] = {};
static can_object_t Object[CAN_NUMBER_OF_MODULE]      = {};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/**
 * @brief      CAN1 Interrupt Callback
 *
 * @param      contextHandle CAN event enumeration
 * @version    0.1
 * @author     Chiou (charlie.chiou@amitatech.com)
 * @date       2022-08-18
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
static void CAN1_Callback(uintptr_t contextHandle) {
    CAN_MODULE_e Instance = CAN_1;
    switch (contextHandle) {
        case CAN_EVENT_RX:
            CANx_MessageReceive[Instance](&recvMessage[Instance].id, &recvMessage[Instance].dlc,
                                          recvMessage[Instance].data, &recvMessage[Instance].timestamp, CAN_FIFONUM_RX0,
                                          &recvMessage[Instance].msgAttr);
            CAN_PushRxQueue(Instance, &recvMessage[Instance]);
            GLED_Toggle();
            break;
        case CAN_EVENT_ERR:
            Object[Instance].errorCount++;
            Object[Instance].errorStatus = CANx_ErrorGet[Instance]();
            switch (Object[Instance].errorStatus) {
                case CANFD_ERROR_TX_RX_WARNING_STATE:
                    Nop();
                    break;
                case CANFD_ERROR_RX_WARNING_STATE:
                    Nop();
                    break;
                case CANFD_ERROR_TX_WARNING_STATE:
                    Nop();
                    break;
                case CANFD_ERROR_RX_BUS_PASSIVE_STATE:
                    Nop();
                    break;
                case CANFD_ERROR_TX_BUS_PASSIVE_STATE:
                    Nop();
                    break;
                case CANFD_ERROR_TX_BUS_OFF_STATE:
                    Nop();
                    break;

                default:
                    break;
            }
            Nop();
            break;
        case CAN_EVENT_TX:
        default:
            break;
    }
}

/**
 * @brief      CAN2 Interrupt Callback
 *
 * @param      contextHandle CAN event enumeration
 * @version    0.1
 * @author     Chiou (charlie.chiou@amitatech.com)
 * @date       2022-08-18
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
static void CAN2_Callback(uintptr_t contextHandle) {
    CAN_MODULE_e Instance = CAN_2;
    switch (contextHandle) {
        case CAN_EVENT_RX:
            CANx_MessageReceive[Instance](&recvMessage[Instance].id, &recvMessage[Instance].dlc,
                                          recvMessage[Instance].data, &recvMessage[Instance].timestamp, CAN_FIFONUM_RX0,
                                          &recvMessage[Instance].msgAttr);
            CAN_PushRxQueue(Instance, &recvMessage[Instance]);
            GLED_Toggle();
            Nop();
            break;
        case CAN_EVENT_ERR:
            Object[Instance].errorCount++;
            Object[Instance].errorStatus = CANx_ErrorGet[Instance]();
            switch (Object[Instance].errorStatus) {
                case CANFD_ERROR_TX_RX_WARNING_STATE:
                    Nop();
                    break;
                case CANFD_ERROR_RX_WARNING_STATE:
                    Nop();
                    break;
                case CANFD_ERROR_TX_WARNING_STATE:
                    Nop();
                    break;
                case CANFD_ERROR_RX_BUS_PASSIVE_STATE:
                    Nop();
                    break;
                case CANFD_ERROR_TX_BUS_PASSIVE_STATE:
                    Nop();
                    break;
                case CANFD_ERROR_TX_BUS_OFF_STATE:
                    Nop();
                    break;

                default:
                    break;
            }
            Nop();
            break;
        case CAN_EVENT_TX:
        default:
            break;
    }
}

/**
 * @brief      CAN3 Interrupt Callback
 *
 * @param      contextHandle CAN event enumeration
 * @version    0.1
 * @author     Chiou (charlie.chiou@amitatech.com)
 * @date       2022-08-18
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
static void CAN3_Callback(uintptr_t contextHandle) {
    CAN_MODULE_e Instance = CAN_3;
    switch (contextHandle) {
        case CAN_EVENT_RX:
            CANx_MessageReceive[Instance](&recvMessage[Instance].id, &recvMessage[Instance].dlc,
                                          recvMessage[Instance].data, &recvMessage[Instance].timestamp, CAN_FIFONUM_RX0,
                                          &recvMessage[Instance].msgAttr);
            CAN_PushRxQueue(Instance, &recvMessage[Instance]);
            GLED_Toggle();
            break;
        case CAN_EVENT_ERR:
            Object[Instance].errorCount++;
            Object[Instance].errorStatus = CANx_ErrorGet[Instance]();
            switch (Object[Instance].errorStatus) {
                case CANFD_ERROR_TX_RX_WARNING_STATE:
                    Nop();
                    break;
                case CANFD_ERROR_RX_WARNING_STATE:
                    Nop();
                    break;
                case CANFD_ERROR_TX_WARNING_STATE:
                    Nop();
                    break;
                case CANFD_ERROR_RX_BUS_PASSIVE_STATE:
                    Nop();
                    break;
                case CANFD_ERROR_TX_BUS_PASSIVE_STATE:
                    Nop();
                    break;
                case CANFD_ERROR_TX_BUS_OFF_STATE:
                    Nop();
                    break;

                default:
                    break;
            }
            Nop();
            break;
        case CAN_EVENT_TX:
        default:
            break;
    }
}

/**
 * @brief      CAN4 Interrupt Callback
 *
 * @param      contextHandle CAN event enumeration
 * @version    0.1
 * @author     Chiou (charlie.chiou@amitatech.com)
 * @date       2022-08-18
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
static void CAN4_Callback(uintptr_t contextHandle) {
    CAN_MODULE_e Instance = CAN_4;
    switch (contextHandle) {
        case CAN_EVENT_RX:
            CANx_MessageReceive[Instance](&recvMessage[Instance].id, &recvMessage[Instance].dlc,
                                          recvMessage[Instance].data, &recvMessage[Instance].timestamp, CAN_FIFONUM_RX0,
                                          &recvMessage[Instance].msgAttr);
            CAN_PushRxQueue(Instance, &recvMessage[Instance]);
            Nop();
            break;
        case CAN_EVENT_ERR:
            Object[Instance].errorCount++;
            Object[Instance].errorStatus = CANx_ErrorGet[Instance]();
            switch (Object[Instance].errorStatus) {
                case CANFD_ERROR_TX_RX_WARNING_STATE:
                    Nop();
                    break;
                case CANFD_ERROR_RX_WARNING_STATE:
                    Nop();
                    break;
                case CANFD_ERROR_TX_WARNING_STATE:
                    Nop();
                    break;
                case CANFD_ERROR_RX_BUS_PASSIVE_STATE:
                    Nop();
                    break;
                case CANFD_ERROR_TX_BUS_PASSIVE_STATE:
                    Nop();
                    break;
                case CANFD_ERROR_TX_BUS_OFF_STATE:
                    Nop();
                    break;

                default:
                    break;
            }
            Nop();
            break;
        case CAN_EVENT_TX:
        default:
            break;
    }
}

/**
 * @brief      Initialize the queue
 *
 * @param      Instance CAN Module number
 * @version    0.1
 * @author     Chiou (charlie.chiou@amitatech.com)
 * @date       2022-08-18
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
static void CAN_InitializeQueue(CAN_MODULE_e Instance) {
    /*CAN Queue Initialize*/
    Object[Instance].txQueue.pHead          = txQueue[Instance];
    Object[Instance].txQueue.pTail          = txQueue[Instance];
    Object[Instance].txQueue.Status.b.empty = true;
    Object[Instance].txQueue.Status.b.full  = false;
    Object[Instance].txQueue.Count          = 0;

    Object[Instance].rxQueue.pHead          = rxQueue[Instance];
    Object[Instance].rxQueue.pTail          = rxQueue[Instance];
    Object[Instance].rxQueue.Status.b.empty = true;
    Object[Instance].rxQueue.Status.b.full  = false;
    Object[Instance].rxQueue.Count          = 0;
}

/**
 * @brief      Create a receive message link and initialize the queue
 *
 * @version    0.1
 * @author     Chiou (charlie.chiou@amitatech.com)
 * @date       2022-08-18
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
void CAN_Initialize(void) {
    // CAN_MSG_t settingMessage = {};

    CAN1_CallbackRegister(CAN1_Callback, (uintptr_t)CAN_EVENT_RX, CAN_FIFONUM_RX0);
    CAN1_ErrorCallbackRegister(CAN1_Callback, (uintptr_t)CAN_EVENT_ERR);
    CAN2_CallbackRegister(CAN2_Callback, (uintptr_t)CAN_EVENT_RX, CAN_FIFONUM_RX0);
    CAN2_ErrorCallbackRegister(CAN2_Callback, (uintptr_t)CAN_EVENT_ERR);
    CAN3_CallbackRegister(CAN3_Callback, (uintptr_t)CAN_EVENT_RX, CAN_FIFONUM_RX0);
    CAN3_ErrorCallbackRegister(CAN3_Callback, (uintptr_t)CAN_EVENT_ERR);
    CAN4_CallbackRegister(CAN4_Callback, (uintptr_t)CAN_EVENT_RX, CAN_FIFONUM_RX0);
    CAN4_ErrorCallbackRegister(CAN4_Callback, (uintptr_t)CAN_EVENT_ERR);
    // TODO(Chiou): Only filter for BMS communication protocol address, can be removed if you want accept all messages

    for (CAN_MODULE_e Instance = CAN_1; Instance < CAN_NUMBER_OF_MODULE; Instance++) {
        CAN_InitializeQueue(Instance);
        CANx_MessageReceive[Instance](&recvMessage[Instance].id, &recvMessage[Instance].dlc, recvMessage[Instance].data,
                                      &recvMessage[Instance].timestamp, CAN_FIFONUM_RX0,
                                      &recvMessage[Instance].msgAttr);
    }
    /* Code begin */
    // CAN2_MessageAcceptanceFilterSet(1,0x18ff4520);
    // CAN2_MessageAcceptanceFilterMaskSet(1,0x7FFF0000);
    /* Code end */
}

/**
 * @brief      Push message to txQueue
 *
 * @param      Instance CAN Module number
 * @param      txMessage The address of the message pulled from the queue
 * @return     uint8_t
 * @version    0.1
 * @author     Chiou (charlie.chiou@amitatech.com)
 * @date       2022-08-18
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
uint8_t CAN_PushTxQueue(CAN_MODULE_e Instance, CAN_MSG_t* txMessage) {
    // check if there is space in the queue
    if (Object[Instance].txQueue.Status.b.full == false) {
        *Object[Instance].txQueue.pTail = *txMessage;
        Object[Instance].txQueue.pTail++;
        Object[Instance].txQueue.Count++;
        // check if the end of the array is reached
        if (Object[Instance].txQueue.pTail == (txQueue[Instance] + CAN_QUEUE_SIZE)) {
            // adjust to restart at the beginning of the array
            Object[Instance].txQueue.pTail = txQueue[Instance];
        }
        // since we added one item to be processed, we know
        // it is not empty, so set the empty status to false
        Object[Instance].txQueue.Status.b.empty = false;
        // check if full
        if (Object[Instance].txQueue.pHead == Object[Instance].txQueue.pTail) {
            // it is full, set the full status to true
            Object[Instance].txQueue.Status.b.full = true;
        }
    }
    return (Object[Instance].txQueue.Status.b.full);
}

/**
 * @brief      Pull messages from txQueue
 *
 * @param      Instance CAN Module number
 * @param      txMessage The address of the message pulled from the queue
 * @return     uint8_t
 * @version    0.1
 * @author     Chiou (charlie.chiou@amitatech.com)
 * @date       2022-08-18
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
uint8_t CAN_PullTxQueue(CAN_MODULE_e Instance, CAN_MSG_t* txMessage) {
    if (Object[Instance].txQueue.Status.b.empty != true) {
        *txMessage = *Object[Instance].txQueue.pHead;
        Object[Instance].txQueue.pHead++;
        Object[Instance].txQueue.Count--;
        // check if the end of the array is reached
        if (Object[Instance].txQueue.pHead == (txQueue[Instance] + CAN_QUEUE_SIZE)) {
            // adjust to restart at the beginning of the array
            Object[Instance].txQueue.pHead = txQueue[Instance];
        }
        // since we moved one item to be processed, we know
        // it is not full, so set the full status to false
        Object[Instance].txQueue.Status.b.full = false;
        // check if the queue is empty
        if (Object[Instance].txQueue.pHead == Object[Instance].txQueue.pTail) {
            // it is empty so set the empty status to true
            Object[Instance].txQueue.Status.b.empty = true;
        }
    }
    return (Object[Instance].txQueue.Status.b.empty);
}

/**
 * @brief      Push message to rxQueue
 *
 * @param      Instance CAN Module number
 * @param      rxMessage The address of the received message The address of the
 * message pushed into the queue
 * @return     uint8_t
 * @version    0.1
 * @author     Chiou (charlie.chiou@amitatech.com)
 * @date       2022-08-18
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
uint8_t CAN_PushRxQueue(CAN_MODULE_e Instance, CAN_MSG_t* rxMessage) {
    // check if there is space in the queue
    if (Object[Instance].rxQueue.Status.b.full == false) {
        *Object[Instance].rxQueue.pTail = *rxMessage;
        Object[Instance].rxQueue.pTail++;
        Object[Instance].rxQueue.Count++;
        // check if the end of the array is reached
        if (Object[Instance].rxQueue.pTail == (rxQueue[Instance] + CAN_QUEUE_SIZE)) {
            // adjust to restart at the beginning of the array
            Object[Instance].rxQueue.pTail = rxQueue[Instance];
        }
        // since we added one item to be processed, we know
        // it is not empty, so set the empty status to false
        Object[Instance].rxQueue.Status.b.empty = false;
        // check if full
        if (Object[Instance].rxQueue.pHead == Object[Instance].rxQueue.pTail) {
            // it is full, set the full status to true
            Object[Instance].rxQueue.Status.b.full = true;
        }
    }
    return (Object[Instance].rxQueue.Status.b.full);
}

/**
 * @brief      Pull messages from rxQueue
 *
 * @param      Instance CAN Module number
 * @param      rxMessage The address of the received message The address of the
 * message pulled from the queue
 * @return     uint8_t
 * @version    0.1
 * @author     Chiou (charlie.chiou@amitatech.com)
 * @date       2022-08-18
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
uint8_t CAN_PullRxQueue(CAN_MODULE_e Instance, CAN_MSG_t* rxMessage) {
    if (Object[Instance].rxQueue.Status.b.empty != true) {
        *rxMessage = *Object[Instance].rxQueue.pHead;
        Object[Instance].rxQueue.pHead++;
        Object[Instance].rxQueue.Count--;
        // check if the end of the array is reached
        if (Object[Instance].rxQueue.pHead == (rxQueue[Instance] + CAN_QUEUE_SIZE)) {
            // adjust to restart at the beginning of the array
            Object[Instance].rxQueue.pHead = rxQueue[Instance];
        }
        // since we moved one item to be processed, we know
        // it is not full, so set the full status to false
        Object[Instance].rxQueue.Status.b.full = false;
        // check if the queue is empty
        if (Object[Instance].rxQueue.pHead == Object[Instance].rxQueue.pTail) {
            // it is empty so set the empty status to true
            Object[Instance].rxQueue.Status.b.empty = true;
        }
    }
    return (Object[Instance].rxQueue.Status.b.empty);
}

/**
 * @brief      Get txQueue count value
 *
 * @param      Instance CAN Module number
 * @return     uint32_t
 * @version    0.1
 * @author     Chiou (charlie.chiou@amitatech.com)
 * @date       2022-08-18
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
uint32_t CAN_GetTxQueueCount(CAN_MODULE_e Instance) {
    return (Object[Instance].txQueue.Count);
}

/**
 * @brief      Get rxQueue count value
 *
 * @param      Instance CAN Module number
 * @return     uint32_t
 * @version    0.1
 * @author     Chiou (charlie.chiou@amitatech.com)
 * @date       2022-08-18
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
uint32_t CAN_GetRxQueueCount(CAN_MODULE_e Instance) {
    return (Object[Instance].rxQueue.Count);
}

bool CAN_QueueDataXfer(CAN_MODULE_e Instance) {
    bool      ret = false;
    CAN_MSG_t canTxMsg;
    if (!CANx_TxFIFOQueueIsFull[Instance](CAN_FIFONUM_TX0)) {
        if (CAN_GetTxQueueCount(Instance) > 0) {
            CAN_PullTxQueue(Instance, &canTxMsg);
            ret = CANx_MessageTransmit[Instance](canTxMsg.id, canTxMsg.dlc, canTxMsg.data, CAN_FIFONUM_TX0,
                                                 CANFD_MODE_NORMAL, CANFD_MSG_TX_DATA_FRAME);
        }
    }
    return ret;
}

/* USER CODE END 0 */
/*******************************************************************************
 End of File
 */
