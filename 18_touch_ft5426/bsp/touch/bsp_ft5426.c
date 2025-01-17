//
// Created by 86157 on 2025/1/17.
//

#include "bsp_ft5426.h"
#include "logger.h"
#include "driver_gpio.h"
#include "driver_interrupt.h"
#include "driver_i2c.h"
#include "driver_delay.h"

#define FT5426_INT_GPIO_PIN GPIO1, 9
#define FT5426_RST_GPIO_PIN GPIO5, 9

#define FT5426_RST_LOW() Driver_GPIO_WritePIn(FT5426_RST_GPIO_PIN, GPIO_PIN_RESET)
#define FT5426_RST_HIGH() Driver_GPIO_WritePIn(FT5426_RST_GPIO_PIN, GPIO_PIN_SET)

#define FT5426_ADDR_WRITE 0x70
#define FT5426_ADDR_READ 0x71


#define FT5426_DEVICE_MODE        0X00    /* 模式寄存器 			*/
#define FT5426_IDGLIB_VERSION    0XA1    /* 固件版本寄存器 			*/
#define FT5426_IDG_MODE            0XA4    /* 中断模式				*/
#define FT5426_TD_STATUS        0X02    /* 触摸状态寄存器 			*/
#define FT5426_TOUCH1_XH        0X03    /* 触摸点坐标寄存器,
										 * 一个触摸点用4个寄存器存储坐标数据*/

#define FT5426_XYCOORDREG_NUM    30        /* 触摸点坐标寄存器数量 */
#define FT5426_INIT_FINISHED    1        /* 触摸屏初始化完成 			*/
#define FT5426_INIT_NOTFINISHED    0        /* 触摸屏初始化未完成 			*/

#define FT5426_TOUCH_EVENT_DOWN            0x00    /* 按下 		*/
#define FT5426_TOUCH_EVENT_UP            0x01    /* 释放 		*/
#define FT5426_TOUCH_EVENT_ON            0x02    /* 接触 		*/
#define FT5426_TOUCH_EVENT_RESERVED        0x03    /* 没有事件 */

ft5426_device_t ft5426_device = {0};

void Bsp_FT5426_WriteBytes(uint8_t regAddr, uint8_t *data, uint8_t len);

void Bsp_FT5426_ReadBytes(uint8_t regAddr, uint8_t *buf, uint8_t len);

void Bsp_FT5426_InitIO(void);

void Bsp_FT5426_ResetAndInit(void);

void Bsp_FT5426_ReadCoord(void);

void Bsp_FT5426_Init(void) {
    Bsp_FT5426_InitIO();
    Driver_I2C_Init(I2C2);
    Bsp_FT5426_ResetAndInit();
    ft5426_device.initfalg = 1;
    LOG_INFO("Bsp_FT5426_Init done");
}

void Bsp_FT5426_WriteBytes(uint8_t regAddr, uint8_t *data, uint8_t len) {
    Driver_I2C_Start(I2C2);
    Driver_I2C_SendAddr(I2C2, FT5426_ADDR_WRITE);
    Driver_I2C_SendBytes(I2C2, &regAddr, 1);
    Driver_I2C_SendBytes(I2C2, data, len);
    Driver_I2C_Stop(I2C2);
}

void Bsp_FT5426_ReadBytes(uint8_t regAddr, uint8_t *buf, uint8_t len) {
    Driver_I2C_Start(I2C2);
    // dummy write
    Driver_I2C_SendAddr(I2C2, FT5426_ADDR_WRITE);
    Driver_I2C_SendBytes(I2C2, &regAddr, 1);

    Driver_I2C_ReStart(I2C2);
    Driver_I2C_SendAddr(I2C2, FT5426_ADDR_READ);
    Driver_I2C_ReadBytesAndStop(I2C2, buf, len);
}

void Bsp_FT5426_INTCallback(uint32_t GICC_IAR, void *params) {
    // 清除中断标志位
    Driver_GPIO_ClearINTFlag(FT5426_INT_GPIO_PIN);
    if (ft5426_device.initfalg) {
        Bsp_FT5426_ReadCoord();
        LOG_DEBUG("x = %u, y = %u", ft5426_device.x[0], ft5426_device.y[0]);
    }
}

void Bsp_FT5426_InitIO(void) {
    // SCL and SDA
    IOMUXC_SetPinMux(IOMUXC_UART5_TX_DATA_I2C2_SCL, 1);
    IOMUXC_SetPinMux(IOMUXC_UART5_RX_DATA_I2C2_SDA, 1);
    // open drain 拉低也需要输出驱动器
    IOMUXC_SetPinConfig(IOMUXC_UART5_TX_DATA_I2C2_SCL, 0x08B0);
    IOMUXC_SetPinConfig(IOMUXC_UART5_RX_DATA_I2C2_SDA, 0x08B0);

    IOMUXC_SetPinMux(IOMUXC_GPIO1_IO09_GPIO1_IO09, 0);       // INT
    IOMUXC_SetPinConfig(IOMUXC_GPIO1_IO09_GPIO1_IO09, 0xF080); // 上拉输入
    IOMUXC_SetPinMux(IOMUXC_SNVS_SNVS_TAMPER9_GPIO5_IO09, 0); // RESET

    // 中断配置
    GPIO_Config_t config = {0};
    config.direction = GPIO_DIR_INPUT;
    config.intTriggerMode = INT_TRI_RISING_OR_FALLING_EDGE;
    Driver_GPIO_Init(FT5426_INT_GPIO_PIN, &config);
    Driver_GPIO_ClearINTFlag(FT5426_INT_GPIO_PIN);
    Driver_GPIO_EnableINT(FT5426_INT_GPIO_PIN);
    // 注册中断回调
    Driver_INT_RegisterIRQHandler(GPIO1_Combined_0_15_IRQn, Bsp_FT5426_INTCallback, NULL);
    // 使能GIC中断号
    GIC_EnableIRQ(GPIO1_Combined_0_15_IRQn);
}

void Bsp_FT5426_ResetAndInit(void) {
    FT5426_RST_LOW();
    Driver_Delay_MS(1);
    FT5426_RST_HIGH();

    uint8_t data = 0;
    Bsp_FT5426_WriteBytes(FT5426_DEVICE_MODE, &data, 1); // 工作在正常模式
    data = 0x01;
    Bsp_FT5426_WriteBytes(FT5426_IDG_MODE, &data, 1); // 中断触发模式
}

void Bsp_FT5426_ReadCoord(void) {
    unsigned char i = 0;
    unsigned char coordBuf[FT5426_XYCOORDREG_NUM];

    Bsp_FT5426_ReadBytes(FT5426_TD_STATUS, &ft5426_device.point_num, 1);

    /*
       * 从寄存器FT5426_TOUCH1_XH开始，连续读取30个寄存器的值，这30个寄存器
       * 保存着5个点的触摸值，每个点占用6个寄存器。
     */
    Bsp_FT5426_ReadBytes(FT5426_TOUCH1_XH, coordBuf, FT5426_XYCOORDREG_NUM);

    for (i = 0; i < ft5426_device.point_num; i++) {
        unsigned char *buf = &coordBuf[i * 6];
        /* 以第一个触摸点为例，寄存器TOUCH1_XH(地址0X03),各位描述如下：
         * bit7:6  Event flag  0:按下 1:释放 2：接触 3：没有事件
         * bit5:4  保留
         * bit3:0  X轴触摸点的11~8位。
         */
        ft5426_device.x[i] = ((buf[2] << 8) | buf[3]) & 0x0fff;
        ft5426_device.y[i] = ((buf[0] << 8) | buf[1]) & 0x0fff;
    }
}

void Bsp_FT5426_Test(void) {
    uint8_t fwVersion[2] = {0};
    Bsp_FT5426_ReadBytes(FT5426_IDGLIB_VERSION, fwVersion, 2);
    LOG_DEBUG("firmware version = %#x", (fwVersion[0] << 8) | fwVersion[1]);

    uint8_t data = 0x55;
    uint8_t read = 0;
    Bsp_FT5426_WriteBytes(0x80, &data, 1);
    Bsp_FT5426_ReadBytes(0x80, &read, 1);
    LOG_DEBUG("read = %#x", read);
}
