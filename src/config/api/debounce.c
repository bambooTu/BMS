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

#define RISING  0
#define FALLING 1
unsigned int     g_DebounceTimeCount[2][DIN_MAPPING_MAX];
DIN_PARAM_t      g_DinParamTable[DIN_MAPPING_MAX];
DIN_TASK_TABLE_t g_DinTaskTable[] = {
    {          DIN_1,           &g_DinParamTable[DIN_1]},
    {          DIN_2,           &g_DinParamTable[DIN_2]},
    {          DIN_3,           &g_DinParamTable[DIN_3]},
    {          DIN_4,           &g_DinParamTable[DIN_4]},
    {DIN_MAPPING_MAX, &g_DinParamTable[DIN_MAPPING_MAX]},
};

/* ************************************************************************** */
/* ************************************************************************** */
// Section: Local Functions                                                   */
/* ************************************************************************** */
/* ************************************************************************** */

static void DIN_SetState(DIN_MAPPING_e dinNum) {
    g_DinParamTable[dinNum].status = true;
}

static void DIN_ClearState(DIN_MAPPING_e dinNum) {
    g_DinParamTable[dinNum].status = false;
}

static bool DIN_PinGet(DIN_MAPPING_e dinNum) {
    bool ret = false;
    switch (dinNum) {
        case DIN_1:
            ret = !SW1_Get();
            break;
        case DIN_2:
            ret = !SW2_Get();
            break;
        case DIN_3:
            ret = false;
            break;
        case DIN_4:
            ret = !SW3_Get();
            break;
        case DIN_MAPPING_MAX:
            ret = false;
            break;
        default:
            ret = false;
            break;
    }
    return ret;
}

static void DIN_Debounce(DIN_TASK_TABLE_t* ptrObj) {
    if (DIN_PinGet(ptrObj->dinNum) == true) {
        if (g_DebounceTimeCount[RISING][ptrObj->dinNum]-- == 0) {
            DIN_SetState(ptrObj->dinNum);
            g_DebounceTimeCount[RISING][ptrObj->dinNum] = ptrObj->dinParam->debounceTime[RISING];
        }
        g_DebounceTimeCount[FALLING][ptrObj->dinNum] = ptrObj->dinParam->debounceTime[FALLING];
    } else {
        if (g_DebounceTimeCount[FALLING][ptrObj->dinNum]-- == 0) {
            DIN_ClearState(ptrObj->dinNum);
            g_DebounceTimeCount[FALLING][ptrObj->dinNum] = ptrObj->dinParam->debounceTime[FALLING];
        }
        g_DebounceTimeCount[RISING][ptrObj->dinNum] = ptrObj->dinParam->debounceTime[RISING];
    }
}

/* ************************************************************************** */
/* ************************************************************************** */
// Section: Interface Functions                                               */
/* ************************************************************************** */
/* ************************************************************************** */

void DIN_ParameterInitialize(void) {
    for (DIN_MAPPING_e i = DIN_1; i < DIN_MAPPING_MAX; i++) {
        g_DinParamTable[i].debounceTime[RISING] = 4;
        g_DinParamTable[i].debounceTime[FALLING] = 4;
    }
}

void DIN_5ms_Tasks(void) {
    for (DIN_MAPPING_e i = DIN_1; i < DIN_MAPPING_MAX; i++) {
        DIN_Debounce(&g_DinTaskTable[i]);
    }
}

bool DIN_StateGet(DIN_MAPPING_e dinNum) {
    return g_DinParamTable[dinNum].status;
}
/* *****************************************************************************
 End of File
 */
