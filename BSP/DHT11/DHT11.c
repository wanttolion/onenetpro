#include "dht11.h"

void Send_Start(void)
{
	DHT11_IO_OUT();
	DHT11_OUTPUT_L();
	DelayMs(20);
	DHT11_OUTPUT_H();
	DelayUs(30);
}

u8 Recv_Ack(void)
{
	int i = 0;
	DHT11_IO_IN();
	while(DHT11_IN_STATE()&& (i < 100)){
		i++;
		DelayUs(1);
	}
	
	if(i >= 100){
		return 1;
	}else{
		i = 0;
	}
	
	while(!DHT11_IN_STATE() && (i < 100)){
		i++;
		DelayUs(1);
	}
	if(i >= 100){
		return 1;
	}
	
	return 0;
}

u8 DHT11_Read_Bit(void)
{
	int i = 0;
	while(DHT11_IN_STATE() && i < 100){  //等待低电平 i作用为超时检测
		i++;
		DelayUs(1);
	}
	
	i = 0;
	while(!DHT11_IN_STATE() && i < 100){  //等待高电平 
		i++;
		DelayUs(1);
	}
	i = 0;
	
	while(DHT11_IN_STATE())    //读取高电平时间
	{
		i++;
		DelayUs(1);
	}
	if(i > 40){
		return 1;
	}else{
		return 0;
	}
	
}

u8 DHT11_ReadByte(void)
{
	u8 data = 0, i = 0, bit = 0;
	for(i = 0; i < 8; i++){
		bit = DHT11_Read_Bit();
		data |= (bit << i);
	}
	
	return data;
}

u8 DHT11_ReadData(u8 *temp, u8 *humi)
{
	u8 Data[5] = "\0";
	u8 i = 0;
	Send_Start();
	Recv_Ack();
	for(i = 0; i < 5; i++){
		Data[i] = DHT11_ReadByte();
	}
	if((Data[0] + Data[1] + Data[2] + Data[3]) != Data[4]){
		return 1;
	}
	
	*temp = Data[2];
	*humi = Data[0];
	
	return 0;
}

u8 DHT11_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_0;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStruct);
	
	DHT11_OUTPUT_H();
    Send_Start();
	return Recv_Ack(); 
}

