#ifndef _UsartTransmitLib_H
#define _UsartTransmitLib_H

#define defUSART1 0
#define defUSART2 1
#define defUSART3 2

struct TTransmitLibStructure;

typedef void (*TUsartTransmitHandle)(unsigned char *dat,unsigned char len);
typedef unsigned char (*TUartAnalyseHandle)(struct TTransmitLibStructure * Uart);
typedef void (*TUartMainTaskHandle)(void);

struct TTransmitLibStructure
{
  unsigned char  ReceiveDataTime;

  unsigned char  SendBufList[3][60];
  unsigned char  SendBufListCount;
	unsigned char  SendBufLastTime;
	unsigned char  SendState;
  unsigned char  SendBuf[60];
	unsigned char  SendBufLen;
	unsigned char  SendBufPos;
  unsigned char  BeginSend;
  unsigned char  BeginSendDelay;
  unsigned char  BeginClearSend;
  unsigned char  BeginClearSendDelay;			

  unsigned char  ReadBuffer[52];  
  unsigned char  ReadBufPos;
	unsigned char  ReceiveOrder;
	unsigned long  LastReceivePacketTime;
  unsigned char  ReceieF6Time;	
	TUsartTransmitHandle Handle;
	TUartAnalyseHandle AnalyseHandle;
	TUartMainTaskHandle MainTaskHandle;
};

unsigned char GetLastSendPacketTime(unsigned char uid);
unsigned char UsartDispatchAskOrder(unsigned char uid,unsigned char * ord);
void UsartTransmitLibSetReceiveHandle(unsigned char uid,TUsartTransmitHandle handle);
void UsartTransmitLibSetAnalyseHandle(unsigned char uid,TUartAnalyseHandle handle);
void UsartTransmitLibSetMainTaskHandle(unsigned char uid,TUartMainTaskHandle handle);
void UsartTransmitLibSendAddToHead(unsigned char uid,unsigned char *buf,unsigned char len);
void UsartTransmitLibSend(unsigned char uid,unsigned char *buf,unsigned char len);
void UsartTransmitLibInit(unsigned char uid,struct TTransmitLibStructure * item);
void UsartTransmitLibProg(void);
void UsartTransmitLibTask(void);

#endif
