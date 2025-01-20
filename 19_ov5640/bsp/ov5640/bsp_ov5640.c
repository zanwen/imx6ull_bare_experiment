//
// Created by 86157 on 2025/1/17.
//

#include "bsp_ov5640.h"
#include "driver_i2c.h"
#include "logger.h"
#include "driver_gpio.h"
#include "driver_delay.h"
#include "bsp_ov5640_cfg.h"
#include "driver_csi.h"
#include "bsp_lcd.h"

#define RESET_PIN GPIO1, 2
#define POWER_DOWN_PIN GPIO1, 4

#define RESET_PIN_LOW() Driver_GPIO_WritePIn(RESET_PIN, GPIO_PIN_RESET)
#define RESET_PIN_HIGH() Driver_GPIO_WritePIn(RESET_PIN, GPIO_PIN_SET)

#define POWER_DOWN_PIN_LOW() Driver_GPIO_WritePIn(POWER_DOWN_PIN, GPIO_PIN_RESET)
#define POWER_DOWN_PIN_HIGH() Driver_GPIO_WritePIn(POWER_DOWN_PIN, GPIO_PIN_SET)

#define OV5640_ADDR_WRITE 0x78
#define OV5640_ADDR_READ 0x79

#define REG_CHIP_ID 0x300A

#define OUTPUT_WIDTH 1024
#define OUTPUT_HEIGHT 600

/* ATK-MC5640模块固件下载超时时间，单位：毫秒（ms） */
#define ATK_MC5640_TIMEOUT  5000

/* ATK-MC5640模块数据结构体 */
static struct {
    struct {
        uint16_t width;
        uint16_t height;
    } isp_input;
    struct {
        uint16_t width;
        uint16_t height;
    } pre_scaling;
    struct {
        uint16_t width;
        uint16_t height;
    } output;
} g_atk_mc5640_sta = {0};

uint8_t atk_mc5640_set_brightness(atk_mc5640_brightness_t brightness);

uint8_t atk_mc5640_set_color_saturation(atk_mc5640_color_saturation_t saturation);

uint8_t atk_mc5640_auto_focus_continuance(void);

uint8_t atk_mc5640_set_output_size(uint16_t width, uint16_t height);

uint8_t atk_mc5640_get_output_size(void);

uint8_t Bsp_OV5640_RegInit(void);

uint8_t atk_mc5640_set_output_format(atk_mc5640_output_format_t format);

uint8_t atk_mc5640_set_light_mode(atk_mc5640_light_mode_t mode);

uint8_t atk_mc5640_set_pre_scaling_window(uint16_t x_offset, uint16_t y_offset);

static void atk_mc5640_get_pre_scaling_size(void);

uint8_t atk_mc5640_set_isp_input_window(uint16_t x, uint16_t y, uint16_t width, uint16_t height);

uint8_t atk_mc5640_set_mirror_flip(atk_mc5640_mirror_flip_t mirror_flip);

uint8_t atk_mc5640_auto_focus_init(void);

uint8_t atk_mc5640_set_contrast(
        atk_mc5640_contrast_t contrast);                                            /* 设置ATK-MC5640模块对比度 */
uint8_t atk_mc5640_set_hue(
        atk_mc5640_hue_t hue);                                                           /* 设置ATK-MC5640模块色相 */
uint8_t atk_mc5640_set_special_effect(
        atk_mc5640_special_effect_t effect);                                  /* 设置ATK-MC5640模块特殊效果 */
uint8_t atk_mc5640_set_exposure_level(
        atk_mc5640_exposure_level_t level);                                   /* 设置ATK-MC5640模块曝光度 */
uint8_t atk_mc5640_set_sharpness_level(
        atk_mc5640_sharpness_t sharpness);                                   /* 设置ATK-MC5640模块锐度 */
uint8_t atk_mc5640_set_test_pattern(
        atk_mc5640_test_pattern_t pattern);                                     /* 设置ATK-MC5640模块测试图案 */


void Bsp_OV5640_IOInit(void) {
    /* SCL and SDA */
    // io mux
    IOMUXC_SetPinMux(IOMUXC_UART5_TX_DATA_I2C2_SCL, 1);
    IOMUXC_SetPinMux(IOMUXC_UART5_RX_DATA_I2C2_SDA, 1);
    // pad config open drain 拉低也需要输出驱动器
    IOMUXC_SetPinConfig(IOMUXC_UART5_TX_DATA_I2C2_SCL, 0x08B0);
    IOMUXC_SetPinConfig(IOMUXC_UART5_RX_DATA_I2C2_SDA, 0x08B0);

    /* Reset# and PowerDown */
    // io mux
    IOMUXC_SetPinMux(IOMUXC_GPIO1_IO02_GPIO1_IO02, 0);
    IOMUXC_SetPinMux(IOMUXC_GPIO1_IO04_GPIO1_IO04, 0);
    // gpio
    GPIO_Config_t config = {0};
    config.direction = GPIO_DIR_OUTPUT;
    config.defaultStatus = GPIO_PIN_SET;
    Driver_GPIO_Init(RESET_PIN, &config);
    config.defaultStatus = GPIO_PIN_RESET;
    Driver_GPIO_Init(POWER_DOWN_PIN, &config);
}

void Bsp_OV5640_HardReset(void) {
    RESET_PIN_LOW();
    Driver_Delay_MS(5);
    POWER_DOWN_PIN_LOW();
    Driver_Delay_MS(1);
    RESET_PIN_HIGH();
    Driver_Delay_MS(20);
}

void Bsp_OV5640_WriteBytes(uint16_t regAddr, uint8_t *data, uint8_t len) {
    uint8_t reg[2] = {regAddr >> 8, regAddr};
    Driver_I2C_Start(I2C2);
    Driver_I2C_SendAddr(I2C2, OV5640_ADDR_WRITE);
    Driver_I2C_SendBytes(I2C2, reg, 2);
    Driver_I2C_SendBytes(I2C2, data, len);
    Driver_I2C_Stop(I2C2);
}

inline void Bsp_OV5640_WriteByte(uint16_t regAddr, uint8_t data) {
    Bsp_OV5640_WriteBytes(regAddr, &data, 1);
}

void Bsp_OV5640_ReadBytes(uint16_t regAddr, uint8_t *buf, uint8_t len) {
    uint8_t reg[2] = {regAddr >> 8, regAddr};
    Driver_I2C_Start(I2C2);
    // dummy write
    Driver_I2C_SendAddr(I2C2, OV5640_ADDR_WRITE);
    Driver_I2C_SendBytes(I2C2, reg, 2);

    Driver_I2C_ReStart(I2C2);
    Driver_I2C_SendAddr(I2C2, OV5640_ADDR_READ);
    Driver_I2C_ReadBytesAndStop(I2C2, buf, len);
}

uint8_t Bsp_OV5640_ReadByte(uint16_t regAddr) {
    uint8_t data = 0;
    uint8_t reg[2] = {regAddr >> 8, regAddr};
    Driver_I2C_Start(I2C2);
    // dummy write
    Driver_I2C_SendAddr(I2C2, OV5640_ADDR_WRITE);
    Driver_I2C_SendBytes(I2C2, reg, 2);

    Driver_I2C_ReStart(I2C2);
    Driver_I2C_SendAddr(I2C2, OV5640_ADDR_READ);
    Driver_I2C_ReadBytesAndStop(I2C2, &data, 1);
    return data;
}

void Bsp_OV5640_Test(void) {
    LOG_DEBUG("lcd ctrl = %#x", LCDIF->CTRL);
//    uint8_t read = 0;
//    Bsp_OV5640_ReadBytes(0x4300, &read, 1);
//    LOG_DEBUG("0x4300 = %#x", read);
//    atk_mc5640_get_output_size();
//    atk_mc5640_set_output_size(OUTPUT_WIDTH, OUTPUT_HEIGHT);
}

void Bsp_OV5640_CSI_Init(void) {
    CSI_Config_t config = {1024, 600, 2};
    Driver_CSI_IOInit();
    Driver_CSI_ControllerInit();

    Driver_CSI_DMA_AddrInit(LCD_FRAMEBUF_ADDR);
    Driver_CSI_ImageParamInit(&config);
    Driver_CSI_DMA_ReflashRFF();
    Driver_CSI_Start();
}

void Bsp_OV5640_Init(void) {
    Bsp_OV5640_IOInit();
    Bsp_OV5640_HardReset();
    Driver_I2C_Init(I2C2);
    Bsp_OV5640_RegInit();
    Bsp_OV5640_CSI_Init();
    LOG_DEBUG("Bsp_OV5640_Init done");
}

uint8_t Bsp_OV5640_RegInit(void) {
    Bsp_OV5640_WriteByte(0x3103, 0x11); // sysclk from pad
    Bsp_OV5640_WriteByte(0x3008, 0x82); // software reset
    // delay 5ms
    Driver_Delay_MS(5);
    // Write initialization table
    size_t len = sizeof(atk_mc5640_init_cfg) / sizeof(atk_mc5640_init_cfg[0]);
    for (size_t i = 0; i < len; i++) {
        Bsp_OV5640_WriteByte(atk_mc5640_init_cfg[i].reg, atk_mc5640_init_cfg[i].dat);
    }

    uint8_t ret = 0;
    ret += atk_mc5640_set_output_format(ATK_MC5640_OUTPUT_FORMAT_RGB565);   /* 设置ATK-MC5640输出RGB565图像数据 */
    ret += atk_mc5640_auto_focus_init();                                    /* 初始化ATK-MC5640模块自动对焦 */
    ret += atk_mc5640_auto_focus_continuance();                             /* ATK-MC5640模块持续自动对焦 */
    ret += atk_mc5640_set_light_mode(ATK_MC5640_LIGHT_MODE_ADVANCED_AWB);   /* 设置ATK-MC5640模块灯光模式 */
    ret += atk_mc5640_set_color_saturation(ATK_MC5640_COLOR_SATURATION_4);  /* 设置ATK-MC5640模块色彩饱度 */
    ret += atk_mc5640_set_brightness(ATK_MC5640_BRIGHTNESS_4);              /* 设置ATK-MC5640模块亮度 */
    ret += atk_mc5640_set_contrast(ATK_MC5640_CONTRAST_4);                  /* 设置ATK-MC5640模块对比度 */
    ret += atk_mc5640_set_hue(ATK_MC5640_HUE_6);                            /* 设置ATK-MC5640模块色相 */
    ret += atk_mc5640_set_special_effect(ATK_MC5640_SPECIAL_EFFECT_NORMAL); /* 设置ATK-MC5640模块特殊效果 */
    ret += atk_mc5640_set_exposure_level(ATK_MC5640_EXPOSURE_LEVEL_5);      /* 设置ATK-MC5640模块曝光度 */
    ret += atk_mc5640_set_sharpness_level(ATK_MC5640_SHARPNESS_OFF);        /* 设置ATK-MC5640模块锐度 */
    ret += atk_mc5640_set_mirror_flip(ATK_MC5640_MIRROR_FLIP_1);            /* 设置ATK-MC5640模块镜像/翻转 */
    ret += atk_mc5640_set_test_pattern(ATK_MC5640_TEST_PATTERN_OFF);        /* 设置ATK-MC5640模块测试图案 */
    ret += atk_mc5640_set_output_size(OUTPUT_WIDTH, OUTPUT_HEIGHT);         /* 设置ATK-MC5640模块输出图像尺寸 */
    if (ret != 0) {
        LOG_ERROR("ATK-MC5640 init failed!");
    }
    return ATK_MC5640_EOK;
}

/**
 * @brief       设置ATK-MC5640模块输出图像尺寸
 * @param       width : 实际输出图像的宽度（可能被缩放）
 *              height: 实际输出图像的高度（可能被缩放）
 * @retval      ATK_MC5640_EOK   : 设置ATK-MC5640模块输出图像窗口成功
 */
uint8_t atk_mc5640_get_output_size(void) {
    uint8_t reg3808;
    uint8_t reg3809;
    uint8_t reg380A;
    uint8_t reg380B;
    uint16_t x_output_size;
    uint16_t y_output_size;

    Bsp_OV5640_ReadBytes(0x3808, &reg3808, 1);
    Bsp_OV5640_ReadBytes(0x3809, &reg3809, 1);
    Bsp_OV5640_ReadBytes(0x380A, &reg380A, 1);
    Bsp_OV5640_ReadBytes(0x380B, &reg380B, 1);

    x_output_size = (uint16_t) ((reg3808 & 0x0F) << 8) | reg3809;
    y_output_size = (uint16_t) ((reg380A & 0x07) << 8) | reg380B;

    LOG_DEBUG("reg3808 = %#x, reg3809 = %#x, reg380A = %#x, reg380B = %#x, "
              "x_output_size = %u, y_output_size = %u",
              reg3808, reg3809, reg380A, reg380B, x_output_size, y_output_size);
    return ATK_MC5640_EOK;
}

/**
 * @brief       设置ATK-MC5640模块输出图像尺寸
 * @param       width : 实际输出图像的宽度（可能被缩放）
 *              height: 实际输出图像的高度（可能被缩放）
 * @retval      ATK_MC5640_EOK   : 设置ATK-MC5640模块输出图像窗口成功
 */
uint8_t atk_mc5640_set_output_size(uint16_t width, uint16_t height) {
    uint8_t reg3808;
    uint8_t reg3809;
    uint8_t reg380A;
    uint8_t reg380B;

    reg3808 = Bsp_OV5640_ReadByte(0x3808);
    reg380A = Bsp_OV5640_ReadByte(0x380A);

    reg3808 &= ~0x0F;
    reg3808 |= (uint8_t) (width >> 8) & 0x0F;
    reg3809 = (uint8_t) width & 0xFF;
    reg380A &= ~0x07;
    reg380A |= (uint8_t) (height >> 8) & 0x07;
    reg380B = (uint8_t) height & 0xFF;

    Bsp_OV5640_WriteByte(0x3212, 0x03);
    Bsp_OV5640_WriteByte(0x3808, reg3808);
    Bsp_OV5640_WriteByte(0x3809, reg3809);
    Bsp_OV5640_WriteByte(0x380A, reg380A);
    Bsp_OV5640_WriteByte(0x380B, reg380B);
    Bsp_OV5640_WriteByte(0x3212, 0x13);
    Bsp_OV5640_WriteByte(0x3212, 0xA3);

    Driver_Delay_MS(500);
    atk_mc5640_get_output_size();

    return ATK_MC5640_EOK;
}

/**
 * @brief       设置ATK-MC5640模块输出图像格式
 * @param       mode: ATK_MC5640_OUTPUT_FORMAT_RGB565: RGB565
 *                    ATK_MC5640_OUTPUT_FORMAT_JPEG  : JPEG
 * @retval      ATK_MC5640_EOK   : 设置ATK-MC5640模块输出图像格式成功
 *              ATK_MC5640_EINVAL: 传入参数错误
 */
uint8_t atk_mc5640_set_output_format(atk_mc5640_output_format_t format) {
    uint32_t cfg_index;
    switch (format) {
        case ATK_MC5640_OUTPUT_FORMAT_RGB565: {
            for (cfg_index = 0;
                 cfg_index < sizeof(atk_mc5640_rgb565_cfg) / sizeof(atk_mc5640_rgb565_cfg[0]); cfg_index++) {
                Bsp_OV5640_WriteByte(atk_mc5640_rgb565_cfg[cfg_index].reg, atk_mc5640_rgb565_cfg[cfg_index].dat);
            }
            break;
        }
        case ATK_MC5640_OUTPUT_FORMAT_JPEG: {
            for (cfg_index = 0; cfg_index < sizeof(atk_mc5640_jpeg_cfg) / sizeof(atk_mc5640_jpeg_cfg[0]); cfg_index++) {
                Bsp_OV5640_WriteByte(atk_mc5640_jpeg_cfg[cfg_index].reg, atk_mc5640_jpeg_cfg[cfg_index].dat);
            }
            break;
        }
        default: {
            return ATK_MC5640_EINVAL;
        }
    }

    return ATK_MC5640_EOK;
}

/**
 * @brief       设置ATK-MC5640模块灯光模式
 * @param       mode: ATK_MC5640_LIGHT_MODE_ADVANCED_AWB : Advanced AWB
 *                    ATK_MC5640_LIGHT_MODE_SIMPLE_AWB   : Simple AWB
 *                    ATK_MC5640_LIGHT_MODE_MANUAL_DAY   : Manual day
 *                    ATK_MC5640_LIGHT_MODE_MANUAL_A     : Manual A
 *                    ATK_MC5640_LIGHT_MODE_MANUAL_CWF   : Manual cwf
 *                    ATK_MC5640_LIGHT_MODE_MANUAL_CLOUDY: Manual cloudy
 * @retval      ATK_MC5640_EOK   : 设置ATK-MC5640模块灯光模式成功
 *              ATK_MC5640_EINVAL: 传入参数错误
 */
uint8_t atk_mc5640_set_light_mode(atk_mc5640_light_mode_t mode) {
    switch (mode) {
        case ATK_MC5640_LIGHT_MODE_ADVANCED_AWB: {
            Bsp_OV5640_WriteByte(0x3406, 0x00);
            Bsp_OV5640_WriteByte(0x5192, 0x04);
            Bsp_OV5640_WriteByte(0x5191, 0xF8);
            Bsp_OV5640_WriteByte(0x5193, 0x70);
            Bsp_OV5640_WriteByte(0x5194, 0xF0);
            Bsp_OV5640_WriteByte(0x5195, 0xF0);
            Bsp_OV5640_WriteByte(0x518D, 0x3D);
            Bsp_OV5640_WriteByte(0x518F, 0x54);
            Bsp_OV5640_WriteByte(0x518E, 0x3D);
            Bsp_OV5640_WriteByte(0x5190, 0x54);
            Bsp_OV5640_WriteByte(0x518B, 0xA8);
            Bsp_OV5640_WriteByte(0x518C, 0xA8);
            Bsp_OV5640_WriteByte(0x5187, 0x18);
            Bsp_OV5640_WriteByte(0x5188, 0x18);
            Bsp_OV5640_WriteByte(0x5189, 0x6E);
            Bsp_OV5640_WriteByte(0x518A, 0x68);
            Bsp_OV5640_WriteByte(0x5186, 0x1C);
            Bsp_OV5640_WriteByte(0x5181, 0x50);
            Bsp_OV5640_WriteByte(0x5184, 0x25);
            Bsp_OV5640_WriteByte(0x5182, 0x11);
            Bsp_OV5640_WriteByte(0x5183, 0x14);
            Bsp_OV5640_WriteByte(0x5184, 0x25);
            Bsp_OV5640_WriteByte(0x5185, 0x24);
            break;
        }
        case ATK_MC5640_LIGHT_MODE_SIMPLE_AWB: {
            Bsp_OV5640_WriteByte(0x3406, 0x00);
            Bsp_OV5640_WriteByte(0x5183, 0x94);
            Bsp_OV5640_WriteByte(0x5191, 0xFF);
            Bsp_OV5640_WriteByte(0x5192, 0x00);
            break;
        }
        case ATK_MC5640_LIGHT_MODE_MANUAL_DAY: {
            Bsp_OV5640_WriteByte(0x3406, 0x01);
            Bsp_OV5640_WriteByte(0x3400, 0x06);
            Bsp_OV5640_WriteByte(0x3401, 0x1C);
            Bsp_OV5640_WriteByte(0x3402, 0x04);
            Bsp_OV5640_WriteByte(0x3403, 0x00);
            Bsp_OV5640_WriteByte(0x3404, 0x04);
            Bsp_OV5640_WriteByte(0x3405, 0xF3);
            break;
        }
        case ATK_MC5640_LIGHT_MODE_MANUAL_A: {
            Bsp_OV5640_WriteByte(0x3406, 0x01);
            Bsp_OV5640_WriteByte(0x3400, 0x04);
            Bsp_OV5640_WriteByte(0x3401, 0x10);
            Bsp_OV5640_WriteByte(0x3402, 0x04);
            Bsp_OV5640_WriteByte(0x3403, 0x00);
            Bsp_OV5640_WriteByte(0x3404, 0x08);
            Bsp_OV5640_WriteByte(0x3405, 0xB6);
            break;
        }
        case ATK_MC5640_LIGHT_MODE_MANUAL_CWF: {
            Bsp_OV5640_WriteByte(0x3406, 0x01);
            Bsp_OV5640_WriteByte(0x3400, 0x05);
            Bsp_OV5640_WriteByte(0x3401, 0x48);
            Bsp_OV5640_WriteByte(0x3402, 0x04);
            Bsp_OV5640_WriteByte(0x3403, 0x00);
            Bsp_OV5640_WriteByte(0x3404, 0x07);
            Bsp_OV5640_WriteByte(0x3405, 0xCF);
            break;
        }
        case ATK_MC5640_LIGHT_MODE_MANUAL_CLOUDY: {
            Bsp_OV5640_WriteByte(0x3406, 0x01);
            Bsp_OV5640_WriteByte(0x3400, 0x06);
            Bsp_OV5640_WriteByte(0x3401, 0x48);
            Bsp_OV5640_WriteByte(0x3402, 0x04);
            Bsp_OV5640_WriteByte(0x3403, 0x00);
            Bsp_OV5640_WriteByte(0x3404, 0x04);
            Bsp_OV5640_WriteByte(0x3405, 0xD3);
            break;
        }
        default: {
            return ATK_MC5640_EINVAL;
        }
    }

    return ATK_MC5640_EOK;
}

/**
 * @brief       初始化ATK-MC5640模块自动对焦
 * @param       无
 * @retval      ATK_MC5640_EOK     : ATK-MC5640模块自动对焦初始化成功
 *              ATK_MC5640_ETIMEOUT: ATK-MC5640模块下载固件超时
 */
uint8_t atk_mc5640_auto_focus_init(void) {
    uint32_t fw_index;
    uint16_t addr_index;
    uint8_t reg3029 = 0;
    uint16_t timeout = 0;

    Bsp_OV5640_WriteByte(0x3000, 0x20);

    for (addr_index = ATK_MC5640_FW_DOWNLOAD_ADDR, fw_index = 0;
         fw_index < sizeof(atk_mc5640_auto_focus_firmware); addr_index++, fw_index++) {
        Bsp_OV5640_WriteByte(addr_index, atk_mc5640_auto_focus_firmware[fw_index]);
    }

    Bsp_OV5640_WriteByte(0x3022, 0x00);
    Bsp_OV5640_WriteByte(0x3023, 0x00);
    Bsp_OV5640_WriteByte(0x3024, 0x00);
    Bsp_OV5640_WriteByte(0x3025, 0x00);
    Bsp_OV5640_WriteByte(0x3026, 0x00);
    Bsp_OV5640_WriteByte(0x3027, 0x00);
    Bsp_OV5640_WriteByte(0x3028, 0x00);
    Bsp_OV5640_WriteByte(0x3029, 0x7F);
    Bsp_OV5640_WriteByte(0x3000, 0x00);

    while ((reg3029 != 0x70) && (timeout < ATK_MC5640_TIMEOUT)) {
        Driver_Delay_MS(1);
        reg3029 = Bsp_OV5640_ReadByte(0x3029);
        timeout++;
    }

    if (reg3029 != 0x70) {
        return ATK_MC5640_ETIMEOUT;
    }

    return ATK_MC5640_EOK;
}

/**
 * @brief       ATK-MC5640模块持续自动对焦
 * @param       无
 * @retval      ATK_MC5640_EOK     : ATK-MC5640模块持续自动对焦成功
 *              ATK_MC5640_ETIMEOUT: ATK-MC5640模块持续自动对焦超时
 */
uint8_t atk_mc5640_auto_focus_continuance(void) {
    uint8_t reg3023 = ~0;
    uint16_t timeout = 0;

    Bsp_OV5640_WriteByte(0x3023, 0x01);
    Bsp_OV5640_WriteByte(0x3022, 0x08);

    while ((reg3023 != 0x00) && (timeout < ATK_MC5640_TIMEOUT)) {
        Driver_Delay_MS(1);
        reg3023 = Bsp_OV5640_ReadByte(0x3023);
        timeout++;
    }

    if (reg3023 != 0x00) {
        return ATK_MC5640_ETIMEOUT;
    }

    reg3023 = ~0;
    timeout = 0;

    Bsp_OV5640_WriteByte(0x3023, 0x01);
    Bsp_OV5640_WriteByte(0x3022, 0x04);

    while ((reg3023 != 0x00) && (timeout < ATK_MC5640_TIMEOUT)) {
        Driver_Delay_MS(1);
        reg3023 = Bsp_OV5640_ReadByte(0x3023);
        timeout++;
    }

    if (reg3023 != 0x00) {
        return ATK_MC5640_ETIMEOUT;
    }

    return ATK_MC5640_EOK;
}

/**
 * @brief       设置ATK-MC5640模块色彩饱和度
 * @param       saturation: ATK_MC5640_COLOR_SATURATION_0: +4
 *                          ATK_MC5640_COLOR_SATURATION_1: +3
 *                          ATK_MC5640_COLOR_SATURATION_2: +2
 *                          ATK_MC5640_COLOR_SATURATION_3: +1
 *                          ATK_MC5640_COLOR_SATURATION_4: 0
 *                          ATK_MC5640_COLOR_SATURATION_5: -1
 *                          ATK_MC5640_COLOR_SATURATION_6: -2
 *                          ATK_MC5640_COLOR_SATURATION_7: -3
 *                          ATK_MC5640_COLOR_SATURATION_8: -4
 * @retval      ATK_MC5640_EOK   : 设置ATK-MC5640模块色彩饱和度成功
 *              ATK_MC5640_EINVAL: 传入参数错误
 */
uint8_t atk_mc5640_set_color_saturation(atk_mc5640_color_saturation_t saturation) {
    switch (saturation) {
        case ATK_MC5640_COLOR_SATURATION_0: {
            Bsp_OV5640_WriteByte(0x5001, 0xFF);
            Bsp_OV5640_WriteByte(0x5583, 0x80);
            Bsp_OV5640_WriteByte(0x5584, 0x80);
            Bsp_OV5640_WriteByte(0x5580, 0x02);
            Bsp_OV5640_WriteByte(0x5588, 0x41);
            break;
        }
        case ATK_MC5640_COLOR_SATURATION_1: {
            Bsp_OV5640_WriteByte(0x5001, 0xFF);
            Bsp_OV5640_WriteByte(0x5583, 0x70);
            Bsp_OV5640_WriteByte(0x5584, 0x70);
            Bsp_OV5640_WriteByte(0x5580, 0x02);
            Bsp_OV5640_WriteByte(0x5588, 0x41);
            break;
        }
        case ATK_MC5640_COLOR_SATURATION_2: {
            Bsp_OV5640_WriteByte(0x5001, 0xFF);
            Bsp_OV5640_WriteByte(0x5583, 0x60);
            Bsp_OV5640_WriteByte(0x5584, 0x60);
            Bsp_OV5640_WriteByte(0x5580, 0x02);
            Bsp_OV5640_WriteByte(0x5588, 0x41);
            break;
        }
        case ATK_MC5640_COLOR_SATURATION_3: {
            Bsp_OV5640_WriteByte(0x5001, 0xFF);
            Bsp_OV5640_WriteByte(0x5583, 0x50);
            Bsp_OV5640_WriteByte(0x5584, 0x50);
            Bsp_OV5640_WriteByte(0x5580, 0x02);
            Bsp_OV5640_WriteByte(0x5588, 0x41);
            break;
        }
        case ATK_MC5640_COLOR_SATURATION_4: {
            Bsp_OV5640_WriteByte(0x5001, 0xFF);
            Bsp_OV5640_WriteByte(0x5583, 0x40);
            Bsp_OV5640_WriteByte(0x5584, 0x40);
            Bsp_OV5640_WriteByte(0x5580, 0x02);
            Bsp_OV5640_WriteByte(0x5588, 0x41);
            break;
        }
        case ATK_MC5640_COLOR_SATURATION_5: {
            Bsp_OV5640_WriteByte(0x5001, 0xFF);
            Bsp_OV5640_WriteByte(0x5583, 0x30);
            Bsp_OV5640_WriteByte(0x5584, 0x30);
            Bsp_OV5640_WriteByte(0x5580, 0x02);
            Bsp_OV5640_WriteByte(0x5588, 0x41);
            break;
        }
        case ATK_MC5640_COLOR_SATURATION_6: {
            Bsp_OV5640_WriteByte(0x5001, 0xFF);
            Bsp_OV5640_WriteByte(0x5583, 0x20);
            Bsp_OV5640_WriteByte(0x5584, 0x20);
            Bsp_OV5640_WriteByte(0x5580, 0x02);
            Bsp_OV5640_WriteByte(0x5588, 0x41);
            break;
        }
        case ATK_MC5640_COLOR_SATURATION_7: {
            Bsp_OV5640_WriteByte(0x5001, 0xFF);
            Bsp_OV5640_WriteByte(0x5583, 0x10);
            Bsp_OV5640_WriteByte(0x5584, 0x10);
            Bsp_OV5640_WriteByte(0x5580, 0x02);
            Bsp_OV5640_WriteByte(0x5588, 0x41);
            break;
        }
        case ATK_MC5640_COLOR_SATURATION_8: {
            Bsp_OV5640_WriteByte(0x5001, 0xFF);
            Bsp_OV5640_WriteByte(0x5583, 0x00);
            Bsp_OV5640_WriteByte(0x5584, 0x00);
            Bsp_OV5640_WriteByte(0x5580, 0x02);
            Bsp_OV5640_WriteByte(0x5588, 0x41);
            break;
        }
        default: {
            return ATK_MC5640_EINVAL;
        }
    }

    return ATK_MC5640_EOK;
}

/**
 * @brief       设置ATK-MC5640模块亮度
 * @param       brightness: ATK_MC5640_BRIGHTNESS_0: +4
 *                          ATK_MC5640_BRIGHTNESS_1: +3
 *                          ATK_MC5640_BRIGHTNESS_2: +2
 *                          ATK_MC5640_BRIGHTNESS_3: +1
 *                          ATK_MC5640_BRIGHTNESS_4: 0
 *                          ATK_MC5640_BRIGHTNESS_5: -1
 *                          ATK_MC5640_BRIGHTNESS_6: -2
 *                          ATK_MC5640_BRIGHTNESS_7: -3
 *                          ATK_MC5640_BRIGHTNESS_8: -4
 * @retval      ATK_MC5640_EOK   : 设置ATK-MC5640模块亮度成功
 *              ATK_MC5640_EINVAL: 传入参数错误
 */
uint8_t atk_mc5640_set_brightness(atk_mc5640_brightness_t brightness) {
    switch (brightness) {
        case ATK_MC5640_BRIGHTNESS_0: {
            Bsp_OV5640_WriteByte(0x5001, 0xFF);
            Bsp_OV5640_WriteByte(0x5587, 0x40);
            Bsp_OV5640_WriteByte(0x5580, 0x04);
            Bsp_OV5640_WriteByte(0x5588, 0x01);
            break;
        }
        case ATK_MC5640_BRIGHTNESS_1: {
            Bsp_OV5640_WriteByte(0x5001, 0xFF);
            Bsp_OV5640_WriteByte(0x5587, 0x30);
            Bsp_OV5640_WriteByte(0x5580, 0x04);
            Bsp_OV5640_WriteByte(0x5588, 0x01);
            break;
        }
        case ATK_MC5640_BRIGHTNESS_2: {
            Bsp_OV5640_WriteByte(0x5001, 0xFF);
            Bsp_OV5640_WriteByte(0x5587, 0x20);
            Bsp_OV5640_WriteByte(0x5580, 0x04);
            Bsp_OV5640_WriteByte(0x5588, 0x01);
            break;
        }
        case ATK_MC5640_BRIGHTNESS_3: {
            Bsp_OV5640_WriteByte(0x5001, 0xFF);
            Bsp_OV5640_WriteByte(0x5587, 0x10);
            Bsp_OV5640_WriteByte(0x5580, 0x04);
            Bsp_OV5640_WriteByte(0x5588, 0x01);
            break;
        }
        case ATK_MC5640_BRIGHTNESS_4: {
            Bsp_OV5640_WriteByte(0x5001, 0xFF);
            Bsp_OV5640_WriteByte(0x5587, 0x00);
            Bsp_OV5640_WriteByte(0x5580, 0x04);
            Bsp_OV5640_WriteByte(0x5588, 0x01);
            break;
        }
        case ATK_MC5640_BRIGHTNESS_5: {
            Bsp_OV5640_WriteByte(0x5001, 0xFF);
            Bsp_OV5640_WriteByte(0x5587, 0x10);
            Bsp_OV5640_WriteByte(0x5580, 0x04);
            Bsp_OV5640_WriteByte(0x5588, 0x09);
            break;
        }
        case ATK_MC5640_BRIGHTNESS_6: {
            Bsp_OV5640_WriteByte(0x5001, 0xFF);
            Bsp_OV5640_WriteByte(0x5587, 0x20);
            Bsp_OV5640_WriteByte(0x5580, 0x04);
            Bsp_OV5640_WriteByte(0x5588, 0x09);
            break;
        }
        case ATK_MC5640_BRIGHTNESS_7: {
            Bsp_OV5640_WriteByte(0x5001, 0xFF);
            Bsp_OV5640_WriteByte(0x5587, 0x30);
            Bsp_OV5640_WriteByte(0x5580, 0x04);
            Bsp_OV5640_WriteByte(0x5588, 0x09);
            break;
        }
        case ATK_MC5640_BRIGHTNESS_8: {
            Bsp_OV5640_WriteByte(0x5001, 0xFF);
            Bsp_OV5640_WriteByte(0x5587, 0x40);
            Bsp_OV5640_WriteByte(0x5580, 0x04);
            Bsp_OV5640_WriteByte(0x5588, 0x09);
            break;
        }
        default: {
            return ATK_MC5640_EINVAL;
        }
    }

    return ATK_MC5640_EOK;
}

/**
 * @brief       设置ATK-MC5640模块对比度
 * @param       contrast: ATK_MC5640_CONTRAST_0: +4
 *                        ATK_MC5640_CONTRAST_1: +3
 *                        ATK_MC5640_CONTRAST_2: +2
 *                        ATK_MC5640_CONTRAST_3: +1
 *                        ATK_MC5640_CONTRAST_4: 0
 *                        ATK_MC5640_CONTRAST_5: -1
 *                        ATK_MC5640_CONTRAST_6: -2
 *                        ATK_MC5640_CONTRAST_7: -3
 *                        ATK_MC5640_CONTRAST_8: -4
 * @retval      ATK_MC5640_EOK   : 设置ATK-MC5640模块对比度成功
 *              ATK_MC5640_EINVAL: 传入参数错误
 */
uint8_t atk_mc5640_set_contrast(atk_mc5640_contrast_t contrast) {
    switch (contrast) {
        case ATK_MC5640_CONTRAST_0: {
            Bsp_OV5640_WriteByte(0x5001, 0xFF);
            Bsp_OV5640_WriteByte(0x5580, 0x04);
            Bsp_OV5640_WriteByte(0x5586, 0x30);
            Bsp_OV5640_WriteByte(0x5585, 0x30);
            Bsp_OV5640_WriteByte(0x5588, 0x41);
            break;
        }
        case ATK_MC5640_CONTRAST_1: {
            Bsp_OV5640_WriteByte(0x5001, 0xFF);
            Bsp_OV5640_WriteByte(0x5580, 0x04);
            Bsp_OV5640_WriteByte(0x5586, 0x2C);
            Bsp_OV5640_WriteByte(0x5585, 0x2C);
            Bsp_OV5640_WriteByte(0x5588, 0x41);
            break;
        }
        case ATK_MC5640_CONTRAST_2: {
            Bsp_OV5640_WriteByte(0x5001, 0xFF);
            Bsp_OV5640_WriteByte(0x5580, 0x04);
            Bsp_OV5640_WriteByte(0x5586, 0x28);
            Bsp_OV5640_WriteByte(0x5585, 0x28);
            Bsp_OV5640_WriteByte(0x5588, 0x41);
            break;
        }
        case ATK_MC5640_CONTRAST_3: {
            Bsp_OV5640_WriteByte(0x5001, 0xFF);
            Bsp_OV5640_WriteByte(0x5580, 0x04);
            Bsp_OV5640_WriteByte(0x5586, 0x24);
            Bsp_OV5640_WriteByte(0x5585, 0x24);
            Bsp_OV5640_WriteByte(0x5588, 0x41);
            break;
        }
        case ATK_MC5640_CONTRAST_4: {
            Bsp_OV5640_WriteByte(0x5001, 0xFF);
            Bsp_OV5640_WriteByte(0x5580, 0x04);
            Bsp_OV5640_WriteByte(0x5586, 0x20);
            Bsp_OV5640_WriteByte(0x5585, 0x20);
            Bsp_OV5640_WriteByte(0x5588, 0x41);
            break;
        }
        case ATK_MC5640_CONTRAST_5: {
            Bsp_OV5640_WriteByte(0x5001, 0xFF);
            Bsp_OV5640_WriteByte(0x5580, 0x04);
            Bsp_OV5640_WriteByte(0x5586, 0x1C);
            Bsp_OV5640_WriteByte(0x5585, 0x1C);
            Bsp_OV5640_WriteByte(0x5588, 0x41);
            break;
        }
        case ATK_MC5640_CONTRAST_6: {
            Bsp_OV5640_WriteByte(0x5001, 0xFF);
            Bsp_OV5640_WriteByte(0x5580, 0x04);
            Bsp_OV5640_WriteByte(0x5586, 0x18);
            Bsp_OV5640_WriteByte(0x5585, 0x18);
            Bsp_OV5640_WriteByte(0x5588, 0x41);
            break;
        }
        case ATK_MC5640_CONTRAST_7: {
            Bsp_OV5640_WriteByte(0x5001, 0xFF);
            Bsp_OV5640_WriteByte(0x5580, 0x04);
            Bsp_OV5640_WriteByte(0x5586, 0x14);
            Bsp_OV5640_WriteByte(0x5585, 0x14);
            Bsp_OV5640_WriteByte(0x5588, 0x41);
            break;
        }
        case ATK_MC5640_CONTRAST_8: {
            Bsp_OV5640_WriteByte(0x5001, 0xFF);
            Bsp_OV5640_WriteByte(0x5580, 0x04);
            Bsp_OV5640_WriteByte(0x5586, 0x10);
            Bsp_OV5640_WriteByte(0x5585, 0x10);
            Bsp_OV5640_WriteByte(0x5588, 0x41);
            break;
        }
        default: {
            return ATK_MC5640_EINVAL;
        }
    }

    return ATK_MC5640_EOK;
}

/**
 * @brief       设置ATK-MC5640模块色相
 * @param       contrast: ATK_MC5640_HUE_0 : -180 degree
 *                        ATK_MC5640_HUE_1 : -150 degree
 *                        ATK_MC5640_HUE_2 : -120 degree
 *                        ATK_MC5640_HUE_3 : -90 degree
 *                        ATK_MC5640_HUE_4 : -60 degree
 *                        ATK_MC5640_HUE_5 : -30 degree
 *                        ATK_MC5640_HUE_6 : 0 degree
 *                        ATK_MC5640_HUE_7 : +30 degree
 *                        ATK_MC5640_HUE_8 : +60 degree
 *                        ATK_MC5640_HUE_9 : +90 degree
 *                        ATK_MC5640_HUE_10: +120 degree
 *                        ATK_MC5640_HUE_11: +150 degree
 * @retval      ATK_MC5640_EOK   : 设置ATK-MC5640模块色相成功
 *              ATK_MC5640_EINVAL: 传入参数错误
 */
uint8_t atk_mc5640_set_hue(atk_mc5640_hue_t hue) {
    switch (hue) {
        case ATK_MC5640_HUE_0: {
            Bsp_OV5640_WriteByte(0x5001, 0xFF);
            Bsp_OV5640_WriteByte(0x5580, 0x01);
            Bsp_OV5640_WriteByte(0x5581, 0x80);
            Bsp_OV5640_WriteByte(0x5582, 0x00);
            Bsp_OV5640_WriteByte(0x5588, 0x32);
            break;
        }
        case ATK_MC5640_HUE_1: {
            Bsp_OV5640_WriteByte(0x5001, 0xFF);
            Bsp_OV5640_WriteByte(0x5580, 0x01);
            Bsp_OV5640_WriteByte(0x5581, 0x6F);
            Bsp_OV5640_WriteByte(0x5582, 0x40);
            Bsp_OV5640_WriteByte(0x5588, 0x32);
            break;
        }
        case ATK_MC5640_HUE_2: {
            Bsp_OV5640_WriteByte(0x5001, 0xFF);
            Bsp_OV5640_WriteByte(0x5580, 0x01);
            Bsp_OV5640_WriteByte(0x5581, 0x40);
            Bsp_OV5640_WriteByte(0x5582, 0x6F);
            Bsp_OV5640_WriteByte(0x5588, 0x32);
            break;
        }
        case ATK_MC5640_HUE_3: {
            Bsp_OV5640_WriteByte(0x5001, 0xFF);
            Bsp_OV5640_WriteByte(0x5580, 0x01);
            Bsp_OV5640_WriteByte(0x5581, 0x00);
            Bsp_OV5640_WriteByte(0x5582, 0x80);
            Bsp_OV5640_WriteByte(0x5588, 0x02);
            break;
        }
        case ATK_MC5640_HUE_4: {
            Bsp_OV5640_WriteByte(0x5001, 0xFF);
            Bsp_OV5640_WriteByte(0x5580, 0x01);
            Bsp_OV5640_WriteByte(0x5581, 0x40);
            Bsp_OV5640_WriteByte(0x5582, 0x6F);
            Bsp_OV5640_WriteByte(0x5588, 0x02);
            break;
        }
        case ATK_MC5640_HUE_5: {
            Bsp_OV5640_WriteByte(0x5001, 0xFF);
            Bsp_OV5640_WriteByte(0x5580, 0x01);
            Bsp_OV5640_WriteByte(0x5581, 0x6F);
            Bsp_OV5640_WriteByte(0x5582, 0x40);
            Bsp_OV5640_WriteByte(0x5588, 0x02);
            break;
        }
        case ATK_MC5640_HUE_6: {
            Bsp_OV5640_WriteByte(0x5001, 0xFF);
            Bsp_OV5640_WriteByte(0x5580, 0x01);
            Bsp_OV5640_WriteByte(0x5581, 0x80);
            Bsp_OV5640_WriteByte(0x5582, 0x00);
            Bsp_OV5640_WriteByte(0x5588, 0x01);
            break;
        }
        case ATK_MC5640_HUE_7: {
            Bsp_OV5640_WriteByte(0x5001, 0xFF);
            Bsp_OV5640_WriteByte(0x5580, 0x01);
            Bsp_OV5640_WriteByte(0x5581, 0x6F);
            Bsp_OV5640_WriteByte(0x5582, 0x40);
            Bsp_OV5640_WriteByte(0x5588, 0x01);
            break;
        }
        case ATK_MC5640_HUE_8: {
            Bsp_OV5640_WriteByte(0x5001, 0xFF);
            Bsp_OV5640_WriteByte(0x5580, 0x01);
            Bsp_OV5640_WriteByte(0x5581, 0x40);
            Bsp_OV5640_WriteByte(0x5582, 0x6F);
            Bsp_OV5640_WriteByte(0x5588, 0x01);
            break;
        }
        case ATK_MC5640_HUE_9: {
            Bsp_OV5640_WriteByte(0x5001, 0xFF);
            Bsp_OV5640_WriteByte(0x5580, 0x01);
            Bsp_OV5640_WriteByte(0x5581, 0x00);
            Bsp_OV5640_WriteByte(0x5582, 0x80);
            Bsp_OV5640_WriteByte(0x5588, 0x31);
            break;
        }
        case ATK_MC5640_HUE_10: {
            Bsp_OV5640_WriteByte(0x5001, 0xFF);
            Bsp_OV5640_WriteByte(0x5580, 0x01);
            Bsp_OV5640_WriteByte(0x5581, 0x40);
            Bsp_OV5640_WriteByte(0x5582, 0x6F);
            Bsp_OV5640_WriteByte(0x5588, 0x31);
            break;
        }
        case ATK_MC5640_HUE_11: {
            Bsp_OV5640_WriteByte(0x5001, 0xFF);
            Bsp_OV5640_WriteByte(0x5580, 0x01);
            Bsp_OV5640_WriteByte(0x5581, 0x6F);
            Bsp_OV5640_WriteByte(0x5582, 0x40);
            Bsp_OV5640_WriteByte(0x5588, 0x31);
            break;
        }
        default: {
            return ATK_MC5640_EINVAL;
        }
    }

    return ATK_MC5640_EOK;
}

/**
 * @brief       设置ATK-MC5640模块特殊效果
 * @param       contrast: ATK_MC5640_SPECIAL_EFFECT_NORMAL  : Normal
 *                        ATK_MC5640_SPECIAL_EFFECT_BW      : B&W
 *                        ATK_MC5640_SPECIAL_EFFECT_BLUISH  : Bluish
 *                        ATK_MC5640_SPECIAL_EFFECT_SEPIA   : Sepia
 *                        ATK_MC5640_SPECIAL_EFFECT_REDDISH : Reddish
 *                        ATK_MC5640_SPECIAL_EFFECT_GREENISH: Greenish
 *                        ATK_MC5640_SPECIAL_EFFECT_NEGATIVE: Negative
 * @retval      ATK_MC5640_EOK   : 设置ATK-MC5640模块特殊效果成功
 *              ATK_MC5640_EINVAL: 传入参数错误
 */
uint8_t atk_mc5640_set_special_effect(atk_mc5640_special_effect_t effect) {
    switch (effect) {
        case ATK_MC5640_SPECIAL_EFFECT_NORMAL: {
            Bsp_OV5640_WriteByte(0x5001, 0x7F);
            Bsp_OV5640_WriteByte(0x5580, 0x00);
            break;
        }
        case ATK_MC5640_SPECIAL_EFFECT_BW: {
            Bsp_OV5640_WriteByte(0x5001, 0xFF);
            Bsp_OV5640_WriteByte(0x5580, 0x18);
            Bsp_OV5640_WriteByte(0x5583, 0x80);
            Bsp_OV5640_WriteByte(0x5584, 0x80);
            break;
        }
        case ATK_MC5640_SPECIAL_EFFECT_BLUISH: {
            Bsp_OV5640_WriteByte(0x5001, 0xFF);
            Bsp_OV5640_WriteByte(0x5580, 0x18);
            Bsp_OV5640_WriteByte(0x5583, 0xA0);
            Bsp_OV5640_WriteByte(0x5584, 0x40);
            break;
        }
        case ATK_MC5640_SPECIAL_EFFECT_SEPIA: {
            Bsp_OV5640_WriteByte(0x5001, 0xFF);
            Bsp_OV5640_WriteByte(0x5580, 0x18);
            Bsp_OV5640_WriteByte(0x5583, 0x40);
            Bsp_OV5640_WriteByte(0x5584, 0xA0);
            break;
        }
        case ATK_MC5640_SPECIAL_EFFECT_REDDISH: {
            Bsp_OV5640_WriteByte(0x5001, 0xFF);
            Bsp_OV5640_WriteByte(0x5580, 0x18);
            Bsp_OV5640_WriteByte(0x5583, 0x80);
            Bsp_OV5640_WriteByte(0x5584, 0xC0);
            break;
        }
        case ATK_MC5640_SPECIAL_EFFECT_GREENISH: {
            Bsp_OV5640_WriteByte(0x5001, 0xFF);
            Bsp_OV5640_WriteByte(0x5580, 0x18);
            Bsp_OV5640_WriteByte(0x5583, 0x60);
            Bsp_OV5640_WriteByte(0x5584, 0x60);
            break;
        }
        case ATK_MC5640_SPECIAL_EFFECT_NEGATIVE: {
            Bsp_OV5640_WriteByte(0x5001, 0xFF);
            Bsp_OV5640_WriteByte(0x5580, 0x40);
            break;
        }
        default: {
            return ATK_MC5640_EINVAL;
        }
    }

    return ATK_MC5640_EOK;
}

/**
 * @brief       设置ATK-MC5640模块曝光度
 * @param       contrast: ATK_MC5640_EXPOSURE_LEVEL_0 : -1.7EV
 *                        ATK_MC5640_EXPOSURE_LEVEL_1 : -1.3EV
 *                        ATK_MC5640_EXPOSURE_LEVEL_2 : -1.0EV
 *                        ATK_MC5640_EXPOSURE_LEVEL_3 : -0.7EV
 *                        ATK_MC5640_EXPOSURE_LEVEL_4 : -0.3EV
 *                        ATK_MC5640_EXPOSURE_LEVEL_5 : default
 *                        ATK_MC5640_EXPOSURE_LEVEL_6 : 0.3EV
 *                        ATK_MC5640_EXPOSURE_LEVEL_7 : 0.7EV
 *                        ATK_MC5640_EXPOSURE_LEVEL_8 : 1.0EV
 *                        ATK_MC5640_EXPOSURE_LEVEL_9 : 1.3EV
 *                        ATK_MC5640_EXPOSURE_LEVEL_10: 1.7EV
 * @retval      ATK_MC5640_EOK   : 设置ATK-MC5640模块曝光度成功
 *              ATK_MC5640_EINVAL: 传入参数错误
 */
uint8_t atk_mc5640_set_exposure_level(atk_mc5640_exposure_level_t level) {
    switch (level) {
        case ATK_MC5640_EXPOSURE_LEVEL_0: {
            Bsp_OV5640_WriteByte(0x3A0F, 0x10);
            Bsp_OV5640_WriteByte(0x3A10, 0x08);
            Bsp_OV5640_WriteByte(0x3A1B, 0x10);
            Bsp_OV5640_WriteByte(0x3A1E, 0x08);
            Bsp_OV5640_WriteByte(0x3A11, 0x20);
            Bsp_OV5640_WriteByte(0x3A1F, 0x10);
            break;
        }
        case ATK_MC5640_EXPOSURE_LEVEL_1: {
            Bsp_OV5640_WriteByte(0x3A0F, 0x18);
            Bsp_OV5640_WriteByte(0x3A10, 0x10);
            Bsp_OV5640_WriteByte(0x3A1B, 0x18);
            Bsp_OV5640_WriteByte(0x3A1E, 0x10);
            Bsp_OV5640_WriteByte(0x3A11, 0x30);
            Bsp_OV5640_WriteByte(0x3A1F, 0x10);
            break;
        }
        case ATK_MC5640_EXPOSURE_LEVEL_2: {
            Bsp_OV5640_WriteByte(0x3A0F, 0x20);
            Bsp_OV5640_WriteByte(0x3A10, 0x18);
            Bsp_OV5640_WriteByte(0x3A11, 0x41);
            Bsp_OV5640_WriteByte(0x3A1B, 0x20);
            Bsp_OV5640_WriteByte(0x3A1E, 0x18);
            Bsp_OV5640_WriteByte(0x3A1F, 0x10);
            break;
        }
        case ATK_MC5640_EXPOSURE_LEVEL_3: {
            Bsp_OV5640_WriteByte(0x3A0F, 0x28);
            Bsp_OV5640_WriteByte(0x3A10, 0x20);
            Bsp_OV5640_WriteByte(0x3A11, 0x51);
            Bsp_OV5640_WriteByte(0x3A1B, 0x28);
            Bsp_OV5640_WriteByte(0x3A1E, 0x20);
            Bsp_OV5640_WriteByte(0x3A1F, 0x10);
            break;
        }
        case ATK_MC5640_EXPOSURE_LEVEL_4: {
            Bsp_OV5640_WriteByte(0x3A0F, 0x30);
            Bsp_OV5640_WriteByte(0x3A10, 0x28);
            Bsp_OV5640_WriteByte(0x3A11, 0x61);
            Bsp_OV5640_WriteByte(0x3A1B, 0x30);
            Bsp_OV5640_WriteByte(0x3A1E, 0x28);
            Bsp_OV5640_WriteByte(0x3A1F, 0x10);
            break;
        }
        case ATK_MC5640_EXPOSURE_LEVEL_5: {
            Bsp_OV5640_WriteByte(0x3A0F, 0x38);
            Bsp_OV5640_WriteByte(0x3A10, 0x30);
            Bsp_OV5640_WriteByte(0x3A11, 0x61);
            Bsp_OV5640_WriteByte(0x3A1B, 0x38);
            Bsp_OV5640_WriteByte(0x3A1E, 0x30);
            Bsp_OV5640_WriteByte(0x3A1F, 0x10);
            break;
        }
        case ATK_MC5640_EXPOSURE_LEVEL_6: {
            Bsp_OV5640_WriteByte(0x3A0F, 0x40);
            Bsp_OV5640_WriteByte(0x3A10, 0x38);
            Bsp_OV5640_WriteByte(0x3A11, 0x71);
            Bsp_OV5640_WriteByte(0x3A1B, 0x40);
            Bsp_OV5640_WriteByte(0x3A1E, 0x38);
            Bsp_OV5640_WriteByte(0x3A1F, 0x10);
            break;
        }
        case ATK_MC5640_EXPOSURE_LEVEL_7: {
            Bsp_OV5640_WriteByte(0x3A0F, 0x48);
            Bsp_OV5640_WriteByte(0x3A10, 0x40);
            Bsp_OV5640_WriteByte(0x3A11, 0x80);
            Bsp_OV5640_WriteByte(0x3A1B, 0x48);
            Bsp_OV5640_WriteByte(0x3A1E, 0x40);
            Bsp_OV5640_WriteByte(0x3A1F, 0x20);
            break;
        }
        case ATK_MC5640_EXPOSURE_LEVEL_8: {
            Bsp_OV5640_WriteByte(0x3A0F, 0x50);
            Bsp_OV5640_WriteByte(0x3A10, 0x48);
            Bsp_OV5640_WriteByte(0x3A11, 0x90);
            Bsp_OV5640_WriteByte(0x3A1B, 0x50);
            Bsp_OV5640_WriteByte(0x3A1E, 0x48);
            Bsp_OV5640_WriteByte(0x3A1F, 0x20);
            break;
        }
        case ATK_MC5640_EXPOSURE_LEVEL_9: {
            Bsp_OV5640_WriteByte(0x3A0F, 0x58);
            Bsp_OV5640_WriteByte(0x3A10, 0x50);
            Bsp_OV5640_WriteByte(0x3A11, 0x91);
            Bsp_OV5640_WriteByte(0x3A1B, 0x58);
            Bsp_OV5640_WriteByte(0x3A1E, 0x50);
            Bsp_OV5640_WriteByte(0x3A1F, 0x20);
            break;
        }
        case ATK_MC5640_EXPOSURE_LEVEL_10: {
            Bsp_OV5640_WriteByte(0x3A0F, 0x60);
            Bsp_OV5640_WriteByte(0x3A10, 0x58);
            Bsp_OV5640_WriteByte(0x3A11, 0xA0);
            Bsp_OV5640_WriteByte(0x3A1B, 0x60);
            Bsp_OV5640_WriteByte(0x3A1E, 0x58);
            Bsp_OV5640_WriteByte(0x3A1F, 0x20);
            break;
        }
        default: {
            return ATK_MC5640_EINVAL;
        }
    }

    return ATK_MC5640_EOK;
}

/**
 * @brief       设置ATK-MC5640模块锐度
 * @param       contrast: ATK_MC5640_SHARPNESS_OFF  : Sharpness OFF
 *                        ATK_MC5640_SHARPNESS_1    : Sharpness 1
 *                        ATK_MC5640_SHARPNESS_2    : Sharpness 2
 *                        ATK_MC5640_SHARPNESS_3    : Sharpness 3
 *                        ATK_MC5640_SHARPNESS_4    : Sharpness 4
 *                        ATK_MC5640_SHARPNESS_5    : Sharpness 5
 *                        ATK_MC5640_SHARPNESS_6    : Sharpness 6
 *                        ATK_MC5640_SHARPNESS_7    : Sharpness 7
 *                        ATK_MC5640_SHARPNESS_8    : Sharpness 8
 *                        ATK_MC5640_SHARPNESS_AUTO : Sharpness Auto
 * @retval      ATK_MC5640_EOK   : 设置ATK-MC5640模块锐度成功
 *              ATK_MC5640_EINVAL: 传入参数错误
 */
uint8_t atk_mc5640_set_sharpness_level(atk_mc5640_sharpness_t sharpness) {
    switch (sharpness) {
        case ATK_MC5640_SHARPNESS_OFF: {
            Bsp_OV5640_WriteByte(0x5308, 0x65);
            Bsp_OV5640_WriteByte(0x5302, 0x00);
            break;
        }
        case ATK_MC5640_SHARPNESS_1: {
            Bsp_OV5640_WriteByte(0x5308, 0x65);
            Bsp_OV5640_WriteByte(0x5302, 0x02);
            break;
        }
        case ATK_MC5640_SHARPNESS_2: {
            Bsp_OV5640_WriteByte(0x5308, 0x65);
            Bsp_OV5640_WriteByte(0x5302, 0x04);
            break;
        }
        case ATK_MC5640_SHARPNESS_3: {
            Bsp_OV5640_WriteByte(0x5308, 0x65);
            Bsp_OV5640_WriteByte(0x5302, 0x08);
            break;
        }
        case ATK_MC5640_SHARPNESS_4: {
            Bsp_OV5640_WriteByte(0x5308, 0x65);
            Bsp_OV5640_WriteByte(0x5302, 0x0C);
            break;
        }
        case ATK_MC5640_SHARPNESS_5: {
            Bsp_OV5640_WriteByte(0x5308, 0x65);
            Bsp_OV5640_WriteByte(0x5302, 0x10);
            break;
        }
        case ATK_MC5640_SHARPNESS_6: {
            Bsp_OV5640_WriteByte(0x5308, 0x65);
            Bsp_OV5640_WriteByte(0x5302, 0x14);
            break;
        }
        case ATK_MC5640_SHARPNESS_7: {
            Bsp_OV5640_WriteByte(0x5308, 0x65);
            Bsp_OV5640_WriteByte(0x5302, 0x18);
            break;
        }
        case ATK_MC5640_SHARPNESS_8: {
            Bsp_OV5640_WriteByte(0x5308, 0x65);
            Bsp_OV5640_WriteByte(0x5302, 0x20);
            break;
        }
        case ATK_MC5640_SHARPNESS_AUTO: {
            Bsp_OV5640_WriteByte(0x5308, 0x25);
            Bsp_OV5640_WriteByte(0x5300, 0x08);
            Bsp_OV5640_WriteByte(0x5301, 0x30);
            Bsp_OV5640_WriteByte(0x5302, 0x10);
            Bsp_OV5640_WriteByte(0x5303, 0x00);
            Bsp_OV5640_WriteByte(0x5309, 0x08);
            Bsp_OV5640_WriteByte(0x530A, 0x30);
            Bsp_OV5640_WriteByte(0x530B, 0x04);
            Bsp_OV5640_WriteByte(0x530C, 0x06);
            break;
        }
        default: {
            return ATK_MC5640_EINVAL;
        }
    }

    return ATK_MC5640_EOK;
}

/**
 * @brief       设置ATK-MC5640模块镜像/翻转
 * @param       contrast: ATK_MC5640_MIRROR_FLIP_0: MIRROR
 *                        ATK_MC5640_MIRROR_FLIP_1: FLIP
 *                        ATK_MC5640_MIRROR_FLIP_2: MIRROR & FLIP
 *                        ATK_MC5640_MIRROR_FLIP_3: Normal
 * @retval      ATK_MC5640_EOK   : 设置ATK-MC5640模块镜像/翻转成功
 *              ATK_MC5640_EINVAL: 传入参数错误
 */
uint8_t atk_mc5640_set_mirror_flip(atk_mc5640_mirror_flip_t mirror_flip) {
    uint8_t reg3820;
    uint8_t reg3821;

    switch (mirror_flip) {
        case ATK_MC5640_MIRROR_FLIP_0: {
            reg3820 = Bsp_OV5640_ReadByte(0x3820);
            reg3820 = reg3820 & 0xF9;
            reg3820 = reg3820 | 0x00;
            Bsp_OV5640_WriteByte(0x3820, reg3820);
            reg3821 = Bsp_OV5640_ReadByte(0x3821);
            reg3821 = reg3821 & 0xF9;
            reg3821 = reg3821 | 0x06;
            Bsp_OV5640_WriteByte(0x3821, reg3821);
            break;
        }
        case ATK_MC5640_MIRROR_FLIP_1: {
            reg3820 = Bsp_OV5640_ReadByte(0x3820);
            reg3820 = reg3820 & 0xF9;
            reg3820 = reg3820 | 0x06;
            Bsp_OV5640_WriteByte(0x3820, reg3820);
            reg3821 = Bsp_OV5640_ReadByte(0x3821);
            reg3821 = reg3821 & 0xF9;
            reg3821 = reg3821 | 0x00;
            Bsp_OV5640_WriteByte(0x3821, reg3821);
            break;
        }
        case ATK_MC5640_MIRROR_FLIP_2: {
            reg3820 = Bsp_OV5640_ReadByte(0x3820);
            reg3820 = reg3820 & 0xF9;
            reg3820 = reg3820 | 0x06;
            Bsp_OV5640_WriteByte(0x3820, reg3820);
            reg3821 = Bsp_OV5640_ReadByte(0x3821);
            reg3821 = reg3821 & 0xF9;
            reg3821 = reg3821 | 0x06;
            Bsp_OV5640_WriteByte(0x3821, reg3821);
            break;
        }
        case ATK_MC5640_MIRROR_FLIP_3: {
            reg3820 = Bsp_OV5640_ReadByte(0x3820);
            reg3820 = reg3820 & 0xF9;
            reg3820 = reg3820 | 0x00;
            Bsp_OV5640_WriteByte(0x3820, reg3820);
            reg3821 = Bsp_OV5640_ReadByte(0x3821);
            reg3821 = reg3821 & 0xF9;
            reg3821 = reg3821 | 0x00;
            Bsp_OV5640_WriteByte(0x3821, reg3821);
            break;
        }
        default: {
            return ATK_MC5640_EINVAL;
        }
    }

    return ATK_MC5640_EOK;
}

/**
 * @brief       设置ATK-MC5640模块测试图案
 * @param       contrast: ATK_MC5640_TEST_PATTERN_OFF         : OFF
 *                        ATK_MC5640_TEST_PATTERN_COLOR_BAR   : Color bar
 *                        ATK_MC5640_TEST_PATTERN_COLOR_SQUARE: Color square
 * @retval      ATK_MC5640_EOK   : 设置ATK-MC5640模块测试图案成功
 *              ATK_MC5640_EINVAL: 传入参数错误
 */
uint8_t atk_mc5640_set_test_pattern(atk_mc5640_test_pattern_t pattern) {
    switch (pattern) {
        case ATK_MC5640_TEST_PATTERN_OFF: {
            Bsp_OV5640_WriteByte(0x503D, 0x00);
            Bsp_OV5640_WriteByte(0x4741, 0x00);
            break;
        }
        case ATK_MC5640_TEST_PATTERN_COLOR_BAR: {
            Bsp_OV5640_WriteByte(0x503D, 0x80);
            Bsp_OV5640_WriteByte(0x4741, 0x00);
            break;
        }
        case ATK_MC5640_TEST_PATTERN_COLOR_SQUARE: {
            Bsp_OV5640_WriteByte(0x503D, 0x82);
            Bsp_OV5640_WriteByte(0x4741, 0x00);
            break;
        }
        default: {
            return ATK_MC5640_EINVAL;
        }
    }

    return ATK_MC5640_EOK;
}

/**
 * @brief       获取ATK-MC5640模块ISP输入窗口尺寸
 * @param       无
 * @retval      无
 */
static void atk_mc5640_get_isp_input_size(void) {
    uint8_t reg3800;
    uint8_t reg3801;
    uint8_t reg3802;
    uint8_t reg3803;
    uint8_t reg3804;
    uint8_t reg3805;
    uint8_t reg3806;
    uint8_t reg3807;
    uint16_t x_addr_st;
    uint16_t y_addr_st;
    uint16_t x_addr_end;
    uint16_t y_addr_end;

    Driver_Delay_MS(100);

    reg3800 = Bsp_OV5640_ReadByte(0x3800);
    reg3801 = Bsp_OV5640_ReadByte(0x3801);
    reg3802 = Bsp_OV5640_ReadByte(0x3802);
    reg3803 = Bsp_OV5640_ReadByte(0x3803);
    reg3804 = Bsp_OV5640_ReadByte(0x3804);
    reg3805 = Bsp_OV5640_ReadByte(0x3805);
    reg3806 = Bsp_OV5640_ReadByte(0x3806);
    reg3807 = Bsp_OV5640_ReadByte(0x3807);

    x_addr_st = (uint16_t) ((reg3800 & 0x0F) << 8) | reg3801;
    y_addr_st = (uint16_t) ((reg3802 & 0x07) << 8) | reg3803;
    x_addr_end = (uint16_t) ((reg3804 & 0x0F) << 8) | reg3805;
    y_addr_end = (uint16_t) ((reg3806 & 0x07) << 8) | reg3807;

    g_atk_mc5640_sta.isp_input.width = x_addr_end - x_addr_st;
    g_atk_mc5640_sta.isp_input.height = y_addr_end - y_addr_st;
}

/**
 * @brief       设置ATK-MC5640模块ISP输入窗口尺寸
 * @note        ATK-MC5640模块ISP输入窗口的最大尺寸为0x0A3F*0x079F
 * @param       x     : ISP输入窗口起始X坐标
 *              y     : ISP输入窗口起始Y坐标
 *              width : ISP输入窗口宽度
 *              height: ISP输入窗口高度
 * @retval      ATK_MC5640_EOK   : 设置ATK-MC5640模块ISP输入窗口尺寸成功
 *              ATK_MC5640_EINVAL: 传入参数错误
 */
uint8_t atk_mc5640_set_isp_input_window(uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
    uint8_t reg3800;
    uint8_t reg3801;
    uint8_t reg3802;
    uint8_t reg3803;
    uint8_t reg3804;
    uint8_t reg3805;
    uint8_t reg3806;
    uint8_t reg3807;
    uint16_t x_end;
    uint16_t y_end;

    x_end = x + width;
    y_end = y + height;

    if ((x_end > ATK_MC5640_ISP_INPUT_WIDTH_MAX) || (y_end > ATK_MC5640_ISP_INPUT_HEIGHT_MAX)) {
        return ATK_MC5640_EINVAL;
    }

    reg3800 = Bsp_OV5640_ReadByte(0x3800);
    reg3802 = Bsp_OV5640_ReadByte(0x3802);
    reg3804 = Bsp_OV5640_ReadByte(0x3804);
    reg3806 = Bsp_OV5640_ReadByte(0x3806);

    reg3800 &= ~0x0F;
    reg3800 |= (uint8_t) (x >> 8) & 0x0F;
    reg3801 = (uint8_t) x & 0xFF;
    reg3802 &= ~0x0F;
    reg3802 |= (uint8_t) (y >> 8) & 0x0F;
    reg3803 = (uint8_t) y & 0xFF;
    reg3804 &= ~0x0F;
    reg3804 |= (uint8_t) (x_end >> 8) & 0x0F;
    reg3805 = (uint8_t) x_end & 0xFF;
    reg3806 &= ~0x07;
    reg3806 |= (uint8_t) (y_end >> 8) & 0x07;
    reg3807 = (uint8_t) y_end & 0xFF;

    Bsp_OV5640_WriteByte(0x3212, 0x03);
    Bsp_OV5640_WriteByte(0x3800, reg3800);
    Bsp_OV5640_WriteByte(0x3801, reg3801);
    Bsp_OV5640_WriteByte(0x3802, reg3802);
    Bsp_OV5640_WriteByte(0x3803, reg3803);
    Bsp_OV5640_WriteByte(0x3804, reg3804);
    Bsp_OV5640_WriteByte(0x3805, reg3805);
    Bsp_OV5640_WriteByte(0x3806, reg3806);
    Bsp_OV5640_WriteByte(0x3807, reg3807);
    Bsp_OV5640_WriteByte(0x3212, 0x13);
    Bsp_OV5640_WriteByte(0x3212, 0xA3);

    atk_mc5640_get_isp_input_size();

    return ATK_MC5640_EOK;
}

/**
 * @brief       获取ATK-MC5640模块预缩放窗口尺寸
 * @param       无
 * @retval      无
 */
static void atk_mc5640_get_pre_scaling_size(void) {
    uint8_t reg3810;
    uint8_t reg3811;
    uint8_t reg3812;
    uint8_t reg3813;
    uint16_t x_offset;
    uint16_t y_offset;

    Driver_Delay_MS(100);

    reg3810 = Bsp_OV5640_ReadByte(0x3810);
    reg3811 = Bsp_OV5640_ReadByte(0x3811);
    reg3812 = Bsp_OV5640_ReadByte(0x3812);
    reg3813 = Bsp_OV5640_ReadByte(0x3813);

    x_offset = (uint16_t) ((reg3810 & 0x0F) << 8) | reg3811;
    y_offset = (uint16_t) ((reg3812 & 0x07) << 8) | reg3813;

    atk_mc5640_get_isp_input_size();

    g_atk_mc5640_sta.pre_scaling.width = g_atk_mc5640_sta.isp_input.width - (x_offset << 1);
    g_atk_mc5640_sta.pre_scaling.height = g_atk_mc5640_sta.isp_input.height - (y_offset << 1);
}

/**
 * @brief       设置ATK-MC5640模块预缩放窗口偏移
 * @note        ATK-MC5640模块预缩放窗口尺寸必须小于ISP输入窗口尺寸
 * @param       x_offset: 预缩放窗口X偏移
 *              y_offset: 预缩放窗口Y偏移
 * @retval      ATK_MC5640_EOK   : 设置ATK-MC5640模块预缩放窗口偏移成功
 */
uint8_t atk_mc5640_set_pre_scaling_window(uint16_t x_offset, uint16_t y_offset) {
    uint8_t reg3810;
    uint8_t reg3811;
    uint8_t reg3812;
    uint8_t reg3813;

    reg3810 = (uint8_t) (x_offset >> 8) & 0x0F;
    reg3811 = (uint8_t) x_offset & 0xFF;
    reg3812 = (uint8_t) (y_offset >> 8) & 0x07;
    reg3813 = (uint8_t) y_offset & 0xFF;

    Bsp_OV5640_WriteByte(0x3212, 0x03);
    Bsp_OV5640_WriteByte(0x3810, reg3810);
    Bsp_OV5640_WriteByte(0x3811, reg3811);
    Bsp_OV5640_WriteByte(0x3812, reg3812);
    Bsp_OV5640_WriteByte(0x3813, reg3813);
    Bsp_OV5640_WriteByte(0x3212, 0x13);
    Bsp_OV5640_WriteByte(0x3212, 0xA3);

    atk_mc5640_get_pre_scaling_size();

    return ATK_MC5640_EOK;
}