#include "fsl_iomuxc.h"

void clk_init() {
    CCM->CCGR0 = 0xFFFFFFFF;
    CCM->CCGR1 = 0xFFFFFFFF;
    CCM->CCGR2 = 0xFFFFFFFF;
    CCM->CCGR3 = 0xFFFFFFFF;
    CCM->CCGR4 = 0xFFFFFFFF;
    CCM->CCGR5 = 0xFFFFFFFF;
    CCM->CCGR6 = 0xFFFFFFFF;
}

void led_init() {
    // IO复用
    // IOMUX_SW_MUX->GPIO1_IO02 &= ~0xF;
    // IOMUX_SW_MUX->GPIO1_IO02 |= 0x5;
    IOMUXC_SetPinMux(IOMUXC_GPIO1_IO03_GPIO1_IO03, 0);
    IOMUXC_SetPinConfig(IOMUXC_GPIO1_IO03_GPIO1_IO03, 0x10B0);
    // IO电气使用默认值
    // IO方向
    GPIO1->GDIR |= (1 << 3);
    // IO初始状态
    GPIO1->DR &= ~(1 << 3);
}

void led_on() { GPIO1->DR &= ~(1 << 3); }

void led_off() { GPIO1->DR |= (1 << 3); }

void delay_short(volatile unsigned int n) {
    while (n--) {
    }
}

void delay_ms(volatile unsigned int ms) {
    while (ms--) {
        delay_short(0x7ff);
    }
}

volatile int a;
volatile int b[10];

int main() {
    clk_init();
    led_init();
    while (1) {
        led_on();
        delay_ms(3000);

        led_off();
        delay_ms(3000);
    }
}