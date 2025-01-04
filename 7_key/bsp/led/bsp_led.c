#include "bsp_led.h"

void Bsp_Led_Init() {
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

void Bsp_Led_On() { GPIO1->DR &= ~(1 << 3); }

void Bsp_Led_Off() { GPIO1->DR |= (1 << 3); }

void Bsp_Led_Switch(SwitchStatus_t status) {
    switch (status) {
        case ON:
            Bsp_Led_On();
            break;
        case OFF:
            Bsp_Led_Off();
            break;
        default:
            break;
    }
}
