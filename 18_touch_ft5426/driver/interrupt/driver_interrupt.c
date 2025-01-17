#include "driver_interrupt.h"

static IRQ_Handler_t irqHandlers[NUMBER_OF_INT_VECTORS];
static volatile uint8_t nestedLevel = 0;

static void Driver_INT_DefaultIRQCallback(uint32_t GICC_IAR, void *params);

void Driver_INT_Init() {
    // 初始化GIC，类比初始化NVIC优先级分组
    GIC_Init();
    // 设置向量表基地址
    __set_VBAR(0x87800000);
    // 初始化中断处理器表
    for (uint8_t i = 0; i < NUMBER_OF_INT_VECTORS; i++) {
        irqHandlers[i].irqCallabck = Driver_INT_DefaultIRQCallback;
        irqHandlers[i].params = NULL;
    }
}

void Driver_INT_RegisterIRQHandler(IRQn_Type IRQn, IRQ_Callback_t callback, void *params) {
    irqHandlers[IRQn].irqCallabck = callback;
    irqHandlers[IRQn].params = params;
}

static void Driver_INT_DefaultIRQCallback(uint32_t GICC_IAR, void *params) {
    while (1)
        ;
}

void System_IRQHandler(uint32_t GICC_IAR) {
    uint16_t interruptID = GICC_IAR & 0x03FF;
    if(interruptID < NUMBER_OF_INT_VECTORS) {
        nestedLevel++;
        irqHandlers[interruptID].irqCallabck(GICC_IAR, irqHandlers[interruptID].params);
        nestedLevel--;
    }
}