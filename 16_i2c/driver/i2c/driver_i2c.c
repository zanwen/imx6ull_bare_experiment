//
// see 31.5 Initialization
//

#include "driver_i2c.h"
#include "logger.h"
#include "driver_delay.h"

void Driver_I2C_WaitFlag(I2C_Type *base, uint8_t bitIndex, uint8_t bitValue);

void Driver_I2C_Init(I2C_Type *base) {
    // I2C disable and reset
    base->I2CR |= (1 << 7);
    // SCL clock freq: 66M / 640 = 103.125kHz
    base->IFDR = 0x15;
    // I2C enable
    base->I2CR |= (1 << 7);
}

void Driver_I2C_Start(I2C_Type *base) {
    // wait for bus idle
    Driver_I2C_WaitFlag(base, 5, 0);
    // clear interrupt flag
    base->I2SR &= ~(1 << 1);
    // [5]: Changing MSTA from 0 to 1 signals a Start on the bus and selects Master mode.
    base->I2CR &= ~(1 << 5);
    base->I2CR |= (1 << 5);
}

void Driver_I2C_ReStart(I2C_Type *base) {
    // clear interrupt flag
    base->I2SR &= ~(1 << 1);
    // [2]: Repeat start
    base->I2CR |= (1 << 2);
}

void Driver_I2C_WaitFlag(I2C_Type *base, uint8_t bitIndex, uint8_t bitValue) {
    uint16_t timeout = 0xFFFF;
    while (timeout && ((base->I2SR >> bitIndex) & 0x01) != bitValue) {
        timeout--;
    }
    if (timeout == 0) {
        LOG_ERROR("wait flag timeout, bitIndex: %d, bitValue: %d", bitIndex, bitValue);
    }
}

void Driver_I2C_Stop(I2C_Type *base) {
    // clear interrupt flag
    base->I2SR &= ~(1 << 1);

    base->I2CR |= (1 << 5);
    base->I2CR &= ~(1 << 5);
}

void Driver_I2C_SendAddr(I2C_Type *base, uint8_t addrRW) {
    // select Transmit mode
    base->I2CR |= (1 << 4);
    // write DR
    base->I2DR = addrRW;
    // wait for transmit complete
    Driver_I2C_WaitFlag(base, 1, 1);
    // clear interrupt flag
    base->I2SR &= ~(1 << 1);
}

void Driver_I2C_SendBytes(I2C_Type *base, uint8_t *data, uint8_t len) {
    // select Transmit mode
    base->I2CR |= (1 << 4);
    for (uint8_t i = 0; i < len; ++i) {
        // wait for transmit complete
        base->I2DR = data[i];
        // wait for transmit complete
        Driver_I2C_WaitFlag(base, 1, 1);
        // clear interrupt flag
        base->I2SR &= ~(1 << 1);
    }
}

void Driver_I2C_ReadBytesAndStop(I2C_Type *base, uint8_t *buf, uint8_t len) {
    // select Receive mode; enable auto ack
    base->I2CR &= ~((1 << 4) | (1 << 3));
    //If Master Receive mode is required, then
    //I2C_I2CR[MTX] should be toggled and a dummy read of the I2C_I2DR register must be
    //executed to trigger receive data.
    base->I2DR;// dummy read

    // For a master receiver to terminate a data transfer, it must inform the slave transmitter by
    //  not acknowledging the last data byte. This is done by setting the transmit acknowledge
    //  bit (I2C_I2CR[TXAK]) before reading the next-to-last byte
    // Before the last byte is read a Stop signal must be generated
    if (len == 1) {
        // disable auto ack
        base->I2CR |= (1 << 3);
        Driver_I2C_WaitFlag(base, 1, 1);
        // generate stop
        base->I2CR &= ~(1 << 5);
        buf[0] = base->I2DR;
    } else {
        for (uint8_t i = 0; i < len; ++i) {
            // wait for transmit complete
            Driver_I2C_WaitFlag(base, 1, 1);
            if (i == len - 2) {
                // disable auto ack 读倒数第二个字节之前
                base->I2CR |= (1 << 3);
            }
            if(i == len - 1){
                // generate stop 读最后一个字节之前
                base->I2CR &= ~(1 << 5);
            }
            buf[i] = base->I2DR;
            // clear interrupt flag
            base->I2SR &= ~(1 << 1);
        }
    }


}

void Driver_I2C_MasterWrite(I2C_Type *base, uint8_t addr, uint8_t regAddr,
                           uint8_t *data, uint8_t len) {
    Driver_I2C_Start(base);
    Driver_I2C_SendAddr(base, addr << 1 | 0);
    Driver_I2C_SendBytes(base, &regAddr, 1);
    Driver_I2C_SendBytes(base, data, len);
    Driver_I2C_Stop(base);
}

void Driver_I2C_MasterRead(I2C_Type *base, uint8_t addr, uint8_t regAddr,
                           uint8_t *buf, uint8_t len) {
    Driver_I2C_Start(base);
    // dummy write
    Driver_I2C_SendAddr(base, addr << 1 | 0);
    Driver_I2C_SendBytes(base, &regAddr, 1);

    Driver_I2C_ReStart(base);
    Driver_I2C_SendAddr(base, addr << 1 | 1);
    Driver_I2C_ReadBytesAndStop(base, buf, len);
}

void Driver_I2C_Test(void) {
    IOMUXC_SetPinMux(IOMUXC_UART4_TX_DATA_I2C1_SCL, 1);
    IOMUXC_SetPinMux(IOMUXC_UART4_RX_DATA_I2C1_SDA, 1);
    // 开漏
    IOMUXC_SetPinConfig(IOMUXC_UART4_TX_DATA_I2C1_SCL, 0x18B0);
    IOMUXC_SetPinConfig(IOMUXC_UART4_RX_DATA_I2C1_SDA, 0x18B0);

    Driver_I2C_Init(I2C1);

    uint8_t data = 0x04;
    Driver_I2C_MasterWrite(I2C1, 0x1E, 0x00, &data, 1);
    Driver_Delay_MS(10);
    data = 0x03;
    Driver_I2C_MasterWrite(I2C1, 0x1E, 0x00, &data, 1);

    uint8_t buf[1] = {0};
    Driver_I2C_MasterRead(I2C1, 0x1E, 0x00, buf, 1);
    LOG_DEBUG("buf = %#x", buf[0]);
    LOG_DEBUG("Driver_I2C_Test done");

    uint8_t alsData[2] = {0};
    uint8_t psData[2] = {0};
    uint16_t als = 0, ps = 0;
    while(1) {
        Driver_I2C_MasterRead(I2C1, 0x1E, 0x0C, alsData, 2);
        Driver_I2C_MasterRead(I2C1, 0x1E, 0x0E, psData, 2);
        als = (alsData[1] << 8) | alsData[0];
        ps = ((alsData[1] & 0x3F) << 4) | (alsData[0] & 0xF);
        LOG_DEBUG("als = %d, ps = %d", als, ps);
        Driver_Delay_MS(500);
    }
}
