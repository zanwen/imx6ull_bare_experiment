//
// Created by 86157 on 2025/1/7.
//

#ifndef INC_14_LCD_BSP_LCD_H
#define INC_14_LCD_BSP_LCD_H

#include "imx6ull.h"

typedef enum {
    LCD_TYPE_UNKNOWN
    LCD_7_INCH_1024x600
} LCD_Type_t;

void Bsp_LCD_Init(void);
uint8_t Bsp_LCD_ReadID(void);

#endif// INC_14_LCD_BSP_LCD_H
