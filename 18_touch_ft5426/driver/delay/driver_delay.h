#ifndef __BSP_DELAY_H__
#define __BSP_DELAY_H__

#include "imx6ull.h"
void Driver_Delay_Init(void);

void Driver_Delay_US(volatile unsigned int n);

void Driver_Delay_MS(volatile unsigned int ms);

#endif /* __BSP_DELAY_H__ */
