#include "driver_delay.h"
#include "bsp_led.h"
#include "driver_interrupt.h"

// void Driver_Delay_GPTCallback(uint32_t GICC_IAR, void *params);

void Driver_Delay_Init(void) {
    // GPT模块软件复位 寄存器重置
    GPT1->CR |= 1 << 15;
    while (GPT1->CR & (0x1 << 15))
        ;

    // 时钟源选择 IPG_CLK 66M
    GPT1->CR &= ~(0x7 << 6);
    GPT1->CR |= (0x1 << 6);
    // 分频 66M/66=1M
    GPT1->PR &= ~(0xFFF << 0);
    GPT1->PR |= (65 << 0);

    GPT1->CR |= 1 << 1; // 关闭定时器时将计数值清零
    GPT1->OCR[0] = 0xFFFFFFFF;

    // GPT1->CR &= ~(1 << 9);  // restart模式，发生比较事件时定时器重置为0重新开始计数
    // GPT1->OCR[0] = 1000000; // 溢出频率1M/1000000=1HZ
    // // 中断配置
    // GPT1->IR |= (1 << 0);
    // Driver_INT_RegisterIRQHandler(GPT1_IRQn, Driver_Delay_GPTCallback, NULL);
    // GIC_EnableIRQ(GPT1_IRQn);

    // 开启定时器
    GPT1->CR |= 1 << 0;
}

// void Driver_Delay_GPTCallback(uint32_t GICC_IAR, void *params) {
//     static SwitchStatus_t status = OFF;
//     if (GPT1->SR & (1 << 0)) {
//         status = !status;
//         Bsp_Led_Switch(status);
//         GPT1->SR |= (1 << 0);
//     }
// }

void Driver_Delay_US(uint32_t us) {
    uint32_t lastCnt, curCnt, elapsedCnt;
    elapsedCnt = 0;
    lastCnt = GPT1->CNT;
    while (elapsedCnt < us) {
        curCnt = GPT1->CNT;
        if (curCnt == lastCnt) {
            continue;
        }
        if (curCnt > lastCnt) {
            elapsedCnt += curCnt - lastCnt;
        } else {
            elapsedCnt += (0xFFFFFFFF - lastCnt) + curCnt;
        }
        lastCnt = curCnt;
    }
}

void Driver_Delay_MS(uint32_t ms) {
    while (ms--) {
        Driver_Delay_US(1000);
    }
}