#ifndef __DRIVER_INTERRUPT_H__
#define __DRIVER_INTERRUPT_H__

#include "imx6ull.h"

typedef void (*IRQ_Callback_t)(uint32_t GICC_IAR, void *params);

typedef struct {
    IRQ_Callback_t irqCallabck;
    void *params;
} IRQ_Handler_t;

void Driver_INT_Init();

void Driver_INT_RegisterIRQHandler(IRQn_Type IRQn, IRQ_Callback_t callback, void *params);

#endif /* __DRIVER_INTERRUPT_H__ */