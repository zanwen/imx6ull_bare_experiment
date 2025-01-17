#include "bsp_key.h"
#include "driver_delay.h"

#define READ_KEY() ((Driver_GPIO_ReadPin(GPIO1, 18) == GPIO_PIN_SET) ? KEY_UP : KEY_DOWN)

void Bsp_Key_Init() {
    IOMUXC_SetPinMux(IOMUXC_UART1_CTS_B_GPIO1_IO18, 0);
    IOMUXC_SetPinConfig(IOMUXC_UART1_CTS_B_GPIO1_IO18, 0xF080);

    // GPIO1->GDIR &= ~(1 << 18);
    GPIO_Config_t config;
    config.direction = GPIO_DIR_INPUT;
    Driver_GPIO_Init(GPIO1, 18, &config);
}

/**
 * @brief 检测按键按下事件
 *
 * @return KeyNo_t 当有按键的状态从抬起变成按下时，返回该按键的键值
 */
KeyNo_t Bsp_Key_DetectPressEvent() {
    static KeyStatus_t lastStatus = KEY_UP;
    if (lastStatus == KEY_UP && READ_KEY() == KEY_DOWN) {
        Driver_Delay_MS(10);
        if (READ_KEY() == KEY_DOWN) {
            lastStatus = KEY_DOWN;
            return KEY0;
        }
    } else if (READ_KEY() == KEY_UP) {
        lastStatus = KEY_UP;
    }
    return KEY_NONE;
}
