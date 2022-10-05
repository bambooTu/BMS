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
#include "coulomb_gauge.h"

#include "commonly_used.h"
#include "sys_parameter.h"
/* ************************************************************************** */
/* ************************************************************************** */
/* Section: File Scope or Global Data                                         */
/* ************************************************************************** */
/* ************************************************************************** */
volatile const double Decay2SOH_BP[DECAY2SOH_TABLE_SIZE]    = {0, 1000, 3000, 4200, 4400, 4500, 4600, 4800, 5000};
volatile const double Decay2SOH_Table[DECAY2SOH_TABLE_SIZE] = {100, 90, 80, 70, 60, 50, 40, 30, 0};

volatile const double SOH2FullCap_BP[SOH2SFULLCAP_TABLE_SIZE]    = {0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
volatile const double SOH2FullCap_Table[SOH2SFULLCAP_TABLE_SIZE] = {0,
                                                                    (CELL_DESIGN_CAP * 0.1),
                                                                    (CELL_DESIGN_CAP * 0.2),
                                                                    (CELL_DESIGN_CAP * 0.3),
                                                                    (CELL_DESIGN_CAP * 0.4),
                                                                    (CELL_DESIGN_CAP * 0.5),
                                                                    (CELL_DESIGN_CAP * 0.6),
                                                                    (CELL_DESIGN_CAP * 0.7),
                                                                    (CELL_DESIGN_CAP * 0.8),
                                                                    (CELL_DESIGN_CAP * 0.9),
                                                                    CELL_DESIGN_CAP};

volatile const double OCV_BP[SOH2SFULLCAP_TABLE_SIZE] = {
    3350, 3426, 3510, 3567, 3607, 3643, 3694, 3782, 3888, 4000, CELL_DESIGN_MAX_VOLT};
volatile const double OCV_Table[SOH2SFULLCAP_TABLE_SIZE] = {0,
                                                            (CELL_DESIGN_CAP * 0.1),
                                                            (CELL_DESIGN_CAP * 0.2),
                                                            (CELL_DESIGN_CAP * 0.3),
                                                            (CELL_DESIGN_CAP * 0.4),
                                                            (CELL_DESIGN_CAP * 0.5),
                                                            (CELL_DESIGN_CAP * 0.6),
                                                            (CELL_DESIGN_CAP * 0.7),
                                                            (CELL_DESIGN_CAP * 0.8),
                                                            (CELL_DESIGN_CAP * 0.9),
                                                            CELL_DESIGN_CAP};

static bool          fDischgCapFull  = false;
static unsigned char gCgTaskState    = 0;
static unsigned char gSelfDischgCurr = 0;
/* ************************************************************************** */
/* ************************************************************************** */
// Section: Interface Functions                                               */
/* ************************************************************************** */
/* ************************************************************************** */
/**
 * @brief      Calculate state of health
 *
 * @param      self
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-09-13
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
static void Calculate_SOH(BMS_DATA_t *self) {
    unsigned short Decay = 0;
    Decay                = self->CycleLife + self->DecayCoefficient;
    self->SOH            = (unsigned char)Lookup_Table(Decay, Decay2SOH_BP, Decay2SOH_Table, DECAY2SOH_TABLE_SIZE);
}
/**
 * @brief      Calculate state of charge
 *
 * @param      self BMS data
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-09-13
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
static void Calculate_SOC(BMS_DATA_t *self) {
    self->SOC = ((self->RemCap) * 100) / (self->FullCap);
    if (self->SOC > 99) {
        self->SOC = 100;
    }
}
/**
 * @brief      Calculate battery remaining capacity
 *
 * @param      self BMS data
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-09-13
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
static void Calculate_BattRemCap(BMS_DATA_t *self) {
    if (self->ChgCap > self->DischgCap) {
        self->RemCap = self->ChgCap - self->DischgCap;
        if (self->RemCap > self->FullCap) {
            self->RemCap = self->FullCap;
        }
    } else {
        fDischgCapFull  = true;
        self->RemCap    = 1;
        self->ChgCap    = 0;
        self->DischgCap = 0;
    }
}
/**
 * @brief      Calculate cycleLife
 *
 * @param      self BMS data
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-09-13
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
static void Calculate_CycleLife(BMS_DATA_t *self) {
    if ((self->ChgCap > self->FullCap) && (self->DischgCap > self->FullCap)) {
        self->ChgCap -= self->FullCap;
        self->DischgCap -= self->FullCap;
        (self->CycleLife)++;
    } else if ((fDischgCapFull == true) && (self->ChgCap > self->FullCap)) {
        fDischgCapFull = false;
        self->ChgCap -= self->FullCap;
        (self->CycleLife)++;
    } else {
    }
}
/**
 * @brief      Coulomb counter
 *
 * @param      self BMS data
 * @param      period_ms execution period
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-09-13
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
static void CG_CoulombCounter(BMS_DATA_t *self, unsigned int period_ms) {
    period_ms = 1000 / period_ms;

    if (self->BusCurrent > 0) {  // Charging
        self->ChgCap = self->ChgCap + (self->BusCurrent / period_ms / 3600.0f);
    } else if (self->BusCurrent < 0) {  // Discharging
        self->DischgCap = self->DischgCap + ((-1) * self->BusCurrent / period_ms / 3600.0f);
    }

    if (++gSelfDischgCurr > (3600 / 10 * period_ms)) {  // Self-Discharge
        gSelfDischgCurr = 0;
        self->DischgCap++;
    }
}
// static void OCV_CorrectDecayCoefficient(BMS_DATA_t *self) {
//     unsigned int OcvCap, DiffCap = 0;
//     OcvCap  = Lookup_Table(self->MinVcell, OCV_BP, OCV_Table, OCV_TABLE_SIZE);
//     DiffCap = ABS(OcvCap - self->RemCap);
//     if (DiffCap > (CELL_DESIGN_CAP * 0.1)) {
//         self->DecayCoefficient += 50;
//     } else if (DiffCap > (CELL_DESIGN_CAP * 0.05)) {
//         self->DecayCoefficient += 20;
//     } else {
//         self->DecayCoefficient += 2;
//     }
// }
/**
 * @brief      OCV point calibrate charge/discharge capacity
 *
 * @param      self BMS data
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-09-13
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
static void Calibration_OcvPoint(BMS_DATA_t *self) {
    if (self->BusCurrent > 0) {  // Charging
        if ((self->MinVcell > 3420UL) && (self->SOC < 80)) {
            self->ChgCap += (CELL_DESIGN_CAP * (0.0001f));
        } else if ((self->MinVcell > 3454UL) && (self->SOC < 90)) {
            self->ChgCap += (CELL_DESIGN_CAP * (0.0002f));
        } else {
        }
    } else if (self->BusCurrent < 0) {  // Discharging
        if ((self->MinVcell < 3055UL) && (self->SOC > 5)) {
            self->DischgCap += (CELL_DESIGN_CAP * 0.0004f);
        }
    }

    if (self->MinVcell > CELL_DESIGN_MAX_VOLT) {
        if (self->SOC > 99) {
            fDischgCapFull  = false;
            self->RemCap    = self->FullCap;
            self->ChgCap    = self->RemCap;
            self->DischgCap = 0;
        } else if (self->BusCurrent > 0) {
            self->ChgCap += (CELL_DESIGN_CAP * 0.0005f);
        } else {
        }
    }
}
/**
 * @brief      SOH calibrate full capacity
 *
 * @param      self BMS data
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-09-13
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
static void Calibration_SOH2FullCap(BMS_DATA_t *self) {
    self->FullCap = Lookup_Table(self->SOH, SOH2FullCap_BP, SOH2FullCap_Table, SOH2SFULLCAP_TABLE_SIZE);
}
/**
 * @brief      Update remaining capacity
 *
 * @param      self BMS data
 * @param      newRemCap New remaining capacity
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-09-13
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
void CoulombGauge_UpdateRemCap(BMS_DATA_t *self, unsigned int newRemCap) {
    fDischgCapFull  = false;
    self->RemCap    = newRemCap;
    self->ChgCap    = self->RemCap;
    self->DischgCap = 0;
}
/**
 * @brief      Coulomb gauge parameter initialize
 *
 * @param      self BMS data
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-09-13
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
static void CG_ParameterInitialize(BMS_DATA_t *self) {
    self->ChgCap           = eepEmg.ChgCap;
    self->CycleLife        = eepEmg.CycleLife;
    self->DischgCap        = eepEmg.DisChgCap;
    self->FullCap          = eepBms.FullCap;
    self->DecayCoefficient = eepBms.DecayCoefficient;
}
/**
 * @brief      Coulomb gauge initialize
 *
 * @param      self BMS data
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-09-13
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
void CG_Initialize(BMS_DATA_t *self) {
    CG_ParameterInitialize(self);
    Calculate_SOH(self);
    Calculate_BattRemCap(self);
    Calculate_SOC(self);
}
/**
 * @brief      Coulomb gauge polling tasks
 *
 * @param      self  BMS data
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-09-13
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
void CG_20ms_Tasks(BMS_DATA_t *self) {
    switch (gCgTaskState) {
        case 0:
            CG_CoulombCounter(self, 100);
            gCgTaskState++;
            break;
        case 1:
            Calibration_OcvPoint(self);
            gCgTaskState++;
            break;
        case 2:
            Calculate_CycleLife(self);
            Calculate_BattRemCap(self);
            gCgTaskState++;
            break;
        case 3:
            Calculate_SOH(self);
            Calibration_SOH2FullCap(self);
            gCgTaskState++;
            break;
        default:
            Calculate_SOC(self);
            gCgTaskState = 0;
            break;
    }
}