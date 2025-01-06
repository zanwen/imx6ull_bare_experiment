#include "driver_epit.h"
#include "driver_interrupt.h"
#include "bsp_led.h"

void Driver_EPIT1_IRQCallabck(uint32_t GICC_IAR, void *params);

/**
 * @brief 
 * 
 * @param prescaler 分频值 0～0xFFF => 1~4096
 * @param loadValue 装载值（向下计数）
 */
void Driver_EPIT_Init(uint16_t prescaler, uint32_t loadValue) {
    if (prescaler > 0xFFF) {
        prescaler = 0xFFF;
    }
    // 时钟源选择 IPG_CLK 66M
    EPIT1->CR &= ~(0x3 << 24);
    EPIT1->CR |= (0x1 << 24);
    // 分频
    EPIT1->CR &= ~0xFFF;
    EPIT1->CR |= prescaler << 4;

    EPIT1->CR |= 1 << 3; // set-and-forget 模式，减到0时自动重装载
    EPIT1->CR |= 1 << 2; // 使能比较中断，计数值等于比较值时产生中断
    EPIT1->CR |= 1 << 1; // 使能模式：从装载值开始计数

    // 装载值和比较值
    EPIT1->LR = loadValue;
    EPIT1->CMPR = 0;

    // 配置中断
    Driver_INT_RegisterIRQHandler(EPIT1_IRQn, Driver_EPIT1_IRQCallabck, NULL);
    GIC_EnableIRQ(EPIT1_IRQn);

    // 使能 EPIT
    EPIT1->CR |= 1 << 0;
}

void Driver_EPIT1_IRQCallabck(uint32_t GICC_IAR, void *params) {
    static SwitchStatus_t status = OFF;
    if(EPIT1->SR & (1 << 0)) {
        status = !status;
        Bsp_Led_Switch(status);
        EPIT1->SR |= 1 << 0; // 清除中断标志位
    }
}