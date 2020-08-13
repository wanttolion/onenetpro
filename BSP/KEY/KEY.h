#ifndef __KEY_H__
#define __KEY_H__

#include "stm32f10x.h"

extern u8 MQ2_SEND_STA;
extern u8 DHT11_SEND_STA;

//使用外部中断来触发
void Key_Init(void);

#endif
