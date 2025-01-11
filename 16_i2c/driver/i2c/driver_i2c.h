//
// Created by 86157 on 2025/1/10.
//

#ifndef INC_16_I2C_DRIVER_I2C_H
#define INC_16_I2C_DRIVER_I2C_H

#include "imx6ull.h"

#define DEV_ADDR 0x1E
void Driver_I2C_Init(I2C_Type *base);
void Driver_I2C_Start(I2C_Type *base);
void Driver_I2C_ReStart(I2C_Type *base);
void Driver_I2C_Stop(I2C_Type *base);
void Driver_I2C_SendAddr(I2C_Type *base, uint8_t addrRW);
void Driver_I2C_SendBytes(I2C_Type *base, uint8_t *data, uint8_t len);
void Driver_I2C_ReadBytesAndStop(I2C_Type *base, uint8_t *buf, uint8_t len);
void Driver_I2C_MasterWrite(I2C_Type *base, uint8_t addr, uint8_t regAddr,
                           uint8_t *data, uint8_t len);

void Driver_I2C_MasterRead(I2C_Type *base, uint8_t addr, uint8_t regAddr,
                           uint8_t *buf, uint8_t len);

void Driver_I2C_Test(void);

#endif//INC_16_I2C_DRIVER_I2C_H
