#include <ioCC2530.h>
#include "delay.h"
#include "OnBoard.h"
//��ʱ����

#if (HW_TIMER_DELAY)

void Delay_us(unsigned int k)//us��ʱ����
{
    T1CC0L = 0x06; 
    T1CC0H = 0x00; 
    T1CTL = 0x02; 
    while(k)
    { 
        while(!(T1CNTL >= 0x04));
        k--;
    }
    T1CTL = 0x00;  //�رն�ʱ��
}

void Delay_ms(unsigned int k)
{
    T1CC0L = 0xe8;
    T1CC0H = 0x03;
    T1CTL = 0x0a; //ģģʽ 32��Ƶ
    while(k)
    {
        while(!((T1CNTL >= 0xe8)&&(T1CNTH >= 0x03)));
        k--;
    }
    T1CTL = 0x00; //�رն�ʱ��
}

#else

void Delay_us(unsigned int k) //1 us��ʱ
{
    MicroWait(k);   
}

static void Delay_10us(void) //10 us��ʱ
{
   MicroWait(10);
}

void Delay_ms(unsigned int k)//n ms��ʱ
{
  unsigned char i;
  while(k--)
  {
    Delay_us(1000);
  }
}



#endif