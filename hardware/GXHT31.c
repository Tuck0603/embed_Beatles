#include "main.h"
#include "i2c.h"
#include "gpio.h"
#include "GXHT31.h"
#include "sh1106oled.h"
#include "sh1106oledfont.h"
#include "stm32f1xx_hal.h" /* 根据实际芯片调整 */



static float gxht31_temperature = 0.0;
static float gxht31_humidity    = 0.0;

/* 静态函数声明 */
static uint8_t CheckCrc8(uint8_t* const message, uint8_t initial_value);
static uint8_t GXHT31_RawToFloat(uint8_t* const dat, float* temperature, float* humidity);

/**
  * @brief  传感器初始化（发送配置命令）
  */
void GXHT31_Init(I2C_HandleTypeDef *hi2c)
{
    //配置传感器测量周期以及重复率，以数组存储
    uint8_t init_cmd[2] = GXHT31_CMD_INIT;
    HAL_I2C_Master_Transmit(hi2c, GXHT31_ADDR_W, init_cmd, sizeof(init_cmd), HAL_MAX_DELAY);
}

/**
  * @brief  触发测量并读取温湿度数据（存入全局变量）
  */
HAL_StatusTypeDef GXHT31_Measure(I2C_HandleTypeDef *hi2c)
{
    //读取周期测量得到的温湿度数据需要发送专用的数据读取命令GXHT31_CMD_MEASURE
    uint8_t measure_cmd[2] = GXHT31_CMD_MEASURE;
    //配置数组大小为6的数组，分别储存温度高八位低八位，湿度高八位低八位，以及温度校验8位，湿度校验8位，共计6个字节
    //该传感器先传高八位，再传低八位，再传校验码
    uint8_t raw_data[6] = {0};
    HAL_StatusTypeDef status;

    /* 发送测量命令 */
    status = HAL_I2C_Master_Transmit(hi2c, GXHT31_ADDR_W, measure_cmd, sizeof(measure_cmd), HAL_MAX_DELAY);
    /* 发送测量命令后等待转换完成 */
    HAL_Delay(20); 
    if (status != HAL_OK) return status;

    /* 读取6字节数据 */
    status = HAL_I2C_Master_Receive(hi2c,GXHT31_ADDR_R, raw_data, sizeof(raw_data), HAL_MAX_DELAY);
    if (status != HAL_OK) return status;

    /* 转换并存储到全局变量 */
    if (GXHT31_RawToFloat(raw_data, &gxht31_temperature, &gxht31_humidity) != 0)
        return HAL_ERROR;

    return HAL_OK;
}


// 获取最近一次测量的温度，保护数据，只能读不能写

float GXHT31_GetTemperature(void)
{
    return gxht31_temperature;
}


 //获取最近一次测量的湿度，保护数据，只能读不能写
float GXHT31_GetHumidity(void)
{
    return gxht31_humidity;
}


static uint8_t CheckCrc8(uint8_t* const message, uint8_t initial_value)
{
    uint8_t remainder = initial_value;
    uint8_t i, j;

    for (j = 0; j < 2; j++)
    {
        remainder ^= message[j];
        for (i = 0; i < 8; i++)
        {
            if (remainder & 0x80)
                remainder = (remainder << 1) ^ CRC8_POLYNOMIAL;
            else
                remainder = (remainder << 1);
        }
    }
    return remainder;
}


 // 原始数据转为温湿度浮点值

static uint8_t GXHT31_RawToFloat(uint8_t* const dat, float* temperature, float* humidity)
{
    uint16_t raw_temp, raw_humi;

    //校验温度数据，校验位是在01235中的25
    if (CheckCrc8(dat, 0xFF) != dat[2])
        return 1;

    /* 校验湿度数据 */
    if (CheckCrc8(&dat[3], 0xFF) != dat[5])
        return 1;

    //合并成16位数据，通过公式转换并存储到temperature指针对应的变量里
    raw_temp = ((uint16_t)dat[0] << 8) | dat[1];
    *temperature = -45.0f + 175.0f * ((float)raw_temp / 65535.0f);

    /* 湿度转换：H = 100 * (raw / 65535) */
    raw_humi = ((uint16_t)dat[3] << 8) | dat[4];
    *humidity = 100.0f * ((float)raw_humi / 65535.0f);

    return 0;
}

//温湿度刷新
void TEM_HUM_Process(void)
{   
    float temp = GXHT31_GetTemperature();
    float hump  = GXHT31_GetHumidity();
    static uint32_t last_display_tick = 0;
    if (HAL_GetTick() - last_display_tick >= 200) // 每 500ms 刷新一次
        {
        last_display_tick = HAL_GetTick();
        GXHT31_Measure(&hi2c2);
		OLED_ShowTEM_HUM(16, 24, temp,hump);
    }
}