/** 
****************************************************************************
* Copyright (C) 2024
File : ens160.h
Author : Marchel99
****************************************************************************
**/

#ifndef ENS160_H
#define ENS160_H

#include "stm32l4xx_hal.h"

// Definicje trybów pracy ENS160
#define ENS160_SLEEP_MODE 0x00
#define ENS160_IDLE_MODE 0x01
#define ENS160_STANDARD_MODE 0x02

#define ENS160_PART_ID 0x0160
#define ENS160_PART_ID_REG 0x00
#define ENS160_OPMODE_REG 0x10
#define ENS160_CONFIG_REG 0x11
#define ENS160_DATA_STATUS_REG 0x20
#define ENS160_DATA_AQI_REG 0x21
#define ENS160_DATA_TVOC_REG 0x22
#define ENS160_DATA_ECO2_REG 0x24
#define ENS160_TEMP_IN_REG 0x13

#define ENS160_CONCAT_BYTES(msb, lsb) (((uint16_t)msb << 8) | (uint16_t)lsb)

#define NO_ERR 0
#define ERR_DATA_BUS -1
#define ERR_IC_VERSION -2

typedef struct {
    I2C_HandleTypeDef *hi2c;
    uint8_t i2cAddr;
    uint8_t misr;
} DFRobot_ENS160_I2C;

void DFRobot_ENS160_I2C_Init(DFRobot_ENS160_I2C *instance, I2C_HandleTypeDef *hi2c, uint8_t i2cAddr);
int DFRobot_ENS160_I2C_Begin(DFRobot_ENS160_I2C *instance);
void DFRobot_ENS160_SetPWRMode(DFRobot_ENS160_I2C *instance, uint8_t mode);
void DFRobot_ENS160_SetINTMode(DFRobot_ENS160_I2C *instance, uint8_t mode);
uint8_t DFRobot_ENS160_GetStatus(DFRobot_ENS160_I2C *instance);
uint8_t DFRobot_ENS160_GetAQI(DFRobot_ENS160_I2C *instance);
uint16_t DFRobot_ENS160_GetTVOC(DFRobot_ENS160_I2C *instance);
uint16_t DFRobot_ENS160_GetECO2(DFRobot_ENS160_I2C *instance);
void DFRobot_ENS160_SetTempAndHum(DFRobot_ENS160_I2C *instance, float ambientTemp, float relativeHumidity);
void read_and_print_ens160_data(void);
#endif
