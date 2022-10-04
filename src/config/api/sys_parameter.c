/**
 * @file       sys_parameter.c
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
#include "sys_parameter.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define EEPROM_EMG_SIZE sizeof(EEPROM_EMERGENCY_t)
#define EEPROM_SPE_SIZE sizeof(EEPROM_SPECIAL_t)
#define EEPROM_BMS_SIZE sizeof(EEPROM_BMS_t)

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Global variables -----------------------------------------------------------*/
/* USER CODE BEGIN GV */
volatile const EEPROM_BMS_t eepBmsDef = {
    EEP_KEY_ID,
    SVN_NUMBER,
 /***BMSParameterSetting***/
  /**Over Temperature**/
  /*Limit,Release,Limit Time,Release Time*/
    {    450,    430, 1000, 3000}, /* OTP */
    {    400,    380, 1000, 3000}, /* OTW */
    {    100,    120, 1000, 3000}, /* UTP */
    {    150,    170, 1000, 3000}, /* UTW */
    {    100,     50, 5000, 3000}, /* UBTW */

  /**OverVoltage**/
  /*Limit,Release,LimitTime,ReleaseTime*/
    { 806000, 800000, 3000, 3000}, /* OVP */
    { 795000, 790000, 3000, 3000}, /* OVW */
    { 560000, 575000, 3000, 3000}, /* UVP */
    { 570000, 590000, 3000, 3000}, /* UVW */
    {    500,    200, 2000, 5000}, /* CellUBP */
    {    350,    300, 2000, 5000}, /* CellUBW */
    {   3650,   3600, 2000, 5000}, /* CellOVP */
    {   2500,   2600, 2000, 5000}, /* CellUVP */

  /**OverCurrent**/
  /*Limit,Release,LimitTime,ReleaseTime*/
    {-120000,  -1000, 1000, 1000}, /* ODCP */
    { -80000,  -1000, 1000, 1000}, /* ODCW */
    {  80000,   1000, 1000, 1000}, /* OCCP */
    {  75000,   1000, 1000, 1000}, /* OCCW */

  /*OCCW*/
  /* OCPLockTime */
    (60000 / 100),

 /***BMS Capacity Record***/
    (78000 / 2), /* mAH,Batt FullCap */
    (0), /* SOH Decay Coefficient */
  /***BMS Factort Date***/
    26U, /* Factory Day */
    10U, /* Factory Month */
    2021UL, /* Factory Year */
    "0000000000000",
 // clang-format off
    {BAL_OFF, CELL_DESIGN_MAX_VOLT,   10     },
  // clang-format on
  /***Dummy***/
    0,
    0,
    0,
 /******/
    0, /* CheckSum */
};

volatile const EEPROM_EMERGENCY_t eepEmgDef = {
    .ChgCap = CELL_DESIGN_CAP, .CycleLife = 0, .DisChgCap = 0, .ErrorCode = {0}};
volatile const EEPROM_SPECIAL_t eepSpeDef = {.BmsAddr = BMS_COMM_ADDR, .AdcGainOffset = 0, .AdcZeroOffset = 0};


EEPROM_BMS_t       eepBms  = {};
EEPROM_EMERGENCY_t eepEmg  = {};
EEPROM_SPECIAL_t   eepSpe  = {};
BMS_DATA_t         bmsData = {};

/* USER CODE END GV */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Function prototypes -------------------------------------------------------*/
/* USER CODE BEGIN FP */

/* USER CODE END FP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*******************************************************************************
 End of File
 */
