#ifndef _VFControlDriver_H
#define _VFControlDriver_H
#include "stdint.h"

typedef struct {
         unsigned char State;//0:运行 1:正反 2:
         unsigned char Speed;//0-100Hz;
	       unsigned char SlowStart;
	       unsigned char SlowStop;
	       unsigned char SlowStartDeta;
         unsigned char MosTemp;  //驱动管温度
         unsigned char Current;//当前电流
         unsigned char Voltage;  //总线电压
         unsigned char Overcurrent;//过流点
	       unsigned char ControlDelay;
       }TAcimModa;

#define  AcimModa_DEFAULTS  {0,50,0,0,0,0,200,0}   // 初始化参数
			 
typedef struct {
	     int32_t   BUS_Curr ;     // 母线电流 DC Bus  Current
	     int32_t   PhaseU_Curr;   // U相电流 Phase U Current
	     int32_t   PhaseV_Curr;   // V相电流Phase V Current
	     int32_t   BUS_Voltage ;  //母线电压DC Bus  Voltage	     
	     int32_t   RP_speed_Voltage ;   // 电位器电压 RP1_Voltage
	     int32_t   OffsetBUS_Curr ;     // 母线电流偏执值 DC Bus  Current
	     int32_t   OffsetPhaseU_Curr;   // U相电流偏执值  Phase U Current
	     int32_t   OffsetPhaseV_Curr;   // V相电流偏执值 Phase V Current
	     int32_t   Coeff_filterK1;   // 一阶低通滤波器系数1
		   int32_t   Coeff_filterK2;   // 一阶低通滤波器系数2
       }ADCSamp;

#define  ADCSamp_DEFAULTS  {0,0,0,0,0,0,0,0,268,756}   // 初始化参数

typedef struct {
	    int32_t      SpeedRef ;    // 给定参考速度
	    int32_t      Speed_target ; // 实际目标速度
	    int32_t      Speederror ;   // 实际目标速度和参考速度差
	    int32_t      step_Speed ;   // 步进速度
	    int32_t      step_angle ;   // 步进角度
	    int32_t      step_anglemax ;// 最大步进角
	    uint16_t     BASE_FREQ ;    //电机的基本频率
	    int32_t      Angle_theta_Q16;  //角度值
	   }VF_Angle;

#define  VF_angle_DEFAULTS   {0,0,0,10,0,0,240,0}  // 初始化参数

void  VF_start_control(void); // VF的变化控制
void  VFAngle_init(void );  // VF的初始化函数
void  VFAngle_cale(void );  // VF的根据频率自加减角度


typedef struct 	{ 
	int32_t  Cos_theta_Q15;
	int32_t  Sin_theta_Q15;
	int32_t  V_alpha_Q15; 		//IQ15格式数据: 二相静止坐标系alpha-轴
	int32_t  V_beta_Q15;	//IQ15格式数据: 二相静止坐标系beta-轴 
	int32_t  Ta_Q15;		//  IQ15格式数据: 三相矢量占空比Ta
	int32_t  Tb_Q15;		//  IQ15格式数据: 三相矢量占空比Tb
	int32_t  Tc_Q15;		//  IQ15格式数据: 三相矢量占空比Tc
} SVPWM , *p_SVPWM ;

#define SVPWM_DEFAULTS  { 0,0,0,0,0}  // 初始化参数

void  SVPWM_Cale(p_SVPWM pV);


#define PWM_FREQ ((u16)12500) // in Hz  (N.b.: pattern type is center aligned),不是中央对齐时12.5kHZ，中央对齐模式是
#define PWM_HalfPerMax   ((int32_t)1920/2)

#define DCBUS_VOLTAGE			(int32_t)_IQ(24)		//   Q10格式,母线电压24V  24576 

#define STATOR_VOLTAGE		 (int32_t)_IQ(13.867) 		// Udc/√3* Q15  13.867   14190  格式   

#define MAX_STATOR_VOLTAGE	(int32_t)_IQ(13.5)  	 // Udc/√3*0.97 13.467   13765 Q15格式  
#define ANGLE_Q16_MAX      65536
#define TWO_PI             6.28318530718f
extern TAcimModa AcimModa;

void Svpwm_Outpwm(void);  //SVPWM输出函数
void VFIRQHandler(void);
void SoftDelay(uint32_t cnt);
void VFAngle_init(void );
void Offset_CurrentReading(void);
void Stop_Motor(void);
void Start_Motor(void);
void AcimControlProg(void);
void AcimRetModaState(void);

#endif
