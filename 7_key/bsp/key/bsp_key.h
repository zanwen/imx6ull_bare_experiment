#ifndef __BSP_KEY_H__
#define __BSP_KEY_H__

#include "driver_gpio.h"
#include "imx6ull.h"

typedef enum {
    KEY_UP = GPIO_PIN_SET,
    KEY_DOWN = GPIO_PIN_RESET
} KeyStatus_t;

typedef enum {
    KEY_NONE = 0,
    KEY0
} KeyNo_t;

void Bsp_Key_Init();

/**
 * @brief 获取被按下的按键的键值
 *
 * @return KeyValue_t 没有按键按下时返回 KEY_NONE
 */
KeyNo_t Bsp_Key_DetectPressEvent();

#endif /* __BSP_KEY_H__ */
