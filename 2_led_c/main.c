#include "main.h"

void clk_init() {
    CCM_CCGR0 = 0xFFFFFFFF;
    CCM_CCGR1 = 0xFFFFFFFF;
    CCM_CCGR2 = 0xFFFFFFFF;
    CCM_CCGR3 = 0xFFFFFFFF;
    CCM_CCGR4 = 0xFFFFFFFF;
    CCM_CCGR5 = 0xFFFFFFFF;
    CCM_CCGR6 = 0xFFFFFFFF;
}

void gpio_init() {
    SW_MUX_GPIO1_IO03 &= ~0xF;
    SW_MUX_GPIO1_IO03 |= 0x5;

    GPIO1_GDIR |= (1 << 3);

    GPIO1_DR &= ~(1 << 3);
}

void led_on() { GPIO1_DR &= ~(1 << 3); }

void led_off() { GPIO1_DR |= (1 << 3); }

void delay_short(volatile unsigned int n) {
    while (n--)
        ;
}

void delay_ms(volatile unsigned int ms) {
    while (ms--)
        delay_short(0x7ff);
}

int main() {
    clk_init();
    gpio_init();
    
    while (1) {
        led_on();
        delay_ms(1000);

        led_off();
        delay_ms(1000);
    }
}