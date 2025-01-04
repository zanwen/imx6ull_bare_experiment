#include "bsp_delay.h"

void Bsp_Delay_Short(volatile unsigned int n) {
    while (n--) {
    }
}

void Bsp_Delay(volatile unsigned int ms) {
    while (ms--) {
        Bsp_Delay_Short(0x7ff);
    }
}