#ifndef _ENCODER_H
#define _ENCODER_H

#include <stdint.h>
#include <stdbool.h>
typedef enum {
    MENU_SELECT = 0,
    FUNC_RUN
} AppState_t;

extern volatile AppState_t app_state;
extern volatile uint8_t current_func;   // 当前进入的功能号(0~4)


void Init(void);
void Show(uint8_t selectdata);
void MENU_show(void);
void GPIO_EXTI(uint16_t GPIO_Pin);

// 若用户需要自行实现 EnterFunction，请定义；否则使用内置空函数
void EnterFunction(uint8_t func_num);

#endif
