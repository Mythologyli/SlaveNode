/**
 * @file    aht20.h
 * @author  Myth
 * @version 0.1
 * @date    2021.10.13
 * @brief   AHT20 驱动
 */

#ifndef __AHT20_H
#define __AHT20_H

#include "sys.h"

#define AHT20_GPIO GPIOB
#define AHT20_SDA_PIN GPIO_PIN_3
#define AHT20_SCL_PIN GPIO_PIN_4

void AHT20_Init(void);
uint8_t AHT20_Read(float *humi, float *temp);

#endif
