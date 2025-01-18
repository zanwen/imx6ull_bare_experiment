#include "driver_gpio.h"


void Driver_GPIO_Init(GPIO_Type *base, uint32_t pin, GPIO_Config_t *config) {
    switch (config->direction) {
        case GPIO_DIR_INPUT:
            base->GDIR &= ~(1 << pin);
            Driver_GPIO_ConfigINT(base, pin, config->intTriggerMode);
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

/**
 * @brief 配置引脚中断触发方式
 *
 * @param base
 * @param pin
 * @param intTriggerMode
 */
void Driver_GPIO_ConfigINT(GPIO_Type *base, uint32_t pin, INT_TriggerMode intTriggerMode) {
    if (intTriggerMode == INT_TRI_NONE)
        return;
    // 失能所有边沿触发
    base->EDGE_SEL &= ~(1 << pin);
    // 读-改-写
    volatile uint32_t *pReg = pin < 16 ? &(base->ICR1) : &(base->ICR2);
    uint8_t shift = (pin % 16) * 2;
    uint32_t reg = *pReg;
    reg &= ~(0x3 << shift);
    switch (intTriggerMode) {
        case INT_TRI_LOW_LEVEL:
            break;
        case INT_TRI_HIGH_LEVEL:
            reg |= (0x1 << shift);
            break;
        case INT_TRI_RISING_EDGE:
            reg |= (0x2 << shift);
            break;
        case INT_TRI_FALLING_EDGE:
            reg |= (0x3 << shift);
            break;
        case INT_TRI_RISING_OR_FALLING_EDGE:
            base->EDGE_SEL |= (1 << pin);
        default:
            break;
    }
    *pReg = reg;
}

void Driver_GPIO_EnableINT(GPIO_Type *base, uint32_t pin) {
    base->IMR |= (1 << pin);
}

void Driver_GPIO_DisableINT(GPIO_Type *base, uint32_t pin) {
    base->IMR &= ~(1 << pin);
}

void Driver_GPIO_ClearINTFlag(GPIO_Type *base, uint32_t pin) {
    base->ISR |= (1 << pin);
}