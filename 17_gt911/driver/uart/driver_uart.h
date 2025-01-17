#ifndef __DRIVER_UART_H__
#define __DRIVER_UART_H__

#include "imx6ull.h"

void Driver_UART_Init(void);

uint8_t Driver_UART_ReceiveByte();

void Driver_UART_SendByte(uint8_t data);

#endif /* __DRIVER_UART_H__ */
