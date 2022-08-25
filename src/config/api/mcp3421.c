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

#include 
#include "sys_parameter.h"

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: File Scope or Global Data                                         */
/* ************************************************************************** */
/* ************************************************************************** */
APP_I2C_DATA_t   I2C0_data      = {0};
MCP3421_ADDR_t   mcp3421_addr   = {0};
MCP3421_CONFIG_t mcp3421_config = {0};
bool             b_adc_complete = 0;
unsigned char    I2C_TxBuffer[I2C_TX_BUFFER_SIZE];
unsigned char    I2C_RxBuffer[I2C_RX_BUFFER_SIZE];
/* ************************************************************************** */
/* ************************************************************************** */
// Section: Interface Functions                                               */
/* ************************************************************************** */
/* ************************************************************************** */
void MCP3421_WriteConfig(unsigned char addr, unsigned char config) {
    I2C0_data.txBuffer[0] = config;
    I2C_BB_Write(addr, I2C0_data.txBuffer, 1);
}

void MCP3421_ReadAdc(unsigned char addr) {
    I2C_BB_Read(addr, I2C0_data.rxBuffer, 3);
}

void MCP3421_Initialize(void) {
    mcp3421_addr.device_code = MCP3421_DEVICE_CDOE;
    mcp3421_addr.r_w         = READ;

    mcp3421_config.gain            = GAIN1;
    mcp3421_config.sampling_rate   = RATE16BIT;
    mcp3421_config.conversion_mode = CONTINUOUS_MODE;

    I2C0_data.taskState   = I2C_STATE_INIT;
    I2C0_data.txBuffer[0] = mcp3421_config.byte;

    MCP3421_WriteConfig(mcp3421_addr.byte >> 1, mcp3421_config.byte);
}

short MCP3421_AdcValueGet(void) {
    short adc_value = 0;
    if (I2C0_data.timeoutCount++ > 50) {
        I2C0_data.errorCount = SATURATION(I2C0_data.timeoutCount, 50, 0);
        I2C0_SCL_OutputEnable();
        I2C0_SDA_OutputEnable();
        I2C0_SDA_Clear();
        I2C0_SCL_Clear();
        I2C0_SDA_InputEnable();
        I2C0_SCL_InputEnable();
        I2C_BB_Initialize();
    }

    switch ((unsigned char)I2C0_data.taskState) {
        default:
        case I2C_STATE_INIT:
            MCP3421_Initialize();
            break;
        case I2C_STATE_READ_ADC:
            if (b_adc_complete == true) {
                b_adc_complete = false;
                adc_value      = (I2C0_data.rxBuffer[0] << 8) + I2C0_data.rxBuffer[1];
            }
            MCP3421_ReadAdc(mcp3421_addr.byte >> 1);
            break;
        case I2C_STATE_COMM_ERROR:
            break;
    }
    return adc_value;
}

void MCP3421_InterruptTasks(void) {
    I2C0_data.timeoutCount = 0;
    switch ((unsigned char)I2C_BB_ErrorGet()) {
        case I2CBB_ERROR_NONE:
            I2C0_data.errorCount = 0;
            if (I2C0_data.taskState == I2C_STATE_INIT) {
                I2C0_data.taskState = I2C_STATE_READ_ADC;
            }
            if (I2C0_data.taskState == I2C_STATE_READ_ADC) {
                b_adc_complete = true;
            }
            break;
        case I2CBB_ERROR_NAK:
            Nop();
        case I2CBB_ERROR_BUS:
            Nop();
        default:
            if (I2C0_data.errorCount++ >= 10) {
                I2C0_data.errorCount = SATURATION(I2C0_data.errorCount, 5, 0);
            }
            I2C0_data.taskState = I2C_STATE_INIT;
            break;
    }
}
