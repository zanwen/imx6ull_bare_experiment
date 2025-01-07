#include "driver_uart.h"

void Driver_Uart_IOInit(void) {
    IOMUXC_SetPinMux(IOMUXC_UART1_TX_DATA_UART1_TX, 0);
    IOMUXC_SetPinMux(IOMUXC_UART1_RX_DATA_UART1_RX, 0);
    IOMUXC_SetPinConfig(IOMUXC_UART1_TX_DATA_UART1_TX, 0x10b0);
    IOMUXC_SetPinConfig(IOMUXC_UART1_RX_DATA_UART1_RX, 0x10b0);
    
}

void Driver_Uart_Init(void) {
    Driver_Uart_IOInit();

    // 时钟源已在 driver_clk 中配置为80M
    UART1->UFCR &= ~(0x7 << 7);
    UART1->UFCR |= (0x5 << 7); // UART内部分频：1
    // 波特率配置 BaudRate = Ref / (16 * (UBMR + 1) / (UBIR + 1))
    // 115200 = 80000000 / (16 * (UBMR + 1) / (UBIR + 1)) => (UBMR + 1) / (UBIR + 1) = 43 令UBIR=0=>UBMR=42
    UART1->UBIR = 0;
    UART1->UBMR = 42;

    UART1->UCR2 |= (1 << 14); // 关闭硬件流控
    UART1->UCR2 &= ~(1 << 8); // 关闭奇偶校验
    UART1->UCR2 &= ~(1 << 6); // 停止位bit数量为1
    UART1->UCR2 |= (1 << 5);  // 数据长度为8 bit，不含 start stop parity
    UART1->UCR2 |= (1 << 2);  // 使能TX
    UART1->UCR2 |= (1 << 1);  // 使能RX

    UART1->UCR3 |= (1 << 2); // RX复用输入引脚

    UART1->UCR1 |= 1 << 0; // 开启UART1
}

uint8_t Driver_Uart_ReceiveByte() {
    while ((UART1->USR2 & 0x1) == 0)
        ;
    return UART1->URXD;
}

void Driver_Uart_SendByte(uint8_t data) {
    while ((UART1->USR2 & (0x1 << 3)) == 0)
        ;
    UART1->UTXD = data & 0xFF;
}

unsigned char getc() {
    while ((UART1->USR2 & 0x1) == 0)
        ;
    return UART1->URXD;
}

void putc(unsigned char c) {
    while ((UART1->USR2 & (0x1 << 3)) == 0)
        ;
    UART1->UTXD = c & 0xFF;
}

/*
 * @description : 防止编译器报错
 * @param 		: 无
 * @return		: 无
 */
void raise(int sig_nr) {
}
