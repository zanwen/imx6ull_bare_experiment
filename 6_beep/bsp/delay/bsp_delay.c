#include "bsp_delay.h"

void delay_short(volatile unsigned int n) {
    while (n--) {
    }
}

void delay_ms(volatile unsigned int ms) {
    while (ms--) {
        delay_short(0x7ff);
    }
}