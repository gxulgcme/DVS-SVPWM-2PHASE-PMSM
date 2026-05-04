/*******************************************************************************
 * 版权所有 (C)2015, LINKO SEMICONDUCTOR Co.ltd
 *
 * 文件名称： Global_Variable.h
 * 文件标识：
 * 内容摘要： 全局变量声明文件
 * 其它说明： 无
 * 当前版本： V 1.0
 * 作    者： Howlet
 * 完成日期： 2020年8月5日
 *
 * 修改记录1：
 * 修改日期：2020年8月5日
 * 版 本 号：V 1.0
 * 修 改 人：Howlet
 * 修改内容：创建
 *
 *******************************************************************************/

#ifndef __GLOBAL_VARIABLE__
#define __GLOBAL_VARIABLE__

#include "basic.h"
#include "function_config.h"

extern const char sVersion[10];                      /* 程序版本 */
                                                  
extern stru_TaskSchedulerDef struTaskScheduler;      /* 任务调度结构体 */

extern stru_FOC_CurrLoopDef struFOC_CurrLoop;        /* 电流内环结构体 */

extern stru_TransCoef struUser2Core, struUser2App;   /* 标幺化处理 */
extern stru_TransCoef struCore2App, struApp2Core;    /* 标幺化处理 */

extern void DebugPWM_OutputFunction(void);

#endif

/* ********************** (C) COPYRIGHT LINKO SEMICONDUCTOR ******************** */
/* ------------------------------END OF FILE------------------------------------ */
