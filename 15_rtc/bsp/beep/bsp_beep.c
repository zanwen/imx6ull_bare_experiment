#include "bsp_beep.h"
#include "driver_gpio.h"

void Bsp_Beep_Init() {
    IOMUXC_SetPinMux(IOMUXC_SNVS_SNVS_TAMPER1_GPIO5_IO01, 0);
    IOMUXC_SetPinConfig(IOMUXC_SNVS_SNVS_TAMPER1_GPIO5_IO01, 0x10B0);

    // GPIO5->GDIR |= (1 << 1); // GPIO5_IO01 as output
    GPIO_Config_t config;
    config.direction = GPIO_DIR_OUTPUT;
    config.defaultStatus = GPIO_PIN_SET;
    Driver_GPIO_Init(GPIO5, 1, &config);
}

void Bsp_Beep_Switch(SwitchStatus_t status) {
    switch (status) {
        case ON:
            GPIO5->DR &= ~(1 << 1);
            // Driver_GPIO_WritePIn(GPIO5, 1, GPIO_PIN_RESET);
            break;
        case OFF:
            GPIO5->DR |= (1 << 1);
            // Driver_GPIO_WritePIn(GPIO5, 1, GPIO_PIN_SET);
            break;
        default:
            break;
    }
}
