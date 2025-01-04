#ifndef __BSP_LED_H__
#define __BSP_LED_H__

#include "imx6ull.h"
#include "bsp_typedef.h"

void Bsp_Led_Init();

void Bsp_Led_On();

void Bsp_Led_Off();
void Bsp_Led_Switch(SwitchStatus_t status);

#endif /* __BSP_LED_H__ */
