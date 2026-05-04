/*******************************************************************************
 * 版权所有 (C)2015, LINKO SEMICONDUCTOR Co.ltd
 *
 * 文件名称： Time_Process.c
 * 文件标识：
 * 内容摘要： 定时相关函数
 * 其它说明： 无
 * 当前版本： V 1.0
 * 作    者： Howlet Li
 * 完成日期： 2020年8月16日
 *
 * 修改记录1：
 * 修改日期：2020年8月16日
 * 版 本 号：V 1.0
 * 修 改 人：Howlet Li
 * 修改内容：创建
 *
 *******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "Global_Variable.h"
#include "hardware_config.h"
#include "function_config.h"
#include "MC_parameter.h"
#include "math.h"

void CurrentOffsetCalibration(void);

void SoftDelay(u32 cnt)
{
    volatile u32 t_cnt;

    for(t_cnt = 0; t_cnt < cnt; t_cnt++)
    {
        __nop();
    }
}

void ADC_NormalModeCFG(void)
{
    ADC_CHN0 = ADC_CURRETN_B_CHANNEL | (ADC_CURRETN_A_CHANNEL << 4) | (ADC_BUS_VOL_CHANNEL << 8) | (AC_ZERO_CHECK_CHANNEL << 12);//ADC_CURRETN_B_CHANNEL
    ADC_CHN1 = NTC_TEMP_CHANNEL | (IPM_TEMP_CHANNEL << 4);
}


void MCPWM0_RegUpdate(void)
{
    MCPWM_TH00 = -struFOC_CurrLoop.mVoltUVW_PWM.nPhaseV;
    MCPWM_TH01 = struFOC_CurrLoop.mVoltUVW_PWM.nPhaseV;

    MCPWM_TH10 = -struFOC_CurrLoop.mVoltUVW_PWM.nPhaseU;
    MCPWM_TH11 = struFOC_CurrLoop.mVoltUVW_PWM.nPhaseU;

    MCPWM_TH20 = -struFOC_CurrLoop.mVoltUVW_PWM.nPhaseW;
    MCPWM_TH21 = struFOC_CurrLoop.mVoltUVW_PWM.nPhaseW;

}


/*******************************************************************************
 函数名称：    int sys_init(void)
 功能描述：    系统变量初始化
 输入参数：    无
 输出参数：    无
 返 回 值：    无
 其它说明：
 修改日期      版本号          修改人            修改内容
 -----------------------------------------------------------------------------
 2020/8/5      V1.0           Howlet Li          创建
 *******************************************************************************/

void sys_init(void)
{
//    CurrentOffsetCalibration();               /* 读取电流采样通道偏置 */

    struTaskScheduler.sVersion = &sVersion[0];/* 初始化版本号 */

    #if (DEBUG_PWM_OUTPUT == TEST_ON)
	  MCPWM_EIF = BIT4 | BIT5;
    DebugPWM_OutputFunction(); /* 调试的时候输出25%的PWM波形 */
    #endif
}


/*******************************************************************************
 函数名称：    void CurrentOffsetCalibration(void)
 功能描述：    读电流Offset值
 输入参数：    stru_FOC_CtrProcDef *this  结构体指针
 输出参数：    无
 返 回 值：    无
 其它说明：
 修改日期      版本号          修改人            修改内容
 -----------------------------------------------------------------------------
 2020/8/5      V1.0           Howlet Li          创建
 *******************************************************************************/
void CurrentOffsetCalibration(void)
{
    u16 CalibCnt = 0;
    volatile u32 t_delay;
    stru_OffsetDef  struOffset;

    __disable_irq();


    ADC_SOFTWARE_TRIG_ONLY();

    ADC_STATE_RESET();

    ADC_NormalModeCFG();

    struFOC_CurrLoop.mVoltUVW_PWM.nPhaseU = 0;
    struFOC_CurrLoop.mVoltUVW_PWM.nPhaseV = 0;
    struFOC_CurrLoop.mVoltUVW_PWM.nPhaseW = 0;
    MCPWM0_RegUpdate();
    PWMOutputs(MCPWM0, ENABLE);
    for(t_delay = 0; t_delay < 0x7ffff; t_delay++);

    struOffset.IPhAFilt    = 0;
    struOffset.IPhBFilt    = 0;
    struOffset.UBusFilt    = 0;
    struOffset.IBusFilt    = 0;

    for(CalibCnt = 0; CalibCnt < (1 << ADC_GET_OFFSET_SAMPLES); CalibCnt++)
    {
        /* Clear the ADC0 JEOC pending flag */
        ADC_SWT = 0x00005AA5;

        while(!(ADC_IF & BIT0));

        ADC_IF |= BIT1 | BIT0;
        ADC_STATE_RESET();
        struOffset.IPhAFilt +=  (s16)((ADC_DAT0));
        struOffset.IPhBFilt +=  (s16)((ADC_DAT1));
        struOffset.IPhCFilt += (s16)((ADC_DAT2));
        struOffset.IBusFilt += (s16)(ADC_DAT3);
    }

    struFOC_CurrLoop.nPhaseAOffset = (s16)(struOffset.IPhAFilt >> ADC_GET_OFFSET_SAMPLES);
    struFOC_CurrLoop.nPhaseBOffset = (s16)(struOffset.IPhBFilt >> ADC_GET_OFFSET_SAMPLES);
    struFOC_CurrLoop.nPhaseCOffset = (s16)(struOffset.IPhCFilt >> ADC_GET_OFFSET_SAMPLES);
    PWMOutputs(MCPWM0, DISABLE);

    ADC_init();

    ADC_NormalModeCFG();

    __enable_irq();


}

/************************ (C) COPYRIGHT LINKO SEMICONDUCTOR **********************/
/* ------------------------------END OF FILE------------------------------------ */
