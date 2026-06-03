#ifndef __COUNTER_H
#define __COUNTER_H

#include <stdint.h>

/* 状态枚举 */
typedef enum {
    STATE_STOP  = 0,
    STATE_RUN   = 1,
    STATE_PAUSE = 2
} CounterState_t;

/* 全局变量声明（保持原名称） */
extern volatile CounterState_t counter_state;
extern volatile uint8_t pc1_pressed;
extern volatile uint8_t pc2_pressed;

/* 函数声明 */
void Counter_Init(void);      /* 初始化状态机（时间归零，触发首次显示） */
void state_counter(void);   /* 状态机主处理（应在主循环中调用） */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);

#endif /* __COUNTER_H */