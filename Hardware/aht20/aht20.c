/**
 * @file    aht20.c
 * @author  Myth
 * @version 0.2
 * @date    2021.10.15
 * @brief   AHT20 驱动
 */

#include "systick.h"

#include "softi2c.h"

#include "aht20.h"

#define I2C_Start SoftI2C_Start(&aht20_i2c)
#define I2C_Stop SoftI2C_Stop(&aht20_i2c)
#define I2C_WriteByte(__byte__) SoftI2C_WriteByte(&aht20_i2c, __byte__)
#define I2C_ReadByte SoftI2C_ReadByte(&aht20_i2c)
#define I2C_Ack SoftI2C_Ack(&aht20_i2c)
#define I2C_NAck SoftI2C_NAck(&aht20_i2c)
#define I2C_WaitAck SoftI2C_WaitAck(&aht20_i2c)

SoftI2C_TypeDef aht20_i2c;

uint8_t AHT20_Read_Status(void);

/**
 * @brief  初始化 AHT20
 */
void AHT20_Init(void)
{
    aht20_i2c.SDA_GPIO = AHT20_GPIO;
    aht20_i2c.SDA_Pin = AHT20_SDA_PIN;

    aht20_i2c.SCL_GPIO = AHT20_GPIO;
    aht20_i2c.SCL_Pin = AHT20_SCL_PIN;

    aht20_i2c.Delay_Time = 5;

    SoftI2C_Init(&aht20_i2c);

    Delay_ms(500);

    if (AHT20_Read_Status() & 0x18 == 0x18)
        return;

    I2C_Start;
    I2C_WriteByte(0x70);
    I2C_WaitAck;
    I2C_WriteByte(0xa8);
    I2C_WaitAck;
    I2C_WriteByte(0x00);
    I2C_WaitAck;
    I2C_WriteByte(0x00);
    I2C_WaitAck;
    I2C_Stop;

    Delay_ms(10);

    I2C_Start;
    I2C_WriteByte(0x70);
    I2C_WaitAck;
    I2C_WriteByte(0xbe);
    I2C_WaitAck;
    I2C_WriteByte(0x08);
    I2C_WaitAck;
    I2C_WriteByte(0x00);
    I2C_WaitAck;
    I2C_Stop;

    Delay_ms(20);
}

/**
 * @brief  读取 AHT20 状态字
 * @retval 状态字
 */
uint8_t AHT20_Read_Status(void)
{
    uint8_t data;

    I2C_Start;

    I2C_WriteByte(0x71);
    I2C_WaitAck;

    data = I2C_ReadByte;
    I2C_NAck;

    I2C_Stop;

    return data;
}

/**
 * @brief  向 AHT20 发送测量命令。需等待 80ms 后才能读取数据
 */
void AHT20_Start(void)
{
    //发送 AC 指令请求数据
    I2C_Start;
    I2C_WriteByte(0x70);
    I2C_WaitAck;
    I2C_WriteByte(0xac);
    I2C_WaitAck;
    I2C_WriteByte(0x33);
    I2C_WaitAck;
    I2C_WriteByte(0x00);
    I2C_WaitAck;
    I2C_Stop;
}

/**
 * @brief  读取 AHT20 数据。在发送测量命令至少 80ms 后调用
 * @param  humi: 浮点数指针，储存湿度
 * @param  temp: 浮点数指针，储存温度
 * @retval 读取成功返回 1，读取失败返回 0
 */
uint8_t AHT20_Read(float *humi, float *temp)
{
    uint8_t byte1 = 0;
    uint8_t byte2 = 0;
    uint8_t byte3 = 0;
    uint8_t byte4 = 0;
    uint8_t byte5 = 0;
    uint8_t byte6 = 0;
    uint32_t data32 = 0;

    uint16_t count = 0;
    while (((AHT20_Read_Status() & 0x80) == 0x80)) //等待状态字最高位变为 1，说明数据准备完成
    {
        Delay_ms(1);
        if (count++ >= 100)
            return 0;
    }

    I2C_Start;
    I2C_WriteByte(0x71);
    I2C_WaitAck;
    byte1 = I2C_ReadByte; //状态字
    I2C_Ack;
    byte2 = I2C_ReadByte; //湿度数据
    I2C_Ack;
    byte3 = I2C_ReadByte; //湿度数据
    I2C_Ack;
    byte4 = I2C_ReadByte; //湿度/温度数据
    I2C_Ack;
    byte5 = I2C_ReadByte; //温度数据
    I2C_Ack;
    byte6 = I2C_ReadByte; //温度数据
    I2C_NAck;
    I2C_Stop;

    data32 = (data32 | byte2) << 8;
    data32 = (data32 | byte3) << 8;
    data32 = (data32 | byte4);
    data32 = data32 >> 4;
    *humi = (float)data32 * 100.0 / 1024.0 / 1024.0; //湿度
    data32 = 0;
    data32 = (data32 | byte4) << 8;
    data32 = (data32 | byte5) << 8;
    data32 = (data32 | byte6);
    data32 = data32 & 0xfffff;
    *temp = ((float)data32 * 200 * 10 / 1024 / 1024 - 500) / 10.0; //温度

    return 1;
}

/**
 * @brief  发送测量命令，等待 80ms 并读取 AHT20 数据
 * @param  humi: 浮点数指针，储存湿度
 * @param  temp: 浮点数指针，储存温度
 * @retval 读取成功返回 1，读取失败返回 0
 */
uint8_t AHT20_StartAndRead(float *humi, float *temp)
{
    AHT20_Start();
    Delay_ms(80); //延时 80ms，等待 AHT20 生成数据
    return AHT20_Read(humi, temp);
}
