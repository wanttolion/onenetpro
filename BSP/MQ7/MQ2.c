#include "MQ2.h"

#define CAL_PPM 	20  	// 校准环境中的ppm
#define RL			5		// RL阻值
static float R0 = 6.00; 	// 元件在洁净空气中的阻值

static u16 ADCConvertedValue;

//采用ADC+DMA方式采集数据
void Mq2_Init(void)
{
	//定义结构体
	GPIO_InitTypeDef GPIO_Struct;
	ADC_InitTypeDef ADC_Struct;
	DMA_InitTypeDef DMA_Struct;
	
	//初始化结构体
	GPIO_StructInit(&GPIO_Struct);
	ADC_StructInit(&ADC_Struct);
	DMA_StructInit(&DMA_Struct);
	
	//使能时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	RCC_ADCCLKConfig(RCC_PCLK2_Div8);
	//引脚配置 PA4
	GPIO_Struct.GPIO_Mode  = GPIO_Mode_AIN;
	GPIO_Struct.GPIO_Pin   = GPIO_Pin_1;
	GPIO_Struct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_Struct);
	
	//DMA配置
	DMA_Struct.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_Struct.DMA_PeripheralBaseAddr = (u32)&ADC1->DR;                  //外设地址
	DMA_Struct.DMA_MemoryBaseAddr = (u32)&ADCConvertedValue;             //内存地址
	DMA_Struct.DMA_BufferSize = sizeof(ADCConvertedValue);               //内存大小
	DMA_Struct.DMA_M2M = DMA_M2M_Disable;                                //关闭内存到内存传输
	DMA_Struct.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;         //外设数据大小为16位
	DMA_Struct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; //内存数据大小为16位
	DMA_Struct.DMA_MemoryInc = DISABLE;                                  //禁止内存地址递增 
	DMA_Struct.DMA_PeripheralInc = DISABLE;                              //禁止外设地址递增
	DMA_Struct.DMA_Mode = DMA_Mode_Circular;                              //DMA循环接收
	DMA_Struct.DMA_Priority = DMA_Priority_High;                          //DMA优先级高
	DMA_Init(DMA1_Channel1, &DMA_Struct);                                                    
	
	DMA_Cmd(DMA1_Channel1, ENABLE);
	
	//ADC配置
	ADC_Struct.ADC_Mode = ADC_Mode_Independent;                               //独立工作模式
	ADC_Struct.ADC_ContinuousConvMode = ENABLE;                               // 连续转换使能
	ADC_Struct.ADC_DataAlign = ADC_DataAlign_Left;                            //左对齐
	ADC_Struct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;              //由软件触发转换
	ADC_Struct.ADC_NbrOfChannel = 1;                                          //通道数量
	ADC_Struct.ADC_ScanConvMode = DISABLE;                                    //扫描不需要，单通道
	ADC_Init(ADC1, &ADC_Struct);
	
	ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 1, ADC_SampleTime_55Cycles5);
	ADC_DMACmd(ADC1, ENABLE);
	ADC_Cmd(ADC1, ENABLE);

	//貌似不需要校准
	ADC_ResetCalibration(ADC1);	//使能复位校准  
	 
	while(ADC_GetResetCalibrationStatus(ADC1));	//等待复位校准结束
	
	ADC_StartCalibration(ADC1);	 //开启AD校准
 
	while(ADC_GetCalibrationStatus(ADC1));	 //等待校准结束
	
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

u16 GetAdcValue()
{
	return ADCConvertedValue;
}

u16 GetAdcValue_Average(void)
{
	u16 sum;
	u8 i;
	//取10次值，求平均值
	for(i = 0; i < 10; i++){
		sum += GetAdcValue();
	}
	
	return sum/10;
}

float Get_PPMValue(void)
{
	u16 val;
	float Vrl, RS, ppm;
	
	val = GetAdcValue_Average();
	Vrl = 5.0f * val/255;
	RS = (5.0f - Vrl)/Vrl * RL;
	R0 = RS/pow(CAL_PPM/613.9f, 1/(-2.074f));
	
	ppm = 613.9f * pow(RS/R0, -2.074f);
	
	return ppm;
}
