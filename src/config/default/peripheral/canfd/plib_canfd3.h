/*******************************************************************************
  CANFD Peripheral Library Interface Header File

  Company:
    Microchip Technology Inc.

  File Name:
    plib_canfd3.h

  Summary:
    CANFD PLIB interface declarations.

  Description:
    The CANFD plib provides a simple interface to manage the CANFD modules on
    Microchip microcontrollers. This file defines the interface declarations
    for the CANFD plib.

  Remarks:
    None.

*******************************************************************************/
//DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2019 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*******************************************************************************/
//DOM-IGNORE-END

#ifndef PLIB_CANFD3_H
#define PLIB_CANFD3_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

/*
 * This section lists the other files that are included in this file.
 */
#include <stdbool.h>
#include <string.h>
#include "device.h"
#include "plib_canfd_common.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility
    extern "C" {
#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Interface Routines
// *****************************************************************************
// *****************************************************************************
void CAN3_Initialize(void);
bool CAN3_MessageTransmit(uint32_t id, uint8_t length, uint8_t* data, uint8_t fifoQueueNum, CANFD_MODE mode, CANFD_MSG_TX_ATTRIBUTE msgAttr);
bool CAN3_MessageReceive(uint32_t *id, uint8_t *length, uint8_t *data, uint32_t *timestamp, uint8_t fifoNum, CANFD_MSG_RX_ATTRIBUTE *msgAttr);
void CAN3_MessageAbort(uint8_t fifoQueueNum);
void CAN3_MessageAcceptanceFilterSet(uint8_t filterNum, uint32_t id);
uint32_t CAN3_MessageAcceptanceFilterGet(uint8_t filterNum);
void CAN3_MessageAcceptanceFilterMaskSet(uint8_t acceptanceFilterMaskNum, uint32_t id);
uint32_t CAN3_MessageAcceptanceFilterMaskGet(uint8_t acceptanceFilterMaskNum);
CANFD_ERROR CAN3_ErrorGet(void);
void CAN3_ErrorCountGet(uint8_t *txErrorCount, uint8_t *rxErrorCount);
bool CAN3_InterruptGet(uint8_t fifoQueueNum, CANFD_FIFO_INTERRUPT_FLAG_MASK fifoInterruptFlagMask);
bool CAN3_TxFIFOQueueIsFull(uint8_t fifoQueueNum);
bool CAN3_AutoRTRResponseSet(uint32_t id, uint8_t length, uint8_t* data, uint8_t fifoNum);
void CAN3_CallbackRegister(CANFD_CALLBACK callback, uintptr_t contextHandle, uint8_t fifoQueueNum);
void CAN3_ErrorCallbackRegister(CANFD_CALLBACK callback, uintptr_t contextHandle);
// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility
    }
#endif
// DOM-IGNORE-END

#endif // PLIB_CANFD3_H