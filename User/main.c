/*******************************************************************************
 * 版权所有 (C)2015, LINKO SEMICONDUCTOR Co.ltd
 *
 * 文件名称： Main.c
 * 文件标识：
 * 内容摘要： 工程主代码
 * 其它说明： 无
 * 当前版本： V 1.0
 * 作    者： Howlet Li
 * 完成日期： 2020年8月5日
 *
 * 修改记录1：
 * 修改日期：2020年8月16日
 * 版 本 号：V 1.0
 * 修 改 人：Howlet Li
 * 修改内容：创建
 *
 *******************************************************************************/
#include "Time_Process.h"
#include "hardware_config.h"
#include "Global_Variable.h"
#include "VFControlDriver.h"
#include "UsartTransmitLib.h"
void Hardware_init(void);
void Task_Scheduler(void);
void sys_init(void);

/*******************************************************************************
 函数名称：    int main(void)
 功能描述：    主程序入口
 输入参数：    无
 输出参数：    无
 返 回 值：    无
 其它说明：
 修改日期      版本号          修改人            修改内容
 -----------------------------------------------------------------------------
 2020/8/5      V1.0           Howlet Li          创建
 *******************************************************************************/
  
int main(void)
{
  SYS_DBG_CFG |= BIT14; //soft reset peripheral regs
  __disable_irq();                  /* 关闭中断 中断总开关 */
  Hardware_init();      /* 硬件初始化 */

  sys_init();           /* 系统初始化 */	
  VFAngle_init( );          //VF产生角度参数初始化	
  __enable_irq();                   /* 开启总中断 */
	
  for(;;)
   {
     if (struTaskScheduler.bTimeCnt1ms >= TASK_SCHEDU_1MS) /* 1毫秒事件，任务调度 */
      {
				UsartTransmitLibProg();
				AcimControlProg();
        struTaskScheduler.bTimeCnt1ms = 0;        
      }

     if (struTaskScheduler.nTimeCnt10ms >= TASK_SCHEDU_10MS) /* 10毫秒事件，任务调度 */
      {
        struTaskScheduler.nTimeCnt10ms = 0;
      }
			
     if (struTaskScheduler.nTimeCnt100ms >= TASK_SCHEDU_100MS) /* 100毫秒事件，任务调度 */
      {
        AcimRetModaState();
        struTaskScheduler.nTimeCnt100ms = 0;			
      }
		
     if (struTaskScheduler.nTimeCnt500ms >= TASK_SCHEDU_500MS) /* 500毫秒事件，任务调度 */
      {
        struTaskScheduler.nTimeCnt500ms = 0;
      }

     if(struTaskScheduler.bPWM_UpdateFlg) /* 每个PWM周期更新一次 */
      {
        struTaskScheduler.bPWM_UpdateFlg = 0;
      }
			UsartTransmitLibTask();
   }
}

/************************ (C) COPYRIGHT LINKO SEMICONDUCTOR **********************/
/* ------------------------------END OF FILE------------------------------------ */

