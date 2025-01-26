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
#include "bsp_ft5426.h"
#include "bsp_ov5640.h"
#include "lvgl.h"
#include "lv_port_disp_template.h"
#include "lv_port_indev_template.h"


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
//    rtc_init();
//    Bsp_FT5426_Init();

    Driver_EPIT_Init(0, 66000000 / 1000); // 1ms
    lv_init();
    lv_port_disp_init();
    lv_port_indev_init();

    lv_obj_t* switch_obj = lv_switch_create(lv_scr_act());
    lv_obj_set_size(switch_obj, 120, 60);
    lv_obj_align(switch_obj, LV_ALIGN_CENTER, 0, 0);

    while(1) {
        Driver_Delay_MS(5);
        lv_timer_handler();
    }

//    tftlcd_dev.forecolor = LCD_RED;
//    lcd_show_string(50, 10, 400, 24, 24, (char *) "IMX6U-ALPHA TOUCH SCREEN TEST");
//    lcd_show_string(50, 40, 200, 16, 16, (char *) "TOUCH SCREEN TEST");
//    lcd_show_string(50, 60, 200, 16, 16, (char *) "ATOM@ALIENTEK");
//    lcd_show_string(50, 80, 200, 16, 16, (char *) "2019/3/27");
//
//    lcd_show_string(50, 110, 400, 16, 16, (char *) "TP Num	:");
//    lcd_show_string(50, 130, 200, 16, 16, (char *) "Point0 X:");
//    lcd_show_string(50, 150, 200, 16, 16, (char *) "Point0 Y:");
//    lcd_show_string(50, 170, 200, 16, 16, (char *) "Point1 X:");
//    lcd_show_string(50, 190, 200, 16, 16, (char *) "Point1 Y:");
//    lcd_show_string(50, 210, 200, 16, 16, (char *) "Point2 X:");
//    lcd_show_string(50, 230, 200, 16, 16, (char *) "Point2 Y:");
//    lcd_show_string(50, 250, 200, 16, 16, (char *) "Point3 X:");
//    lcd_show_string(50, 270, 200, 16, 16, (char *) "Point3 Y:");
//    lcd_show_string(50, 290, 200, 16, 16, (char *) "Point4 X:");
//    lcd_show_string(50, 310, 200, 16, 16, (char *) "Point4 Y:");
//    tftlcd_dev.forecolor = LCD_BLUE;
//
//    SwitchStatus_t state = OFF;
//    unsigned char i = 0;
//    while (1) {
//        lcd_shownum(50 + 72, 110, ft5426_device.point_num, 1, 16);
//        lcd_shownum(50 + 72, 130, ft5426_device.x[0], 5, 16);
//        lcd_shownum(50 + 72, 150, ft5426_device.y[0], 5, 16);
//        lcd_shownum(50 + 72, 170, ft5426_device.x[1], 5, 16);
//        lcd_shownum(50 + 72, 190, ft5426_device.y[1], 5, 16);
//        lcd_shownum(50 + 72, 210, ft5426_device.x[2], 5, 16);
//        lcd_shownum(50 + 72, 230, ft5426_device.y[2], 5, 16);
//        lcd_shownum(50 + 72, 250, ft5426_device.x[3], 5, 16);
//        lcd_shownum(50 + 72, 270, ft5426_device.y[3], 5, 16);
//        lcd_shownum(50 + 72, 290, ft5426_device.x[4], 5, 16);
//        lcd_shownum(50 + 72, 310, ft5426_device.y[4], 5, 16);
//        Driver_Delay_MS(10);
//        i++;
//
//        if (i == 50) {
//            i = 0;
//            state = !state;
//            Bsp_Led_Switch(state);
//        }
//    }
}