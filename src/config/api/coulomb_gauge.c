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


#include "commonly_used.h"
#include "current_gauge.h"
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

static bool          b_dischgCapFull = false;
static unsigned char g_cgTaskState   = 0;
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
        b_dischgCapFull = 1;
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
    } else if ((b_dischgCapFull == true) && (self->ChgCap > self->FullCap)) {
        b_dischgCapFull = false;
        self->ChgCap -= self->FullCap;
        (self->CycleLife)++;
    } else {
    }
}

static void Coulomb_Counter(BMS_DATA_t *self, unsigned int period_ms) {
    period_ms = 1000 / period_ms;

    if (self->BusCurrent > 0) {
        self->ChgCap = self->ChgCap + (self->BusCurrent / period_ms / 3600.0);
    } else {
        self->DischgCap = self->DischgCap + ((-1) * self->BusCurrent / period_ms / 3600.0);
    }
}

static void Update_Rem_Cap(BMS_DATA_t *self) {
    b_dischgCapFull = 0;
    self->RemCap    = self->FullCap;
    self->ChgCap    = self->RemCap;
    self->DischgCap = 0;
}

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
            self->DischgCap += (CELL_DESIGN_CAP * 0.004);
        }
    }

    if (self->MinVcell > CELL_DESIGN_MAX_VOLT) {
        if (self->SOC > 99) {
            Update_Rem_Cap(self);
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

void CoulombGauge_Initialize() {
    CoulombGauge_ParameterInitialize(&bmsData);
    SOH_Calculate(&bmsData);
    Batt_RemCap_Calculate(&bmsData);
    SOC_Calculate(&bmsData);
}

void CoulombGauge_Tasks() {
    Coulomb_Counter(&bmsData, 20);
    switch (g_cgTaskState) {
        case 0:
            OCV_Calibration_Point(&bmsData);
            g_cgTaskState++;
            break;
        case 1:
            Batt_RemCap_Calculate(&bmsData);
            Cycle_Life_Count(&bmsData);
            g_cgTaskState++;
            break;
        case 2:
            SOH_Calculate(&bmsData);
            SOH_Correct_Full_Cap(&bmsData);
            g_cgTaskState++;
            break;
        case 3:
            SOC_Calculate(&bmsData);
            g_cgTaskState++;
            break;
        default:
            g_cgTaskState = 0;
            break;
    }
}