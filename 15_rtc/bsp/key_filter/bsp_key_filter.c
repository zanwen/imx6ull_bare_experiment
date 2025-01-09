#include "bsp_key_filter.h"
#include "bsp_led.h"
#include "driver_interrupt.h"

void Bsp_KeyFilter_Init() {
    IOMUXC_SetPinMux(IOMUXC_UART1_CTS_B_GPIO1_IO18, 0);
    IOMUXC_SetPinConfig(IOMUXC_UART1_CTS_B_GPIO1_IO18, 0xF080);

    GPIO_Config_t config;
    config.direction = GPIO_DIR_INPUT;
    config.intTriggerMode = INT_TRI_FALLING_EDGE; // 下降沿触发
    Driver_GPIO_Init(GPIO1, 18, &config);

    // 使能GPIO中断
    Driver_GPIO_EnableINT(GPIO1, 18);
    // 注册中断回调
    Driver_INT_RegisterIRQHandler(GPIO1_Combined_16_31_IRQn, Bsp_KeyFilter_ExtiCallback, NULL);
    // 使能GIC中断号
    GIC_EnableIRQ(GPIO1_Combined_16_31_IRQn);

    Bsp_KeyFilter_TimerInit();
}

void Bsp_KeyFilter_TimerInit() {
    // 时钟源选择 IPG_CLK 66M
    EPIT1->CR &= ~(0x3 << 24);
    EPIT1->CR |= (0x1 << 24);
    // 分频
    EPIT1->CR &= ~0xFFF;
    EPIT1->CR |= 0 << 4; // 1分频

    EPIT1->CR |= 1 << 3; // set-and-forget 模式，减到0时自动重装载
    EPIT1->CR |= 1 << 2; // 使能比较中断，计数值等于比较值时产生中断
    EPIT1->CR |= 1 << 1; // 使能模式：从装载值开始计数

    // 装载值和比较值 中断频率100HZ 10ms
    EPIT1->LR = 66000000 / 100;
    EPIT1->CMPR = 0;

    // 配置中断
    Driver_INT_RegisterIRQHandler(EPIT1_IRQn, Bsp_KeyFilter_TimerCallback, NULL);
    GIC_EnableIRQ(EPIT1_IRQn);
}

void Bsp_KeyFilter_TimerRestart() {
    // 使能 EPIT
    EPIT1->CR |= 1 << 0;
}

void Bsp_KeyFilter_TimerStop() {
    EPIT1->CR &= ~(1 << 0);
}

void Bsp_KeyFilter_ExtiCallback(uint32_t GICC_IAR, void *params) {
    Bsp_KeyFilter_TimerRestart();
    Driver_GPIO_ClearINTFlag(GPIO1, 18);
}

void Bsp_KeyFilter_TimerCallback(uint32_t GICC_IAR, void *params) {
    static SwitchStatus_t status = OFF;
    Bsp_KeyFilter_TimerStop();
    if (Driver_GPIO_ReadPin(GPIO1, 18) == GPIO_PIN_RESET) {
        status = !status;
        Bsp_Led_Switch(status);
    }
    EPIT1->SR |= 1 << 0;
}
