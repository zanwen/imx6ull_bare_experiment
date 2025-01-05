#include "bsp_delay.h"

void Bsp_Delay_Short(volatile unsigned int n) {
    while (n--) {
    }
}

/**
 * @brief 396M主频下1ms的延时
 * 
 * @param ms 
 */
void Bsp_Delay(volatile unsigned int ms) {
    while (ms--) {
        Bsp_Delay_Short(0x7ff);
    }
}