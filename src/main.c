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

#include "config/api/sys_parameter.h"
#include "definitions.h"  // SYS function prototypes

#define CALCULTAE_TIME_MS(A) ((A < 10) ? 10 - 1 : 10 * A - 1)

// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
// *****************************************************************************

struct {
    unsigned _1ms   : 1;
    unsigned _5ms   : 1;
    unsigned _500ms : 1;
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
unsigned int  cnt_1ms, cnt_5ms, cnt_500ms = 0;
bool          polar = false;
unsigned char step  = 0;

// *****************************************************************************
// *****************************************************************************
// Section: Interrupt Handler
// *****************************************************************************
// *****************************************************************************

void TMR4_EvnetHandler(uint32_t status, uintptr_t context) {
    if (cnt_1ms++ >= CALCULTAE_TIME_MS(1)) {
        cnt_1ms      = 0;
        tmrData._1ms = true;
    }
    if (cnt_5ms++ >= CALCULTAE_TIME_MS(5)) {
        cnt_5ms      = 0;
        tmrData._5ms = true;
    }
    if (cnt_500ms++ >= CALCULTAE_TIME_MS(500)) {
        cnt_500ms      = 0;
        tmrData._500ms = true;
    }
    X2CScope_Update();
}
// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************

int main(void) {
    /* Initialize all modules */
    SYS_Initialize(NULL);
    eepBms = eepBmsDef;
    eepSpe = eepSpeDef;
    while (true) {
        /* Maintain state machines of all polled MPLAB Harmony modules. */
        SYS_Tasks();
        X2CScope_Communicate();

        switch ((APP_STATUS_e)appData.state) {
            case APP_EEPROM_READ:
                appData.state = APP_STATE_INIT;
                break;
            case APP_STATE_INIT:
                TMR4_CallbackRegister(TMR4_EvnetHandler, (uintptr_t)NULL);
                TMR4_Start();
                DIN_ParameterInitialize();
                HV_Initialize();
                BMU_Initialize();
                CAN_Initialize();
                appData.state = APP_STATE_SERVICE_TASKS;
                break;
            case APP_STATE_SERVICE_TASKS:
                if (tmrData._1ms) {
                    tmrData._1ms = false;
                    HV_1ms_Tasks();
                    //DTC_1ms_Tasks();
                    BMU_1ms_Tasks();
                    BMS_1ms_Tasks();
                    switch (step) {
                        case 0:
                            if (DIN_StateGet(DIN_2) == true) {
                                step++;
                            }
                            break;
                        case 1:
                            if (DIN_StateGet(DIN_2) == false) {
                                step++;
                            }
                            break;
                        case 2:
                            polar = !polar;
                            if (polar) {
                                BMS_ModeCommand(BMS_DISCHG_ON);
                            } else {
                                BMS_ModeCommand(BMS_OFF);
                            }
                            step = 0;
                        default:
                            step = 0;
                            break;
                    }
                }

                if (tmrData._5ms) {
                    tmrData._5ms = false;
                    DIN_5ms_Tasks();
                    CAN_QueueDataXfer(CAN_1);
                }
                if (tmrData._500ms) {
                    tmrData._500ms = false;
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
