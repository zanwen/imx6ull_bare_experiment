#ifndef __BSP_BEEP_H__
#define __BSP_BEEP_H__

#include "imx6ull.h"
#include "bsp_typedef.h"

void beep_init();

void beep_control(SwitchStatus_t status);


#endif /* __BSP_BEEP_H__ */
