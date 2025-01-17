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
    Bsp_FT5426_Init();

    KeyNo_t key;
    SwitchStatus_t status = ON;
    while (1) {
        key = Bsp_Key_DetectPressEvent();
        if (key == KEY0) {
            Bsp_FT5426_Test();
            status = !status;
            Bsp_Led_Switch(status);
        }
    }

    //    KeyNo_t key = 0;
    //    int i = 3, t = 0;
    //    char buf[160];
    //    struct rtc_datetime rtcdate;
    //    SwitchStatus_t state = OFF;
    //    tftlcd_dev.forecolor = LCD_RED;
    //    lcd_show_string(50, 10, 400, 24, 24, (char *) "ALPHA-IMX6UL RTC TEST"); /* 显示字符串 */
    //    lcd_show_string(50, 40, 200, 16, 16, (char *) "ATOM@ALIENTEK");
    //    lcd_show_string(50, 60, 200, 16, 16, (char *) "2019/3/27");
    //    tftlcd_dev.forecolor = LCD_BLUE;
    //    memset(buf, 0, sizeof(buf));
    //
    //    while (1) {
    //        if (t == 100)//1s时间到了
    //        {
    //            t = 0;
    //            printf("will be running %d s......\r", i);
    //
    //            lcd_fill(50, 90, 370, 110, tftlcd_dev.backcolor); /* 清屏 */
    //            sprintf(buf, "will be running %ds......", i);
    //            lcd_show_string(50, 90, 300, 16, 16, buf);
    //            i--;
    //            if (i < 0)
    //                break;
    //        }
    //
    //        key = Bsp_Key_DetectPressEvent();
    //        if (key == KEY0) {
    //            rtcdate.year = 2018;
    //            rtcdate.month = 1;
    //            rtcdate.day = 15;
    //            rtcdate.hour = 16;
    //            rtcdate.minute = 23;
    //            rtcdate.second = 0;
    //            rtc_setdatetime(&rtcdate); /* 初始化时间和日期 */
    //            printf("\r\n RTC Init finish\r\n");
    //            break;
    //        }
    //
    //        Driver_Delay_MS(10);
    //        t++;
    //    }
    //    tftlcd_dev.forecolor = LCD_RED;
    //    lcd_fill(50, 90, 370, 110, tftlcd_dev.backcolor);               /* 清屏 */
    //    lcd_show_string(50, 90, 200, 16, 16, (char *) "Current Time:"); /* 显示字符串 */
    //    tftlcd_dev.forecolor = LCD_BLUE;
    //
    //    while (1) {
    //        rtc_getdatetime(&rtcdate);
    //        sprintf(buf, "%d/%d/%d %d:%d:%d", rtcdate.year, rtcdate.month, rtcdate.day, rtcdate.hour, rtcdate.minute, rtcdate.second);
    //        lcd_fill(50, 110, 300, 130, tftlcd_dev.backcolor);
    //        lcd_show_string(50, 110, 250, 16, 16, (char *) buf); /* 显示字符串 */
    //
    //        state = !state;
    //        Bsp_Led_Switch(state);
    //        Driver_Delay_MS(1000); /* 延时一秒 */
    //    }
}