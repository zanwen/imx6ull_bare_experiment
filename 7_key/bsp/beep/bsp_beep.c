#include "bsp_beep.h"

void Bsp_Beep_init() {
    IOMUXC_SetPinMux(IOMUXC_SNVS_SNVS_TAMPER1_GPIO5_IO01, 0);
    IOMUXC_SetPinConfig(IOMUXC_SNVS_SNVS_TAMPER1_GPIO5_IO01, 0x10B0);

    GPIO5->GDIR |= (1 << 1); // GPIO5_IO01 as output
    Bsp_Beep_Control(OFF);
}

void Bsp_Beep_Control(SwitchStatus_t status) {
    switch (status) {
        case ON:
            GPIO5->DR &= ~(1 << 1);
            break;
        case OFF:
            GPIO5->DR |= (1 << 1);
            break;
        default:
            break;
    }
}
