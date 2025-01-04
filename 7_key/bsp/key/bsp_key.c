#include "bsp_key.h"
#include "bsp_delay.h"

void Bsp_Key_Init() {
    IOMUXC_SetPinMux(IOMUXC_UART1_CTS_B_GPIO1_IO18, 0);
    IOMUXC_SetPinConfig(IOMUXC_UART1_CTS_B_GPIO1_IO18, 0xF080);

    GPIO1->GDIR &= ~(1 << 18);
}

KeyStatus_t Bsp_Key_ReadStatus() {
    if (GPIO1->DR & (1 << 18)) {
        return KEY_UP;
    } else {
        return KEY_DOWN;
    }
}

/**
 * @brief 检测按键按下事件
 * 
 * @return KeyNo_t 当有按键的状态从抬起变成按下时，返回该按键的键值
 */
KeyNo_t Bsp_Key_DetectPressEvent() {
    static KeyStatus_t lastStatus = KEY_UP;
    if (lastStatus == KEY_UP && Bsp_Key_ReadStatus() == KEY_DOWN) {
        Bsp_Delay(10);
        if (Bsp_Key_ReadStatus() == KEY_DOWN) {
            lastStatus = KEY_DOWN;
            return KEY0;
        }
    } else if (Bsp_Key_ReadStatus() == KEY_UP) {
        lastStatus = KEY_UP;
    }
    return KEY_NONE;
}
