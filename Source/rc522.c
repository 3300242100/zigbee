//#include <intrins.h>
//#include "reg52.h"
//#include "main.h"

#include <string.h>

#include "rc522.h"



#define MAXRLEN 18




char PcdRequest(unsigned char req_code, unsigned char *pTagType)
{
    char status;
    unsigned int  unLen;
    unsigned char ucComMF522Buf[MAXRLEN];
//  unsigned char xTest ;
    ClearBitMask(Status2Reg, 0x08);
    WriteRawRC(BitFramingReg, 0x07);

//  xTest = ReadRawRC(BitFramingReg);
//  if(xTest == 0x07 )
//   { LED_GREEN  =0 ;}
// else {LED_GREEN =1 ;while(1){}}
    SetBitMask(TxControlReg, 0x03);

    ucComMF522Buf[0] = req_code;

    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 1, ucComMF522Buf, &unLen);
//     if(status  == MI_OK )
//   { LED_GREEN  =0 ;}
//   else {LED_GREEN =1 ;}
    if ((status == MI_OK) && (unLen == 0x10))
    {
        *pTagType     = ucComMF522Buf[0];
        *(pTagType + 1) = ucComMF522Buf[1];
    }
    else
    {   status = MI_ERR;   }

    return status;
}

/////////////////////////////////////////////////////////////////////
//¹¦    ÄÜ£º·À³å×²
//²ÎÊýËµÃ÷: pSnr[OUT]:¿¨Æ¬ÐòÁÐºÅ£¬4×Ö½Ú
//·µ    »Ø: ³É¹¦·µ»ØMI_OK
/////////////////////////////////////////////////////////////////////
char PcdAnticoll(unsigned char *pSnr)
{
    char status;
    unsigned char i, snr_check = 0;
    unsigned int  unLen;
    unsigned char ucComMF522Buf[MAXRLEN];


    ClearBitMask(Status2Reg, 0x08);
    WriteRawRC(BitFramingReg, 0x00);
    ClearBitMask(CollReg, 0x80);

    ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x20;

    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 2, ucComMF522Buf, &unLen);

    if (status == MI_OK)
    {
        for (i = 0; i < 4; i++)
        {
            *(pSnr + i)  = ucComMF522Buf[i];
            snr_check ^= ucComMF522Buf[i];
        }
        if (snr_check != ucComMF522Buf[i])
        {   status = MI_ERR;    }
    }

    SetBitMask(CollReg, 0x80);
    return status;
}

/////////////////////////////////////////////////////////////////////
//¹¦    ÄÜ£ºÑ¡¶¨¿¨Æ¬
//²ÎÊýËµÃ÷: pSnr[IN]:¿¨Æ¬ÐòÁÐºÅ£¬4×Ö½Ú
//·µ    »Ø: ³É¹¦·µ»ØMI_OK
/////////////////////////////////////////////////////////////////////
char PcdSelect(unsigned char *pSnr)
{
    char status;
    unsigned char i;
    unsigned int  unLen;
    unsigned char ucComMF522Buf[MAXRLEN];

    ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x70;
    ucComMF522Buf[6] = 0;
    for (i = 0; i < 4; i++)
    {
        ucComMF522Buf[i + 2] = *(pSnr + i);
        ucComMF522Buf[6]  ^= *(pSnr + i);
    }
    CalulateCRC(ucComMF522Buf, 7, &ucComMF522Buf[7]);

    ClearBitMask(Status2Reg, 0x08);

    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 9, ucComMF522Buf, &unLen);

    if ((status == MI_OK) && (unLen == 0x18))
    {   status = MI_OK;  }
    else
    {   status = MI_ERR;    }

    return status;
}

/////////////////////////////////////////////////////////////////////
//¹¦    ÄÜ£ºÑéÖ¤¿¨Æ¬ÃÜÂë
//²ÎÊýËµÃ÷: auth_mode[IN]: ÃÜÂëÑéÖ¤Ä£Ê½
//                 0x60 = ÑéÖ¤AÃÜÔ¿
//                 0x61 = ÑéÖ¤BÃÜÔ¿
//          addr[IN]£º¿éµØÖ·
//          pKey[IN]£ºÃÜÂë
//          pSnr[IN]£º¿¨Æ¬ÐòÁÐºÅ£¬4×Ö½Ú
//·µ    »Ø: ³É¹¦·µ»ØMI_OK
/////////////////////////////////////////////////////////////////////
char PcdAuthState(unsigned char auth_mode, unsigned char addr, unsigned char *pKey, unsigned char *pSnr)
{
    char status;
    unsigned int  unLen;
    unsigned char i, ucComMF522Buf[MAXRLEN];

    ucComMF522Buf[0] = auth_mode;
    ucComMF522Buf[1] = addr;
    for (i = 0; i < 6; i++)
    {    ucComMF522Buf[i + 2] = *(pKey + i);   }
    for (i = 0; i < 6; i++)
    {    ucComMF522Buf[i + 8] = *(pSnr + i);   }
//   memcpy(&ucComMF522Buf[2], pKey, 6);
//   memcpy(&ucComMF522Buf[8], pSnr, 4);

    status = PcdComMF522(PCD_AUTHENT, ucComMF522Buf, 12, ucComMF522Buf, &unLen);
    if ((status != MI_OK) || (!(ReadRawRC(Status2Reg) & 0x08)))
    {   status = MI_ERR;   }

    return status;
}

/////////////////////////////////////////////////////////////////////
//¹¦    ÄÜ£º¶ÁÈ¡M1¿¨Ò»¿éÊý¾Ý
//²ÎÊýËµÃ÷: addr[IN]£º¿éµØÖ·
//          pData[OUT]£º¶Á³öµÄÊý¾Ý£¬16×Ö½Ú
//·µ    »Ø: ³É¹¦·µ»ØMI_OK
/////////////////////////////////////////////////////////////////////
char PcdRead(unsigned char addr, unsigned char *pData)
{
    char status;
    unsigned int  unLen;
    unsigned char i, ucComMF522Buf[MAXRLEN];

    ucComMF522Buf[0] = PICC_READ;
    ucComMF522Buf[1] = addr;
    CalulateCRC(ucComMF522Buf, 2, &ucComMF522Buf[2]);

    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen);
    if ((status == MI_OK) && (unLen == 0x90))
//   {   memcpy(pData, ucComMF522Buf, 16);   }
    {
        for (i = 0; i < 16; i++)
        {    *(pData + i) = ucComMF522Buf[i];   }
    }
    else
    {   status = MI_ERR;   }

    return status;
}

/////////////////////////////////////////////////////////////////////
//¹¦    ÄÜ£ºÐ´Êý¾Ýµ½M1¿¨Ò»¿é
//²ÎÊýËµÃ÷: addr[IN]£º¿éµØÖ·
//          pData[IN]£ºÐ´ÈëµÄÊý¾Ý£¬16×Ö½Ú
//·µ    »Ø: ³É¹¦·µ»ØMI_OK
/////////////////////////////////////////////////////////////////////
char PcdWrite(unsigned char addr, unsigned char *pData)
{
    char status;
    unsigned int  unLen;
    unsigned char i, ucComMF522Buf[MAXRLEN];

    ucComMF522Buf[0] = PICC_WRITE;
    ucComMF522Buf[1] = addr;
    CalulateCRC(ucComMF522Buf, 2, &ucComMF522Buf[2]);

    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen);

    if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
    {   status = MI_ERR;   }

    if (status == MI_OK)
    {
        //memcpy(ucComMF522Buf, pData, 16);
        for (i = 0; i < 16; i++)
        {    ucComMF522Buf[i] = *(pData + i);   }
        CalulateCRC(ucComMF522Buf, 16, &ucComMF522Buf[16]);

        status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 18, ucComMF522Buf, &unLen);
        if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
        {   status = MI_ERR;   }
    }

    return status;
}



/////////////////////////////////////////////////////////////////////
//¹¦    ÄÜ£ºÃüÁî¿¨Æ¬½øÈëÐÝÃß×´Ì¬
//·µ    »Ø: ³É¹¦·µ»ØMI_OK
/////////////////////////////////////////////////////////////////////
char PcdHalt(void)
{
    char status;
    unsigned int  unLen;
    unsigned char ucComMF522Buf[MAXRLEN];

    ucComMF522Buf[0] = PICC_HALT;
    ucComMF522Buf[1] = 0;
    CalulateCRC(ucComMF522Buf, 2, &ucComMF522Buf[2]);

    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen);

    return MI_OK;
}

/////////////////////////////////////////////////////////////////////
//ÓÃMF522¼ÆËãCRC16º¯Êý
/////////////////////////////////////////////////////////////////////
void CalulateCRC(unsigned char *pIndata, unsigned char len, unsigned char *pOutData)
{
    unsigned char i, n;
    ClearBitMask(DivIrqReg, 0x04);
    WriteRawRC(CommandReg, PCD_IDLE);
    SetBitMask(FIFOLevelReg, 0x80);
    for (i = 0; i < len; i++)
    {   WriteRawRC(FIFODataReg, *(pIndata + i));   }
    WriteRawRC(CommandReg, PCD_CALCCRC);
    i = 0xFF;
    do
    {
        n = ReadRawRC(DivIrqReg);
        i--;
    }
    while ((i != 0) && !(n & 0x04));
    pOutData[0] = ReadRawRC(CRCResultRegL);
    pOutData[1] = ReadRawRC(CRCResultRegM);
}

/////////////////////////////////////////////////////////////////////
//¹¦    ÄÜ£º¸´Î»RC522
//·µ    »Ø: ³É¹¦·µ»ØMI_OK
/////////////////////////////////////////////////////////////////////
char PcdReset(void)
{
    MF522_RST = 1;
    //    _nop_();
    asm("nop");
    MF522_RST = 0;
    //    _nop_();
    asm("nop");
    MF522_RST = 1;
    //     _nop_();
    asm("nop");
    WriteRawRC(CommandReg, PCD_RESETPHASE);
    //    _nop_();
    asm("nop");

    WriteRawRC(ModeReg, 0x3D);           //ºÍMifare¿¨Í¨Ñ¶£¬CRC³õÊ¼Öµ0x6363
    WriteRawRC(TReloadRegL, 30);
    WriteRawRC(TReloadRegH, 0);
    WriteRawRC(TModeReg, 0x8D);
    WriteRawRC(TPrescalerReg, 0x3E);
    WriteRawRC(TxAutoReg, 0x40);
    return MI_OK;
}
//////////////////////////////////////////////////////////////////////
//ÉèÖÃRC632µÄ¹¤×÷·½Ê½
//////////////////////////////////////////////////////////////////////
char M500PcdConfigISOType(unsigned char type)
{
    if (type == 'A')                     //ISO14443_A
    {
        ClearBitMask(Status2Reg, 0x08);

        /*     WriteRawRC(CommandReg,0x20);    //as default
              WriteRawRC(ComIEnReg,0x80);     //as default
              WriteRawRC(DivlEnReg,0x0);      //as default
           WriteRawRC(ComIrqReg,0x04);     //as default
           WriteRawRC(DivIrqReg,0x0);      //as default
           WriteRawRC(Status2Reg,0x0);//80    //trun off temperature sensor
           WriteRawRC(WaterLevelReg,0x08); //as default
              WriteRawRC(ControlReg,0x20);    //as default
           WriteRawRC(CollReg,0x80);    //as default
        */
        WriteRawRC(ModeReg, 0x3D); //3F
        /*     WriteRawRC(TxModeReg,0x0);      //as default???
               WriteRawRC(RxModeReg,0x0);      //as default???
               WriteRawRC(TxControlReg,0x80);  //as default???

               WriteRawRC(TxSelReg,0x10);      //as default???
           */
        WriteRawRC(RxSelReg, 0x86); //84
//      WriteRawRC(RxThresholdReg,0x84);//as default
//      WriteRawRC(DemodReg,0x4D);      //as default

//      WriteRawRC(ModWidthReg,0x13);//26
        WriteRawRC(RFCfgReg, 0x7F);  //4F
        /*   WriteRawRC(GsNReg,0x88);        //as default???
           WriteRawRC(CWGsCfgReg,0x20);    //as default???
           WriteRawRC(ModGsCfgReg,0x20);   //as default???
        */
        WriteRawRC(TReloadRegL, 30); //tmoLength);// TReloadVal = 'h6a =tmoLength(dec)
        WriteRawRC(TReloadRegH, 0);
        WriteRawRC(TModeReg, 0x8D);
        WriteRawRC(TPrescalerReg, 0x3E);


        //     PcdSetTmo(106);
        delay_10ms(1);
        PcdAntennaOn();
    }
    else { return -1; }

    return MI_OK;
}
/////////////////////////////////////////////////////////////////////
//¹¦    ÄÜ£º¶ÁRC632¼Ä´æÆ÷
//²ÎÊýËµÃ÷£ºAddress[IN]:¼Ä´æÆ÷µØÖ·
//·µ    »Ø£º¶Á³öµÄÖµ
/////////////////////////////////////////////////////////////////////
unsigned char ReadRawRC(unsigned char Address)
{
    unsigned char i, ucAddr;
    unsigned char ucResult = 0;

    MF522_SCK = 0;
    MF522_NSS = 0;
    ucAddr = ((Address << 1) & 0x7E) | 0x80;

    for (i = 8; i > 0; i--)
    {
        MF522_SI = ((ucAddr & 0x80) == 0x80);
        MF522_SCK = 1;
        ucAddr <<= 1;
        MF522_SCK = 0;
    }

    for (i = 8; i > 0; i--)
    {
        MF522_SCK = 1;
        ucResult <<= 1;
        ucResult |= MF522_SO;
        MF522_SCK = 0;
    }

    MF522_NSS = 1;
    MF522_SCK = 1;
    return ucResult;
}

/////////////////////////////////////////////////////////////////////
//¹¦    ÄÜ£ºÐ´RC632¼Ä´æÆ÷
//²ÎÊýËµÃ÷£ºAddress[IN]:¼Ä´æÆ÷µØÖ·
//          value[IN]:Ð´ÈëµÄÖµ
/////////////////////////////////////////////////////////////////////
void WriteRawRC(unsigned char Address, unsigned char value)
{
    unsigned char i, ucAddr;

    MF522_SCK = 0;
    MF522_NSS = 0;
    ucAddr = ((Address << 1) & 0x7E);

    for (i = 8; i > 0; i--)
    {
        MF522_SI = ((ucAddr & 0x80) == 0x80);
        MF522_SCK = 1;
        ucAddr <<= 1;
        MF522_SCK = 0;
    }

    for (i = 8; i > 0; i--)
    {
        MF522_SI = ((value & 0x80) == 0x80);
        MF522_SCK = 1;
        value <<= 1;
        MF522_SCK = 0;
    }
    MF522_NSS = 1;
    MF522_SCK = 1;
}

/////////////////////////////////////////////////////////////////////
//¹¦    ÄÜ£ºÖÃRC522¼Ä´æÆ÷Î»
//²ÎÊýËµÃ÷£ºreg[IN]:¼Ä´æÆ÷µØÖ·
//          mask[IN]:ÖÃÎ»Öµ
/////////////////////////////////////////////////////////////////////
void SetBitMask(unsigned char reg, unsigned char mask)
{
    char tmp = 0x0;
    tmp = ReadRawRC(reg);
    WriteRawRC(reg, tmp | mask); // set bit mask
}

/////////////////////////////////////////////////////////////////////
//¹¦    ÄÜ£ºÇåRC522¼Ä´æÆ÷Î»
//²ÎÊýËµÃ÷£ºreg[IN]:¼Ä´æÆ÷µØÖ·
//          mask[IN]:ÇåÎ»Öµ
/////////////////////////////////////////////////////////////////////
void ClearBitMask(unsigned char reg, unsigned char mask)
{
    char tmp = 0x0;
    tmp = ReadRawRC(reg);
    WriteRawRC(reg, tmp & ~mask);  // clear bit mask
}

/////////////////////////////////////////////////////////////////////
//¹¦    ÄÜ£ºÍ¨¹ýRC522ºÍISO14443¿¨Í¨Ñ¶
//²ÎÊýËµÃ÷£ºCommand[IN]:RC522ÃüÁî×Ö
//          pInData[IN]:Í¨¹ýRC522·¢ËÍµ½¿¨Æ¬µÄÊý¾Ý
//          InLenByte[IN]:·¢ËÍÊý¾ÝµÄ×Ö½Ú³¤¶È
//          pOutData[OUT]:½ÓÊÕµ½µÄ¿¨Æ¬·µ»ØÊý¾Ý
//          *pOutLenBit[OUT]:·µ»ØÊý¾ÝµÄÎ»³¤¶È
/////////////////////////////////////////////////////////////////////
char PcdComMF522(unsigned char Command,
                 unsigned char *pInData,
                 unsigned char InLenByte,
                 unsigned char *pOutData,
                 unsigned int  *pOutLenBit)
{
    char status = MI_ERR;
    unsigned char irqEn   = 0x00;
    unsigned char waitFor = 0x00;
    unsigned char lastBits;
    unsigned char n;
    unsigned int i;
    switch (Command)
    {
    case PCD_AUTHENT:
        irqEn   = 0x12;
        waitFor = 0x10;
        break;
    case PCD_TRANSCEIVE:
        irqEn   = 0x77;
        waitFor = 0x30;
        break;
    default:
        break;
    }

    WriteRawRC(ComIEnReg, irqEn | 0x80);
    ClearBitMask(ComIrqReg, 0x80);
    WriteRawRC(CommandReg, PCD_IDLE);
    SetBitMask(FIFOLevelReg, 0x80);

    for (i = 0; i < InLenByte; i++)
    {   WriteRawRC(FIFODataReg, pInData[i]);    }
    WriteRawRC(CommandReg, Command);


    if (Command == PCD_TRANSCEIVE)
    {    SetBitMask(BitFramingReg, 0x80);  }

//    i = 600;//¸ù¾ÝÊ±ÖÓÆµÂÊµ÷Õû£¬²Ù×÷M1¿¨×î´óµÈ´ýÊ±¼ä25ms
    i = 2000;
    do
    {
        n = ReadRawRC(ComIrqReg);
        i--;
    }
    while ((i != 0) && !(n & 0x01) && !(n & waitFor));
    ClearBitMask(BitFramingReg, 0x80);

    if (i != 0)
    {
        if (!(ReadRawRC(ErrorReg) & 0x1B))
        {
            status = MI_OK;
            if (n & irqEn & 0x01)
            {   status = MI_NOTAGERR;   }
            if (Command == PCD_TRANSCEIVE)
            {
                n = ReadRawRC(FIFOLevelReg);
                lastBits = ReadRawRC(ControlReg) & 0x07;
                if (lastBits)
                {   *pOutLenBit = (n - 1) * 8 + lastBits;   }
                else
                {   *pOutLenBit = n * 8;   }
                if (n == 0)
                {   n = 1;    }
                if (n > MAXRLEN)
                {   n = MAXRLEN;   }
                for (i = 0; i < n; i++)
                {   pOutData[i] = ReadRawRC(FIFODataReg);    }
            }
        }
        else
        {   status = MI_ERR;   }

    }


    SetBitMask(ControlReg, 0x80);          // stop timer now
    WriteRawRC(CommandReg, PCD_IDLE);
    return status;
}


/////////////////////////////////////////////////////////////////////
//¿ªÆôÌìÏß
//Ã¿´ÎÆô¶¯»ò¹Ø±ÕÌìÏÕ·¢ÉäÖ®¼äÓ¦ÖÁÉÙÓÐ1msµÄ¼ä¸ô
/////////////////////////////////////////////////////////////////////
void PcdAntennaOn()
{
    unsigned char i;
    i = ReadRawRC(TxControlReg);
    if (!(i & 0x03))
    {
        SetBitMask(TxControlReg, 0x03);
    }
}


/////////////////////////////////////////////////////////////////////
//¹Ø±ÕÌìÏß
/////////////////////////////////////////////////////////////////////
void PcdAntennaOff()
{
    ClearBitMask(TxControlReg, 0x03);
}


/////////////////////////////////////////////////////////////////////
//¹¦    ÄÜ£º¿Û¿îºÍ³äÖµ
//²ÎÊýËµÃ÷: dd_mode[IN]£ºÃüÁî×Ö
//               0xC0 = ¿Û¿î
//               0xC1 = ³äÖµ
//          addr[IN]£ºÇ®°üµØÖ·
//          pValue[IN]£º4×Ö½ÚÔö(¼õ)Öµ£¬µÍÎ»ÔÚÇ°
//·µ    »Ø: ³É¹¦·µ»ØMI_OK
/////////////////////////////////////////////////////////////////////
char PcdValue(unsigned char dd_mode, unsigned char addr, unsigned char *pValue)
{
    char status;
    unsigned int  unLen;
    unsigned char ucComMF522Buf[MAXRLEN];

    ucComMF522Buf[0] = dd_mode;
    ucComMF522Buf[1] = addr;
    CalulateCRC(ucComMF522Buf, 2, &ucComMF522Buf[2]);

    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen);

    if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
    {   status = MI_ERR;   }

    if (status == MI_OK)
    {
        memcpy(ucComMF522Buf, pValue, 4);
//       for (i=0; i<16; i++)
//       {    ucComMF522Buf[i] = *(pValue+i);   }
        CalulateCRC(ucComMF522Buf, 4, &ucComMF522Buf[4]);
        unLen = 0;
        status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 6, ucComMF522Buf, &unLen);
        if (status != MI_ERR)
        {    status = MI_OK;    }
    }

    if (status == MI_OK)
    {
        ucComMF522Buf[0] = PICC_TRANSFER;
        ucComMF522Buf[1] = addr;
        CalulateCRC(ucComMF522Buf, 2, &ucComMF522Buf[2]);

        status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen);

        if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
        {   status = MI_ERR;   }
    }
    return status;
}

/////////////////////////////////////////////////////////////////////
//¹¦    ÄÜ£º±¸·ÝÇ®°ü
//²ÎÊýËµÃ÷: sourceaddr[IN]£ºÔ´µØÖ·
//          goaladdr[IN]£ºÄ¿±êµØÖ·
//·µ    »Ø: ³É¹¦·µ»ØMI_OK
/////////////////////////////////////////////////////////////////////
char PcdBakValue(unsigned char sourceaddr, unsigned char goaladdr)
{
    char status;
    unsigned int  unLen;
    unsigned char ucComMF522Buf[MAXRLEN];

    ucComMF522Buf[0] = PICC_RESTORE;
    ucComMF522Buf[1] = sourceaddr;
    CalulateCRC(ucComMF522Buf, 2, &ucComMF522Buf[2]);

    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen);

    if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
    {   status = MI_ERR;   }

    if (status == MI_OK)
    {
        ucComMF522Buf[0] = 0;
        ucComMF522Buf[1] = 0;
        ucComMF522Buf[2] = 0;
        ucComMF522Buf[3] = 0;
        CalulateCRC(ucComMF522Buf, 4, &ucComMF522Buf[4]);

        status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 6, ucComMF522Buf, &unLen);
        if (status != MI_ERR)
        {    status = MI_OK;    }
    }

    if (status != MI_OK)
    {    return MI_ERR;   }

    ucComMF522Buf[0] = PICC_TRANSFER;
    ucComMF522Buf[1] = goaladdr;

    CalulateCRC(ucComMF522Buf, 2, &ucComMF522Buf[2]);

    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen);

    if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
    {   status = MI_ERR;   }

    return status;
}


///////////////////////////////////////////////////////////////////////
// Delay 10ms
///////////////////////////////////////////////////////////////////////
void delay_10ms(unsigned int _10ms)
{
#ifndef NO_TIMER2

#if 0
    RCAP2LH = RCAP2_10ms;
    T2LH    = RCAP2_10ms;

    TR2 = TRUE;
    while (_10ms--)
    {
        while (!TF2);
        TF2 = FALSE;
    }
    TR2 = FALSE;
#endif

#else
    while (_10ms--)
    {
        delay_50us(19);
        if (CmdValid)
            return;
        delay_50us(20);
        if (CmdValid)
            return;
        delay_50us(20);
        if (CmdValid)
            return;
        delay_50us(20);
        if (CmdValid)
            return;
        delay_50us(20);
        if (CmdValid )
            return;
        delay_50us(20);
        if (CmdValid)
            return;
        delay_50us(20);
        if (CmdValid)
            return;
        delay_50us(20);
        if (CmdValid)
            return;
        delay_50us(20);
        if (CmdValid)
            return;
        delay_50us(19);
        if (CmdValid)
            return;
    }
#endif
}







void rc522_hw_init(void)
{
    
    rc522_register_init();
    PcdReset();
    PcdAntennaOff();
    PcdAntennaOn();
    M500PcdConfigISOType( 'A' );
}


