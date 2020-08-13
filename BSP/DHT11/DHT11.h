#ifndef __DHT11_H__
#define __DHT11_H__

#include "stm32f10x.h"
#include "delay.h"

#define DHT11_IO_IN()  {GPIOC->CRL &= 0xfffffff0; GPIOC->CRL |= 0x08;}
#define DHT11_IO_OUT() {GPIOC->CRL &= 0xfffffff0; GPIOC->CRL |= 0x03;}

#define DHT11_OUTPUT_H()   GPIO_SetBits(GPIOC, GPIO_Pin_0)
#define DHT11_OUTPUT_L()   GPIO_ResetBits(GPIOC, GPIO_Pin_0)
#define DHT11_IN_STATE()   GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_0)
#define DHT11_OUT_STATE()  GPIO_ReadOutputDataBit(GPIOC, GPIO_Pin_0)

u8 DHT11_Init(void);
u8 DHT11_ReadData(u8 *temp, u8 *humi);



#endif

