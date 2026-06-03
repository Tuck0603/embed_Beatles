#include "main.h"
#include "sh1106oled.h"   // OLED显示函数/变量
#include "sh1106oledfont.h" 
#include <stdio.h>
#include <stdbool.h>
#include "Encoder.h"


// ---------- 静态全局变量（封装） ----------
volatile AppState_t app_state = MENU_SELECT;
volatile uint8_t current_func = 0;   // 当前进入的功能号(0~4)
static volatile bool key_pressed = false;   // 按键按下标志(主循环处理)

static int8_t count = 0;                    // 有符号，支持边界回绕
static volatile bool is_ccw_start = false;
static volatile bool is_ccw_end = false;
static volatile bool is_cw_start = false;
static volatile bool is_cw_end = false;

// 按键消抖时间戳（单位 ms）
static volatile uint32_t key_press_time = 0;

// ---------- 内部函数声明 ----------
static void set_ccw_status(void);
static void set_cw_status(void);

// ---------- 弱函数实现（避免未定义错误） ----------
__weak void EnterFunction(uint8_t func_num) {
    (void)func_num;   // 空实现，用户可覆盖
}
__weak void User_EXTI_Callback(uint16_t GPIO_Pin) {
    (void)GPIO_Pin;
}

// ---------- 初始化 ----------
void Init(void) {
    count = 0;
    is_ccw_start = false;
    is_ccw_end   = false;
    is_cw_start  = false;
    is_cw_end    = false;
    key_pressed  = false;
    key_press_time = 0;
    app_state = MENU_SELECT;
    Show(count);
}

// ---------- 菜单显示（直接调用 OLED 绘图） ----------
void Show(uint8_t selectdata) {
    //后期切换成这个，因为左右两边会留切换键（菜单界面）
    sh1106_fill_rect(46, 16, 32, 32, 0);

    sh1106_draw_bitmap(108, 24, gImage_switch01, 16, 16);
    sh1106_draw_bitmap(2, 24, gImage_switch02, 16, 16);
    switch(selectdata) {
        case 0: sh1106_draw_bitmap(46, 16, gImage_personal_information, 32, 32); break;
        case 1: sh1106_draw_bitmap(46, 16, gImage_222,     32, 32); break;
        case 2: sh1106_draw_bitmap(46, 16, gImage_counter, 32, 32); break;
        case 3: sh1106_draw_bitmap(46, 16, gImage_clock,   32, 32); break;
        default: break;
    }
    sh1106_refresh_gram();
}

// ---------- 主循环调用的处理函数 ----------
void MENU_show(void) {
    // 1. 处理旋转（仅在菜单选择状态下有效）
    if (app_state == MENU_SELECT) {
        if (is_ccw_end) {
            is_ccw_end = false;
            count++;
            if (count > 3) count = 0;
            Show(count);
        }
        if (is_cw_end) {
            is_cw_end = false;
            count--;
            if (count < 0) count = 3;
            Show(count);
        }
    }

    // 2. 按键消抖处理（主循环中判断）
    if (key_press_time != 0) {
        if (HAL_GetTick() - key_press_time > 20) {
            key_pressed = true;
            key_press_time = 0;      // 防止重复触发
        }
    }

    // 3. 按键事件处理
    if (key_pressed) {
        key_pressed = false;          // 清除标志

        if (app_state == MENU_SELECT) {
            current_func = (uint8_t)count;
            app_state = FUNC_RUN;
            EnterFunction(current_func); //子程序对应的函数
        }
        else if (app_state == FUNC_RUN) {
            app_state = MENU_SELECT;
            Show(count);              // 返回菜单并刷新当前图标
        }
    }
}

// ---------- 编码器方向标志位设置 ----------
void set_ccw_status(void) {
    if (is_ccw_start) {
        is_ccw_end = true;    // 第二次触发为有效信号
    } else {
        is_ccw_start = true;  // 首次触发为开始
    }
}

void set_cw_status(void) {
    if (is_cw_start) {
        is_cw_end = true;
    } else {
        is_cw_start = true;
    }
}

// ---------- 中断回调（仅做最快速记录） ----------
void GPIO_EXTI(uint16_t GPIO_Pin) {
    // 编码器 A 相处理
    if (GPIO_Pin == A1_Pin) {
        if (HAL_GPIO_ReadPin(A1_GPIO_Port, A1_Pin) == 0) {
            if (HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == 0) {
                set_ccw_status();
                is_cw_start = false;
            } else {
                set_cw_status();
                is_ccw_start = false;
            }
        } else {
            if (HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == 0) {
                set_cw_status();
                is_ccw_start = false;
            } else {
                set_ccw_status();
                is_cw_start = false;
            }
        }
    }

  
    // 按键中断触发
    if (GPIO_Pin == KEY_Pin) {
        if (HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin) == GPIO_PIN_RESET) {
            key_press_time = HAL_GetTick();
        }
    }
    // 最后调用用户钩子,为什么
    User_EXTI_Callback(GPIO_Pin);
}