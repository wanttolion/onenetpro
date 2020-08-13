#ifndef  __RTC_H__
#define __RTC_H__

#include "stm32f10x.h"

typedef struct {
	int sec;
	int min;
	int hour;
	int w_date;
	int w_month;
	int w_year;
	int week;
}time;

extern time tm;

void RTC_Init(void);
u8 RTC_Set(u16 year,u8 mon,u8 day,u8 hour,u8 min,u8 sec);
u8 RTC_Get(void);

#endif
