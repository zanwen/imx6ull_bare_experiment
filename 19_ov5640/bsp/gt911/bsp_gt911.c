//
// Created by 86157 on 2025/1/11.
//

#include "bsp_gt911.h"
#include "driver_delay.h"
#include "driver_gpio.h"
#include "driver_i2c.h"
#include "driver_interrupt.h"
#include "logger.h"

#define GT911_INT_GPIO_PIN GPIO1, 9
#define GT911_INT_GPIO_PIN_OUTPUT() GPIO1->GDIR |= (1 << 9)
#define GT911_INT_GPIO_PIN_INPUT() GPIO1->GDIR &= ~(1 << 9)
#define GT911_RST_GPIO_PIN GPIO5, 9
#define GT911_RST_GPIO_PIN_OUTPUT() GPIO5->GDIR |= (1 << 9)

#define GT911_INT_LOW() Driver_GPIO_WritePIn(GT911_INT_GPIO_PIN, GPIO_PIN_RESET)
#define GT911_INT_HIGH() Driver_GPIO_WritePIn(GT911_INT_GPIO_PIN, GPIO_PIN_SET)

#define GT911_RST_LOW() Driver_GPIO_WritePIn(GT911_RST_GPIO_PIN, GPIO_PIN_RESET)
#define GT911_RST_HIGH() Driver_GPIO_WritePIn(GT911_RST_GPIO_PIN, GPIO_PIN_SET)

#define GT911_ADDR_WRITE 0xBA
#define GT911_ADDR_READ 0xBB

#define GT911_REG_PRODUCT_ID 0x8140
#define GT911_REG_TOUCH_NUMBER 0x804C
#define GT911_REG_REAL_CMD 0x8040
#define GT911_REG_STATUS 0x814E
#define GT911_REG_INT_MODE 0x804D

void Bsp_GT911_WriteBytes(uint16_t regAddr, uint8_t *data, uint8_t len);
void Bsp_GT911_ReadBytes(uint16_t regAddr, uint8_t *buf, uint8_t len);
void Bsp_GT911_INTConfig(void);

uint8_t Bsp_GT911_ReadAndClearStatus(void);

void Bsp_GT911_INTCallback(uint32_t GICC_IAR, void *params) {
    LOG_DEBUG("Bsp_GT911_INTCallback invoke");
    // 清除中断标志位
    Driver_GPIO_ClearINTFlag(GT911_INT_GPIO_PIN);
    uint8_t status = Bsp_GT911_ReadAndClearStatus();
    LOG_DEBUG("read status = %#x", status);
}

uint8_t Bsp_GT911_ReadAndClearStatus(void) {
    uint8_t readStatus = 0, clearValue = 0;
    Bsp_GT911_ReadBytes(GT911_REG_STATUS, &readStatus, 1);
    Bsp_GT911_WriteBytes(GT911_REG_STATUS, &clearValue, 1);
    return readStatus;
}

void Bsp_GT911_INTConfig(void) {
    IOMUXC_SetPinConfig(IOMUXC_GPIO1_IO09_GPIO1_IO09, 0x3080);
    // 中断配置
    Driver_GPIO_ClearINTFlag(GT911_INT_GPIO_PIN);
    Driver_GPIO_ConfigINT(GT911_INT_GPIO_PIN, INT_TRI_RISING_EDGE);
    Driver_GPIO_EnableINT(GT911_INT_GPIO_PIN);
    // 注册中断回调
    Driver_INT_RegisterIRQHandler(GPIO1_Combined_0_15_IRQn, Bsp_GT911_INTCallback, NULL);
    // 使能GIC中断号
    GIC_EnableIRQ(GPIO1_Combined_0_15_IRQn);
}

void Bsp_GT911_RegConfig(void) {
    uint8_t regValue;
    // 触摸点个数上限
    regValue = 0x05;
    Bsp_GT911_WriteBytes(GT911_REG_TOUCH_NUMBER, &regValue, 1);
    // 实时命令 读取坐标
    regValue = 0x00;
    Bsp_GT911_WriteBytes(GT911_REG_REAL_CMD, &regValue, 1);
}

void Bsp_GT911_InitPin(void) {
    // SCL and SDA
    IOMUXC_SetPinMux(IOMUXC_UART5_TX_DATA_I2C2_SCL, 1);
    IOMUXC_SetPinMux(IOMUXC_UART5_RX_DATA_I2C2_SDA, 1);
    // open drain
    IOMUXC_SetPinConfig(IOMUXC_UART5_TX_DATA_I2C2_SCL, 0x18B0);
    IOMUXC_SetPinConfig(IOMUXC_UART5_RX_DATA_I2C2_SDA, 0x18B0);

    IOMUXC_SetPinMux(IOMUXC_GPIO1_IO09_GPIO1_IO09, 0);       // INT
    IOMUXC_SetPinMux(IOMUXC_SNVS_SNVS_TAMPER9_GPIO5_IO09, 0);// RESET
}

void Bsp_GT911_ResetAndSetAddr(void) {
    GT911_RST_GPIO_PIN_OUTPUT();
    GT911_INT_GPIO_PIN_OUTPUT();

    GT911_RST_LOW();
    GT911_INT_LOW();
    Driver_Delay_MS(10);// 复位延时

    // 设定地址时序
    GT911_INT_LOW();
    Driver_Delay_US(200);
    GT911_RST_HIGH();
    Driver_Delay_MS(10);
    GT911_INT_GPIO_PIN_INPUT();
}


void Bsp_GT911_Init(void) {
    Bsp_GT911_InitPin();
    Bsp_GT911_ResetAndSetAddr();
    Driver_I2C_Init(I2C2);
    Bsp_GT911_INTConfig();
    Bsp_GT911_RegConfig();
    LOG_INFO("Bsp_GT911_Init done");
}

void Bsp_GT911_WriteBytes(uint16_t regAddr, uint8_t *data, uint8_t len) {
    uint8_t reg[2] = {regAddr >> 8, regAddr & 0xFF};
    Driver_I2C_Start(I2C2);
    Driver_I2C_SendAddr(I2C2, GT911_ADDR_WRITE);
    Driver_I2C_SendBytes(I2C2, reg, 2);
    Driver_I2C_SendBytes(I2C2, data, len);
    Driver_I2C_Stop(I2C2);
}

void Bsp_GT911_ReadBytes(uint16_t regAddr, uint8_t *buf, uint8_t len) {
    uint8_t reg[2] = {regAddr >> 8, regAddr & 0xFF};
    Driver_I2C_Start(I2C2);
    // dummy write
    Driver_I2C_SendAddr(I2C2, GT911_ADDR_WRITE);
    Driver_I2C_SendBytes(I2C2, reg, 2);

    Driver_I2C_ReStart(I2C2);
    Driver_I2C_SendAddr(I2C2, GT911_ADDR_READ);
    Driver_I2C_ReadBytesAndStop(I2C2, buf, len);
}

void Bsp_GT911_Test(void) {
    uint8_t read = 0;
    //    uint8_t buf[4] = {0};
    //    Bsp_GT911_ReadBytes(GT911_REG_PRODUCT_ID, buf, 4);
    //    LOG_DEBUG("GT911 Read Product ID: GT%s", buf);

    read = 0;
    Bsp_GT911_ReadBytes(GT911_REG_INT_MODE, &read, 1);
    LOG_DEBUG("read int mode = %#x", read);

    read = 0;
    Bsp_GT911_ReadBytes(GT911_REG_STATUS, &read, 1);
    LOG_DEBUG("read status = %#x", read);

    LOG_DEBUG("GPIO1-9 dir = %#x, dr = %#x, psr = %#x, mux = %#x, pad = %#x, isr = %#x, icr1 = %#x",
              (GPIO1->GDIR >> 9) & 0x01,
              (GPIO1->DR >> 9) & 0x01,
              (GPIO1->PSR >> 9) & 0x01,
              *(uint32_t *) (0x020E0080),
              *(uint32_t *) (0x020E030C),
              (GPIO1->ISR >> 9) & 0x01,
              (GPIO1->ICR1 >> 18) & 0x03);

    LOG_DEBUG("GPIO5-9 dir = %#x, dr = %#x, psr = %#x, isr = %#x",
              (GPIO5->GDIR >> 9) & 0x01,
              (GPIO5->DR >> 9) & 0x01,
              (GPIO5->PSR >> 9) & 0x01,
              (GPIO5->ISR >> 9) & 0x01);
    //
    //    uint8_t status = 0;
    //    Bsp_GT911_ReadBytes(GT911_REG_STATUS, &status, 1);
    //    LOG_DEBUG("status = %#x", status);
}
