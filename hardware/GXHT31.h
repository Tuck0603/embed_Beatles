#ifndef __GXHT31_H
#define __GXHT31_H

#include "stm32f1xx_hal.h"   /* 根据实际芯片型号调整，如 stm32f4xx_hal.h */


//I2C 从机地址（7位地址左移后的读写地址） 
#define GXHT31_ADDR_W            0x88  //写地址 
#define GXHT31_ADDR_R             0x89   // 读地址 

// 命令码 
#define GXHT31_CMD_INIT             {0x20, 0x32}   // 初始化/配置为0.5s测试一次
#define GXHT31_CMD_MEASURE          {0xE0, 0x00}   // 触发测量（高重复性） 
// CRC8 多项式 
#define CRC8_POLYNOMIAL  0x31


//初始化函数 
void GXHT31_Init(I2C_HandleTypeDef *hi2c);

//触发一次测量并读取结果（阻塞式） 
HAL_StatusTypeDef GXHT31_Measure(I2C_HandleTypeDef *hi2c);

// 获取最近一次测量的温度 
float GXHT31_GetTemperature(void);

// 获取最近一次测量的湿度 
float GXHT31_GetHumidity(void);

void TEM_HUM_Process(void);

#endif // __GXHT31_H 