/**
 * @file    bh1750.h
 * @author  Myth
 * @version 0.2
 * @date    2021.10.15
 * @brief   BH1750 驱动
 */

#ifndef __BH1750_H
#define __BH1750_H

#include "sys.h"

// BH1750 引脚设置
#define BH1750_GPIO GPIOB
#define BH1750_SDA_PIN GPIO_PIN_10
#define BH1750_SCL_PIN GPIO_PIN_11

void BH1750_Init(void);
void BH1750_Start(void);
float BH1750_Read(void);
float BH1750_StartAndRead(void);

#endif
