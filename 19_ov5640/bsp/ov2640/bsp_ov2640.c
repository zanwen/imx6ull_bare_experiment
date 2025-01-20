//
// Created by 86157 on 2025/1/17.
//

#include "bsp_ov2640.h"
#include "driver_i2c.h"
#include "logger.h"
#include "driver_gpio.h"
#include "driver_delay.h"
#include "bsp_ov2640_cfg.h"
#include "driver_csi.h"
#include "bsp_lcd.h"

#define RESET_PIN GPIO1, 2
#define POWER_DOWN_PIN GPIO1, 4

#define RESET_PIN_LOW() Driver_GPIO_WritePIn(RESET_PIN, GPIO_PIN_RESET)
#define RESET_PIN_HIGH() Driver_GPIO_WritePIn(RESET_PIN, GPIO_PIN_SET)

#define POWER_DOWN_PIN_LOW() Driver_GPIO_WritePIn(POWER_DOWN_PIN, GPIO_PIN_RESET)
#define POWER_DOWN_PIN_HIGH() Driver_GPIO_WritePIn(POWER_DOWN_PIN, GPIO_PIN_SET)

#define OV2640_ADDR_WRITE 0x30
#define OV2640_ADDR_READ 0x31

#define REG_CHIP_ID 0x300A

#define OUTPUT_WIDTH 1024
#define OUTPUT_HEIGHT 600

/* ATK-MC2640模块制造商ID和产品ID */
#define ATK_MC2640_MID  0x7FA2
#define ATK_MC2640_PID  0x2642

/* ATK-MC2640寄存器块枚举 */
typedef enum
{
    ATK_MC2640_REG_BANK_DSP = 0x00, /* DSP寄存器块 */
    ATK_MC2640_REG_BANK_SENSOR,     /* Sensor寄存器块 */
} atk_mc2640_reg_bank_t;

/* ATK-MC2640模块数据结构体 */
static struct
{
    struct {
        uint16_t width;
        uint16_t height;
    } output;
} g_atk_mc2640_sta = {0};


uint8_t Bsp_OV2640_RegInit(void);
uint8_t atk_mc2640_set_light_mode(atk_mc2640_light_mode_t mode);                                            /* 设置ATK-MC2640模块灯光模式 */
uint8_t atk_mc2640_set_color_saturation(atk_mc2640_color_saturation_t saturation);                          /* 设置ATK-MC2640模块色彩饱和度 */
uint8_t atk_mc2640_set_brightness(atk_mc2640_brightness_t brightness);                                      /* 设置ATK-MC2640模块亮度 */
uint8_t atk_mc2640_set_contrast(atk_mc2640_contrast_t contrast);                                            /* 设置ATK-MC2640模块对比度 */
uint8_t atk_mc2640_set_special_effect(atk_mc2640_special_effect_t effect);
static uint8_t atk_mc2640_reg_bank_select(atk_mc2640_reg_bank_t bank);
static void atk_mc2640_init_reg(void);

void Bsp_OV2640_IOInit(void) {
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

void Bsp_OV2640_HardReset(void) {
    RESET_PIN_LOW();
    Driver_Delay_MS(5);
    POWER_DOWN_PIN_LOW();
    Driver_Delay_MS(1);
    RESET_PIN_HIGH();
    Driver_Delay_MS(20);
}

void Bsp_OV2640_WriteBytes(uint16_t regAddr, uint8_t *data, uint8_t len) {
    uint8_t reg[2] = {regAddr >> 8, regAddr};
    Driver_I2C_Start(I2C2);
    Driver_I2C_SendAddr(I2C2, OV2640_ADDR_WRITE);
    Driver_I2C_SendBytes(I2C2, reg, 2);
    Driver_I2C_SendBytes(I2C2, data, len);
    Driver_I2C_Stop(I2C2);
}

inline void Bsp_OV2640_WriteByte(uint16_t regAddr, uint8_t data) {
    Bsp_OV2640_WriteBytes(regAddr, &data, 1);
}

void Bsp_OV2640_ReadBytes(uint16_t regAddr, uint8_t *buf, uint8_t len) {
    uint8_t reg[2] = {regAddr >> 8, regAddr};
    Driver_I2C_Start(I2C2);
    // dummy write
    Driver_I2C_SendAddr(I2C2, OV2640_ADDR_WRITE);
    Driver_I2C_SendBytes(I2C2, reg, 2);

    Driver_I2C_ReStart(I2C2);
    Driver_I2C_SendAddr(I2C2, OV2640_ADDR_READ);
    Driver_I2C_ReadBytesAndStop(I2C2, buf, len);
}

uint8_t Bsp_OV2640_ReadByte(uint16_t regAddr) {
    uint8_t data = 0;
    uint8_t reg[2] = {regAddr >> 8, regAddr};
    Driver_I2C_Start(I2C2);
    // dummy write
    Driver_I2C_SendAddr(I2C2, OV2640_ADDR_WRITE);
    Driver_I2C_SendBytes(I2C2, reg, 2);

    Driver_I2C_ReStart(I2C2);
    Driver_I2C_SendAddr(I2C2, OV2640_ADDR_READ);
    Driver_I2C_ReadBytesAndStop(I2C2, &data, 1);
    return data;
}

void Bsp_OV2640_Test(void) {
    uint8_t read = 0;
    Bsp_OV2640_ReadBytes(0x4300, &read, 1);
    LOG_DEBUG("0x4300 = %#x", read);
    Driver_CSI_DMA_ReflashRFF();
//    atk_MC2640_get_output_size();
//    atk_MC2640_set_output_size(OUTPUT_WIDTH, OUTPUT_HEIGHT);
}

void Bsp_OV2640_CSI_Init(void) {
    CSI_Config_t config = {1024, 600, 2};
    Driver_CSI_IOInit();
    Driver_CSI_ControllerInit();

    Driver_CSI_DMA_AddrInit(LCD_FRAMEBUF_ADDR);
    Driver_CSI_ImageParamInit(&config);
    Driver_CSI_DMA_ReflashRFF();
    Driver_CSI_Start();
}

void Bsp_OV2640_Init(void) {
    Bsp_OV2640_IOInit();
    Bsp_OV2640_HardReset();
    Driver_I2C_Init(I2C2);
    Bsp_OV2640_RegInit();
    Bsp_OV2640_CSI_Init();
    LOG_DEBUG("Bsp_OV2640_Init done");
}

/**
 * @brief       ATK-MC2640模块软件复位
 * @param       无
 * @retval      无
 */
static void atk_mc2640_sw_reset(void)
{
    atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_SENSOR);
    Bsp_OV2640_WriteByte(ATK_MC2640_REG_SENSOR_COM7, 0x80);
    Driver_Delay_MS(50);
}


uint8_t Bsp_OV2640_RegInit(void) {
    atk_mc2640_sw_reset();
    atk_mc2640_init_reg();
    uint8_t ret = 0;
    atk_mc2640_set_light_mode(ATK_MC2640_LIGHT_MODE_SUNNY);                 /* 设置灯光模式 */
    atk_mc2640_set_color_saturation(ATK_MC2640_COLOR_SATURATION_1);         /* 设置色彩饱和度 */
    atk_mc2640_set_brightness(ATK_MC2640_BRIGHTNESS_1);                     /* 设置亮度 */
    atk_mc2640_set_contrast(ATK_MC2640_CONTRAST_2);                         /* 设置对比度 */
    atk_mc2640_set_special_effect(ATK_MC2640_SPECIAL_EFFECT_NORMAL);        /* 设置特殊效果 */

    if (ret != 0) {
        LOG_ERROR("ATK-MC2640 init failed!");
    }
    return ATK_MC2640_EOK;
}


/**
 * @brief       初始化ATK-MC2640寄存器配置
 * @param       无
 * @retval      无
 */
static void atk_mc2640_init_reg(void)
{
    uint32_t cfg_index;
    uint8_t zmow;
    uint8_t zmoh;
    uint8_t zmhh;

    for (cfg_index=0; cfg_index<(sizeof(atk_mc2640_init_uxga_cfg)/sizeof(atk_mc2640_init_uxga_cfg[0])); cfg_index++)
    {
        Bsp_OV2640_WriteByte(atk_mc2640_init_uxga_cfg[cfg_index][0], atk_mc2640_init_uxga_cfg[cfg_index][1]);
    }

    atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_DSP);
    zmow = Bsp_OV2640_ReadByte(ATK_MC2640_REG_DSP_ZMOW);
    zmoh = Bsp_OV2640_ReadByte(ATK_MC2640_REG_DSP_ZMOH);
    zmhh = Bsp_OV2640_ReadByte(ATK_MC2640_REG_DSP_ZMHH);

    g_atk_mc2640_sta.output.width = ((uint16_t)zmow | ((zmhh & 0x03) << 8)) << 2;
    g_atk_mc2640_sta.output.height = ((uint16_t)zmoh | ((zmhh & 0x04) << 6)) << 2;
}



/**
 * @brief       设置ATK-MC2640模块启用的寄存器块
 * @param       set: ATK_MC2640_REG_BANK_DSP   : DSP寄存器块
 *                   ATK_MC2640_REG_BANK_SENSOR: Sensor寄存器块
 * @retval      ATK_MC2640_EOK   : 设置ATK-MC2640模块启用的寄存器块成功
 *              ATK_MC2640_EINVAL: 传入参数错误
 */
static uint8_t atk_mc2640_reg_bank_select(atk_mc2640_reg_bank_t bank)
{
    switch (bank)
    {
        case ATK_MC2640_REG_BANK_DSP:
        {
            Bsp_OV2640_WriteByte(ATK_MC2640_REG_BANK_SEL, 0x00);
            break;
        }
        case ATK_MC2640_REG_BANK_SENSOR:
        {
            Bsp_OV2640_WriteByte(ATK_MC2640_REG_BANK_SEL, 0x01);
            break;
        }
        default:
        {
            return ATK_MC2640_EINVAL;
        }
    }

    return ATK_MC2640_EOK;
}



/**
 * @brief       设置ATK-MC2640模块灯光模式
 * @param       mode: ATK_MC2640_LIGHT_MOED_AUTO  : Auto
 *                    ATK_MC2640_LIGHT_MOED_SUNNY : Sunny
 *                    ATK_MC2640_LIGHT_MOED_CLOUDY: Cloudy
 *                    ATK_MC2640_LIGHT_MOED_OFFICE: Office
 *                    ATK_MC2640_LIGHT_MOED_HOME  : Home
 * @retval      ATK_MC2640_EOK   : 设置ATK-MC2640模块灯光模式成功
 *              ATK_MC2640_EINVAL: 传入参数错误
 */
uint8_t atk_mc2640_set_light_mode(atk_mc2640_light_mode_t mode)
{
    switch (mode)
    {
        case ATK_MC2640_LIGHT_MODE_AUTO:
        {
            atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_DSP);
            Bsp_OV2640_WriteByte(0xC7, 0x00);
            break;
        }
        case ATK_MC2640_LIGHT_MODE_SUNNY:
        {
            atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_DSP);
            Bsp_OV2640_WriteByte(0xC7, 0x40);
            Bsp_OV2640_WriteByte(0xCC, 0x5E);
            Bsp_OV2640_WriteByte(0xCD, 0x41);
            Bsp_OV2640_WriteByte(0xCE, 0x54);
            break;
        }
        case ATK_MC2640_LIGHT_MODE_CLOUDY:
        {
            atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_DSP);
            Bsp_OV2640_WriteByte(0xC7, 0x40);
            Bsp_OV2640_WriteByte(0xCC, 0x65);
            Bsp_OV2640_WriteByte(0xCD, 0x41);
            Bsp_OV2640_WriteByte(0xCE, 0x4F);
            break;
        }
        case ATK_MC2640_LIGHT_MODE_OFFICE:
        {
            atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_DSP);
            Bsp_OV2640_WriteByte(0xC7, 0x40);
            Bsp_OV2640_WriteByte(0xCC, 0x52);
            Bsp_OV2640_WriteByte(0xCD, 0x41);
            Bsp_OV2640_WriteByte(0xCE, 0x66);
            break;
        }
        case ATK_MC2640_LIGHT_MODE_HOME:
        {
            atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_DSP);
            Bsp_OV2640_WriteByte(0xC7, 0x40);
            Bsp_OV2640_WriteByte(0xCC, 0x42);
            Bsp_OV2640_WriteByte(0xCD, 0x3F);
            Bsp_OV2640_WriteByte(0xCE, 0x71);
            break;
        }
        default:
        {
            return ATK_MC2640_EINVAL;
        }
    }

    return ATK_MC2640_EOK;
}

/**
 * @brief       设置ATK-MC2640模块色彩饱和度
 * @param       saturation: ATK_MC2640_COLOR_SATURATION_0: +2
 *                          ATK_MC2640_COLOR_SATURATION_1: +1
 *                          ATK_MC2640_COLOR_SATURATION_2: 0
 *                          ATK_MC2640_COLOR_SATURATION_3: -1
 *                          ATK_MC2640_COLOR_SATURATION_4: -2
 * @retval      ATK_MC2640_EOK   : 设置ATK-MC2640模块色彩饱和度成功
 *              ATK_MC2640_EINVAL: 传入参数错误
 */
uint8_t atk_mc2640_set_color_saturation(atk_mc2640_color_saturation_t saturation)
{
    switch (saturation)
    {
        case ATK_MC2640_COLOR_SATURATION_0:
        {
            atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_DSP);
            Bsp_OV2640_WriteByte(0x7C, 0x00);
            Bsp_OV2640_WriteByte(0x7D, 0x02);
            Bsp_OV2640_WriteByte(0x7C, 0x03);
            Bsp_OV2640_WriteByte(0x7D, 0x68);
            Bsp_OV2640_WriteByte(0x7D, 0x68);
            break;
        }
        case ATK_MC2640_COLOR_SATURATION_1:
        {
            atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_DSP);
            Bsp_OV2640_WriteByte(0x7C, 0x00);
            Bsp_OV2640_WriteByte(0x7D, 0x02);
            Bsp_OV2640_WriteByte(0x7C, 0x03);
            Bsp_OV2640_WriteByte(0x7D, 0x58);
            Bsp_OV2640_WriteByte(0x7D, 0x58);
            break;
        }
        case ATK_MC2640_COLOR_SATURATION_2:
        {
            atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_DSP);
            Bsp_OV2640_WriteByte(0x7C, 0x00);
            Bsp_OV2640_WriteByte(0x7D, 0x02);
            Bsp_OV2640_WriteByte(0x7C, 0x03);
            Bsp_OV2640_WriteByte(0x7D, 0x48);
            Bsp_OV2640_WriteByte(0x7D, 0x48);
            break;
        }
        case ATK_MC2640_COLOR_SATURATION_3:
        {
            atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_DSP);
            Bsp_OV2640_WriteByte(0x7C, 0x00);
            Bsp_OV2640_WriteByte(0x7D, 0x02);
            Bsp_OV2640_WriteByte(0x7C, 0x03);
            Bsp_OV2640_WriteByte(0x7D, 0x38);
            Bsp_OV2640_WriteByte(0x7D, 0x38);
            break;
        }
        case ATK_MC2640_COLOR_SATURATION_4:
        {
            atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_DSP);
            Bsp_OV2640_WriteByte(0x7C, 0x00);
            Bsp_OV2640_WriteByte(0x7D, 0x02);
            Bsp_OV2640_WriteByte(0x7C, 0x03);
            Bsp_OV2640_WriteByte(0x7D, 0x28);
            Bsp_OV2640_WriteByte(0x7D, 0x28);
            break;
        }
        default:
        {
            return ATK_MC2640_EINVAL;
        }
    }

    return ATK_MC2640_EOK;
}

/**
 * @brief       设置ATK-MC2640模块亮度
 * @param       brightness: ATK_MC2640_BRIGHTNESS_0: +2
 *                          ATK_MC2640_BRIGHTNESS_1: +1
 *                          ATK_MC2640_BRIGHTNESS_2: 0
 *                          ATK_MC2640_BRIGHTNESS_3: -1
 *                          ATK_MC2640_BRIGHTNESS_4: -2
 * @retval      ATK_MC2640_EOK   : 设置ATK-MC2640模块亮度成功
 *              ATK_MC2640_EINVAL: 传入参数错误
 */
uint8_t atk_mc2640_set_brightness(atk_mc2640_brightness_t brightness)
{
    switch (brightness)
    {
        case ATK_MC2640_BRIGHTNESS_0:
        {
            atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_DSP);
            Bsp_OV2640_WriteByte(0x7C, 0x00);
            Bsp_OV2640_WriteByte(0x7D, 0x04);
            Bsp_OV2640_WriteByte(0x7C, 0x09);
            Bsp_OV2640_WriteByte(0x7D, 0x40);
            Bsp_OV2640_WriteByte(0x7D, 0x00);
            break;
        }
        case ATK_MC2640_BRIGHTNESS_1:
        {
            atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_DSP);
            Bsp_OV2640_WriteByte(0x7C, 0x00);
            Bsp_OV2640_WriteByte(0x7D, 0x04);
            Bsp_OV2640_WriteByte(0x7C, 0x09);
            Bsp_OV2640_WriteByte(0x7D, 0x30);
            Bsp_OV2640_WriteByte(0x7D, 0x00);
            break;
        }
        case ATK_MC2640_BRIGHTNESS_2:
        {
            atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_DSP);
            Bsp_OV2640_WriteByte(0x7C, 0x00);
            Bsp_OV2640_WriteByte(0x7D, 0x04);
            Bsp_OV2640_WriteByte(0x7C, 0x09);
            Bsp_OV2640_WriteByte(0x7D, 0x20);
            Bsp_OV2640_WriteByte(0x7D, 0x00);
            break;
        }
        case ATK_MC2640_BRIGHTNESS_3:
        {
            atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_DSP);
            Bsp_OV2640_WriteByte(0x7C, 0x00);
            Bsp_OV2640_WriteByte(0x7D, 0x04);
            Bsp_OV2640_WriteByte(0x7C, 0x09);
            Bsp_OV2640_WriteByte(0x7D, 0x10);
            Bsp_OV2640_WriteByte(0x7D, 0x00);
            break;
        }
        case ATK_MC2640_BRIGHTNESS_4:
        {
            atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_DSP);
            Bsp_OV2640_WriteByte(0x7C, 0x00);
            Bsp_OV2640_WriteByte(0x7D, 0x04);
            Bsp_OV2640_WriteByte(0x7C, 0x09);
            Bsp_OV2640_WriteByte(0x7D, 0x00);
            Bsp_OV2640_WriteByte(0x7D, 0x00);
            break;
        }
        default:
        {
            return ATK_MC2640_EINVAL;
        }
    }

    return ATK_MC2640_EOK;
}

/**
 * @brief       设置ATK-MC2640模块对比度
 * @param       contrast: ATK_MC2640_CONTRAST_0: +2
 *                        ATK_MC2640_CONTRAST_1: +1
 *                        ATK_MC2640_CONTRAST_2: 0
 *                        ATK_MC2640_CONTRAST_3: -1
 *                        ATK_MC2640_CONTRAST_4: -2
 * @retval      ATK_MC2640_EOK   : 设置ATK-MC2640模块对比度成功
 *              ATK_MC2640_EINVAL: 传入参数错误
 */
uint8_t atk_mc2640_set_contrast(atk_mc2640_contrast_t contrast)
{
    switch (contrast)
    {
        case ATK_MC2640_CONTRAST_0:
        {
            atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_DSP);
            Bsp_OV2640_WriteByte(0x7C, 0x00);
            Bsp_OV2640_WriteByte(0x7D, 0x04);
            Bsp_OV2640_WriteByte(0x7C, 0x07);
            Bsp_OV2640_WriteByte(0x7D, 0x20);
            Bsp_OV2640_WriteByte(0x7D, 0x28);
            Bsp_OV2640_WriteByte(0x7D, 0x0C);
            Bsp_OV2640_WriteByte(0x7D, 0x06);
            break;
        }
        case ATK_MC2640_CONTRAST_1:
        {
            atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_DSP);
            Bsp_OV2640_WriteByte(0x7C, 0x00);
            Bsp_OV2640_WriteByte(0x7D, 0x04);
            Bsp_OV2640_WriteByte(0x7C, 0x07);
            Bsp_OV2640_WriteByte(0x7D, 0x20);
            Bsp_OV2640_WriteByte(0x7D, 0x24);
            Bsp_OV2640_WriteByte(0x7D, 0x16);
            Bsp_OV2640_WriteByte(0x7D, 0x06);
            break;
        }
        case ATK_MC2640_CONTRAST_2:
        {
            atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_DSP);
            Bsp_OV2640_WriteByte(0x7C, 0x00);
            Bsp_OV2640_WriteByte(0x7D, 0x04);
            Bsp_OV2640_WriteByte(0x7C, 0x07);
            Bsp_OV2640_WriteByte(0x7D, 0x20);
            Bsp_OV2640_WriteByte(0x7D, 0x20);
            Bsp_OV2640_WriteByte(0x7D, 0x20);
            Bsp_OV2640_WriteByte(0x7D, 0x06);
            break;
        }
        case ATK_MC2640_CONTRAST_3:
        {
            atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_DSP);
            Bsp_OV2640_WriteByte(0x7C, 0x00);
            Bsp_OV2640_WriteByte(0x7D, 0x04);
            Bsp_OV2640_WriteByte(0x7C, 0x07);
            Bsp_OV2640_WriteByte(0x7D, 0x20);
            Bsp_OV2640_WriteByte(0x7D, 0x1C);
            Bsp_OV2640_WriteByte(0x7D, 0x2A);
            Bsp_OV2640_WriteByte(0x7D, 0x06);
            break;
        }
        case ATK_MC2640_CONTRAST_4:
        {
            atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_DSP);
            Bsp_OV2640_WriteByte(0x7C, 0x00);
            Bsp_OV2640_WriteByte(0x7D, 0x04);
            Bsp_OV2640_WriteByte(0x7C, 0x07);
            Bsp_OV2640_WriteByte(0x7D, 0x20);
            Bsp_OV2640_WriteByte(0x7D, 0x18);
            Bsp_OV2640_WriteByte(0x7D, 0x34);
            Bsp_OV2640_WriteByte(0x7D, 0x06);
            break;
        }
        default:
        {
            return ATK_MC2640_EINVAL;
        }
    }

    return ATK_MC2640_EOK;
}


/**
 * @brief       设置ATK-MC2640模块特殊效果
 * @param       contrast: ATK_MC2640_SPECIAL_EFFECT_ANTIQUE    : Antique
 *                        ATK_MC2640_SPECIAL_EFFECT_BLUISH     : Bluish
 *                        ATK_MC2640_SPECIAL_EFFECT_GREENISH   : Greenish
 *                        ATK_MC2640_SPECIAL_EFFECT_REDISH     : Redish
 *                        ATK_MC2640_SPECIAL_EFFECT_BW         : B&W
 *                        ATK_MC2640_SPECIAL_EFFECT_NEGATIVE   : Negative
 *                        ATK_MC2640_SPECIAL_EFFECT_BW_NEGATIVE: B&W Negative
 *                        ATK_MC2640_SPECIAL_EFFECT_NORMAL     : Normal
 * @retval      ATK_MC2640_EOK   : 设置ATK-MC2640模块特殊效果成功
 *              ATK_MC2640_EINVAL: 传入参数错误
 */
uint8_t atk_mc2640_set_special_effect(atk_mc2640_special_effect_t effect)
{
    switch (effect)
    {
        case ATK_MC2640_SPECIAL_EFFECT_ANTIQUE:
        {
            atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_DSP);
            Bsp_OV2640_WriteByte(0x7C, 0x00);
            Bsp_OV2640_WriteByte(0x7D, 0x18);
            Bsp_OV2640_WriteByte(0x7C, 0x05);
            Bsp_OV2640_WriteByte(0x7D, 0x40);
            Bsp_OV2640_WriteByte(0x7D, 0xA6);
            break;
        }
        case ATK_MC2640_SPECIAL_EFFECT_BLUISH:
        {
            atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_DSP);
            Bsp_OV2640_WriteByte(0x7C, 0x00);
            Bsp_OV2640_WriteByte(0x7D, 0x18);
            Bsp_OV2640_WriteByte(0x7C, 0x05);
            Bsp_OV2640_WriteByte(0x7D, 0xA0);
            Bsp_OV2640_WriteByte(0x7D, 0x40);
            break;
        }
        case ATK_MC2640_SPECIAL_EFFECT_GREENISH:
        {
            atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_DSP);
            Bsp_OV2640_WriteByte(0x7C, 0x00);
            Bsp_OV2640_WriteByte(0x7D, 0x18);
            Bsp_OV2640_WriteByte(0x7C, 0x05);
            Bsp_OV2640_WriteByte(0x7D, 0x40);
            Bsp_OV2640_WriteByte(0x7D, 0x40);
            break;
        }
        case ATK_MC2640_SPECIAL_EFFECT_REDISH:
        {
            atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_DSP);
            Bsp_OV2640_WriteByte(0x7C, 0x00);
            Bsp_OV2640_WriteByte(0x7D, 0x18);
            Bsp_OV2640_WriteByte(0x7C, 0x05);
            Bsp_OV2640_WriteByte(0x7D, 0x40);
            Bsp_OV2640_WriteByte(0x7D, 0xC0);
            break;
        }
        case ATK_MC2640_SPECIAL_EFFECT_BW:
        {
            atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_DSP);
            Bsp_OV2640_WriteByte(0x7C, 0x00);
            Bsp_OV2640_WriteByte(0x7D, 0x18);
            Bsp_OV2640_WriteByte(0x7C, 0x05);
            Bsp_OV2640_WriteByte(0x7D, 0x80);
            Bsp_OV2640_WriteByte(0x7D, 0x80);
            break;
        }
        case ATK_MC2640_SPECIAL_EFFECT_NEGATIVE:
        {
            atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_DSP);
            Bsp_OV2640_WriteByte(0x7C, 0x00);
            Bsp_OV2640_WriteByte(0x7D, 0x40);
            Bsp_OV2640_WriteByte(0x7C, 0x05);
            Bsp_OV2640_WriteByte(0x7D, 0x80);
            Bsp_OV2640_WriteByte(0x7D, 0x80);
            break;
        }
        case ATK_MC2640_SPECIAL_EFFECT_BW_NEGATIVE:
        {
            atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_DSP);
            Bsp_OV2640_WriteByte(0x7C, 0x00);
            Bsp_OV2640_WriteByte(0x7D, 0x58);
            Bsp_OV2640_WriteByte(0x7C, 0x05);
            Bsp_OV2640_WriteByte(0x7D, 0x80);
            Bsp_OV2640_WriteByte(0x7D, 0x80);
            break;
        }
        case ATK_MC2640_SPECIAL_EFFECT_NORMAL:
        {
            atk_mc2640_reg_bank_select(ATK_MC2640_REG_BANK_DSP);
            Bsp_OV2640_WriteByte(0x7C, 0x00);
            Bsp_OV2640_WriteByte(0x7D, 0x00);
            Bsp_OV2640_WriteByte(0x7C, 0x05);
            Bsp_OV2640_WriteByte(0x7D, 0x80);
            Bsp_OV2640_WriteByte(0x7D, 0x80);
            break;
        }
        default:
        {
            return ATK_MC2640_EINVAL;
        }
    }

    return ATK_MC2640_EOK;
}