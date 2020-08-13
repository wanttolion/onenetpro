#include "MQ2.h"

#define CAL_PPM 	20  	// У׼�����е�ppm
#define RL			5		// RL��ֵ
static float R0 = 6.00; 	// Ԫ���ڽྻ�����е���ֵ

static u16 ADCConvertedValue;

//����ADC+DMA��ʽ�ɼ�����
void Mq2_Init(void)
{
	//����ṹ��
	GPIO_InitTypeDef GPIO_Struct;
	ADC_InitTypeDef ADC_Struct;
	DMA_InitTypeDef DMA_Struct;
	
	//��ʼ���ṹ��
	GPIO_StructInit(&GPIO_Struct);
	ADC_StructInit(&ADC_Struct);
	DMA_StructInit(&DMA_Struct);
	
	//ʹ��ʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	RCC_ADCCLKConfig(RCC_PCLK2_Div8);
	//�������� PA4
	GPIO_Struct.GPIO_Mode  = GPIO_Mode_AIN;
	GPIO_Struct.GPIO_Pin   = GPIO_Pin_1;
	GPIO_Struct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_Struct);
	
	//DMA����
	DMA_Struct.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_Struct.DMA_PeripheralBaseAddr = (u32)&ADC1->DR;                  //�����ַ
	DMA_Struct.DMA_MemoryBaseAddr = (u32)&ADCConvertedValue;             //�ڴ��ַ
	DMA_Struct.DMA_BufferSize = sizeof(ADCConvertedValue);               //�ڴ��С
	DMA_Struct.DMA_M2M = DMA_M2M_Disable;                                //�ر��ڴ浽�ڴ洫��
	DMA_Struct.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;         //�������ݴ�СΪ16λ
	DMA_Struct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; //�ڴ����ݴ�СΪ16λ
	DMA_Struct.DMA_MemoryInc = DISABLE;                                  //��ֹ�ڴ��ַ���� 
	DMA_Struct.DMA_PeripheralInc = DISABLE;                              //��ֹ�����ַ����
	DMA_Struct.DMA_Mode = DMA_Mode_Circular;                              //DMAѭ������
	DMA_Struct.DMA_Priority = DMA_Priority_High;                          //DMA���ȼ���
	DMA_Init(DMA1_Channel1, &DMA_Struct);                                                    
	
	DMA_Cmd(DMA1_Channel1, ENABLE);
	
	//ADC����
	ADC_Struct.ADC_Mode = ADC_Mode_Independent;                               //��������ģʽ
	ADC_Struct.ADC_ContinuousConvMode = ENABLE;                               // ����ת��ʹ��
	ADC_Struct.ADC_DataAlign = ADC_DataAlign_Left;                            //�����
	ADC_Struct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;              //���������ת��
	ADC_Struct.ADC_NbrOfChannel = 1;                                          //ͨ������
	ADC_Struct.ADC_ScanConvMode = DISABLE;                                    //ɨ�費��Ҫ����ͨ��
	ADC_Init(ADC1, &ADC_Struct);
	
	ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 1, ADC_SampleTime_55Cycles5);
	ADC_DMACmd(ADC1, ENABLE);
	ADC_Cmd(ADC1, ENABLE);

	//ò�Ʋ���ҪУ׼
	ADC_ResetCalibration(ADC1);	//ʹ�ܸ�λУ׼  
	 
	while(ADC_GetResetCalibrationStatus(ADC1));	//�ȴ���λУ׼����
	
	ADC_StartCalibration(ADC1);	 //����ADУ׼
 
	while(ADC_GetCalibrationStatus(ADC1));	 //�ȴ�У׼����
	
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
	//ȡ10��ֵ����ƽ��ֵ
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
