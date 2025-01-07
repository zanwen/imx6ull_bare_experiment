#include "bsp_led.h"
#include "driver_gpio.h"

void Bsp_Led_Init() {
    // IO复用
    // IOMUX_SW_MUX->GPIO1_IO02 &= ~0xF;
    // IOMUX_SW_MUX->GPIO1_IO02 |= 0x5;
    IOMUXC_SetPinMux(IOMUXC_GPIO1_IO03_GPIO1_IO03, 0);
    IOMUXC_SetPinConfig(IOMUXC_GPIO1_IO03_GPIO1_IO03, 0x10B0);
    // IO电气使用默认值
    // IO方向
    // GPIO1->GDIR |= (1 << 3);
    // IO初始状态
    // GPIO1->DR &= ~(1 << 3);
    GPIO_Config_t config;
    config.direction = GPIO_DIR_OUTPUT;
    config.defaultStatus = GPIO_PIN_SET;
    Driver_GPIO_Init(GPIO1, 3, &config);
}

void Bsp_Led_On() {
    // GPIO1->DR &= ~(1 << 3);
    Driver_GPIO_WritePIn(GPIO1, 3, GPIO_PIN_RESET);
}

void Bsp_Led_Off() {
    // GPIO1->DR |= (1 << 3);
    Driver_GPIO_WritePIn(GPIO1, 3, GPIO_PIN_SET);
}

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
