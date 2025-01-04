#include "main.h"

int main() {
    clk_init();
    led_init();
    beep_init();

    for (uint8_t i = 0; i < 5; i++) {
        beep_control(ON);
        led_on();
        delay_ms(1000);

        beep_control(OFF);
        led_off();
        delay_ms(1000);
    }

    while (1) {
        led_on();
        delay_ms(300);

        led_off();
        delay_ms(300);
    }
}