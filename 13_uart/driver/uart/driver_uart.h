#ifndef __DRIVER_UART_H__
#define __DRIVER_UART_H__

#include "imx6ull.h"

void Driver_Uart_Init(void);

uint8_t Driver_Uart_ReceiveByte();

void Driver_Uart_SendByte(uint8_t data);

#endif /* __DRIVER_UART_H__ */
