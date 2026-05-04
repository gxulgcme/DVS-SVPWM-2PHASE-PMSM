/*******************************************************************************
 * 版权所有 (C)2015, LINKO SEMICONDUCTOR Co.ltd
 *
 * 文件名称： hardware_config.h
 * 文件标识：
 * 内容摘要： 硬件相关文件
 * 其它说明： 无
 * 当前版本： V 1.0
 * 作    者： Howlet
 * 完成日期： 2020年8月20日
 *
 * 修改记录1：
 * 修改日期： 2020年8月20日
 * 版 本 号： V 2.0
 * 修 改 人： Howlet
 * 修改内容： 创建
 *
 *******************************************************************************/

/*------------------------------prevent recursive inclusion -------------------*/
#ifndef __HARDWARE_CONFIG_H_
#define __HARDWARE_CONFIG_H_

#include "lks32mc03x.h"
#include "lks32mc03x_hal.h"
#include "hardware_init.h"
#include "MC_Parameter.h"

#define  LKS32MC051              1 
#define  LKS32MC051D             2
#define  LKS32MC052              3
#define  LKS32MC053              4
#define  LKS32MC054D             5
#define  LKS32MC055              6
#define  LKS32MC056              7
#define  LKS32MC057              8
#define  LKS32MC057D             9
#define  LKS32MC057E             10
#define  LKS32MC057D_V0          11

#define  CHIP_PART_NUMBER              LKS32MC038         /* 芯片型号选择，选择不正确将影响芯片模块的初始化 */
#define  P_HIGH__N_HIGH                1
#define  P_HIGH__N_LOW                 2

#if ((CHIP_PART_NUMBER == LKS32MC057D)||(CHIP_PART_NUMBER == LKS32MC054D)||(CHIP_PART_NUMBER == LKS32MC051D)\
     ||(CHIP_PART_NUMBER == LKS32MC057D_V0)) 
    #define  MCPWM_SWAP_FUNCTION           1
    #define  PRE_DRIVER_POLARITY           P_HIGH__N_HIGH     /* 预驱预动极性设置 上管高电平有效，下管高电平有效 */
#else
    #define  PRE_DRIVER_POLARITY           P_HIGH__N_HIGH      /* 预驱预动极性设置 上管高电平有效，下管低电平有效 */
#endif

/* ----------------------PWM 频率及死区定义----------------------------------- */
#define MCU_MCLK                       (48000000LL)       /* PWM模块运行主频 */
#define PWM_MCLK                       ((u32)MCU_MCLK)    /* PWM模块运行主频 */
#define PWM_PRSC                       ((u8)0)            /* PWM模块运行预分频器 */
#define PWM_FREQ                       ((u16)12500)       /* PWM斩波频率 22000*/

/* 电机控制PWM 周期计数器值 */
#define PWM_PERIOD                     ((u16) (PWM_MCLK / (u32)(2 * PWM_FREQ *(PWM_PRSC+1))))
/* PFC控制PWM 周期计数器值 */
#define PFC_PERIOD                     ((u16) (PWM_MCLK / (u32)(2 * PFC_FREQ *(PWM_PRSC+1))))

#define DEADTIME_NS                    ((u16)300)       /* 死区时间 */
#define DEADTIME                       (u16)(((unsigned long long)PWM_MCLK * (unsigned long long)DEADTIME_NS)/1000000000uL)
  
#define DEADTIMECOMPVOLTAGE            (u16)(DEADTIME_NS/(1000000000.0/PWM_FREQ)*MAX_MODULE_VALUE)  
                                                                                 
/* --------------------------------ADC通道号定义------------------------------ */
#define ADC_CHANNEL_OPA0              ADC_CHANNEL_0
#define ADC_CHANNEL_OPA1              ADC_CHANNEL_8

/* ADC相电流采样时序，硬件相关 ------------------------------------------------ */
/* Porting Application Notice 注意采样序列 ------------------------------------ */
#define ADC_CURRETN_A_CHANNEL         (ADC_CHANNEL_OPA1)
#define ADC_CURRETN_B_CHANNEL         (ADC_CHANNEL_OPA0)
//#define ADC_CURRETN_C_CHANNEL       (ADC_CHANNEL_OPA2)

#define ADC_1SHUNT_CURR_CH            (ADC_CHANNEL_OPA1)  /* 单电阻采样电流通道 */

#define ADC_BUS_VOL_CHANNEL           (ADC_CHANNEL_5)      /* 母线电压ADC采样通道 */
#define AC_ZERO_CHECK_CHANNEL         (ADC_CHANNEL_7)      /* AC电压ADC采样通道 */
#define M0_ADC_BUS_CURR_CH            (ADC_CHANNEL_OPA1)      /* 母线电流ADC采样通道 */

#define IPM_TEMP_CHANNEL              (ADC_CHANNEL_4)      /* 模块温度检测 */
#define NTC_TEMP_CHANNEL              (ADC_CHANNEL_3)      /* 发热丝温度检测 */

#define BEMF_CH_A                      ADC_CHANNEL_2       /* A相反电势检测ADC通道号 */
#define BEMF_CH_B                      ADC_CHANNEL_2       /* B相反电势检测ADC通道号 */
#define BEMF_CH_C                      ADC_CHANNEL_2       /* C相反电势检测ADC通道号 */

/* 电流内环相电流采样，ADC通道采样结果宏定义 */
#define GET_ADC0_DATA(value)           {value.nData0 = (s16)ADC_DAT0;}
#define GET_ADC1_DATA(value)           {value.nData0 = (s16)ADC_DAT1;}
#define GET_MOTOR_CURRENT_U            ((s16)ADC_DAT0)
#define GET_MOTOR_CURRENT_V            ((s16)ADC_DAT1)

/* 母线电压ADC通道采样结果宏定义 */
#define GET_BUS_VOL_ADC_RESULT         (ADC_DAT2)
#define GET_AC_ZERO_CHECK_CHANNEL      (ADC_DAT3)
#define GET_NTC_TEMP_RESULT          (ADC_DAT4)
#define GET_IPM_TEMP_RESULT           (ADC_DAT5)

#define ADC_STATE_RESET()              {ADC_CFG |= BIT11;}   /* ADC0 状态机复位,用以极限情况下确定ADC工作状态 */
#define ADC_SOFTWARE_TRIG_ONLY()       {ADC_CFG = 0;}       /* ADC设置为仅软件触发 */


/* ------------------------------PGA操作相关定义 ------------------------------ */
#define PGA_GAIN_20                    0                   /* 反馈电阻200:10 */
#define PGA_GAIN_9P5                   1                   /* 反馈电阻190:20 */
#define PGA_GAIN_6                     2                   /* 反馈电阻180:30 */
#define PGA_GAIN_4P25                  3                   /* 反馈电阻170:40 */
                                                                                  
#define OPA0_GIAN                      (PGA_GAIN_20)
#define OPA1_GIAN                      (PGA_GAIN_20 << 2)
#define OPA2_GIAN                      (PGA_GAIN_20 << 4)
#define OPA3_GIAN                      (PGA_GAIN_20 << 6)

/* ------------------------------DAC操作相关定义 ------------------------------ */
#define DAC_RANGE_1V2                  1                   /* DAC 1.2V量程 */
#define DAC_RANGE_3V0                  0                   /* DAC 3.0V量程 */
#define DAC_RANGE_4V85                 2                   /* DAC 4.85V量程 */

/* HALL自学习设置 ------------------------------------------------------------- */
#define HALL_LEARN_CURRENT             3000      /* HALL自学习电流定义 */

/* ------------------------------编译器选项------------------------------------ */
#define RUN_IN_RAM_FUNC  __attribute__ ((used, section ("ram3functions")))

/* ------------------------------硬件调试接口---------------------------------- */
#define DEBUG_SIG_OUT_HIGH()           {GPIO0_PDO |= BIT6;}
#define DEBUG_SIG_OUT_LOW()            {GPIO0_PDO &= ~BIT6;}
#define DEBUG_SIG_OUT_TANGO()          {GPIO0_PDO ^= BIT0;}


typedef struct
{
    s16 nBusVoltage;                             /* 直流母线电压 */             
                                                                          
    stru_VoltPhaseUVW mVdtComp;                  /* 死区时间补偿电压 */
    stru_VoltPhaseUVW mVoltUVW_PWM;              /* UVW PWM输出电压 */
  
    s16 nPhaseAOffset;                           /* A相ADC采样 Offset值 */
    s16 nPhaseBOffset;                           /* B相ADC采样 Offset值 */
    s16 nPhaseCOffset;                           /* C相ADC采样 Offset值 */
    s16 nBusShuntOffset;                         /* 直流母线电流ADC采样 Offset值 */
} stru_FOC_CurrLoopDef;


/* Exported functions ------------------------------------------------------- */
stru_TrigComponents Trig_Functions(s16 hAngle);

void MCPWM0_RegUpdate(void);
void MCPWM1_RegUpdate(void);

s32 App2CoreCurTrans(s32 val);
s32 App2CoreVolTrans(s32 val);
s32 App2CoreFreqTrans(s32 val);
s32 App2CoreVdcTrans(s32 val);
u32 App2CoreAngleTrans(u16 val);
s32 Core2AppCurTrans(s32 val);
s32 Core2AppVolTrans(s32 val);
s32 Core2AppFreqTrans(s32 val);
s32 Core2AppVdcTrans(s32 val);
u16 Core2AppAngleTrans(u32 val);
s32 User2AppCurTrans(float val);
s32 User2AppVolTrans(float val);
s32 User2AppFreqTrans(float val);
u16 User2AppAngleTrans(float val);

#define SetDebug2Bit (GPIO0->BSRR = GPIO_Pin_9)
#define ClrDebug2Bit (GPIO0->BRR  = GPIO_Pin_9)
#define GetDebug2Bit (GPIO0->PDO  & GPIO_Pin_9)

#define SetDebug1Bit (GPIO1->BSRR = GPIO_Pin_4)
#define ClrDebug1Bit (GPIO1->BRR  = GPIO_Pin_4)
#define GetDebug1Bit (GPIO1->PDO  & GPIO_Pin_4)


#endif  /* __HARDWARE_CONFIG_H_ */

 
/************************ (C) COPYRIGHT LINKO SEMICONDUCTOR **********************/
/* ------------------------------END OF FILE------------------------------------ */
 
