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
#include "bsp_ov2640.h"

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
    Bsp_LCD_Init();
    rtc_init();
//    Bsp_FT5426_Init();

    Bsp_OV5640_Init();

    SwitchStatus_t state = OFF;
    unsigned char index = 0;
    tftlcd_dev.forecolor = LCD_RED;
    while (1) {
//        lcd_clear(backcolor[index]);
//        Driver_Delay_MS(1);
//        lcd_show_string(10, 40, 260, 32, 32, (char *) "ALPHA IMX6U");
//        lcd_show_string(10, 80, 240, 24, 24, (char *) "RGBLCD TEST");
//        lcd_show_string(10, 110, 240, 16, 16, (char *) "ATOM@ALIENTEK");
//        lcd_show_string(10, 130, 240, 12, 12, (char *) "2019/8/14");
//        index = (index + 1) % 10;
//        Bsp_Led_On();
//        Driver_Delay_MS(1000);
//        Bsp_Led_Off();
//        Driver_Delay_MS(1000);
        KeyNo_t key = Bsp_Key_DetectPressEvent();
        if (key == KEY0) {
            Bsp_OV5640_Test();
            state = !state;
            Bsp_Led_Switch(state);
        }
    }
}