#include <ioCC2530.h>
#include "delay.h"
#include "OnBoard.h"
//延时函数

#if (HW_TIMER_DELAY)

void Delay_us(unsigned int k)//us延时函数
{
    T1CC0L = 0x06; 
    T1CC0H = 0x00; 
    T1CTL = 0x02; 
    while(k)
    { 
        while(!(T1CNTL >= 0x04));
        k--;
    }
    T1CTL = 0x00;  //关闭定时器
}

void Delay_ms(unsigned int k)
{
    T1CC0L = 0xe8;
    T1CC0H = 0x03;
    T1CTL = 0x0a; //模模式 32分频
    while(k)
    {
        while(!((T1CNTL >= 0xe8)&&(T1CNTH >= 0x03)));
        k--;
    }
    T1CTL = 0x00; //关闭定时器
}

#else

void Delay_us(unsigned int k) //1 us延时
{
    MicroWait(k);   
}

static void Delay_10us(void) //10 us延时
{
   MicroWait(10);
}

void Delay_ms(unsigned int k)//n ms延时
{
  unsigned char i;
  while(k--)
  {
    Delay_us(1000);
  }
}



#endif