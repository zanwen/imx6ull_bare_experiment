//
// Created by 86157 on 2025/1/17.
//

#ifndef INC_18_TOUCH_FT5426_BSP_OV2640_H
#define INC_18_TOUCH_FT5426_BSP_OV2640_H

#include "imx6ull.h"


/* ATK-MC2640 SCCB通讯地址 */
#define ATK_MC2640_SCCB_ADDR                    0x30

/* ATK-MC2640模块灯光模式枚举 */
typedef enum
{
    ATK_MC2640_LIGHT_MODE_AUTO = 0x00,          /* Auto */
    ATK_MC2640_LIGHT_MODE_SUNNY,                /* Sunny */
    ATK_MC2640_LIGHT_MODE_CLOUDY,               /* Cloudy */
    ATK_MC2640_LIGHT_MODE_OFFICE,               /* Office */
    ATK_MC2640_LIGHT_MODE_HOME,                 /* Home */
} atk_mc2640_light_mode_t;

/* ATK-MC2640模块色彩饱和度枚举 */
typedef enum
{
    ATK_MC2640_COLOR_SATURATION_0 = 0x00,       /* +2 */
    ATK_MC2640_COLOR_SATURATION_1,              /* +1 */
    ATK_MC2640_COLOR_SATURATION_2,              /* 0 */
    ATK_MC2640_COLOR_SATURATION_3,              /* -1 */
    ATK_MC2640_COLOR_SATURATION_4,              /* -2 */
} atk_mc2640_color_saturation_t;

/* ATK-MC2640模块亮度枚举 */
typedef enum
{
    ATK_MC2640_BRIGHTNESS_0 = 0x00,             /* +2 */
    ATK_MC2640_BRIGHTNESS_1,                    /* +1 */
    ATK_MC2640_BRIGHTNESS_2,                    /* 0 */
    ATK_MC2640_BRIGHTNESS_3,                    /* -1 */
    ATK_MC2640_BRIGHTNESS_4,                    /* -2 */
} atk_mc2640_brightness_t;

/* ATK-MC2640模块对比度枚举 */
typedef enum
{
    ATK_MC2640_CONTRAST_0 = 0x00,               /* +2 */
    ATK_MC2640_CONTRAST_1,                      /* +1 */
    ATK_MC2640_CONTRAST_2,                      /* 0 */
    ATK_MC2640_CONTRAST_3,                      /* -1 */
    ATK_MC2640_CONTRAST_4,                      /* -2 */
} atk_mc2640_contrast_t;

/* ATK-MC2640模块特殊效果枚举 */
typedef enum
{
    ATK_MC2640_SPECIAL_EFFECT_ANTIQUE = 0x00,   /* Antique */
    ATK_MC2640_SPECIAL_EFFECT_BLUISH,           /* Bluish */
    ATK_MC2640_SPECIAL_EFFECT_GREENISH,         /* Greenish */
    ATK_MC2640_SPECIAL_EFFECT_REDISH,           /* Redish */
    ATK_MC2640_SPECIAL_EFFECT_BW,               /* B&W */
    ATK_MC2640_SPECIAL_EFFECT_NEGATIVE,         /* Negative */
    ATK_MC2640_SPECIAL_EFFECT_BW_NEGATIVE,      /* B&W Negative */
    ATK_MC2640_SPECIAL_EFFECT_NORMAL,           /* Normal */
} atk_mc2640_special_effect_t;

/* ATK-MC2640输出图像格式枚举 */
typedef enum
{
    ATK_MC2640_OUTPUT_FORMAT_RGB565 = 0x00,     /* RGB565 */
    ATK_MC2640_OUTPUT_FORMAT_JPEG,              /* JPEG */
} atk_mc2640_output_format_t;

/* ATK-MC2640获取帧数据方式枚举 */
typedef enum
{
    ATK_MC2640_GET_TYPE_DTS_8B_NOINC = 0x00,    /* 图像数据以字节方式写入目的地址，目的地址固定不变 */
    ATK_MC2640_GET_TYPE_DTS_8B_INC,             /* 图像数据以字节方式写入目的地址，目的地址自动增加 */
    ATK_MC2640_GET_TYPE_DTS_16B_NOINC,          /* 图像数据以半字方式写入目的地址，目的地址固定不变 */
    ATK_MC2640_GET_TYPE_DTS_16B_INC,            /* 图像数据以半字方式写入目的地址，目的地址自动增加 */
    ATK_MC2640_GET_TYPE_DTS_32B_NOINC,          /* 图像数据以字方式写入目的地址，目的地址固定不变 */
    ATK_MC2640_GET_TYPE_DTS_32B_INC,            /* 图像数据以字方式写入目的地址，目的地址自动增加 */
} atk_mc2640_get_type_t;

/* 错误代码 */
#define ATK_MC2640_EOK      0   /* 没有错误 */
#define ATK_MC2640_ERROR    1   /* 错误 */
#define ATK_MC2640_EINVAL   2   /* 非法参数 */
#define ATK_MC2640_ENOMEM   3   /* 内存不足 */
#define ATK_MC2640_EEMPTY   4   /* 资源为空 */


void Bsp_OV2640_Init(void);
void Bsp_OV2640_Test(void);

#endif //INC_18_TOUCH_FT5426_BSP_OV2640_H
