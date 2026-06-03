#include "main.h"
#include "spi.h"
#include "tim.h"
#include "gpio.h"
#include "main.h" 
#include "sh1106oled.h"
#include "sh1106oledfont.h"

typedef enum {
    STATE_STOP = 0,
    STATE_RUN =1,
    STATE_PAUSE =2
} CounterState_t;


volatile CounterState_t counter_state = STATE_STOP;  // 初始停止状态
volatile uint8_t pc1_pressed = 0;                    // PC1 按键标志
volatile uint8_t pc2_pressed = 0;                    // PC2 按键标志

void Counter_Init(void)
{
    counter_state = STATE_STOP;
    pc1_pressed = 0;
    pc2_pressed = 0;
    CLOCK_Init();
    Flag_count = 1;   /* 立即显示 00:00:00 */
}

void state_counter(void)
{
// ----- 状态机处理（无延时，每次循环都检查）-----
        // 1. 复位按键 PC2（优先级最高，任意状态均可复位）
        if (pc2_pressed) {
            pc2_pressed = 0;
            counter_state = STATE_STOP;
            CLOCK_Init();               // 时间归零
            Flag_count= 1;                   // 立即刷新显示 00:00:00
        }

        // 2. 启动/暂停按键 PC1
        if (pc1_pressed) {
            pc1_pressed = 0;
            switch (counter_state) {
                case STATE_STOP:
                    counter_state = STATE_RUN;   // 停止 → 运行
                    // 时间已为 00:00:00，不需要额外操作
                    Flag_count = 1;                    // 刷新一次显示（可选）
                    break;
                case STATE_RUN:
                    counter_state = STATE_PAUSE; // 运行 → 暂停
                    // 保持当前显示不变
                    break;
                case STATE_PAUSE:
                    counter_state = STATE_RUN;   // 暂停 → 继续运行
                    break;
                default:
                    break;
            }
        }

        // 3. 显示刷新（仅当 Flag 为 1 时更新 OLED）
        if (Flag_count) {
            Flag_count = 0;
            OLED_ShowTime(clock_hour, clock_min, clock_sec);
        }

        HAL_Delay(1);   // 降低 CPU 占用，1ms 足够响应按键

}



