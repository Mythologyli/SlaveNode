/**
 * @file    bh1750.c
 * @author  Myth
 * @version 0.2
 * @date    2021.10.15
 * @brief   BH1750 驱动
 */

#include "systick.h"

#include "softi2c.h"

#include "bh1750.h"

#define SLAVE_ADDR_WR 0x46
#define SLAVE_ADDR_RD 0x47

#define I2C_Start SoftI2C_Start(&bh1750_i2c)
#define I2C_Stop SoftI2C_Stop(&bh1750_i2c)
#define I2C_WriteByte(__byte__) SoftI2C_WriteByte(&bh1750_i2c, __byte__)
#define I2C_ReadByte SoftI2C_ReadByte(&bh1750_i2c)
#define I2C_Ack SoftI2C_Ack(&bh1750_i2c)
#define I2C_NAck SoftI2C_NAck(&bh1750_i2c)
#define I2C_WaitAck SoftI2C_WaitAck(&bh1750_i2c)

SoftI2C_TypeDef bh1750_i2c;

/**
 * @brief  初始化 BH1750
 */
void BH1750_Init(void)
{
    bh1750_i2c.SDA_GPIO = BH1750_GPIO;
    bh1750_i2c.SDA_Pin = BH1750_SDA_PIN;

    bh1750_i2c.SCL_GPIO = BH1750_GPIO;
    bh1750_i2c.SCL_Pin = BH1750_SCL_PIN;

    bh1750_i2c.Delay_Time = 5;

    SoftI2C_Init(&bh1750_i2c);

    Delay_ms(80);

    I2C_Start;
    I2C_WriteByte(SLAVE_ADDR_WR);
    I2C_WaitAck;
    I2C_WriteByte(0x01);
    I2C_WaitAck;
    I2C_Stop;
}

/**
 * @brief  向 BH1750 发送测量命令。需等待 180ms 后才能读取数据
 */
void BH1750_Start(void)
{
    I2C_Start;
    I2C_WriteByte(SLAVE_ADDR_WR);
    I2C_WaitAck;
    I2C_WriteByte(0x10);
    I2C_WaitAck;
    I2C_Stop;
}

/**
 * @brief  读取 BH1750 数据。在发送测量命令至少 180ms 后调用
 * @retval 读数
 */
float BH1750_Read(void)
{
    uint8_t i;
    uint8_t byte1;
    uint8_t byte2;

    I2C_Start;
    I2C_WriteByte(SLAVE_ADDR_RD);
    I2C_WaitAck;

    byte1 = I2C_ReadByte;
    I2C_Ack;

    byte2 = I2C_ReadByte;
    I2C_NAck;

    I2C_Stop;

    return (float)((byte1 << 8) + byte2) / 1.2;
}

/**
 * @brief  发送测量命令，等待 180ms 并读取 BH1750 数据
 * @retval 读数
 */
float BH1750_StartAndRead(void)
{
    BH1750_Start();
    Delay_ms(180);
    return BH1750_Read();
}
