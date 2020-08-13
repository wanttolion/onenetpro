#include "stm32f10x.h"   
#include "include.h"

#define API_KEY "Ry1jUAGuOMbzTfcClxmDpFy3B=E="
#define DEV_ID  "608464608"

extern DEV dev;

static u32 Send_Succ = 0;
static u32 Send_Fail = 0;

int main(void)
{
	
	u8 mq2_sec = 0, temp_sec = 0, wifi_sec = 0;
	u8 temp = 0, humi = 0;
	float ppm = 0.0;
	u8 display[64] = "\0";
	int ret = 0, upload_state = 0;
    //外设初始化
	//延时函数初始化
	DelayInit();
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    //显示屏模块初始化
    ILI9341_Init();
	LCD_SetFont(&Font16x24);
    LCD_SetColors(RED,BLACK);
    ILI9341_Clear(0,0,LCD_X_LENGTH,LCD_Y_LENGTH);	/* 清屏，显示全黑 */
	LCD_SetTextColor(RED);
	ILI9341_DispStringLine_EN(LINE(0), "  Wifi Test");
    LCD_SetFont(&Font8x16);
	
	//串口初始化    //调试用
    USART_Config();
    //DHT11初始化
	ret = DHT11_Init();
	if(ret == 1){
		printf("dht11 error\n");
	}
	//LED初始化
	Led_Init();
	//KEY初始化
	Key_Init();
    //MQ2传感器初始化
	Mq2_Init();
    //wifi模块初始化,初始化时每一步都需要判断
	ret = Esp8266_Init();
	if(ret == -1){
			printf("wifi config error, reset\n");
	}
	
	strcpy((char *)dev.api_key, API_KEY);
	strcpy((char *)dev.device_id, DEV_ID);
	
    while(1) {
        //传感器定时接收数据
		//MQ2每隔5秒上传一次数据
		//设置界面提示信息

		//需判断是否与服务器断开连接，应当需要一个定时器中断，去定时检测wifi状态,返回一个wifi状态标志;
		//现用循环代替
		if(wifi_sec % 5 == 0){
			Dev_StatusHandler();
			if(dev.dev_status != 3){
				printf("wifi error\n");
			}
		}
		
		printf("start send data\n");
		if((mq2_sec == 5 && dev.dev_status == 3 ) || (MQ2_SEND_STA == 1 && dev.dev_status == 3) ){
			ppm = Get_PPMValue();
			sprintf((char *)dev.data.MQ2_id, "MQ2_ppm");
			sprintf((char *)dev.data.MQ2_value, "%.2f", ppm);
			sprintf((char *)display, "     MQ2: %.2f ppm", ppm);
			
			LCD_ClearLine(LINE(3));
			ILI9341_DispStringLine_EN(LINE(3), (char *)display);
			//数据封装
			upload_state = Upload_DataPoint(dev.data.MQ2_id, dev.data.MQ2_value);
			if(upload_state == 0){
				Send_Succ++;
				printf("upload succ: %d\n", Send_Succ);
			}else{
				printf("upload fail: %d\n", Send_Fail);
			}
			
			memset(display, 0, sizeof(display));
			memset(&dev.data, 0, sizeof(dev.data));
			MQ2_SEND_STA = 0;
			LED_MQ2_CLOSE;
			mq2_sec = 0;
		}
		
/*		//DHT11每隔12秒上传一次数据
		if((temp_sec == 12 && dev.dev_status == 3)|| (DHT11_SEND_STA == 1 && dev.dev_status == 3)){
			DHT11_ReadData(&temp, &humi);
			sprintf((char *)dev.data.temp_id, "Temp");			
			sprintf((char *)display, "     Temp: %s T", &temp);
			
			LCD_ClearLine(LINE(5));
			ILI9341_DispStringLine_EN(LINE(4), (char *)display);
			upload_state = Upload_DataPoint(dev.data.temp_id, &temp);
			if(upload_state == 0){
				printf("upload succ: %d\n", Send_Succ);
			}else{
				printf("upload fail: %d\n", Send_Fail);
			}
			
			memset(display, 0, sizeof(display));
			memset(&dev.data, 0, sizeof(dev.data));
			
			sprintf((char *)dev.data.humi_id, "Humi");
			sprintf((char *)display, "     Humi: %s H", &humi);
			
			LCD_ClearLine(LINE(7));
			ILI9341_DispStringLine_EN(LINE(5), (char *)display);
			upload_state = Upload_DataPoint(dev.data.humi_id, &humi);
			if(upload_state == 0){
				printf("upload succ: %d\n", Send_Succ);
			}else{
				printf("upload fail: %d\n", Send_Fail);
			}
			memset(display, 0, sizeof(display));
			memset(&dev.data, 0, sizeof(dev.data));
			DHT11_SEND_STA = 0;
			LED_DHT11_CLOSE;
			temp_sec = 0;
		}
*/	
		
		
		mq2_sec++;
		temp_sec++;
		wifi_sec++;
		
		DelayMs(1000);
    }
	
}


