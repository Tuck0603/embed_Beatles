
#ifndef __sh1106oledfont_H
#define __sh1106oledfont_H

#include "stm32f1xx_hal.h"

// 声明三个图标数组（不在此处定义数值）
extern const uint8_t c_chSingal816[16];
extern const uint8_t c_chMsg816[16];
extern const uint8_t c_chBat816[16];
//数字
extern const uint8_t num_5x7[10][7];
extern const uint8_t num_8x16[10][16];
extern const uint8_t num_16x16[10][32];
//冒号
extern const uint8_t colon_4x16[16] ;
//学校名
extern const uint8_t school[324];
extern const uint8_t name[64];
extern const unsigned char student_number[160];
//温湿度
extern const unsigned char gImage_tem_hum[134];
extern const unsigned char gImage_222[128];
extern const unsigned char gImage_personal_information[134];
extern const unsigned char gImage_counter[134];
extern const unsigned char gImage_clock[134];
extern const unsigned char gImage_switch01[32];
extern const unsigned char gImage_switch02[32];
//ASCII码
extern const uint8_t ascii_8x16[96][16];
//负号
extern const uint8_t minus_sign_8x16[16];

extern const unsigned char bitmap_years[32];
extern const unsigned char bitmap_mouths[32];
extern const unsigned char bitmap_days[32];
//星期
extern const uint8_t week_16x16[7][96];
#endif