/**
 * @file       current_sensor.c
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @brief      
 * @version    0.1
 * @date       2022-10-24
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
#include "current_sensor.h"
#include "commonly_used.h"
#include "sys_parameter.h"
#include "mcp3421.h"
#include "csnv500.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define CURR_SNSR_TYPE          1             // 0:MCP3421 1:CSNV500
#define CURR_SNSR_TIMEOUT_TIMES 10            // unit:10ms
#define IP_VALUE_OFFSET         0x80000000UL  // ref. CSNV500 datasheet
#define ADC_COEFFICIENT         19966L
#define MOVING_AVG_TIMES        12L
#define ZERO_THRESHOLD          0L
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Global variables -----------------------------------------------------------*/
/* USER CODE BEGIN GV */
int          gCurrentSensorGain    = 1;
/* USER CODE END GV */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
static short adcZeroOffset = 0;  // convert error value
static short adcGainOffset = 0;  // caculate error value
/* USER CODE END PV */

/* Function prototypes -------------------------------------------------------*/
/* USER CODE BEGIN FP */

/* USER CODE END FP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/**
 * @brief      Transfer the MCP3421's ADC value to milli Ampere
 *
 * @param      adcValue
 * @return     int
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-09-06
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
static int CURRSNSR_AdcValueToCurr(int adcValue) {
    int ret;
    int gain;
    /* Adjustment */
    adcValue += adcZeroOffset;
    /*                        EEPROM  */
    gain = ADC_COEFFICIENT + adcGainOffset;
    ret  = (adcValue * gain) / 1000;

    return ret;
}
/**
 * @brief      Sensor Type 0: Get the MCP3421 ADC value and do a moving average, then convert the value to the current
 * value (mA). Sensor Type 1: Get the CSNVC500 current value(mA) after doing the moving average.
 *
 * @param      arr
 * @param      arrSize
 * @return     int
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-09-06
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
static inline int CURRSNSR_ProcessedCurrGet(void) {
    int                  ret = 0;
    static double        Buffer[MOVING_AVG_TIMES];
    static unsigned char Index = 0;

#if (CURR_SNSR_TYPE == 0)
    Buffer[adcIndex] = MCP3421_AdcValueGet();
    if (++Index > MOVING_AVG_TIMES - 1) {
        Index = 0;
    }
    ret = (int)Filter_ArrayAverage(Buffer, MOVING_AVG_TIMES);
    ret = CURRSNSR_AdcValueToCurr(ret);
#elif (CURR_SNSR_TYPE == 1)

    Buffer[Index] = CSNV500_CurrGet();
    if (++Index > MOVING_AVG_TIMES - 1) {
        Index = 0;
    }
    ret = (int)Filter_ArrayAverage(Buffer, MOVING_AVG_TIMES);
#endif
    Filter_Deadzone(ret, ZERO_THRESHOLD);
    return ret;
}

/**
 * @brief      Set the current sensor's parameter.
 *
 * @param      gainOffset
 * @param      zeroOffset
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-09-06
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
void CURRSNSR_ParamSet(short gainOffset, short zeroOffset) {
    adcGainOffset = SATURATION(gainOffset, 1000, -1000);
    adcZeroOffset = SATURATION(zeroOffset, 1000, -1000);
}
/**
 * @brief      Initialize the current sensor's parameter after EEPROM read done.
 *
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-09-06
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
void CURRSNSR_Intialize(void) {
    CURRSNSR_ParamSet(eepSpe.AdcGainOffset, eepSpe.AdcZeroOffset);
#if (CURR_SNSR_TYPE == 0)
    MCP3421_Initialize();
#elif (CURR_SNSR_TYPE == 1)
    CSNV500_Initialize();
#endif
}
/**
 * @brief      Sensor Type 0: Get the MCP3421 ADC value and then transfer the value to current value(mA).
 *             Sensor Type 1: Get the CSNVC500 current value(mA).
 *             The polling period is 10ms.
 * @version    0.1
 * @author     Tu (Bamboo.Tu@amitatech.com)
 * @date       2022-09-06
 * @copyright  Copyright (c) 2022 Amita Technologies Inc.
 */
void CURRSNSR_10ms_Tasks(void) {
    bmsData.BusCurrent = CURRSNSR_ProcessedCurrGet() * gCurrentSensorGain;
}
/* USER CODE END 0 */

/*******************************************************************************
 End of File
 */
