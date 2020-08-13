#include "esp8266.h"

u8 USART_RX_BUF[USART_REC_LEN] = "\0";     //接收缓冲,最大USART_REC_LEN个字节.

u16 USART_RX_STA=0;       //接收状态标记

u8 AT[] = "AT\r\n";                                                                  //测试模块是否能正常通讯
u8 AT_CWMODE[] = "AT+CWMODE=1\r\n";                                                  //将wifi模块配置为AP+STA
u8 AT_RST[] = "AT+RST\r\n";                                                          //重启生效
u8 AT_CIFSR[] = "AT+CIFSR\r\n";                                                      //查询设备IP
u8 AT_CWJAP[] = "AT+CWJAP=\"xiaomi\",\"201201201\"\r\n";                             //连接一个热点 名称+密码
u8 AT_CWJAP_SELECT[] = "AT+CWJAP?\r\n";
u8 AT_CIPSTART[] = "AT+CIPSTART=\"TCP\",\"183.230.40.33\",80\r\n";                  //连接onenet IP地址与端口号
u8 AT_CIPMODE[] = "AT+CIPMODE=1\r\n";                                                //开启透传模式
u8 AT_CIPSEND[] = "AT+CIPSEND\r\n";                                                  //开始透传
u8 AT_CIPMUX[] = "AT+CIPMUX=0\r\n";                                                   //设置单链接
u8 AT_CWAUTOCONN[] = "AT+CWAUTOCONN=0\r\n";                                          //关闭开机自动连接wifi
u8 AT_Exit[] = "+++";                                                                //退出透传
u8 AT_CIPSTATUS[] = "AT+CIPSTATUS\r\n";                                              //查询wifi状态

DEV dev;

void Uart2_Config(void)
{
	//定义结构体
	GPIO_InitTypeDef GPIO_InitStruct;
	USART_InitTypeDef USART_InitStruct;
	NVIC_InitTypeDef NVIC_InitStruct;
	
	//开启外设时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

	//引脚配置
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	//串口参数配置
	USART_InitStruct.USART_BaudRate = USART2_BAUD;
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_InitStruct.USART_Parity = USART_Parity_No;
	USART_InitStruct.USART_StopBits = USART_StopBits_1;
	USART_InitStruct.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART2, &USART_InitStruct);
	
	
	//使能串口中断
	
	NVIC_InitStruct.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2;
	NVIC_Init(&NVIC_InitStruct);
	
	
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	USART_Cmd(USART2, ENABLE);
}

/*****************  发送一个字符 **********************/
void Usart_SendByte( USART_TypeDef * pUSARTx, uint8_t ch)
{
	/* 发送一个字节数据到USART */
	USART_SendData(pUSARTx,ch);
		
	/* 等待发送数据寄存器为空 */
	while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET);	
}

void Usart_SendString(USART_TypeDef * pUSARTx, u8 *str)
{
	unsigned int k=0;
    do 
    {
        Usart_SendByte( pUSARTx, *(str + k) );
        k++;
    } while(*(str + k)!='\0');
  
    /* 等待发送完成 */
    while(USART_GetFlagStatus(pUSARTx,USART_FLAG_TC)==RESET);
    
}

int Usart_RecvByte(USART_TypeDef *pUSARTx)
{
	int ch = 0;
	if(USART_GetFlagStatus(pUSARTx, USART_FLAG_RXNE) != RESET){
		ch = USART_ReceiveData(USART2);
		return ch;
	}
	
	return -1;
}

int Usart_RecvString(USART_TypeDef *pUSARTx, u8 *str, int LEN)
{
	int len = 0;
	int ch;
	while(len < LEN){
		ch = Usart_RecvByte(pUSARTx);
		if(ch != -1){
			str[len] = ch;
			len++;
		}else{
			break;
		}
	}
	
	if(len > 0){
		return len;
	}else{
		return -1;
	}
}




u8 Is_OK(void)
{
	u8 i = 0;
	u8 *str = USART_RX_BUF;
	
	for(i = 0; i < USART_RX_STA; i++){
		if(memcmp(str, "No", 2) == 0){
			return 2;
		}
		
		if(memcmp(str, "OK", 2) == 0){
			return 1;
		}
		
		
		str++;
	}
	return 0;
}


void Send_AT_Exit(void)
{
	Usart_SendString(USART2, AT_Exit);
	DelayMs(500);
	USART_RX_STA = 0;
	memset(USART_RX_BUF, 0, sizeof(USART_RX_BUF));		
}

int Send_AT_CWMODE(void)
{
	u8 i = 0;
	Usart_SendString(USART2, AT_CWMODE);
	DelayMs(100);
	while(i < 5){
		if(Is_OK()){
			printf("%s\n", USART_RX_BUF);
			USART_RX_STA = 0;
		    memset(USART_RX_BUF, 0, sizeof(USART_RX_BUF));
			break;
		}
		i++;
		DelayMs(100);
	}
	
	if(i >= 5){
		return -1;
	}
	
	return 0;
}

int Send_AT_RST(void)
{
	u8 i = 0;
	Usart_SendString(USART2, AT_RST);
	DelayMs(1000);
	while(i < 5){
		if(Is_OK()){
			printf("%s\n", USART_RX_BUF);
			USART_RX_STA = 0;
		    memset(USART_RX_BUF, 0, sizeof(USART_RX_BUF));
			break;
		}
		i++;
		DelayMs(1000);
	}
	
	if(i >= 5){
		return -1;
	}
	
	return 0;
}
int Send_AT_CWJAP_SELECT(void)
{
	u8 i = 0;
	Usart_SendString(USART2, AT_CWJAP_SELECT);
	DelayMs(1000);
	while(i < 5){
		if(Is_OK() == 1){
			printf("Ok:%s\n", USART_RX_BUF);
			USART_RX_STA = 0;
		    memset(USART_RX_BUF, 0, sizeof(USART_RX_BUF));
			break;
		}else if(Is_OK() == 2){
			printf("No:%s\n", USART_RX_BUF);
			USART_RX_STA = 0;
		    memset(USART_RX_BUF, 0, sizeof(USART_RX_BUF));
			return 2;
		}
		i++;
		DelayMs(1000);
	}
	
	if(i >= 5){
		return -1;
	}
	
	return 0;
}

int Send_AT_CWJAP(void)
{
	u8 i = 0;
	Usart_SendString(USART2, AT_CWJAP);
	DelayMs(5000);
	while(i < 5){
		
		if(Is_OK() == 1){
			printf("%s\n", USART_RX_BUF);
			USART_RX_STA = 0;
		    memset(USART_RX_BUF, 0, sizeof(USART_RX_BUF));
			break;
		}
		i++;
		DelayMs(1000);
	}
	
	if(i >= 5){
		return -1;
	}
	
	return 0;
}

int Send_AT_CIPMUX(void)
{
	u8 i = 0;
	Usart_SendString(USART2, AT_CIPMUX);
	DelayMs(100);
	while(i < 5){
		if(Is_OK()){
			printf("%s\n", USART_RX_BUF);
			USART_RX_STA = 0;
		    memset(USART_RX_BUF, 0, sizeof(USART_RX_BUF));
			break;
		}
		i++;
		DelayMs(100);
	}
	
	if(i >= 5){
		return -1;
	}
	
	return 0;
}

int Send_AT_CIPSTART(void)
{
	u8 i = 0;
	Usart_SendString(USART2, AT_CIPSTART);
	DelayMs(100);
	while(i < 5){
		if(Is_OK()){
			printf("%s\n", USART_RX_BUF);
			USART_RX_STA = 0;
		    memset(USART_RX_BUF, 0, sizeof(USART_RX_BUF));
			break;
		}
		i++;
		DelayMs(100);
	}
	
	if(i >= 5){
		return -1;
	}
	
	return 0;
}

int Send_AT_CIPMODE(void)
{
	u8 i = 0;
	Usart_SendString(USART2, AT_CIPMODE);
	DelayMs(500);
	while(i < 5){
		if(Is_OK()){
			printf("%s\n", USART_RX_BUF);
			USART_RX_STA = 0;
		    memset(USART_RX_BUF, 0, sizeof(USART_RX_BUF));
			break;
		}
		i++;
		DelayMs(100);
	}
	
	if(i >= 5){
		return -1;
	}
	
	return 0;
}

int Send_AT_CIPSEND(void)
{
	u8 i = 0;
	Usart_SendString(USART2, AT_CIPSEND);
	DelayMs(500);
	while(i < 5){
		if(Is_OK()){
			printf("%s\n", USART_RX_BUF);
			USART_RX_STA = 0;
		    memset(USART_RX_BUF, 0, sizeof(USART_RX_BUF));
			break;
		}
		i++;
		DelayMs(100);
	}
	
	if(i >= 5){
		return -1;
	}
	
	return 0;
}

u8 Check_Status(void)
{
	if(strstr((char *)USART_RX_BUF, "STATUS:2")){
		return 2;
	}else if(strstr((char *)USART_RX_BUF, "STATUS:3")){
		return 3;
	}else if(strstr((char *)USART_RX_BUF, "STATUS:4")){
		return 4;
	}else if(strstr((char *)USART_RX_BUF, "STATUS:5")){
		return 5;
	}
	return 0;
}

void Send_AT_CIPSTATUS(void)
{
	Usart_SendString(USART2, AT_CIPSTATUS);
	DelayMs(500);
	if(USART_RX_STA > 0){
		dev.dev_status = Check_Status();
		USART_RX_STA = 0;
	    memset(USART_RX_BUF, 0, sizeof(USART_RX_BUF));
	}
}

int Wifi_Config(void)
{
	u8 i = 0;
	int ret = 0;
	//尝试3次
	while(i < 3){
		i++;
		//退出透传
		Send_AT_Exit();
		
		//将wifi模块配置为AT+STA
		if(Send_AT_CWMODE() == -1){
			continue;
		}
	
		//重启生效
		if(Send_AT_RST() == -1){
			continue;
		}
		
		ret = Send_AT_CWJAP_SELECT();
		//连接一个热点 名称+密码  此处应当查询一下，是否连接上wifi  
		if(ret == 2){
			if(Send_AT_CWJAP() == -1){
				continue;
			}
		}else if(ret == -1){
			continue;
		}
		//设置单链接
		if(Send_AT_CIPMUX() == -1){
			continue;
		}	
	
		//连接onenet IP地址与端口号    此处需等待连接上onenet
		if(Send_AT_CIPSTART() == -1){
			continue;
		}
		
		break;
	}
	if(i >= 3){
		return -1;
	}
	
	dev.dev_status = 3;
	return 0;
}

int Analyse_Data(u8 *str, u8 len)
{
	u8 i = 0;
	for(i = 0; i < len; i++){
		if(memcmp(str, "succ", 4) == 0){
			return 0;
		}
		
		str++;
	}
	
	return -1;
}

int Upload_DataPoint(u8 *id ,u8 *value)
{
	u8 content[256] = "\0";
	u8 Send_Data[256] = "\0";
	int ret = 0;
	//数据打包为Json格式
	sprintf((char *)content, "{\"datastreams\": [{\"id\": \"%s\",\"datapoints\": [{\"value\": \"%s\"}]}]}", id, value);
	
	//封装为http数据包
	sprintf((char *)Send_Data, "POST /devices/%s/datapoints HTTP/1.1\r\napi-key:%s\r\nHost:api.heclouds.com\r\nContent-Length:%d\r\n\r\n%s\r\n", dev.device_id, dev.api_key, strlen((char *)content), content);
	Usart_SendString(USART2, Send_Data);
	printf("Send: %s\n", Send_Data);
	DelayMs(500);
	
	if(USART_RX_STA > 0){	
		printf("%s\n",USART_RX_BUF);
		//解析返回数据
		ret = Analyse_Data(USART_RX_BUF, USART_RX_STA);
		USART_RX_STA = 0;
		memset(USART_RX_BUF, 0, sizeof(USART_RX_BUF));
	}
	
	return ret;
}


int Esp8266_Init(void)
{
	//串口初始化
	Uart2_Config();
	//利用AT指令配置wifi模块   //后续完善时，会加上超时检测
	printf("start config wifi\n");
	if(Wifi_Config()== -1){
		return -1;
	};
	return 0;   //此返回值应当用来判断esp8266初始化是否成功
}

void Dev_StatusHandler(void)
{
	Send_AT_Exit();
	Send_AT_CIPSTATUS();
	switch(dev.dev_status){
		case 2:
			//连接服务器
			Send_AT_CIPSTART();
			break;
		case 3:break;
		case 4:
			//连接服务器
			Send_AT_CIPSTART();
			break;
		case 5:
			Wifi_Config();
			break;
	}
	
	//开启透传
	Send_AT_CIPMODE();
	Send_AT_CIPSEND();
	
}


