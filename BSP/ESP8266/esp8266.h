#ifndef __ESP8266_H__
#define __ESP8266_H__

#include "stm32f10x.h"
#include <stdio.h>
#include <string.h>
#include "bsp_ili9341_lcd.h"
#include "delay.h"


#define USART2_BAUD 9600
#define USART_REC_LEN  1024 	//定义最大接收字节数 1024

typedef struct{
	u8 temp_id[24];
	u8 temp_value[24];
	u8 humi_id[24];
	u8 humi_value[24];
	u8 MQ2_id[24];
	u8 MQ2_value[24];     
}DEV_Data;    //为每一个传感器设置ID与value是为了一次上传多个数据


typedef struct{
	u8 device_id[20];
	u8 api_key[32];
	u8 dev_status;        // 2:已连接热点  3:已建立tcp连接  4:已连接热点，但未与服务器建立连接  5:未连接热点
	DEV_Data data;
}DEV;

extern u8  USART_RX_BUF[USART_REC_LEN]; 
extern u16 USART_RX_STA;         		//接收状态标记	


int Esp8266_Init(void);
int Wifi_Config(void);
void Uart2_Config(void);
void Dev_StatusHandler(void);
int Upload_DataPoint(u8 *id ,u8 *value);
void Esp8266_SendData(u8 *content, u8 len);

#endif
