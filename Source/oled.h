#ifndef __OLED_H__
#define __OLED_H__


// ------------------------------------------------------------
// IO口模拟I2C通信
// OLED_SCL接P1^3
// OLED_SDA接P1^2
// ------------------------------------------------------------
#define OLED_SCL    P1_7 //串行时钟
#define	OLED_SDA    P0_0 //串行数据

#define	OLED_Tick_delay()  do{asm("NOP");}while(0);//如果mcu主频太高，请延时以让oled适应

#define OLED_PIN_GPIO_MODE()  do{P1SEL &= ~(1<<7);P1DIR |= (1<<7);\
    P0SEL &= ~(1<<0);P0DIR |= (1<<0);}while(0)

#define high 1
#define low 0

#define Brightness  0xCF
#define X_WIDTH   128
#define Y_WIDTH   64



/*********************OLED初始化************************************/
void OLED_Init(void);

/*********************OLED全屏************************************/
void OLED_Fill(unsigned char bmp_dat);
/*********************OLED复位************************************/
void OLED_CLS(void);

void OLED_ClearLine_P8x16(unsigned char y, unsigned char count);


/***************功能描述：显示6*8一组标准ASCII字符串	显示的坐标（x,y），y为页范围0～7****************/
void OLED_P6x8Str(unsigned char x, unsigned char y, unsigned char * ch);

/*******************功能描述：显示8*16一组标准ASCII字符串	 显示的坐标（x,y），y为页范围0～7****************/
void OLED_P8x16Str(unsigned char x, unsigned char y, unsigned char * ch);

/*****************功能描述：显示16*16点阵  显示的坐标（x,y），y为页范围0～7****************************/
//void OLED_P16x16Ch(unsigned char x,unsigned char y,unsigned char N);
void OLED_ShowCHinese(unsigned char x,unsigned char y,unsigned char no);

/***********功能描述：显示显示BMP图片128×64起始点坐标(x,y),x的范围0～127，y为页的范围0～7*****************/
void OLED_Draw_BMP(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1, unsigned char * BMP);



#endif



