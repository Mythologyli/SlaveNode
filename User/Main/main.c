/**
 * @file    main.c
 * @author  Myth
 * @version 1.0
 * @date    2021.10.17
 * @brief   工程主函数文件
 * @details 初始化及主循环
 * @note    此版本实现功能：
 *
 *          网关通过 LoRa 模块轮流向节点请求数据，节点（本机）将数据返回给主节点
 *
 *          JTAG 已禁用，请使用 SWD 调试
 */

#include "sys.h"
#include "systick.h"
#include "uart.h"

#include "led.h"
#include "lora.h"
#include "aht20.h"
#include "bh1750.h"

#define NODE_SEQ_NUM '2' //节点标号：2 ~ 5

typedef struct
{
    uint8_t seq;
    float humi;
    float temp;
    float light;
    uint8_t end;
} DataPack; //数据包定义，由于平台相同，网关和节点通讯无需考虑对齐问题

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

    LED_Init();    //初始化 LED
    LoRa_Init();   //初始化 LoRa 模块
    AHT20_Init();  //初始化 AHT20
    BH1750_Init(); //初始化 BH1750

    DataPack pack;           //发送的数据包
    pack.seq = NODE_SEQ_NUM; //数据包序号，标明发送节点
    pack.end = '@';          //数据包结束字节，用于网关校验

    uint8_t buffer[3]; //接收缓冲区
    uint8_t size;      //接收到的包大小
    int32_t time;      //当前时间，控制时序

    //首先获取一次数据，防止发送空数据
    AHT20_Start();  // AHT20 开始测量
    BH1750_Start(); // BH1750 开始测量

    Delay_ms(80);
    AHT20_Read(&(pack.humi), &(pack.temp));
    Delay_ms(100);
    pack.light = BH1750_Read();

    while (1)
    {
        //程序主循环
        AHT20_Start();  // AHT20 开始测量
        BH1750_Start(); // BH1750 开始测量

        //在等待测量的过程中检测是否有命令包索取数据
        //说明：
        //下面一段代码显然应该封装为函数。但在封装为函数后出现恶性 Bug，猜测为溢出导致，正在排除
        //即使不封装为函数，将 buffer size 声明为全局变量也会导致此 Bug，表现为接收后死机
        time = SysTick_GetRunTime();
        while (SysTick_CheckRunTime(time) < 80) //等待 80 ms
        {
            size = LoRa_Receive(buffer); //接收
            if (size < 1)
            {
                continue;
            }
            else if (size != 3 || buffer[0] != '#' || buffer[1] != NODE_SEQ_NUM || buffer[2] != '@') //判断命令包是否正确，是否向自己查询
            {
                printf("Get Wrong Command[%d]: %c%c%c\n", size, buffer[0], buffer[1], buffer[2]);
                continue;
            }

            //命令包正确
            LED1_Toggle; // LED 翻转
            printf("Get Right Command[%d]: %c%c%c\n", size, buffer[0], buffer[1], buffer[2]);

            //返回最近一个获得的数据。由于无 DIO0 的 LoRa 通讯不可靠，发送 3 次
            for (uint8_t i = 0; i < 3; i++)
            {
                printf("Send!\n");
                LoRa_Send(&pack, sizeof(DataPack));
                Delay_ms(300);
            }
        }

        AHT20_Read(&(pack.humi), &(pack.temp)); // 80ms 后可读取 AHT20

        time = SysTick_GetRunTime();
        while (SysTick_CheckRunTime(time) < 100) //等待 100 ms
        {
            size = LoRa_Receive(buffer); //接收
            if (size < 1)
            {
                continue;
            }
            else if (size != 3 || buffer[0] != '#' || buffer[1] != NODE_SEQ_NUM || buffer[2] != '@') //判断命令包是否正确，是否向自己查询
            {
                printf("Get Wrong Command[%d]: %c%c%c\n", size, buffer[0], buffer[1], buffer[2]);
                continue;
            }

            //命令包正确
            LED1_Toggle; // LED 翻转
            printf("Get Right Command[%d]: %c%c%c\n", size, buffer[0], buffer[1], buffer[2]);

            //返回最近一个获得的数据。由于无 DIO0 的 LoRa 通讯不可靠，发送 3 次
            for (uint8_t i = 0; i < 3; i++)
            {
                printf("Send!\n");
                LoRa_Send(&pack, sizeof(DataPack));
                Delay_ms(300);
            }
        }

        pack.light = BH1750_Read(); // 180ms 后才能读取 BH1750
    }

    return 1;
}
