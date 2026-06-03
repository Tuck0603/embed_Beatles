#ifndef __WIFI_H
#define __WIFI_H
#include <stdint.h>

#include <stdbool.h>
extern volatile uint8_t index;
extern volatile uint8_t NPThour;
extern volatile uint8_t NPTmin;
extern volatile uint8_t NPTsec;
extern volatile uint16_t NPTyears;
extern volatile uint8_t NPTmouths;
extern volatile uint8_t NPTdays;
extern volatile uint8_t NPTweek;
extern volatile uint32_t ntp_utc_timestamp;
extern volatile uint8_t Flag;
//
void WIFI_Init(void);

void WIFI_Callback();
void receive_internet();
int Get_NPT(uint32_t *timestamp);
void transform_time(uint32_t base_time);
void wifi_send_tem_hum(float tem,float hum);
void NPT_Increment(void);

//通信验证
#define AT "AT\r\n"  
//设置模式
#define AT_WIFI_MODE "AT+CWMODE=3\r\n"
//设置状态
#define AT_SET_PSW "AT+CWJAP=\"anything\",\"onetwothree\"\r\n"
//设置MOOT协议
#define AT_SET_MQTT_CFG "AT+MQTTUSERCFG=0,1,\"NULL\",\"69e484a97f2e6c302f6893e3_d_1_1\",\"4fe3f9cd879e95357e31d0d6c0b83847a5733bd1d5dbd6f3b35cb193dafe1801\",0,0,\"\"\r\n"
//设置MOOT的CLENTID 
#define AT_SET_MQTT_CLENTID "AT+MQTTCLIENTID=0,\"69e484a97f2e6c302f6893e3_d_1_1_0_0_2026042111\"\r\n"
//设置
#define AT_SET_MQTT_CONN "AT+MQTTCONN=0,\"aefc4ce455.st1.iotda-device.cn-north-4.myhuaweicloud.com\",1883,1\r\n"
//
#define AT_MQTT_SUB_PROP "AT+MQTTSUB=0,\"$oc/devices/69e484a97f2e6c302f6893e3_d_1_1/sys/properties/report\",1\r\n"
//
#define AT_MQTT_PUB_TEM "AT+MQTTPUB=0,\"$oc/devices/69e484a97f2e6c302f6893e3_d_1_1/sys/properties/report\",\"{\\\"services\\\":[{\\\"service_id\\\":\\\"a1\\\"\\,\\\"properties\\\":{\\\"tem\\\":%.1f}}]}\",1,0\r\n"
#define AT_MQTT_PUB_HUM "AT+MQTTPUB=0,\"$oc/devices/69e484a97f2e6c302f6893e3_d_1_1/sys/properties/report\",\"{\\\"services\\\":[{\\\"service_id\\\":\\\"a1\\\"\\,\\\"properties\\\":{\\\"hum\\\":%.1f}}]}\",1,0\r\n"
#define AT_MQTT_PUB_TEM_HUM "AT+MQTTPUB=0,\"$oc/devices/69e484a97f2e6c302f6893e3_d_1_1/sys/properties/report\",\"{\\\"services\\\":[{\\\"service_id\\\":\\\"a1\\\"\\,\\\"properties\\\":{\\\"tem\\\":%.1f\\,\\\"hum\\\":%.1f}}]}\",1,0\r\n"
//返回值
#define ACK_OK "OK"
// 配置SNTP，启用并设置时区为东八区（中国）
#define AT_CIPSNTPCFG "AT+CIPSNTPCFG=1,8,\"ntp.aliyun.com\"\r\n"
// 查询当前网络时间
#define AT_CIPSNTPTIME "AT+CIPSNTPTIME?\r\n"
#endif