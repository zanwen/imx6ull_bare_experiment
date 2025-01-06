#include "main.h"
#include "bsp_beep.h"
#include "bsp_delay.h"
#include "bsp_key.h"
#include "bsp_led.h"
#include "driver_clk.h"
#include "driver_epit.h"
#include "driver_exti.h"
#include "driver_interrupt.h"

volatile uint8_t a = 1;
// volatile uint8_t b = 2;
// volatile uint8_t c = 3;

int main() {
    Driver_INT_Init();
    Driver_Exti_Init();
    Driver_CLK_Init();
    Driver_CLK_Enable();
    Bsp_Led_Init();
    Bsp_Beep_Init();
    Bsp_Key_Init();
    Driver_EPIT_Init(0, 66000000 / 2); // 66M 1分频，定时周期0.5s

    // uint8_t cnt = 0;
    // SwitchStatus_t ledStatus = ON;
    // SwitchStatus_t beepStatus = ON;
    // KeyNo_t keyNo;
    while (1) {
        // Bsp_Led_On();
        // Bsp_Delay(1000);
        // Bsp_Led_Off();
        // Bsp_Delay(1000);

        // keyNo = Bsp_Key_DetectPressEvent();
        // if (keyNo == KEY0) {
        //     Bsp_Beep_Switch(beepStatus);
        //     beepStatus = !beepStatus;
        // }

        // // 大概 50 * 10ms，反转led状态
        // if (cnt++ == 50) {
        //     Bsp_Led_Switch(ledStatus);
        //     ledStatus = !ledStatus;
        //     cnt = 0;
        // }
        // Bsp_Delay(10);
    }
}