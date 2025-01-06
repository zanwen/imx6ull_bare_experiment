#include "driver_exti.h"
#include "bsp_beep.h"
#include "bsp_delay.h"
#include "bsp_led.h"
#include "driver_gpio.h"
#include "driver_interrupt.h"

void Driver_Exti_IRQCallabck(uint32_t GICC_IAR, void *params);

void Driver_Exti_Init() {
    IOMUXC_SetPinMux(IOMUXC_UART1_CTS_B_GPIO1_IO18, 0);
    IOMUXC_SetPinConfig(IOMUXC_UART1_CTS_B_GPIO1_IO18, 0xF080);

    GPIO_Config_t config;
    config.direction = GPIO_DIR_INPUT;
    config.intTriggerMode = INT_TRI_FALLING_EDGE; // 下降沿触发
    Driver_GPIO_Init(GPIO1, 18, &config);

    // 使能GPIO中断
    Driver_GPIO_EnableINT(GPIO1, 18);
    // 注册中断回调
    Driver_INT_RegisterIRQHandler(GPIO1_Combined_16_31_IRQn, Driver_Exti_IRQCallabck, NULL);
    // 使能GIC中断号
    GIC_EnableIRQ(GPIO1_Combined_16_31_IRQn);
}

void Driver_Exti_IRQCallabck(uint32_t GICC_IAR, void *params) {
    static SwitchStatus_t status = ON;

    Bsp_Delay(10);
    if (Driver_GPIO_ReadPin(GPIO1, 18) == GPIO_PIN_RESET) {
        Bsp_Led_Switch(status);
        // Bsp_Beep_Switch(status);
        status = !status;
    }

    // 清除中断标志位
    Driver_GPIO_ClearINTFlag(GPIO1, 18);
}