#ifndef __BSP_KEYFILTER_H__
#define __BSP_KEYFILTER_H__

#include "imx6ull.h"
#include "driver_gpio.h"

void Bsp_KeyFilter_Init();

void Bsp_KeyFilter_TimerInit();

void Bsp_KeyFilter_TimerRestart();

void Bsp_KeyFilter_TimerStop();

void Bsp_KeyFilter_ExtiCallback(uint32_t GICC_IAR, void *params);

void Bsp_KeyFilter_TimerCallback(uint32_t GICC_IAR, void *params);

#endif /* __BSP_KEYFILTER_H__ */