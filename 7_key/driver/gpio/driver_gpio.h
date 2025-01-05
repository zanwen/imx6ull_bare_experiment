#ifndef __DRIVER_GPIO_H__
#define __DRIVER_GPIO_H__

#include "imx6ull.h"

typedef enum { GPIO_DIR_INPUT, GPIO_DIR_OUTPUT } GPIO_Direction_t;

typedef enum { GPIO_PIN_RESET = 0U, GPIO_PIN_SET = 1U } GPIO_Pin_Status_t;

typedef struct {
    GPIO_Direction_t direction;
    GPIO_Pin_Status_t defaultStatus;
} GPIO_Config_t;

void Driver_GPIO_Init(GPIO_Type *base, uint32_t pin, GPIO_Config_t * config);
GPIO_Pin_Status_t Driver_GPIO_ReadPin(GPIO_Type *base, uint32_t pin);
void Driver_GPIO_WritePIn(GPIO_Type *base, uint32_t pin, GPIO_Pin_Status_t status);

#endif /* __DRIVER_GPIO_H__ */