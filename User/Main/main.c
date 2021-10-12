/**
 * @file    main.c
 * @author  Myth
 * @version 0.3
 * @date    2021.10.12
 * @brief   工程主函数文件
 * @details 初始化及主循环
 * @note    此版本实现功能：
 *          串口回显，回显时 PC13 上的 LED 闪烁
 *          软件 I2C
 *          JTAG 已禁用，请使用 SWD 调试
 */

#include "sys.h"
#include "systick.h"
#include "uart.h"

#include "softi2c.h"

#include "led.h"

void Echo(uint8_t byte);

int main(void)
{
    if (HAL_Init() != HAL_OK) //初始化 HAL 库
    {
        Error_Handler(__FILE__, __LINE__); //错误处理
    }
    SystemClock_Config(); //初始化系统时钟为 72MHz
    DisableJTAG();        //禁用 JTAG
    SysTick_Init();       //初始化 SysTick 和软件定时器
    UART_Init();          //初始化串口

    LED_Init(); //初始化 LED

    UART_BindReceiveHandle(COM1, Echo); //绑定 COM1 串口接收中断至 Echo 函数

    SoftI2C_TypeDef SoftI2C;

    SoftI2C.SDA_GPIO = GPIOB;
    SoftI2C.SDA_Pin = GPIO_PIN_3;
    SoftI2C.SCL_GPIO = GPIOB;
    SoftI2C.SCL_Pin = GPIO_PIN_4;
    SoftI2C.Delay_Time = 10;

    if (SoftI2C_Init(&SoftI2C) != HAL_OK) //初始化软件 I2C
    {
        Error_Handler(__FILE__, __LINE__); //错误处理
    }

    while (1)
    {
        //程序主循环
        SoftI2C_Start(&SoftI2C);

        SoftI2C_WriteByte(&SoftI2C, 0x10);
        SoftI2C_WaitAck(&SoftI2C);

        SoftI2C_WriteByte(&SoftI2C, 0x22);
        SoftI2C_WaitAck(&SoftI2C);

        SoftI2C_WriteByte(&SoftI2C, 0x33);
        SoftI2C_WaitAck(&SoftI2C);

        SoftI2C_WriteByte(&SoftI2C, 0x44);
        SoftI2C_WaitAck(&SoftI2C);

        SoftI2C_Stop(&SoftI2C);

        Delay_us(1000);
    }

    return 1;
}

/**
  * @brief  串口回显函数
  * @param  byte: 本次中断接收到的字节
  */
void Echo(uint8_t byte)
{
    LED1_Slow_Toggle;
    UART_SendChar(COM1, byte);
}
