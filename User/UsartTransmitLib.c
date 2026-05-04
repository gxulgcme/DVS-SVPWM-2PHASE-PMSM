#include "lks32mc03x.h"
#include "lks32mc03x_uart.h"
#include "lks32mc03x_gpio.h"
#include "UsartTransmitLib.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct TTransmitLibStructure * UsartLib[2]={0,0};
extern unsigned char UartAnalyseThisBuffer(struct TTransmitLibStructure * Uart);

//#define EnabledUart1ENBit 1
//#define EnabledUart2ENBit 1
//#define EnabledUart3ENBit 1

#ifdef EnabledUart1ENBit
  #define   SetUart1ENBits   (GPIOD->ODSET = GPIO_PIN_3)
  #define   ClrUart1ENBits   (GPIOD->ODCLR = GPIO_PIN_3)
#endif

#ifdef EnabledUart2ENBit
  #define   SetUart2ENBits   (GPIOA->ODSET = GPIO_PIN_2)
  #define   ClrUart2ENBits   (GPIOA->ODCLR = GPIO_PIN_2)
#endif

#ifdef EnabledUart3ENBit
  #define   SetUart3ENBits   (GPIOA->ODSET = GPIO_PIN_2)
  #define   ClrUart3ENBits   (GPIOA->ODCLR = GPIO_PIN_2)
#endif

unsigned char UART_IRQHandlerSendCnt;
unsigned char UART_IRQHandlerRecCnt;
void UART_IRQHandler(void)
{
  unsigned char ch;
	struct TTransmitLibStructure * Uart;
	if(UART0->IF & UART_IF_SendBufEmpty)  // 发送完成中断
	 {
		UART_ClearIRQFlag(UART0, UART_IF_SendBufEmpty);  // 清除发送完成标志位
		 
 	  Uart = UsartLib[defUSART1];
    if(Uart != 0 && Uart->SendState)
	   {
	    if(Uart->SendBufPos < Uart->SendBufLen)
				UART0->BUFF = (uint8_t)Uart->SendBuf[Uart->SendBufPos++];
		  else
       {
	      UART_IRQHandlerSendCnt++;
		    Uart->BeginClearSend=1;
        Uart->BeginClearSendDelay=0;							
        UART0->IE &= ~ (UART_IRQEna_SendBuffEmpty);
		   }
	   }
		 
	 }
	
	if (UART0->IF & UART_IF_RcvOver)   // 接收完成中断
	 {
		UART_ClearIRQFlag(UART0, UART_IF_RcvOver);	 // 清除接收完成标志位
		ch = UART_ReadData(UART0);			     // 接收 1 Byte数据
	  Uart = UsartLib[defUSART1];
	  if(Uart != 0)
	   {
      Uart->ReceiveDataTime=0;
	    if(Uart->ReadBufPos  < 50)
	       Uart->ReadBuffer[Uart->ReadBufPos++] = ch;
	   }			
	 }
}

//------------------------------------------------------------------------------------------------------
void UsartTransmitLibSetReceiveHandle(unsigned char uid,TUsartTransmitHandle handle)
{
	if(UsartLib[uid]!=0)
		UsartLib[uid]->Handle = handle;
}

//------------------------------------------------------------------------------------------------------
void UsartTransmitLibSetAnalyseHandle(unsigned char uid,TUartAnalyseHandle handle)
{
	if(UsartLib[uid]!=0)
		UsartLib[uid]->AnalyseHandle = handle;
}

//------------------------------------------------------------------------------------------------------
void UsartTransmitLibSetMainTaskHandle(unsigned char uid,TUartMainTaskHandle handle)
{
	if(UsartLib[uid]!=0)
		UsartLib[uid]->MainTaskHandle = handle;
}

//------------------------------------------------------------------------------------------------------
unsigned char GetLastSendPacketTime(unsigned char uid)
{
	if(UsartLib[uid]!=0)
	  return UsartLib[uid]->SendBufLastTime;
	else
		return 0xff;
}

//------------------------------------------------------------------------------------------------------
unsigned char UsartDispatchAskOrder(unsigned char uid,unsigned char * ord)
{
  unsigned char UartSendBuf[60];
  unsigned char len, crc, i;
  len = ord[0];
  if(len >0 && len < 50)
  {
    UartSendBuf[0] = 0xF5;
    UartSendBuf[1] = len;
    memcpy(&UartSendBuf[2], &ord[1], len);
    crc=0;
    for(i=1; i<len+2; i++)
      crc += UartSendBuf[i];
    UartSendBuf[len+2] = crc;
    UsartTransmitLibSend(uid, UartSendBuf, len+3);
    return 1;
  }
  return 0;
}

//------------------------------------------------------------------------------------------------------
void UsartTransmitLibSend(unsigned char uid,unsigned char *buf,unsigned char len)
{
	struct TTransmitLibStructure * uart;
	uart = UsartLib[uid];
  if(uart!=NULL)
	 {
		if(len >= 55)
			len = 55;
		memcpy((char*)&(uart->SendBufList[uart->SendBufListCount][1]),(const char*)buf,len); 
		uart->SendBufList[uart->SendBufListCount][0]=len;
		if(uart->SendBufListCount < 2)
			uart->SendBufListCount++;
	 }
	return;
}

//------------------------------------------------------------------------------------------------------
void UsartTransmitLibClearData(struct TTransmitLibStructure * Uart)
{
  Uart->ReceiveDataTime=0;

  Uart->SendBufListCount=0;
	Uart->SendBufLastTime=0;
	Uart->SendState=0;
  Uart->BeginSend=0;
	Uart->SendBufLen=0;
	Uart->SendBufPos=0;
	
  Uart->BeginSendDelay=0;
  Uart->BeginClearSend=0;
  Uart->BeginClearSendDelay=0;								 

  Uart->ReadBufPos=0;
}

//------------------------------------------------------------------------------------------------------
void UsartTransmitLibInit(unsigned char uid,struct TTransmitLibStructure * item)
{
  UART_InitTypeDef UART_InitStruct = {0};
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	struct TTransmitLibStructure * Uart;
	
	if(item !=0)
	 {
		UsartLib[uid] = item; 
	  memset(UsartLib[uid],0,sizeof(struct TTransmitLibStructure));
    UsartLib[uid]->AnalyseHandle = UartAnalyseThisBuffer;
	 }
	Uart = UsartLib[uid];
	if(Uart == NULL) 
		 return;
			
  UsartTransmitLibClearData(Uart);
  if(uid == defUSART1)
	 {
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
		 
#ifdef EnabledUart1ENBit
    GPIO_InitStruct.GPIO_Pin =  GPIO_Pin_8;
    GPIO_Init(GPIO0, &GPIO_InitStruct);
#endif	 

    /* P0.9-RX0, P0.7-TX0  UART0 */
    GPIO_InitStruct.GPIO_Pin =  GPIO_Pin_5;
    GPIO_Init(GPIO0, &GPIO_InitStruct);

	  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_Pin =  GPIO_Pin_4;
    GPIO_Init(GPIO0, &GPIO_InitStruct);
		
    GPIO_PinAFConfig(GPIO0, GPIO_PinSource_4, AF4_UART); //P0.4复用为UART_RX
    GPIO_PinAFConfig(GPIO0, GPIO_PinSource_5, AF4_UART); //P0.5复用为UART_TX


    UART_StructInit(&UART_InitStruct);
    UART_InitStruct.BaudRate     = 14400;                /* 设置波特率9600 */
    UART_InitStruct.WordLength   = UART_WORDLENGTH_8b;  /* 多机通讯数据长度需选择9bit模式 */
    UART_InitStruct.StopBits     = UART_STOPBITS_1b;    /* 停止位长度1位 */
    UART_InitStruct.FirstSend    = UART_FIRSTSEND_LSB;  /* 先发送LSB */
    UART_InitStruct.ParityMode   = UART_Parity_NO;      /* 多机通讯需要关闭数据校验*/
	  UART_InitStruct.RXD_INV      = DISABLE;             /* RXD电平正常输出*/            
	  UART_InitStruct.TXD_INV      = DISABLE;             /* TXD电平正常输出*/
	  UART_InitStruct.IRQEna       = UART_IRQEna_RcvOver; 
    UART_Init(UART0, &UART_InitStruct);
		 
#ifdef EnabledUart1ENBit
    ClrUart1ENBits;
#endif		 
	 }
  else if(uid == defUSART2)
	 {								 
#ifdef EnabledUart2ENBit
    ClrUart2ENBits;
#endif		 
	 }	 
  return;	 
}

//------------------------------------------------------------------------------------------------------
unsigned char UartAnalyseThisBuffer(struct TTransmitLibStructure * Uart)
{
 unsigned char  buflen,i;
 unsigned char  crc;
	
 if(Uart->ReadBufPos > 1 && Uart->ReadBuffer[0]==0xFF)
 {
	 if(Uart->ReceieF6Time < 20)
	  { 
     if(Uart->ReadBufPos>=5)
		  {			
			 //crc=0; 
	     //for(i=0;i<4;i++)
	     // crc+=Uart->ReadBuffer[i];
	     //if(crc == Uart->ReadBuffer[4])
		    {
			   Uart->ReceiveOrder=1;//接收到指令
			   return 0;
		    }
			 //else
			 // {
       //   memcpy(Uart->ReadBuffer,&Uart->ReadBuffer[1],Uart->ReadBufPos);
	     //   Uart->ReadBufPos--;
			 // }				 
	    }
		 return 0;
	  }
	 else
	  {
     memcpy(Uart->ReadBuffer,&Uart->ReadBuffer[1],Uart->ReadBufPos);
	   Uart->ReadBufPos--;
	  }
	 return 0;
 }
 else
	 Uart->ReceieF6Time=0;
	
  while(Uart->ReadBufPos)
  {
    if(Uart->ReadBuffer[0]!=0xF5)
    {
      memcpy(Uart->ReadBuffer, &Uart->ReadBuffer[1], Uart->ReadBufPos);
      Uart->ReadBufPos--;
    }
    else
      break;
  }
  if(Uart->ReadBufPos>=3)
  {
    buflen = Uart->ReadBuffer[1];//原来的长度是包括自己的，现在要减去
    if(buflen > 30)
    {
      memcpy(Uart->ReadBuffer, &Uart->ReadBuffer[1], Uart->ReadBufPos);
      Uart->ReadBufPos--;
      return 1;
    }
    if(Uart->ReadBufPos >= (buflen+3))
    {
      crc = 0;
      for(i=1; i<buflen+2; i++)
        crc+=Uart->ReadBuffer[i];
      if(crc == Uart->ReadBuffer[buflen+2])
      {
        //这是个合法的包
        Uart->ReceiveOrder=1;//接收到指令
        return 0;
      }
      else
      {
        memcpy(Uart->ReadBuffer, &Uart->ReadBuffer[1], Uart->ReadBufPos);
        Uart->ReadBufPos--;
        return 1;
      }
    }
  }
 return 0; 
}

//------------------------------------------------------------------------------------------------------
void UartSendRecivePacketTask(unsigned char no,struct TTransmitLibStructure * Uart)
{
  unsigned char len,i;
  if(Uart->SendBufListCount)
   {
    if(!Uart->SendState && Uart->SendBufLastTime>5)
		 {
      if(no == defUSART1)					
			{
#ifdef EnabledUart1ENBit
				SetUart1ENBits;
#endif				
			}
      else if(no == defUSART2)					
		  {
#ifdef EnabledUart2ENBit
				SetUart2ENBits;
#endif				
			}
      else if(no == defUSART3)					
		  {
#ifdef EnabledUart3ENBit
				SetUart3ENBits;
#endif				
			}
			len = Uart->SendBufList[0][0];
      memcpy(Uart->SendBuf,&Uart->SendBufList[0][1],len);
			Uart->SendBufLen = len;
			Uart->SendBufPos=0;
			if(Uart->SendBufListCount)
			{
		    Uart->SendBufListCount--;
		    for(i=0;i<Uart->SendBufListCount;i++)
		      memcpy(Uart->SendBufList[i],Uart->SendBufList[i+1],60);						
			}
			else if(Uart->SendBufListCount > 2)
				Uart->SendBufListCount=0;
			Uart->SendState=1;
		  Uart->BeginSend=1;
      Uart->BeginSendDelay=0;
		 }
	 }
  if(!Uart->ReceiveOrder && (Uart->ReadBufPos>=4))
	  while(Uart->AnalyseHandle(Uart));			 	
  return;
}

//------------------------------------------------------------------------------------------------------
void UsartTransmitPacketTask(unsigned char no,struct TTransmitLibStructure * Uart)
{
  unsigned char len,i;
  if(Uart->ReceiveOrder==1)//接收到指令
	 {  
		 
		if(Uart->ReadBuffer[0]==0xFF)
		 {
			if(Uart->Handle!=0)
		    Uart->Handle(Uart->ReadBuffer,0xFF);
		  Uart->ReadBufPos = Uart->ReadBufPos - 5;
      if(Uart->ReadBufPos > 0 && Uart->ReadBufPos < 100)          
        memcpy(Uart->ReadBuffer,&Uart->ReadBuffer[5],Uart->ReadBufPos);
			else
			  Uart->ReadBufPos = 0;
		 }
		else
		{		 
      Uart->LastReceivePacketTime=0;
      len = Uart->ReadBuffer[1];
      Uart->Handle(&Uart->ReadBuffer[2], len);
      Uart->ReadBufPos = Uart->ReadBufPos - len - 3;
      if(Uart->ReadBufPos > 0 && Uart->ReadBufPos < 100)
        memcpy(Uart->ReadBuffer, &Uart->ReadBuffer[len+3], Uart->ReadBufPos);
      else
        Uart->ReadBufPos = 0;
	   }
    Uart->ReceiveOrder=0;          //接收到指令
	}
 return;
}

//------------------------------------------------------------------------------------------------------
void UsartTransmitLibProg(void)
{
  unsigned char i;
  struct TTransmitLibStructure * Uart;	
	for(i=0;i<2;i++)
	 {
	  Uart = UsartLib[i];
	  if(Uart != 0)
	   {		 
      if(Uart->BeginSend)
       {
        if(Uart->BeginSendDelay)
          Uart->BeginSendDelay--;
        else
         {
          Uart->BeginSend = 0;
          if(i == defUSART1)
			     {			
            UART0->IE |= UART_IRQEna_SendBuffEmpty;
				    UART0->BUFF = (uint8_t)Uart->SendBuf[Uart->SendBufPos++];
			     }
          else if(i == defUSART2)
			     {
			     }
          else if(i == defUSART3)
			     {
			     }
		     }		
	     }

     if(Uart->BeginClearSend)
	    {
	     if(Uart->BeginClearSendDelay)
	       Uart->BeginClearSendDelay--;
	     else
	      {
         if(i == defUSART1)
			    {
           #ifdef EnabledUart1ENBit
				     ClrUart1ENBits;
           #endif				
			    }
         else if(i == defUSART2)
          {
           #ifdef EnabledUart2ENBit
				     ClrUart2ENBits;
           #endif				
			    }
         else if(i == defUSART3)
          {
           #ifdef EnabledUart3ENBit
				     ClrUart3ENBits;
           #endif				
			    }
			   Uart->BeginClearSend = 0;
			   Uart->SendState = 0;
			   Uart->SendBufLastTime=0;
		    }		
	    }		 
     if(Uart->ReceieF6Time < 250)
       Uart->ReceieF6Time++;
	   if(!Uart->SendState && Uart->SendBufLastTime < 250) 
		   Uart->SendBufLastTime++;	
		 if(Uart->ReceiveDataTime < 250)
		  {
			 Uart->ReceiveDataTime++;				
       if(Uart->ReceiveDataTime>20)
			  {
	       if(Uart->ReadBufPos)
	         Uart->ReadBufPos=0;
		    }				
		  }
		 Uart->LastReceivePacketTime++;	
     UartSendRecivePacketTask(i,Uart);
	  }
  }
}

//------------------------------------------------------------------------------------------------------
void UsartTransmitLibTask(void)
{
  unsigned char i;
	struct TTransmitLibStructure * Uart;
	for(i=0;i<2;i++)
	 {
	  Uart = UsartLib[i];
	  if(Uart != 0)
	  {		 
			if(Uart->MainTaskHandle!=NULL)
				 Uart->MainTaskHandle();
			if(Uart->LastReceivePacketTime > 30000)
			 {	
			  UsartTransmitLibInit(i,0);
				Uart->LastReceivePacketTime=0; 
			 }
			UsartTransmitPacketTask(i,Uart);
		}
	}
}

//------------------------------------------------------------------------------------------------------
