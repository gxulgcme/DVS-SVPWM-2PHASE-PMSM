#include "UsartTransmitLib.h"
#include "RS485Device.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"

struct TTransmitLibStructure Rs485LibItem;

//--------------------------------------------------------------------------------
void AcimRetModaState(void)
{
  unsigned char SendBuf[10];
  SendBuf[0]=4;
  SendBuf[1]=0xa0;
  SendBuf[2]=0x81;
  SendBuf[3]=0x1;
  SendBuf[4]=0x2;
  UsartDispatchAskOrder(defUSART1, SendBuf);
}

//--------------------------------------------------------------------------------
void RS485ReceiveHandle(unsigned char *dat,unsigned char len)
{
	if(dat[0]==0xa0)
	 {
	  switch(dat[2])
	   {	 		 
		  case 0x1://我来查询你信息的
				   AcimRetModaState();
			     break;
		  case 0x2://这是控制指令
			     break;
     }
	 }
}

//--------------------------------------------------------------------------------
void RS485ProDevice_Init(void)                               //初始化
{  
	UsartTransmitLibInit(defUSART1,&Rs485LibItem);
  UsartTransmitLibSetReceiveHandle(defUSART1,&RS485ReceiveHandle);
  return;	
}

//--------------------------------------------------------------------------------
