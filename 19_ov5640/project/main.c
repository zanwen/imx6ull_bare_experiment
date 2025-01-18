#include "main.h"
#include "bsp_beep.h"
#include "bsp_gt911.h"
#include "bsp_key.h"
#include "bsp_key_filter.h"
#include "bsp_lcd.h"
#include "bsp_lcdapi.h"
#include "bsp_led.h"
#include "bsp_rtc.h"
#include "driver_clk.h"
#include "driver_delay.h"
#include "driver_epit.h"
#include "driver_exti.h"
#include "driver_i2c.h"
#include "driver_interrupt.h"
#include "driver_uart.h"
#include "stdio.h"
#include "bsp_ft5426.h"
#include "logger.h"
#include "bsp_ov5640.h"
#include "driver_csi.h"

/* 背景颜色索引 */
unsigned int backcolor[10] = {
        LCD_BLUE,
        LCD_GREEN,
        LCD_RED,
        LCD_CYAN,
        LCD_YELLOW,
        LCD_LIGHTBLUE,
        LCD_DARKBLUE,
        LCD_WHITE,
        LCD_BLACK,
        LCD_ORANGE,
};

int main() {
    Driver_INT_Init();
    Driver_CLK_Init();
    Driver_CLK_Enable();
    Bsp_Led_Init();
    Bsp_Beep_Init();
    Bsp_Key_Init();
    Driver_Delay_Init();
    Driver_UART_Init();
//    Bsp_LCD_Init();
    Bsp_LCD_InitRGB565();
    rtc_init();
//    Bsp_FT5426_Init();
    Bsp_OV5640_Init();
    Driver_CSI_Init();
    
    SwitchStatus_t state = OFF;
    while (1) {
        KeyNo_t key = Bsp_Key_DetectPressEvent();
        if (key == KEY0) {
            Bsp_OV5640_Test();
            state = !state;
            Bsp_Led_Switch(state);
        }
    }
}