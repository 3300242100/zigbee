#ifndef __OLED_H__
#define __OLED_H__


// ------------------------------------------------------------
// IO��ģ��I2Cͨ��
// OLED_SCL��P1^3
// OLED_SDA��P1^2
// ------------------------------------------------------------
#define OLED_SCL    P1_7 //����ʱ��
#define	OLED_SDA    P0_0 //��������

#define	OLED_Tick_delay()  do{asm("NOP");}while(0);//���mcu��Ƶ̫�ߣ�����ʱ����oled��Ӧ

#define OLED_PIN_GPIO_MODE()  do{P1SEL &= ~(1<<7);P1DIR |= (1<<7);\
    P0SEL &= ~(1<<0);P0DIR |= (1<<0);}while(0)

#define high 1
#define low 0

#define Brightness  0xCF
#define X_WIDTH   128
#define Y_WIDTH   64



/*********************OLED��ʼ��************************************/
void OLED_Init(void);

/*********************OLEDȫ��************************************/
void OLED_Fill(unsigned char bmp_dat);
/*********************OLED��λ************************************/
void OLED_CLS(void);

void OLED_ClearLine_P8x16(unsigned char y, unsigned char count);


/***************������������ʾ6*8һ���׼ASCII�ַ���	��ʾ�����꣨x,y����yΪҳ��Χ0��7****************/
void OLED_P6x8Str(unsigned char x, unsigned char y, unsigned char * ch);

/*******************������������ʾ8*16һ���׼ASCII�ַ���	 ��ʾ�����꣨x,y����yΪҳ��Χ0��7****************/
void OLED_P8x16Str(unsigned char x, unsigned char y, unsigned char * ch);

/*****************������������ʾ16*16����  ��ʾ�����꣨x,y����yΪҳ��Χ0��7****************************/
//void OLED_P16x16Ch(unsigned char x,unsigned char y,unsigned char N);
void OLED_ShowCHinese(unsigned char x,unsigned char y,unsigned char no);

/***********������������ʾ��ʾBMPͼƬ128��64��ʼ������(x,y),x�ķ�Χ0��127��yΪҳ�ķ�Χ0��7*****************/
void OLED_Draw_BMP(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1, unsigned char * BMP);



#endif



