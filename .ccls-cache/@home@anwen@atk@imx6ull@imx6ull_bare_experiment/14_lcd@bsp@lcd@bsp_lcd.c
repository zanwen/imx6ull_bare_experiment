//
// Created by 86157 on 2025/1/7.
//

#include "bsp_lcd.h"
#include "driver_gpio.h"
#include "logger.h"

void Bsp_LCD_Init(void) {
    Bsp_LCD_ReadID();
}

LCD_Type_t Bsp_LCD_ReadID(void) {
    // 打开模拟开关 SGM_CTRL
    GPIO_Config_t config = {0};
    IOMUXC_SetPinMux(IOMUXC_LCD_VSYNC_GPIO3_IO03, 0);
    IOMUXC_SetPinConfig(IOMUXC_LCD_VSYNC_GPIO3_IO03, 0x10b0);
    config.direction = GPIO_DIR_OUTPUT;
    config.defaultStatus = GPIO_PIN_SET;
    Driver_GPIO_Init(GPIO3, 3, &config);

    // 读取LCD板子上三个引脚的状态（对应LCD-ID，标识了是正点原子哪一款LCD）
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA07_GPIO3_IO12, 0);
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA15_GPIO3_IO20, 0);
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA23_GPIO3_IO28, 0);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA07_GPIO3_IO12, 0x10b0);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA15_GPIO3_IO20, 0x10b0);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA23_GPIO3_IO28, 0x10b0);

    config.direction = GPIO_DIR_INPUT;
    Driver_GPIO_Init(GPIO3, 12, &config);
    Driver_GPIO_Init(GPIO3, 20, &config);
    Driver_GPIO_Init(GPIO3, 28, &config);

    GPIO_Pin_Status_t m0 = Driver_GPIO_ReadPin(GPIO3, 12);
    GPIO_Pin_Status_t m1 = Driver_GPIO_ReadPin(GPIO3, 20);
    GPIO_Pin_Status_t m2 = Driver_GPIO_ReadPin(GPIO3, 28);
    uint8_t lcdID = m0 | (m1 << 1) | (m2 << 2);
    if (lcdID == 0x2) {
        LOG_DEBUG("LCD Type: LCD_7_INCH_1024x600");
        return LCD_7_INCH_1024x600;
    }
    return LCD_TYPE_UNKNOWN;
}