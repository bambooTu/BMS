/**
 * @file       coulomb_gauge.c
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @brief
 * @version    0.1
 * @date       2022-11-09
 *
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 *
 * Abbreviation:
 * SOC          State Of Charge
 * SOH          State Of Health
 * FCC          Fully Charge Capacity
 * OCV          Open Circuit Voltage
 */
/* Global define -------------------------------------------------------------*/
/* USER CODE BEGIN GD */

/* USER CODE END GD */

/* Includes ------------------------------------------------------------------*/

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "coulomb_gauge.h"

#include "commonly_used.h"
#include "sys_parameter.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Global variables -----------------------------------------------------------*/
/* USER CODE BEGIN GV */
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
/* USER CODE END GV */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
static bool          fDischgCapFull = false;
static unsigned char cgTaskState    = 0;
/* USER CODE END PV */

/* Function prototypes -------------------------------------------------------*/
/* USER CODE BEGIN FP */

/* USER CODE END FP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
    period_ms                      = 3600 * 1000 / period_ms;
    static float selfDischgCurrent = 0;
    if (self->BusCurrent > 0) {  // Charging
        self->ChgCap += (self->BusCurrent / (float)period_ms);
    } else if (self->BusCurrent < 0) {  // Discharging
        self->DischgCap += ((-1) * self->BusCurrent / (float)period_ms);
    }
    selfDischgCurrent += (SELF_DISCHG_CURRENT / (float)period_ms);
    self->DischgCap += (SELF_DISCHG_CURRENT / (float)period_ms);
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
        if ((self->MinVcell > 3948UL) && (self->SOC < 85)) {
            self->ChgCap += (CELL_DESIGN_CAP * (0.0002f));
        } else if ((self->MinVcell > 4086UL) && (self->SOC < 95)) {
            self->ChgCap += (CELL_DESIGN_CAP * (0.0005f));
        } else {
        }
    } else if (self->BusCurrent < 0) {  // Discharging
        if ((self->MinVcell < 3468UL) && (self->SOC > 5)) {
            self->DischgCap += (CELL_DESIGN_CAP * 0.0005f);
        }
    }

    if (self->MinVcell > CELL_DESIGN_MAX_VOLT) {
        if (self->SOC > 99) {
            fDischgCapFull  = false;
            self->RemCap    = self->FullCap;
            self->ChgCap    = self->RemCap;
            self->DischgCap = 0;
        } else if (self->BusCurrent > 0) {
            self->ChgCap += (CELL_DESIGN_CAP * 0.001f);
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
    switch (cgTaskState) {
        case 0:
            CG_CoulombCounter(self, 100);
            cgTaskState++;
            break;
        case 1:
            Calibration_OcvPoint(self);
            cgTaskState++;
            break;
        case 2:
            Calculate_CycleLife(self);
            Calculate_BattRemCap(self);
            cgTaskState++;
            break;
        case 3:
            Calculate_SOH(self);
            Calibration_SOH2FullCap(self);
            cgTaskState++;
            break;
        default:
            Calculate_SOC(self);
            cgTaskState = 0;
            break;
    }
}
/* USER CODE END 0 */

/*******************************************************************************
 End of File
 */
