//
// Created by 86157 on 2025/1/7.
//

#ifndef INC_14_LCD_BSP_LCD_H
#define INC_14_LCD_BSP_LCD_H

#include "imx6ull.h"

#define USE_LCD_RGB888 0

#if USE_LCD_RGB888
    /* 颜色 */
    #define LCD_BLUE 0x000000FF
    #define LCD_GREEN 0x0000FF00
    #define LCD_RED 0x00FF0000
    #define LCD_CYAN 0x0000FFFF
    #define LCD_MAGENTA 0x00FF00FF
    #define LCD_YELLOW 0x00FFFF00
    #define LCD_LIGHTBLUE 0x008080FF
    #define LCD_LIGHTGREEN 0x0080FF80
    #define LCD_LIGHTRED 0x00FF8080
    #define LCD_LIGHTCYAN 0x0080FFFF
    #define LCD_LIGHTMAGENTA 0x00FF80FF
    #define LCD_LIGHTYELLOW 0x00FFFF80
    #define LCD_DARKBLUE 0x00000080
    #define LCD_DARKGREEN 0x00008000
    #define LCD_DARKRED 0x00800000
    #define LCD_DARKCYAN 0x00008080
    #define LCD_DARKMAGENTA 0x00800080
    #define LCD_DARKYELLOW 0x00808000
    #define LCD_WHITE 0x00FFFFFF
    #define LCD_LIGHTGRAY 0x00D3D3D3
    #define LCD_GRAY 0x00808080
    #define LCD_DARKGRAY 0x00404040
    #define LCD_BLACK 0x00000000
    #define LCD_BROWN 0x00A52A2A
    #define LCD_ORANGE 0x00FFA500
    #define LCD_TRANSPARENT 0x00000000
#else
    #define LCD_BLUE         0x001F
    #define LCD_GREEN        0x07E0
    #define LCD_RED          0xF800
    #define LCD_CYAN         0x07FF
    #define LCD_MAGENTA      0xF81F
    #define LCD_YELLOW       0xFFE0
    #define LCD_LIGHTBLUE    0x841F
    #define LCD_LIGHTGREEN   0x87F0
    #define LCD_LIGHTRED     0xFC10
    #define LCD_LIGHTCYAN    0x87FF
    #define LCD_LIGHTMAGENTA 0xFC1F
    #define LCD_LIGHTYELLOW  0xFFF0
    #define LCD_DARKBLUE     0x0010
    #define LCD_DARKGREEN    0x0400
    #define LCD_DARKRED      0x8000
    #define LCD_DARKCYAN     0x0410
    #define LCD_DARKMAGENTA  0x8010
    #define LCD_DARKYELLOW   0x8400
    #define LCD_WHITE        0xFFFF
    #define LCD_LIGHTGRAY    0xD69A
    #define LCD_GRAY         0x8410
    #define LCD_DARKGRAY     0x4208
    #define LCD_BLACK        0x0000
    #define LCD_BROWN        0xA145
    #define LCD_ORANGE       0xFD20
    #define LCD_TRANSPARENT  0x0000
#endif

/* 屏幕ID */
#define ATK4342 0X4342 /* 4.3寸480*272 	*/
#define ATK4384 0X4384 /* 4.3寸800*480 	*/
#define ATK7084 0X7084 /* 7寸800*480 		*/
#define ATK7016 0X7016 /* 7寸1024*600 		*/
#define ATK1018 0X1018 /* 10.1寸1280*800 	*/
#define ATKVGA 0xff00  /* VGA */

/* LCD显存地址 */
#define LCD_FRAMEBUF_ADDR (0x89000000)

/* LCD控制参数结构体 */
struct tftlcd_typedef {
    unsigned short height; /* LCD屏幕高度 */
    unsigned short width;  /* LCD屏幕宽度 */
    unsigned char pixsize; /* LCD每个像素所占字节大小 */
    unsigned short vspw;
    unsigned short vbpd;
    unsigned short vfpd;
    unsigned short hspw;
    unsigned short hbpd;
    unsigned short hfpd;
    unsigned int framebuffer; /* LCD显存首地址   	  */
    unsigned int forecolor;   /* 前景色 */
    unsigned int backcolor;   /* 背景色 */
    unsigned int id;          /*	屏幕ID */
};

extern struct tftlcd_typedef tftlcd_dev;

void Bsp_LCD_Init(void);

void Bsp_LCD_InitRGB888(void);

void Bsp_LCD_InitRGB565(void);

uint16_t Bsp_LCD_ReadID(void);

void Bsp_LCD_IOInit(void);

void Bsp_LCD_ClkInit(unsigned char pllDiv, unsigned char preDiv, unsigned char div);


void lcd_drawpoint(unsigned short x, unsigned short y, unsigned int color);

void lcd_clear(unsigned int color);

void lcd_fill(unsigned short x0, unsigned short y0,
              unsigned short x1, unsigned short y1, unsigned int color);

#endif// INC_14_LCD_BSP_LCD_H
