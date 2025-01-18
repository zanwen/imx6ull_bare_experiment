//
// Created by 86157 on 2025/1/17.
//

#include "bsp_ov5640.h"
#include "driver_i2c.h"
#include "logger.h"
#include "driver_gpio.h"
#include "driver_delay.h"

#define RESET_PIN GPIO1, 2
#define POWER_DOWN_PIN GPIO1, 4

#define RESET_PIN_LOW() Driver_GPIO_WritePIn(RESET_PIN, GPIO_PIN_RESET)
#define RESET_PIN_HIGH() Driver_GPIO_WritePIn(RESET_PIN, GPIO_PIN_SET)

#define POWER_DOWN_PIN_LOW() Driver_GPIO_WritePIn(POWER_DOWN_PIN, GPIO_PIN_RESET)
#define POWER_DOWN_PIN_HIGH() Driver_GPIO_WritePIn(POWER_DOWN_PIN, GPIO_PIN_SET)

#define OV5640_ADDR_WRITE 0x3C
#define OV5640_ADDR_READ 0x3D

#define REG_CHIP_ID 0x300A


void Bsp_OV5640_PadInit(void) {
    /* SCL and SDA */
    // io mux
    IOMUXC_SetPinMux(IOMUXC_UART5_TX_DATA_I2C2_SCL, 1);
    IOMUXC_SetPinMux(IOMUXC_UART5_RX_DATA_I2C2_SDA, 1);
    // pad config open drain 拉低也需要输出驱动器
    IOMUXC_SetPinConfig(IOMUXC_UART5_TX_DATA_I2C2_SCL, 0x08B0);
    IOMUXC_SetPinConfig(IOMUXC_UART5_RX_DATA_I2C2_SDA, 0x08B0);

    /* Reset# and PowerDown */
    // io mux
    IOMUXC_SetPinMux(IOMUXC_GPIO1_IO02_GPIO1_IO02, 0);
    IOMUXC_SetPinMux(IOMUXC_GPIO1_IO04_GPIO1_IO04, 0);
    // gpio
    GPIO_Config_t config = {0};
    config.direction = GPIO_DIR_OUTPUT;
    config.defaultStatus = GPIO_PIN_SET;
    Driver_GPIO_Init(RESET_PIN, &config);
    config.defaultStatus = GPIO_PIN_RESET;
    Driver_GPIO_Init(POWER_DOWN_PIN, &config);
}

void Bsp_OV5640_HardReset(void) {
    RESET_PIN_LOW();
    Driver_Delay_MS(5);
    POWER_DOWN_PIN_LOW();
    Driver_Delay_MS(1);
    RESET_PIN_HIGH();
    Driver_Delay_MS(20);
}

void Bsp_OV5640_WriteBytes(uint16_t regAddr, uint8_t *data, uint8_t len) {
    uint8_t reg[2] = {regAddr >> 8, regAddr};
    Driver_I2C_Start(I2C2);
    Driver_I2C_SendAddr(I2C2, OV5640_ADDR_WRITE);
    Driver_I2C_SendBytes(I2C2, reg, 2);
    Driver_I2C_SendBytes(I2C2, data, len);
    Driver_I2C_Stop(I2C2);
}

void Bsp_OV5640_ReadBytes(uint16_t regAddr, uint8_t *buf, uint8_t len) {
    uint8_t reg[2] = {regAddr >> 8, regAddr};
    Driver_I2C_Start(I2C2);
    // dummy write
    Driver_I2C_SendAddr(I2C2, OV5640_ADDR_WRITE);
    Driver_I2C_SendBytes(I2C2, reg, 2);

    Driver_I2C_ReStart(I2C2);
    Driver_I2C_SendAddr(I2C2, OV5640_ADDR_READ);
    Driver_I2C_ReadBytesAndStop(I2C2, buf, len);
}

void Bsp_OV5640_Test(void) {
    uint8_t chipID[2] = {0};
    Bsp_OV5640_ReadBytes(REG_CHIP_ID, chipID, 2);
    LOG_DUMP("chip id", chipID, 2);
}

void Bsp_OV5640_Init(void) {
    Bsp_OV5640_PadInit();
    Bsp_OV5640_HardReset();
    Driver_I2C_Init(I2C2);
}

