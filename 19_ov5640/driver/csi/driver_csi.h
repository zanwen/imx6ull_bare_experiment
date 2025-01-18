//
// Created by 86157 on 2025/1/18.
//

#ifndef INC_19_OV5640_DRIVER_CSI_H
#define INC_19_OV5640_DRIVER_CSI_H

#include "imx6ull.h"

typedef struct {
    uint16_t width;
    uint16_t height;
    uint8_t bytesPerPixel;
    uint32_t frameBufferAddr;
} CSI_Config_t;

void Driver_CSI_Init(void);
void Driver_CSI_Start(void);
void Driver_CSI_Stop(void);

#endif //INC_19_OV5640_DRIVER_CSI_H
