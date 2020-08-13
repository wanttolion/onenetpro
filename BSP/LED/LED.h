#ifndef __LED_H__
#define __LED_H__

#include "stm32f10x.h"

#define LED_MQ2_OPEN  GPIO_ResetBits(GPIOC, GPIO_Pin_2)
#define LED_MQ2_CLOSE GPIO_SetBits(GPIOC, GPIO_Pin_2)
 
#define LED_DHT11_OPEN  GPIO_ResetBits(GPIOC, GPIO_Pin_3)
#define LED_DHT11_CLOSE GPIO_SetBits(GPIOC, GPIO_Pin_3)

void Led_Init(void);


#endif
