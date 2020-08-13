#include "RTC.h"

const u8 mon_table[12]={31,28,31,30,31,30,31,31,30,31,30,31};
time tm;

void RTC_NVIC_Config(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
	/* Enable the RTC Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void RTC_Configuration(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR|RCC_APB1Periph_BKP,ENABLE);

    PWR_BackupAccessCmd(ENABLE);
  
    BKP_DeInit();
   
    RCC_LSEConfig(RCC_LSE_ON);
  
    while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET) {}
  
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
   
    RCC_RTCCLKCmd(ENABLE);
   
    RTC_WaitForSynchro();
             
    RTC_WaitForLastTask();
    
    RTC_SetPrescaler(32767); /* RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1)*/
   
    RTC_WaitForLastTask();
     
    RTC_ITConfig(RTC_IT_SEC, ENABLE);
          
    RTC_WaitForLastTask();
}

void RTC_Init(void)
{
	RTC_NVIC_Config();
    if(BKP_ReadBackupRegister(BKP_DR1)!=0x1016)
    {
            RTC_Configuration();
            RTC_Set(2016,5,11,9,7,55);
            
            BKP_WriteBackupRegister(BKP_DR1, 0x1016);
    }
    else
    {
        RTC_WaitForSynchro();
        RTC_ITConfig(RTC_IT_SEC, ENABLE);
        RTC_WaitForLastTask();
       
    }
    RTC_Get();
}

u8 Is_Leap_Year(u16 pyear)
{
    if(pyear%4==0)
    {
        if(pyear%100==0)
        {
            if(pyear%400==0)    
				return 1;
            else    
				return 0;
        }
        else
            return 1;
    }
    else
        return 0;
}

u8 RTC_Set(u16 year,u8 mon,u8 day,u8 hour,u8 min,u8 sec)
{
    u16 t;
    u32 secCount=0;
    if(year<1970||year>2099)
        return 1;
    for(t=1970;t<year;t++)    
    {
        if(Is_Leap_Year(t))
            secCount+=31622400;
        else
            secCount+=31536000;    
    }
    mon-=1;
    for(t=0;t<mon;t++)
    {
        secCount+=(u32)mon_table[t]*86400;
        if(Is_Leap_Year(year)&&t==1)
            secCount+=86400;
    }
    
    secCount+=(u32)(day-1)*86400;
    secCount+=(u32)hour*3600;
    secCount+=(u32)min*60;
    secCount+=sec;

    RTC_SetCounter(secCount);
    RTC_WaitForLastTask();    
    RTC_Get();
    return 0;
}

u8 RTC_Get(void)
{
        static u16 dayCount=0;
        u32 secCount=0;
        u32 tmp=0;
        u16 tmp1=0;
        secCount=RTC_GetCounter();
        tmp=secCount/86400;
        if(dayCount!=tmp)
        {
            dayCount=tmp;
            tmp1=1970;
            while(tmp>=365)
            {
                if(Is_Leap_Year(tmp1))
                {
                    if(tmp>=366)    
                        tmp-=366;
                    else
                    {
                        break;
                    }
                }
                else
                    tmp-=365;
                tmp1++;
            }
            tm.w_year=tmp1;
            tmp1=0;
            while(tmp>=28)
            {
                if(Is_Leap_Year(tm.w_year)&&tmp1==1)
                {
                    if(tmp>=29)    
                        tmp-=29;
                    else
                        break;
                }
                else
                {
                    if(tmp>=mon_table[tmp1])
                        tmp-=mon_table[tmp1];
                    else
                        break;
                }
                tmp1++;
            }
            tm.w_month=tmp1+1;
            tm.w_date=tmp+1;    
        }
        tmp=secCount%86400;
        tm.hour=tmp/3600;
        tm.min=(tmp%3600)/60;
        tm.sec=(tmp%3600)%60;
        return 0;
}

