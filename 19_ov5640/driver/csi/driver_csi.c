//
// Created by 86157 on 2025/1/18.
//

#include "driver_csi.h"
#include "bsp_lcd.h"
#include "logger.h"


void Driver_CSI_IOInit(void) {
    IOMUXC_SetPinMux(IOMUXC_CSI_PIXCLK_CSI_PIXCLK, 0);
    IOMUXC_SetPinMux(IOMUXC_CSI_VSYNC_CSI_VSYNC, 0);
    IOMUXC_SetPinMux(IOMUXC_CSI_HSYNC_CSI_HSYNC, 0);

    IOMUXC_SetPinMux(IOMUXC_CSI_DATA00_CSI_DATA02, 0);
    IOMUXC_SetPinMux(IOMUXC_CSI_DATA01_CSI_DATA03, 0);
    IOMUXC_SetPinMux(IOMUXC_CSI_DATA02_CSI_DATA04, 0);
    IOMUXC_SetPinMux(IOMUXC_CSI_DATA03_CSI_DATA05, 0);
    IOMUXC_SetPinMux(IOMUXC_CSI_DATA04_CSI_DATA06, 0);
    IOMUXC_SetPinMux(IOMUXC_CSI_DATA05_CSI_DATA07, 0);
    IOMUXC_SetPinMux(IOMUXC_CSI_DATA06_CSI_DATA08, 0);
    IOMUXC_SetPinMux(IOMUXC_CSI_DATA07_CSI_DATA09, 0);
}

void Driver_CSI_ControllerInit(CSI_Config_t *config) {
    uint16_t widthBytes = config->width * config->bytesPerPixel;
    CSI->CSICR1 = (1 << 1) | (1 << 4) | (1 << 7) | (1 << 11) | (1 << 30);
    CSI->CSIIMAG_PARA = (widthBytes << 16) | config->height;
    CSI->CSICR2 |= 3 << 30;
    CSI->CSICR3 |= (1 << 0) | (3 << 4);
    CSI->CSIDMASA_FB1 = config->frameBufferAddr;
    CSI->CSIDMASA_FB2 = config->frameBufferAddr;
}

void Driver_CSI_DMA_ReflashRFF(void) {
    CSI->CSICR3 |= 1 << 14;
    while (CSI->CSICR3 & (1 << 14));
}

void Driver_CSI_Start(void) {
    CSI->CSICR3 |= (1 << 12);
    CSI->CSICR18 |= (1 << 31);
}

void Driver_CSI_Stop(void) {
    CSI->CSICR3 &= ~(1 << 12);
    CSI->CSICR18 &= ~(1 << 31);
}

void Driver_CSI_Init(void) {
    CSI_Config_t config = {
            1024,
            600,
            2,
            LCD_FRAMEBUF_RGB565_ADDR,
    };
    Driver_CSI_IOInit();
    Driver_CSI_ControllerInit(&config);
    Driver_CSI_DMA_ReflashRFF();
    Driver_CSI_Start();
    LOG_DEBUG("Driver_CSI_Init done");
}