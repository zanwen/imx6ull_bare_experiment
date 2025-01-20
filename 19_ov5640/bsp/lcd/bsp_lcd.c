//
// Created by 86157 on 2025/1/7.
//

#include "bsp_lcd.h"
#include "driver_delay.h"
#include "driver_gpio.h"
#include "logger.h"

/* 液晶屏参数结构体 */
struct tftlcd_typedef tftlcd_dev;

void Bsp_LCD_Reset(void) {
    LCDIF->CTRL = 1 << 31; /* 强制复位 */
}

void Bsp_LCD_UnReset(void) {
    LCDIF->CTRL = 0 << 31; /* 取消强制复位 */
}

void Bsp_LCD_Enable(void) {
    LCDIF->CTRL |= 1 << 0; /* 使能ELCDIF */
}

void Bsp_LCD_Init(void) {
#if USE_LCD_RGB888
    Bsp_LCD_InitRGB888();
#else
    Bsp_LCD_InitRGB565();
#endif
}

void Bsp_LCD_InitRGB888(void) {
    uint16_t lcdid = Bsp_LCD_ReadID();
    tftlcd_dev.id = lcdid;
    Bsp_LCD_IOInit();

    Bsp_LCD_Reset();     /* 复位LCD  			*/
    Driver_Delay_MS(10); /* 延时10ms 			*/
    Bsp_LCD_UnReset();   /* 结束复位 			*/

    /* TFTLCD参数结构体初始化 */
    tftlcd_dev.height = 600;
    tftlcd_dev.width = 1024;
    tftlcd_dev.vspw = 3;
    tftlcd_dev.vbpd = 20;
    tftlcd_dev.vfpd = 12;
    tftlcd_dev.hspw = 20;
    tftlcd_dev.hbpd = 140;
    tftlcd_dev.hfpd = 160;
    Bsp_LCD_ClkInit(32, 3, 5); /* 初始化LCD时钟 51.2MHz */
    tftlcd_dev.pixsize = 4;    /* ARGB8888模式，每个像素4字节 */
    tftlcd_dev.framebuffer = LCD_FRAMEBUF_ADDR;
    tftlcd_dev.backcolor = LCD_WHITE; /* 背景色为白色 */
    tftlcd_dev.forecolor = LCD_BLACK; /* 前景色为黑色 */

    /* 初始化ELCDIF的CTRL寄存器
     * bit [31] 0 : 停止复位
     * bit [19] 1 : 旁路计数器模式
     * bit [17] 1 : LCD工作在dotclk模式
     * bit [15:14] 00 : 输入数据不交换（是否需要交换字节顺序参考屏幕说明书）
     * bit [13:12] 00 : CSC不交换
     * bit [11:10] 11 : 24位总线宽度
     * bit [9:8]   11 : 24位数据宽度,也就是RGB888
     * bit [5]     1  : elcdif工作在主模式
     * bit [1]     0  : 所有的24位均有效
	 */
    LCDIF->CTRL |= (1 << 19) | (1 << 17) | (0 << 14) | (0 << 12) |
                   (3 << 10) | (3 << 8) | (1 << 5) | (0 << 1);
    /*
     * 初始化ELCDIF的寄存器CTRL1
     * bit [19:16]  : 0X7 ARGB模式下，传输24位数据，A通道不用传输
	 */
    LCDIF->CTRL1 = 0X7 << 16;

    /*
      * 初始化ELCDIF的寄存器TRANSFER_COUNT寄存器
      * bit [31:16]  : 高度
      * bit [15:0]   : 宽度
	  */
    LCDIF->TRANSFER_COUNT = (tftlcd_dev.height << 16) | (tftlcd_dev.width << 0);

    /*
     * 初始化ELCDIF的VDCTRL0寄存器
     * bit [29] 0 : VSYNC输出
     * bit [28] 1 : 使能ENABLE输出
     * bit [27] 0 : VSYNC低电平有效
     * bit [26] 0 : HSYNC低电平有效
     * bit [25] 0 : DOTCLK上升沿有效
     * bit [24] 1 : ENABLE信号高电平有效
     * bit [21] 1 : DOTCLK模式下设置为1
     * bit [20] 1 : DOTCLK模式下设置为1
     * bit [17:0] : vsw参数
	 */
    LCDIF->VDCTRL0 = 0;   //先清零
    if (lcdid == ATKVGA) {//VGA需要特殊处理
        LCDIF->VDCTRL0 = (0 << 29) | (1 << 28) | (0 << 27) |
                         (0 << 26) | (1 << 25) | (0 << 24) |
                         (1 << 21) | (1 << 20) | (tftlcd_dev.vspw << 0);
    } else {
        LCDIF->VDCTRL0 = (0 << 29) | (1 << 28) | (0 << 27) |
                         (0 << 26) | (0 << 25) | (1 << 24) |
                         (1 << 21) | (1 << 20) | (tftlcd_dev.vspw << 0);
    }

    /*
	 * 初始化ELCDIF的VDCTRL1寄存器
	 * 设置VSYNC总周期
	 */
    LCDIF->VDCTRL1 = tftlcd_dev.height + tftlcd_dev.vspw + tftlcd_dev.vfpd + tftlcd_dev.vbpd;//VSYNC周期

    /*
	  * 初始化ELCDIF的VDCTRL2寄存器
	  * 设置HSYNC周期
	  * bit[31:18] ：hsw
	  * bit[17:0]  : HSYNC总周期
	  */
    LCDIF->VDCTRL2 = (tftlcd_dev.hspw << 18) | (tftlcd_dev.width + tftlcd_dev.hspw + tftlcd_dev.hfpd + tftlcd_dev.hbpd);

    /*
	 * 初始化ELCDIF的VDCTRL3寄存器
	 * 设置HSYNC周期
	 * bit[27:16] ：水平等待时钟数
	 * bit[15:0]  : 垂直等待时钟数
	 */
    LCDIF->VDCTRL3 = ((tftlcd_dev.hbpd + tftlcd_dev.hspw) << 16) | (tftlcd_dev.vbpd + tftlcd_dev.vspw);

    /*
	 * 初始化ELCDIF的VDCTRL4寄存器
	 * 设置HSYNC周期
	 * bit[18] 1 : 当使用VSHYNC、HSYNC、DOTCLK的话此为置1
	 * bit[17:0]  : 宽度
	 */

    LCDIF->VDCTRL4 = (1 << 18) | (tftlcd_dev.width);

    /*
     * 初始化ELCDIF的CUR_BUF和NEXT_BUF寄存器
     * 设置当前显存地址和下一帧的显存地址
	 */
    LCDIF->CUR_BUF = (unsigned int) tftlcd_dev.framebuffer;
    LCDIF->NEXT_BUF = (unsigned int) tftlcd_dev.framebuffer;


    Bsp_LCD_Enable(); /* 使能LCD 	*/
    Driver_Delay_MS(10);
    lcd_clear(LCD_WHITE); /* 清屏 		*/
}

void Bsp_LCD_InitRGB565(void) {
    uint16_t lcdid = Bsp_LCD_ReadID();
    tftlcd_dev.id = lcdid;
    Bsp_LCD_IOInit();

    Bsp_LCD_Reset();     /* 复位LCD  			*/
    Driver_Delay_MS(10); /* 延时10ms 			*/
    Bsp_LCD_UnReset();   /* 结束复位 			*/

    /* TFTLCD参数结构体初始化 */
    tftlcd_dev.height = 600;
    tftlcd_dev.width = 1024;
    tftlcd_dev.vspw = 3;
    tftlcd_dev.vbpd = 20;
    tftlcd_dev.vfpd = 12;
    tftlcd_dev.hspw = 20;
    tftlcd_dev.hbpd = 140;
    tftlcd_dev.hfpd = 160;
    Bsp_LCD_ClkInit(32, 3, 5); /* 初始化LCD时钟 51.2MHz */
    tftlcd_dev.pixsize = 4;    /* TODO 每个像素2字节 */
    tftlcd_dev.framebuffer = LCD_FRAMEBUF_ADDR;
    tftlcd_dev.backcolor = LCD_WHITE; /* 背景色为白色 */
    tftlcd_dev.forecolor = LCD_BLACK; /* 前景色为黑色 */

    /* 初始化ELCDIF的CTRL寄存器
     * bit [31] 0 : 停止复位
     * bit [19] 1 : 旁路计数器模式
     * bit [17] 1 : LCD工作在dotclk模式
     * bit [15:14] 00 : 输入数据不交换（是否需要交换字节顺序参考屏幕说明书）
     * bit [13:12] 00 : CSC不交换
     * bit [11:10] 11 : TODO 24位总线宽度
     * bit [9:8]   00 : TODO 16位像素比特,也就是RGB565
     * bit [5]     1  : elcdif工作在主模式
     * bit [1]     0  : 所有的24位均有效
	 */
    LCDIF->CTRL |= (1 << 19) | (1 << 17) | (0 << 14) | (0 << 12) |
                   (3 << 10) | (0 << 8) | (1 << 5) | (0 << 1);
    /*
     * 初始化ELCDIF的寄存器CTRL1
     * bit [19:16]  : 0X7 ARGB模式下，传输24位数据，A通道不用传输
     *                0Xf TODO 16位有效
	 */
    LCDIF->CTRL1 &= ~(0xf << 16);
    LCDIF->CTRL1 |= (0xf << 16);

    /*
      * 初始化ELCDIF的寄存器TRANSFER_COUNT寄存器
      * bit [31:16]  : 高度
      * bit [15:0]   : 宽度
	  */
    LCDIF->TRANSFER_COUNT = (tftlcd_dev.height << 16) | (tftlcd_dev.width << 0);

    /*
     * 初始化ELCDIF的VDCTRL0寄存器
     * bit [29] 0 : VSYNC输出
     * bit [28] 1 : 使能ENABLE输出
     * bit [27] 0 : VSYNC低电平有效
     * bit [26] 0 : HSYNC低电平有效
     * bit [25] 0 : DOTCLK上升沿有效
     * bit [24] 1 : ENABLE信号高电平有效
     * bit [21] 1 : DOTCLK模式下设置为1
     * bit [20] 1 : DOTCLK模式下设置为1
     * bit [17:0] : vsw参数
	 */
    LCDIF->VDCTRL0 = 0;   //先清零
    if (lcdid == ATKVGA) {//VGA需要特殊处理
        LCDIF->VDCTRL0 = (0 << 29) | (1 << 28) | (0 << 27) |
                         (0 << 26) | (1 << 25) | (0 << 24) |
                         (1 << 21) | (1 << 20) | (tftlcd_dev.vspw << 0);
    } else {
        LCDIF->VDCTRL0 = (0 << 29) | (1 << 28) | (0 << 27) |
                         (0 << 26) | (0 << 25) | (1 << 24) |
                         (1 << 21) | (1 << 20) | (tftlcd_dev.vspw << 0);
    }

    /*
	 * 初始化ELCDIF的VDCTRL1寄存器
	 * 设置VSYNC总周期
	 */
    LCDIF->VDCTRL1 = tftlcd_dev.height + tftlcd_dev.vspw + tftlcd_dev.vfpd + tftlcd_dev.vbpd;//VSYNC周期

    /*
	  * 初始化ELCDIF的VDCTRL2寄存器
	  * 设置HSYNC周期
	  * bit[31:18] ：hsw
	  * bit[17:0]  : HSYNC总周期
	  */
    LCDIF->VDCTRL2 = (tftlcd_dev.hspw << 18) | (tftlcd_dev.width + tftlcd_dev.hspw + tftlcd_dev.hfpd + tftlcd_dev.hbpd);

    /*
	 * 初始化ELCDIF的VDCTRL3寄存器
	 * 设置HSYNC周期
	 * bit[27:16] ：水平等待时钟数
	 * bit[15:0]  : 垂直等待时钟数
	 */
    LCDIF->VDCTRL3 = ((tftlcd_dev.hbpd + tftlcd_dev.hspw) << 16) | (tftlcd_dev.vbpd + tftlcd_dev.vspw);

    /*
	 * 初始化ELCDIF的VDCTRL4寄存器
	 * 设置HSYNC周期
	 * bit[18] 1 : 当使用VSHYNC、HSYNC、DOTCLK的话此为置1
	 * bit[17:0]  : 宽度
	 */

    LCDIF->VDCTRL4 = (1 << 18) | (tftlcd_dev.width);

    /*
     * 初始化ELCDIF的CUR_BUF和NEXT_BUF寄存器
     * 设置当前显存地址和下一帧的显存地址
	 */
    LCDIF->CUR_BUF = (unsigned int) tftlcd_dev.framebuffer;
    LCDIF->NEXT_BUF = (unsigned int) tftlcd_dev.framebuffer;


    Bsp_LCD_Enable(); /* 使能LCD 	*/
    Driver_Delay_MS(10);
    lcd_clear(LCD_WHITE); /* 清屏 		*/
}

uint16_t Bsp_LCD_ReadID(void) {
    // 打开模拟开关 SGM_CTRL
    GPIO_Config_t config = {0};
    IOMUXC_SetPinMux(IOMUXC_LCD_VSYNC_GPIO3_IO03, 0);
    IOMUXC_SetPinConfig(IOMUXC_LCD_VSYNC_GPIO3_IO03, 0x10b0);
    config.direction = GPIO_DIR_OUTPUT;
    config.defaultStatus = GPIO_PIN_SET;
    Driver_GPIO_Init(GPIO3, 3, &config);

    // 读取LCD板子上三个引脚的状态（对应LCD-ID，标识了是正点原子哪一款LCD）
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA07_GPIO3_IO12, 0);
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA15_GPIO3_IO20, 0);
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA23_GPIO3_IO28, 0);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA07_GPIO3_IO12, 0x10b0);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA15_GPIO3_IO20, 0x10b0);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA23_GPIO3_IO28, 0x10b0);

    config.direction = GPIO_DIR_INPUT;
    Driver_GPIO_Init(GPIO3, 12, &config);
    Driver_GPIO_Init(GPIO3, 20, &config);
    Driver_GPIO_Init(GPIO3, 28, &config);

    GPIO_Pin_Status_t m0 = Driver_GPIO_ReadPin(GPIO3, 12);
    GPIO_Pin_Status_t m1 = Driver_GPIO_ReadPin(GPIO3, 20);
    GPIO_Pin_Status_t m2 = Driver_GPIO_ReadPin(GPIO3, 28);
    uint8_t lcdID = m0 | (m1 << 1) | (m2 << 2);
    if (lcdID == 0x2) {
        LOG_DEBUG("LCD Type: ATK7016");
        return ATK7016;
    }
    return 0;
}

void Bsp_LCD_IOInit(void) {
    GPIO_Config_t gpioConfig;
    /* 1、IO初始化复用功能 */
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA00_LCDIF_DATA00, 0);
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA01_LCDIF_DATA01, 0);
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA02_LCDIF_DATA02, 0);
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA03_LCDIF_DATA03, 0);
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA04_LCDIF_DATA04, 0);
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA05_LCDIF_DATA05, 0);
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA06_LCDIF_DATA06, 0);
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA07_LCDIF_DATA07, 0);

    IOMUXC_SetPinMux(IOMUXC_LCD_DATA08_LCDIF_DATA08, 0);
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA09_LCDIF_DATA09, 0);
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA10_LCDIF_DATA10, 0);
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA11_LCDIF_DATA11, 0);
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA12_LCDIF_DATA12, 0);
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA13_LCDIF_DATA13, 0);
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA14_LCDIF_DATA14, 0);
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA15_LCDIF_DATA15, 0);

    IOMUXC_SetPinMux(IOMUXC_LCD_DATA16_LCDIF_DATA16, 0);

    IOMUXC_SetPinMux(IOMUXC_LCD_DATA17_LCDIF_DATA17, 0);
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA18_LCDIF_DATA18, 0);
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA19_LCDIF_DATA19, 0);
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA20_LCDIF_DATA20, 0);
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA21_LCDIF_DATA21, 0);
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA22_LCDIF_DATA22, 0);
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA23_LCDIF_DATA23, 0);

    IOMUXC_SetPinMux(IOMUXC_LCD_CLK_LCDIF_CLK, 0);
    IOMUXC_SetPinMux(IOMUXC_LCD_ENABLE_LCDIF_ENABLE, 0);
    IOMUXC_SetPinMux(IOMUXC_LCD_HSYNC_LCDIF_HSYNC, 0);
    IOMUXC_SetPinMux(IOMUXC_LCD_VSYNC_LCDIF_VSYNC, 0);

    IOMUXC_SetPinMux(IOMUXC_GPIO1_IO08_GPIO1_IO08, 0); /* 背光BL引脚      */
    IOMUXC_SetPinConfig(IOMUXC_UART1_CTS_B_GPIO1_IO18, 0xF080);


    /* 2、配置LCD IO属性
	 *bit 16:0 HYS关闭
	 *bit [15:14]: 0 默认22K上拉
	 *bit [13]: 0 pull功能
	 *bit [12]: 0 pull/keeper使能
	 *bit [11]: 0 关闭开路输出
	 *bit [7:6]: 10 速度100Mhz
	 *bit [5:3]: 111 驱动能力为R0/7
	 *bit [0]: 1 高转换率
	 */
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA00_LCDIF_DATA00, 0xB9);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA01_LCDIF_DATA01, 0xB9);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA02_LCDIF_DATA02, 0xB9);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA03_LCDIF_DATA03, 0xB9);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA04_LCDIF_DATA04, 0xB9);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA05_LCDIF_DATA05, 0xB9);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA06_LCDIF_DATA06, 0xB9);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA07_LCDIF_DATA07, 0xB9);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA08_LCDIF_DATA08, 0xB9);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA09_LCDIF_DATA09, 0xB9);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA10_LCDIF_DATA10, 0xB9);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA11_LCDIF_DATA11, 0xB9);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA12_LCDIF_DATA12, 0xB9);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA13_LCDIF_DATA13, 0xB9);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA14_LCDIF_DATA14, 0xB9);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA15_LCDIF_DATA15, 0xB9);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA16_LCDIF_DATA16, 0xB9);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA17_LCDIF_DATA17, 0xB9);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA18_LCDIF_DATA18, 0xB9);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA19_LCDIF_DATA19, 0xB9);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA20_LCDIF_DATA20, 0xB9);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA21_LCDIF_DATA21, 0xB9);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA22_LCDIF_DATA22, 0xB9);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA23_LCDIF_DATA23, 0xB9);

    IOMUXC_SetPinConfig(IOMUXC_LCD_CLK_LCDIF_CLK, 0xB9);
    IOMUXC_SetPinConfig(IOMUXC_LCD_ENABLE_LCDIF_ENABLE, 0xB9);
    IOMUXC_SetPinConfig(IOMUXC_LCD_HSYNC_LCDIF_HSYNC, 0xB9);
    IOMUXC_SetPinConfig(IOMUXC_LCD_VSYNC_LCDIF_VSYNC, 0xB9);

    IOMUXC_SetPinConfig(IOMUXC_GPIO1_IO08_GPIO1_IO08, 0xB9); /* 背光BL引脚 		*/

    /* GPIO初始化 */
    gpioConfig.direction = GPIO_DIR_OUTPUT;       /* 输出 			*/
    gpioConfig.defaultStatus = 1;                 /* 默认关闭背光 */
    Driver_GPIO_Init(GPIO1, 8, &gpioConfig);      /* 背光默认打开 */
    Driver_GPIO_WritePIn(GPIO1, 8, GPIO_PIN_SET); /* 打开背光     */
}

/**
 * LCD时钟初始化, LCD时钟计算: LCD CLK = 24 * loopDiv / prediv / div
 * @param pllDiv 锁相环
 * @param preDiv CSCDR2[LCDIF1_PRED]
 * @param div CBCMR[LCDIF1_PODF]
 */
void Bsp_LCD_ClkInit(unsigned char pllDiv, unsigned char preDiv, unsigned char div) {
    /* 先初始化 Video PLL(PLL5)
     * PLL output frequency = Fref * (DIV_SELECT + NUM/DENOM)
     * Fref 24M晶振
 	 */
    CCM_ANALOG->PLL_VIDEO_NUM = 0; /* 不使用小数分频器 */
    CCM_ANALOG->PLL_VIDEO_DENOM = 0;

    /*
     * PLL_VIDEO寄存器设置
     * bit[6:0] :  DIV_SELECT PLL锁相环倍频 例如24M * 32 = 768M
     * bit[13]:    1  使能PLL输出
     * bit[20:19]  2  POST_DIV_SELECT PLL倍频后1分频
	 */
    CCM_ANALOG->PLL_VIDEO = (2 << 19) | (1 << 13) | (pllDiv << 0);

    /*
     * CCM_ANALOG_MISC2n[VIDEO_DIV] POST_DIV_SELECT 之后再次分频
	 */
    CCM_ANALOG->MISC2 &= ~(3 << 30);
    CCM_ANALOG->MISC2 = 0 << 30;// 1分频

    // CSCDR2[LCDIF1_PRE_CLK_SEL] 前置muxer选择PLL5
    CCM->CSCDR2 &= ~(7 << 15);
    CCM->CSCDR2 |= (2 << 15);

    // 分频：CSCDR2[LCDIF1_PRED]
    CCM->CSCDR2 &= ~(7 << 12);
    CCM->CSCDR2 |= (preDiv - 1) << 12; /* 设置分频  */

    // 分频 CBCMR[LCDIF1_PODF]
    CCM->CBCMR &= ~(7 << 23);
    CCM->CBCMR |= (div - 1) << 23;

    // CSCDR2[LCDIF1_CLK_SEL] muxer选择
    CCM->CSCDR2 &= ~(7 << 9);
    CCM->CSCDR2 |= (0 << 9);
}

/*
 * @description		: 画点函数
 * @param - x		: x轴坐标
 * @param - y		: y轴坐标
 * @param - color	: 颜色值
 * @return 			: 无
 */
void lcd_drawpoint(unsigned short x, unsigned short y, unsigned int color) {
    *(unsigned int *) ((unsigned int) tftlcd_dev.framebuffer +
                       tftlcd_dev.pixsize * (tftlcd_dev.width * y + x)) = color;
}


/*
 * @description		: 读取指定点的颜色值
 * @param - x		: x轴坐标
 * @param - y		: y轴坐标
 * @return 			: 读取到的指定点的颜色值
 */
inline unsigned int lcd_readpoint(unsigned short x, unsigned short y) {
    return *(unsigned int *) ((unsigned int) tftlcd_dev.framebuffer +
                              tftlcd_dev.pixsize * (tftlcd_dev.width * y + x));
}

/*
 * @description		: 清屏
 * @param - color	: 颜色值
 * @return 			: 读取到的指定点的颜色值
 */
void lcd_clear(unsigned int color) {
    unsigned int num;
    unsigned int i = 0;

    unsigned int *startaddr = (unsigned int *) tftlcd_dev.framebuffer;//指向帧缓存首地址
    num = (unsigned int) tftlcd_dev.width * tftlcd_dev.height;        //缓冲区总长度
    for (i = 0; i < num; i++) {
        startaddr[i] = color;
    }
}

/*
 * @description		: 以指定的颜色填充一块矩形
 * @param - x0		: 矩形起始点坐标X轴
 * @param - y0		: 矩形起始点坐标Y轴
 * @param - x1		: 矩形终止点坐标X轴
 * @param - y1		: 矩形终止点坐标Y轴
 * @param - color	: 要填充的颜色
 * @return 			: 读取到的指定点的颜色值
 */
void lcd_fill(unsigned short x0, unsigned short y0,
              unsigned short x1, unsigned short y1, unsigned int color) {
    unsigned short x, y;

    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 >= tftlcd_dev.width) x1 = tftlcd_dev.width - 1;
    if (y1 >= tftlcd_dev.height) y1 = tftlcd_dev.height - 1;

    for (y = y0; y <= y1; y++) {
        for (x = x0; x <= x1; x++)
            lcd_drawpoint(x, y, color);
    }
}
