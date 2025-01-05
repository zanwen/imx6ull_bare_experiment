#include "driver_gpio.h"

void Driver_GPIO_Init(GPIO_Type *base, uint32_t pin, GPIO_Config_t *config) {
    switch (config->direction) {
        case GPIO_DIR_INPUT:
            base->GDIR &= ~(1 << pin);
            break;
        case GPIO_DIR_OUTPUT:
            base->GDIR |= (1 << pin);
            Driver_GPIO_WritePIn(base, pin, config->defaultStatus);
            break;
        default:
            break;
    }
}

GPIO_Pin_Status_t Driver_GPIO_ReadPin(GPIO_Type *base, uint32_t pin) {
    return (base->DR & (1 << pin)) ? GPIO_PIN_SET : GPIO_PIN_RESET;
}

void Driver_GPIO_WritePIn(GPIO_Type *base, uint32_t pin, GPIO_Pin_Status_t status) {
    if (status == GPIO_PIN_SET) {
        base->DR |= (1 << pin);
    } else {
        base->DR &= ~(1 << pin);
    }
}