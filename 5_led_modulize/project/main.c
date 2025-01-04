#include "main.h"

int main() {
    clk_init();
    led_init();
    while (1) {
        led_on();
        delay_ms(300);

        led_off();
        delay_ms(300);
    }
}