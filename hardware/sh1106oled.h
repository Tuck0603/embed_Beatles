#ifndef __sh1106oled_H
#define __sh1106oled_H

#include "stm32f1xx_hal.h"   /* 根据实际芯片型号调整，如 stm32f4xx_hal.h */
//声明外部静态变量
extern volatile uint8_t Flag_count;
extern volatile uint8_t clock_sec;
extern volatile uint8_t clock_min;
extern volatile uint8_t clock_hour; 

//初始化显示屏
void sh1106_init();

//复位，调用复位
void sh1106_oled_rest();

//清屏切换间隙调用清屏处理
void clr_oled_screen();

//画点函数
void sh1106_draw_point(uint8_t chXpos, uint8_t chYpos, uint8_t chPoint);

//画位图
//数字位图
void sh1106_draw_bitmap(uint8_t chXpos, uint8_t chYpos, const uint8_t *pchBmp, uint8_t chWidth, uint8_t chHeight);
void sh1106_refresh_gram(void);
//局部擦除
void sh1106_fill_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color);

//时钟初始化
void CLOCK_Init(void);

//动态变化
void CLOCK_Increment(void);

//数字显示
void OLED_ShowNum(uint8_t x, uint8_t y, uint8_t num, uint8_t fontSize);

//冒号显示
void OLED_ShowColon(uint8_t x, uint8_t y);

//时间封装
void OLED_ShowTime(uint8_t clock_hour, uint8_t clock_min, uint8_t clock_sec);

//小数点
void OLED_Showfloat(uint8_t x, uint8_t y);
//显示日期
void Show_year_mouth_day(uint16_t NPTyears, uint8_t NPTmouths, uint8_t NPTdays);
//温湿度
void value_show(uint8_t x, uint8_t y, float value);
void OLED_ShowTEM_HUM(uint8_t x, uint8_t y, float value1,float value2);

//显示字符
void OLED_ShowChar(uint8_t x, uint8_t y, char ch);


#endif