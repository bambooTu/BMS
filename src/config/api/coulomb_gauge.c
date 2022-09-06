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

static bool          fDischgCapFull = false;
static unsigned char gCgTaskState   = 0;
/* ************************************************************************** */
/* ************************************************************************** */
// Section: Interface Functions                                               */
/* ************************************************************************** */
/* ************************************************************************** */

static void SOH_Calculate(BMS_DATA_t *self) {
    unsigned short Decay = 0;

    Decay     = self->CycleLife + self->DecayCoefficient;
    self->SOH = (unsigned char)Lookup_Table(Decay, Decay2SOH_BP, Decay2SOH_Table, DECAY2SOH_TABLE_SIZE);
}

static void SOC_Calculate(BMS_DATA_t *self) {
    self->SOC = ((self->RemCap) * 100) / (self->FullCap);
    if (self->SOC > 99) {
        self->SOC = 100;
    }
}

static void Batt_RemCap_Calculate(BMS_DATA_t *self) {
    if (self->ChgCap > self->DischgCap) {
        self->RemCap = self->ChgCap - self->DischgCap;
        if (self->RemCap > self->FullCap) {
            self->RemCap = self->FullCap;
        }
    } else {
        fDischgCapFull  = 1;
        self->RemCap    = 1;
        self->ChgCap    = 0;
        self->DischgCap = 0;
    }
}

static void Cycle_Life_Count(BMS_DATA_t *self) {
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

static void Coulomb_Counter(BMS_DATA_t *self, unsigned int period_ms) {
    period_ms = 1000 / period_ms;

    if (self->BusCurrent > 0) {
        self->ChgCap = self->ChgCap + (self->BusCurrent / period_ms / 3600.0f);
    } else {
        self->DischgCap = self->DischgCap + ((-1) * self->BusCurrent / period_ms / 3600.0f);
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
static void OCV_Calibration_Point(BMS_DATA_t *self) {
    if (self->BusCurrent > 0) {
        if ((self->MinVcell > 3420UL) && (self->SOC < 80)) {
            self->ChgCap += (CELL_DESIGN_CAP * (0.001f));
        } else if ((self->MinVcell > 3454UL) && (self->SOC < 90)) {
            self->ChgCap += (CELL_DESIGN_CAP * (0.002f));
        } else {
        }
    } else {
        if ((self->MinVcell < 3055UL) && (self->SOC > 5)) {
            self->DischgCap += (CELL_DESIGN_CAP * 0.004f);
        }
    }

    if (self->MinVcell > CELL_DESIGN_MAX_VOLT) {
        if (self->SOC > 99) {
            fDischgCapFull  = 0;
            self->RemCap    = self->FullCap;
            self->ChgCap    = self->RemCap;
            self->DischgCap = 0;
        } else if (self->BusCurrent > 0) {
            self->ChgCap += (CELL_DESIGN_CAP * 0.005);
        } else {
        }
    }
}

static void SOH_Correct_Full_Cap(BMS_DATA_t *self) {
    self->FullCap = Lookup_Table(self->SOH, SOH2FullCap_BP, SOH2FullCap_Table, SOH2SFULLCAP_TABLE_SIZE);
}

inline static void CoulombGauge_ParameterInitialize(BMS_DATA_t *self) {
    self->ChgCap           = eepEmg.ChgCap;
    self->CycleLife        = eepEmg.CycleLife;
    self->DischgCap        = eepEmg.DisChgCap;
    self->FullCap          = eepBms.FullCap;
    self->DecayCoefficient = eepBms.DecayCoefficient;
}

void CoulombGauge_Initialize(BMS_DATA_t *self) {
    CoulombGauge_ParameterInitialize(self);
    SOH_Calculate(self);
    Batt_RemCap_Calculate(self);
    SOC_Calculate(self);
}

void CoulombGauge_Tasks(BMS_DATA_t *self) {
    Coulomb_Counter(self, 20);
    switch (gCgTaskState) {
        case 0:
            OCV_Calibration_Point(self);
            gCgTaskState++;
            break;
        case 1:
            Cycle_Life_Count(self);
            Batt_RemCap_Calculate(self);
            gCgTaskState++;
            break;
        case 2:
            SOH_Calculate(self);
            SOH_Correct_Full_Cap(self);
            gCgTaskState++;
            break;
        case 3:
            SOC_Calculate(self);
            gCgTaskState++;
            break;
        default:
            gCgTaskState = 0;
            break;
    }
}