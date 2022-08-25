/* ************************************************************************** */
/** Descriptive File Name

  @Company
    Company Name

  @File Name
    filename.c

  @Summary
    Brief description of the file.

  @Description
    Describe the purpose of this file.
 */
/* ************************************************************************** */
/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */

#include "sys_parameter.h"

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: File Scope or Global Data                                         */
/* ************************************************************************** */
/* ************************************************************************** */

DTC_FAULT_t FaultIndicator, FaultIndicatorMask;

/* ************************************************************************** */
/* ************************************************************************** */
// Section: Interface Functions                                               */
/* ************************************************************************** */
/* ************************************************************************** */

void Fault_ParameterInitialize(void) {
    FaultIndicator.l                  = 0;
    FaultIndicatorMask.l              = 0x1FFFFFFFF;
    FaultIndicatorMask.b.SHUT_DOWN    = 0;
    FaultIndicatorMask.b.EMERGENCY    = 0;
    FaultIndicatorMask.b.CURR_DIR_ERR = 0;
}

void Fault_EventSet(DTC_EVENT_e event) {
    FaultIndicator.l |= 1 << event;
}

void Fault_EventClear(DTC_EVENT_e event) {
    FaultIndicator.l &= ~(1 << event);
}

bool Fault_EventGet(DTC_EVENT_e event) {
    bool ret;
    ret = (FaultIndicator.l >> event) & 0x01;
    return ret;
}
/*******************************************************************************
 End of File
 */
