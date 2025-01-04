#ifndef __BSP_BEEP_H__
#define __BSP_BEEP_H__

#include "bsp_typedef.h"
#include "imx6ull.h"

void Bsp_Beep_init();

void Bsp_Beep_Control(SwitchStatus_t status);

#endif /* __BSP_BEEP_H__ */
