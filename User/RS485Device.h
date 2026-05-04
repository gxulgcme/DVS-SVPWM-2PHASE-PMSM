#ifndef _RS485ProDevice_h
#define _RS485ProDevice_h

void AcimRetModaState(void);
void RS485DispatchAskOrder(unsigned char * ord);  //处理广播指令，返回1表示已处理，不用再向下广播
void RS485ProDevice_Init(void);                               //初始化

#endif
