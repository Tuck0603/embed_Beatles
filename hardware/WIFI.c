#include "main.h"
#include "usart.h"
#include "gpio.h"
#include <stdio.h>
#include <string.h>   // 注意不是 "string.h"，但双引号也能找到
#include "WIFI.h"
#include <stdlib.h>   // 提供 atol() 函数声明
#include "sh1106oled.h"  //下面的函数有调用显示屏文件里的函数
#include "sh1106oledfont.h"   
static volatile uint8_t rx_part; //内部使用static，且中断访问volatile
static volatile uint8_t rx[256];//同上
volatile uint8_t index =0;
static volatile bool is_ready =false;
static bool ntp_synced = false;
volatile uint8_t NPThour =0;
volatile uint8_t NPTmin =0;
volatile uint8_t NPTsec =0;
volatile uint16_t NPTyears =0;
volatile uint8_t NPTmouths =0;
volatile uint8_t NPTdays =0;
volatile uint8_t NPTweek =0;
volatile uint32_t ntp_utc_timestamp=0;
volatile uint8_t Flag =0;
//
static void WIFI_SendReceive_cmd(uint8_t *ATdata,uint8_t *ACK);
static void WIFI_SendReceive_submmit(uint8_t *ATdata,uint8_t *ACK);
static int month_to_num(const char *mon) ;
static int wday_to_num(const char *wday);
//指令处理
void WIFI_SendReceive_cmd(uint8_t *ATdata,uint8_t *ACK)
{
	printf("ATdata=%s\n",ATdata);
	uint16_t time=2000;
	HAL_UART_Transmit(&huart4,ATdata,strlen((char *)ATdata),0xff);
	while(time--){
		if(is_ready){
			if(strstr((char *)rx,(char *)ACK) != NULL){
			printf("OK\n");
			break;
			}
		}
	HAL_Delay(1);
	}
	is_ready =false;
	index =0;  //将下标置零，为下次完整数据做准备
	memset((void*)rx,0,sizeof(rx));//将数据清空0，为下次接收字节做准备
}

void WIFI_SendReceive_submmit(uint8_t *ATsubmmit,uint8_t *ACK)
{
	printf("ATsubmmit=%s\n",ATsubmmit);
	uint8_t timesubmmit=20;
	HAL_UART_Transmit(&huart4,ATsubmmit,strlen((char *)ATsubmmit),0xff);
	while(timesubmmit--){
		if(is_ready){
			if(strstr((char *)rx,(char *)ACK) != NULL){
			printf("OK\n");
			break;
			}
		}
	HAL_Delay(10);
	}
	is_ready =false;
	index =0;  //将下标置零，为下次完整数据做准备
	memset((void*)rx,0,sizeof(rx));//将数据清空0，为下次接收字节做准备
}
//发送初始化指令
void WIFI_Init(void)
{
	HAL_UART_Receive_IT(&huart4, (uint8_t *)&rx_part,1);

	WIFI_SendReceive_cmd((uint8_t *)AT,(uint8_t *)ACK_OK);
	WIFI_SendReceive_cmd((uint8_t *)AT_WIFI_MODE,(uint8_t *)ACK_OK);
	WIFI_SendReceive_cmd((uint8_t *)AT_SET_PSW,(uint8_t *)ACK_OK);
	WIFI_SendReceive_cmd((uint8_t *)AT_SET_MQTT_CFG,(uint8_t *)ACK_OK);
	WIFI_SendReceive_cmd((uint8_t *)AT_SET_MQTT_CLENTID,(uint8_t *)ACK_OK);
	WIFI_SendReceive_cmd((uint8_t *)AT_SET_MQTT_CONN,(uint8_t *)ACK_OK);
	WIFI_SendReceive_cmd((uint8_t *)AT_MQTT_SUB_PROP,(uint8_t *)ACK_OK);
	//发送指令，发送温度传感器数字的,AT_MQTT_PUB是你定义的数据数组名
	//WIFI_SendReceive_cmd((uint8_t *)AT_MQTT_PUB,(uint8_t *)ACK_OK);
	//WIFI_SendReceive_cmd((uint8_t *)AT_MQTT_PUB_HUM,(uint8_t *)ACK_OK);
}

//发送温湿度指令给物联网平台
void wifi_send_tem_hum(float tem, float hum)
{
    char cmd[256];
    sprintf(cmd, AT_MQTT_PUB_TEM_HUM, tem, hum);
    WIFI_SendReceive_submmit((uint8_t *)cmd, (uint8_t *)ACK_OK);
}

//WiFi串口中断
void WIFI_Callback()
{
		rx[index] =rx_part;
		index ++;
		if(rx_part =='\n')
		{
			is_ready=true;
		}
		HAL_UART_Receive_IT(&huart4, (uint8_t *)&rx_part,1);

}


//转化逻辑
// 月份缩写转数字，失败返回 -1
int month_to_num(const char *mon) {
    //读取字符，确定下标
    static const char *months[] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };
    char buf[4];
    strncpy(buf, mon, 3);
    buf[3] = '\0';
    for (int i = 0; i < 12; i++) {
        if (strcmp(buf, months[i]) == 0)
            return i + 1;
    }
    return -1;
}

// 星期缩写转中文
int wday_to_num(const char *wday) {
    static const char *wdays[] = {
         "Mon", "Tue", "Wed", "Thu", "Fri", "Sat","Sun"
    };
    char buf[4];
    strncpy(buf, wday, 3);
    buf[3] = '\0';
    for (int i = 0; i < 7; i++) {
        if (strcmp(buf, wdays[i]) == 0)
        return i;
    }
	return -1;
}


//直接从网络获取到具体的时分秒，不是一长串时间戳数字，直接赋值显示就行了
int Get_NPT(uint32_t *timestamp)
{
	uint8_t ntp_buf[256];
	uint32_t timeout = 2000; // 2000ms
	WIFI_SendReceive_cmd((uint8_t *)AT_CIPSNTPCFG, (uint8_t *)ACK_OK);
	HAL_Delay(3000);  
	HAL_UART_Transmit(&huart4, (uint8_t *)AT_CIPSNTPTIME, 
                      strlen(AT_CIPSNTPTIME), 0xffff);
	printf("ATdata=%s\n",AT_CIPSNTPTIME);

    while (timeout--) {
        if (is_ready && strstr((char*)rx, "+CIPSNTPTIME:") != NULL) {
            HAL_Delay(50);  // 50ms 足够模块发完剩余字符
            break;
        }
        HAL_Delay(1);
    }

    // 超时仍没收到响应，返回失败
    if (timeout == 0) {
        printf("NTP timeout\r\n");
        is_ready = false;
        index = 0;
        memset((void*)rx, 0, sizeof(rx));
        return 0;
    }
    else{
    // 此时 rx 中已经存有类似 "+CIPSNTPTIME:1716123456" 的字符串
    // 在清空前，把有效内容复制到 ntp_buf 保护起来
    uint8_t len = index;            // 当前接收到的字符数
    if (len > sizeof(ntp_buf)-1) len = sizeof(ntp_buf)-1;
    memcpy((void*)ntp_buf, (const void*)rx, len);
    ntp_buf[len] = '\0';            // 加上字符串结束符
	
	printf("Copy has been completed!!!\n");
	is_ready = false;
    index = 0;
    memset((void*)rx, 0, sizeof(rx));
	}
	printf("ntp_buf: [%s]\r\n", ntp_buf);
	char *start = strstr((char*)ntp_buf, "+CIPSNTPTIME:");
    if (start == NULL) {
        return 0;
    }
	printf("detect CIPSNTPTIME!!!\n");
    start += strlen("+CIPSNTPTIME:");
    while (*start == ' ') start++;  // 跳过可能存在的空格
	printf("ntp_buf: [%s]\r\n", start);

    //*timestamp = atol(start);       // 字符串转长整数（秒级时间戳）
	char wday[4],month[4];
    int hour, min, sec, year,day,week,num_month;
    if (sscanf(start, "%3s %3s %d %d:%d:%d %d",
               wday, month, &day, &hour, &min, &sec, &year) == 7) {

        // 检查年份是否合理（防止 1970）
        num_month = month_to_num(month);
        week = wday_to_num(wday);
        if (num_month == -1 || year < 2025) {
            printf("Invalid date: %s %s %d %d:%d:%d %d\r\n", wday, month, day, hour, min, sec, year);
        return 0;
        }

        // 直接设置北京时间（因为配置了时区 8，返回的就是东八区时间）
        NPThour = hour;
        NPTmin  = min;
        NPTsec  = sec;
        NPTyears  = year;
        NPTmouths  = num_month;
        NPTdays  = day;
        NPTweek  = week;
	}
    printf("NTP time set: %04d:%02d:%02d\r\n", NPTyears, NPTmouths, NPTdays);
	printf("NTP time set: %02d:%02d:%02d\r\n", NPThour, NPTmin, NPTsec);
    return 1;
}
//自增逻辑没问题，因为现在显示的具体数字，这种进位可行

void NPT_Increment(void)
{
    NPTsec++;
    if (NPTsec >= 60) {
        NPTsec = 0;
        NPTmin++;
        if (NPTmin >= 60) {
            NPTmin = 0;
            NPThour++;
            if (NPThour >= 24) {
                NPThour = 0;
            }
        }
    }
    Flag = 1;
}

