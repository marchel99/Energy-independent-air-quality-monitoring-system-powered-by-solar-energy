/** 
****************************************************************************
* Copyright (C) 2024
File : ens160.c
Author : Marchel99
****************************************************************************
**/

#include "ens160.h"
#include "main.h"
#include <stdio.h>
#include <string.h>

// Definicje brakujących symboli
#define eINTDataDrdyEN 0x02
#define eIntGprDrdyDIS 0x00
extern UART_HandleTypeDef huart2;
DFRobot_ENS160_I2C ens160;

void DFRobot_ENS160_I2C_Init(DFRobot_ENS160_I2C *instance, I2C_HandleTypeDef *hi2c, uint8_t i2cAddr)
{
    instance->hi2c = hi2c;
    instance->i2cAddr = (i2cAddr != 0) ? i2cAddr : 0x53; // Ustaw domyślny adres, jeśli nie podano
    instance->misr = 0;                                  // Mirror of DATA_MISR (0 is hardware default)
}

int DFRobot_ENS160_I2C_Begin(DFRobot_ENS160_I2C *instance)
{
    uint8_t idBuf[2];
    HAL_StatusTypeDef status;

    // Odczyt identyfikatora urządzenia
    status = HAL_I2C_Mem_Read(instance->hi2c, instance->i2cAddr << 1, ENS160_PART_ID_REG, I2C_MEMADD_SIZE_8BIT, idBuf, sizeof(idBuf), HAL_MAX_DELAY);
    if (status != HAL_OK) {
        char buffer[50];
        snprintf(buffer, sizeof(buffer), "I2C read error: %d\r\n", status);
        HAL_UART_Transmit(&huart2, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);
        return ERR_DATA_BUS;
    }

    uint16_t part_id = ENS160_CONCAT_BYTES(idBuf[1], idBuf[0]);
    char buffer[50];
    snprintf(buffer, sizeof(buffer), "Part ID read: 0x%04X\r\n", part_id);
    HAL_UART_Transmit(&huart2, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);

    if (ENS160_PART_ID != part_id) {
        snprintf(buffer, sizeof(buffer), "Part ID mismatch: 0x%04X\r\n", part_id);
        HAL_UART_Transmit(&huart2, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);
        return ERR_IC_VERSION;
    }

    DFRobot_ENS160_SetPWRMode(instance, ENS160_STANDARD_MODE);
    DFRobot_ENS160_SetINTMode(instance, 0x00);
    return NO_ERR;
}

void DFRobot_ENS160_SetPWRMode(DFRobot_ENS160_I2C *instance, uint8_t mode)
{
    HAL_I2C_Mem_Write(instance->hi2c, instance->i2cAddr << 1, ENS160_OPMODE_REG, I2C_MEMADD_SIZE_8BIT, &mode, 1, HAL_MAX_DELAY);
    HAL_Delay(20); // Daj czas na przełączenie trybu
}

void DFRobot_ENS160_SetINTMode(DFRobot_ENS160_I2C *instance, uint8_t mode)
{
    mode |= (eINTDataDrdyEN | eIntGprDrdyDIS);
    HAL_I2C_Mem_Write(instance->hi2c, instance->i2cAddr << 1, ENS160_CONFIG_REG, I2C_MEMADD_SIZE_8BIT, &mode, 1, HAL_MAX_DELAY);
    HAL_Delay(20); // Daj czas na przełączenie trybu
}

uint8_t DFRobot_ENS160_GetStatus(DFRobot_ENS160_I2C *instance)
{
    uint8_t status;
    HAL_I2C_Mem_Read(instance->hi2c, instance->i2cAddr << 1, ENS160_DATA_STATUS_REG, I2C_MEMADD_SIZE_8BIT, &status, 1, HAL_MAX_DELAY);
    return status;
}

uint8_t DFRobot_ENS160_GetAQI(DFRobot_ENS160_I2C *instance)
{
    uint8_t aqi;
    HAL_I2C_Mem_Read(instance->hi2c, instance->i2cAddr << 1, ENS160_DATA_AQI_REG, I2C_MEMADD_SIZE_8BIT, &aqi, 1, HAL_MAX_DELAY);
    return aqi;
}

uint16_t DFRobot_ENS160_GetTVOC(DFRobot_ENS160_I2C *instance)
{
    uint8_t buf[2];
    HAL_I2C_Mem_Read(instance->hi2c, instance->i2cAddr << 1, ENS160_DATA_TVOC_REG, I2C_MEMADD_SIZE_8BIT, buf, 2, HAL_MAX_DELAY);
    return ENS160_CONCAT_BYTES(buf[1], buf[0]);
}

uint16_t DFRobot_ENS160_GetECO2(DFRobot_ENS160_I2C *instance)
{
    uint8_t buf[2];
    HAL_I2C_Mem_Read(instance->hi2c, instance->i2cAddr << 1, ENS160_DATA_ECO2_REG, I2C_MEMADD_SIZE_8BIT, buf, 2, HAL_MAX_DELAY);
    return ENS160_CONCAT_BYTES(buf[1], buf[0]);
}

void DFRobot_ENS160_SetTempAndHum(DFRobot_ENS160_I2C *instance, float ambientTemp, float relativeHumidity)
{
    uint16_t temp = (ambientTemp + 273.15) * 64;
    uint16_t rh = relativeHumidity * 512;
    uint8_t buf[4];

    buf[0] = temp & 0xFF;
    buf[1] = (temp & 0xFF00) >> 8;
    buf[2] = rh & 0xFF;
    buf[3] = (rh & 0xFF00) >> 8;
    HAL_I2C_Mem_Write(instance->hi2c, instance->i2cAddr << 1, ENS160_TEMP_IN_REG, I2C_MEMADD_SIZE_8BIT, buf, 4, HAL_MAX_DELAY);
}


void read_and_print_ens160_data(void)
{
  uint8_t aqi = DFRobot_ENS160_GetAQI(&ens160);
  uint16_t tvoc = DFRobot_ENS160_GetTVOC(&ens160);
  uint16_t eco2 = DFRobot_ENS160_GetECO2(&ens160);

  printf("AQI: %d, TVOC: %d ppb, eCO2: %d ppm\n", aqi, tvoc, eco2);
}