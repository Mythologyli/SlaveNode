/**
 * @file    main.c
 * @author  Myth
 * @version 0.7
 * @date    2021.10.15
 * @brief   工程主函数文件
 * @details 初始化及主循环
 * @note    此版本实现功能：
 *          串口回显，回显时 PC13 上的 LED 闪烁
 *          AHT20 温度湿度读取及 BH1750 光照度读取
 *          此版本 AHT20 和 BH1750 在两次读取期间关闭 GPIO 时钟以降低功耗。每一次读取结束后 LED 闪烁
 *          JTAG 已禁用，请使用 SWD 调试
 */

#include "sys.h"
#include "systick.h"
#include "uart.h"

#include "led.h"
#include "aht20.h"
#include "bh1750.h"

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

    AHT20_Init(); //初始化 AHT20
    float humi, temp;

    BH1750_Init(); //初始化 BH1750
    float light;

    while (1)
    {
        //程序主循环
        __HAL_RCC_GPIOB_CLK_ENABLE(); //开启 GPIOB 时钟

        AHT20_Start();  // AHT20 开始测量
        BH1750_Start(); // BH1750 开始测量

        Delay_ms(80);

        AHT20_Read(&humi, &temp); //读取 AHT20

        Delay_ms(100);

        light = BH1750_Read(); //读取 BH1750

        printf("humi: %.1f    temp: %.1f    light: %.1f\n", humi, temp, light);
        LED1_Toggle;

        __HAL_RCC_GPIOB_CLK_DISABLE(); //关闭 GPIOB 时钟以降低功耗

        Delay_ms(500);
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
