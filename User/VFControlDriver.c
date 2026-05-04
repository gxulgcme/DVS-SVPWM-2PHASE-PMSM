/**
 * @file VFControlDriver.c
 * @brief 两相PMSM专用控制 - 双变量对称注入法
 * @version 45.0
 * @date 2026-04-03
 *
 * 创新点：
 *   1. 直接求解三相占空比 d_u, d_v, d_w，使得绕组电压 Vuw = Vdc*(d_u-d_w) 和 Vvw = Vdc*(d_v-d_w)
 *      为正弦波且相位差90°，同时保证所有占空比在[0,1]内。
 *   2. 实时零序偏移注入，最大化电压利用率（理论最大相电压峰值 = Vdc/√2，线电压峰值 = Vdc）。
 *   3. 角度浮点累加，消除相位抖动，输出纯净正弦波。
 *   4. 开环VF斜坡启动，确保可靠旋转，可扩展电流闭环。
 *
 * 硬件约束：U、V相分别接两相绕组一端，W相接两相绕组另一端。
 */

#include "hardware_config.h"
#include "Global_Variable.h"
#include "VFControlDriver.h"
#include "IQ_math.h"
#include "UsartTransmitLib.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

 /*=============================================================================
  * 控制参数
  *============================================================================*/
#define STARTUP_VOLTAGE_START_PU   0.05f   /* 初始电压标幺值（相对于最大理论值） */
#define STARTUP_VOLTAGE_STEP       0.01f
#define STARTUP_VOLTAGE_MAX_PU     1.0f    /* 最大电压对应绕组峰值 = Vdc/√2 */
#define STARTUP_FREQ_START_HZ      2.0f
#define STARTUP_FREQ_STEP_HZ       0.1f
#define STARTUP_FREQ_MAX_HZ        50.0f

  /* 电压理论最大值：绕组峰值 = Vdc * MAX_MODULATION */
#define MAX_MODULATION      (1.0f / 1.41421356237f)  /* 0.70710678 */

/*=============================================================================
 * 全局变量
 *============================================================================*/
struct TTransmitLibStructure AcimLibItem;
SVPWM     Svpwmdq;
ADCSamp   ADCSampPare = ADCSamp_DEFAULTS;
VF_Angle  VF_AnglePare = VF_angle_DEFAULTS;
TAcimModa AcimModa = AcimModa_DEFAULTS;

uint16_t VFp = 25;
uint16_t Speed_Ref = 4000;
#define ModaIsRun (MCPWM0->PWM_FAIL012 & MCPWM_MOE_ENABLE_MASK)

/* 开环状态变量 */
static float rotor_angle_rad = 0.0f;
static float voltage_pu = 0.0f;          /* 0~1，对应绕组峰值 = MAX_MODULATION * Vdc */
static float freq_hz = 0.0f;

/*=============================================================================
 * 核心创新函数：双变量对称注入法
 * 输入：目标绕组电压标幺值 u_uw (sin) 和 u_vw (cos)，范围 [-MAX_MODULATION, MAX_MODULATION]
 * 输出：三相占空比 duty[3] 范围 [0,1]
 * 原理：设 d_w = x, d_u = x + u_uw, d_v = x + u_vw，求解最优 x 使所有占空式在 [0,1] 内且居中。
 *============================================================================*/
static void compute_optimal_duty(float u_uw, float u_vw, float duty[3])
{
    /* 计算三个电压值（不含偏移） */
    float v_u = u_uw;
    float v_v = u_vw;
    float v_w = 0.0f;

    /* 求最小值与最大值 */
    float min_val = v_u;
    float max_val = v_u;
    if (v_v < min_val) min_val = v_v;
    if (v_v > max_val) max_val = v_v;
    if (v_w < min_val) min_val = v_w;
    if (v_w > max_val) max_val = v_w;

    /* 计算最优偏移量，使波形对称于0.5 */
    float x = (1.0f - (max_val - min_val)) / 2.0f - min_val;
    /* 限幅保护 */
    if (x < 0.0f) x = 0.0f;
    if (x > 1.0f) x = 1.0f;

    duty[0] = v_u + x;
    duty[1] = v_v + x;
    duty[2] = v_w + x;

    /* 最终限幅，确保数值稳定 */
    for (int i = 0; i < 3; i++) {
        if (duty[i] > 1.0f) duty[i] = 1.0f;
        if (duty[i] < 0.0f) duty[i] = 0.0f;
    }
}

/*=============================================================================
 * PWM 输出函数（占空比 -> 比较寄存器）
 *============================================================================*/
void Svpwm_Outpwm(void)
{
    int16_t duty[3];
    /* 将浮点占空比 [0,1] 映射到 [0, PWM_PERIOD] */
    duty[0] = (int16_t)(Svpwmdq.Ta_Q15 * PWM_PERIOD / 32767.0f);  // 注意原代码使用Q15，这里直接映射
    duty[1] = (int16_t)(Svpwmdq.Tb_Q15 * PWM_PERIOD / 32767.0f);
    duty[2] = (int16_t)(Svpwmdq.Tc_Q15 * PWM_PERIOD / 32767.0f);

    /* 限幅 */
    int16_t max_duty = PWM_PERIOD - 1;
    for (int i = 0; i < 3; i++) {
        if (duty[i] > max_duty) duty[i] = max_duty;
        if (duty[i] < 0) duty[i] = 0;
    }

    /* 相序选择（方向控制） */
    if (AcimModa.State & 0x2) {
        MCPWM_TH00 = duty[1];   MCPWM_TH01 = PWM_PERIOD;
        MCPWM_TH10 = duty[0];   MCPWM_TH11 = PWM_PERIOD;
        MCPWM_TH20 = duty[2];   MCPWM_TH21 = PWM_PERIOD;
    }
    else {
        MCPWM_TH00 = duty[0];   MCPWM_TH01 = PWM_PERIOD;
        MCPWM_TH10 = duty[1];   MCPWM_TH11 = PWM_PERIOD;
        MCPWM_TH20 = duty[2];   MCPWM_TH21 = PWM_PERIOD;
    }
}

/*=============================================================================
 * 角度累加（浮点，保证平滑）
 *============================================================================*/
static void update_angle(void)
{
    float step_rad = TWO_PI * freq_hz / 12500.0f;  /* 12500Hz PWM中断频率 */
    rotor_angle_rad += step_rad;
    if (rotor_angle_rad >= TWO_PI) rotor_angle_rad -= TWO_PI;
    if (rotor_angle_rad < 0.0f) rotor_angle_rad += TWO_PI;
    /* 同步Q16角度（供其他模块） */
    VF_AnglePare.Angle_theta_Q16 = (int32_t)(rotor_angle_rad / TWO_PI * ANGLE_Q16_MAX);
}

/*=============================================================================
 * 中断服务函数（主控入口）
 *============================================================================*/
void VFIRQHandler(void)
{
    static unsigned char calib_done = 0;
    static unsigned int offset_cnt = 0;
    static int32_t offset_u = 0, offset_v = 0, offset_bus = 0;
    static float duty[3];
    float u_uw, u_vw;   /* 绕组电压标幺值（相对于 Vdc） */

    /* ADC偏置校准 */
    if (!calib_done) {
        if (offset_cnt < 128) {
            offset_u += GET_MOTOR_CURRENT_U;
            offset_v += GET_MOTOR_CURRENT_V;
            offset_bus += GET_BUS_VOL_ADC_RESULT;
            offset_cnt++;
            return;
        }
        else {
            ADCSampPare.OffsetPhaseU_Curr = offset_u >> 7;
            ADCSampPare.OffsetPhaseV_Curr = offset_v >> 7;
            ADCSampPare.OffsetBUS_Curr = offset_bus >> 7;
            calib_done = 1;
            voltage_pu = STARTUP_VOLTAGE_START_PU;
            freq_hz = STARTUP_FREQ_START_HZ;
            rotor_angle_rad = 0.0f;
        }
    }

    /* 读取电流（保留供后续闭环） */
    int32_t iu_raw = GET_MOTOR_CURRENT_U - ADCSampPare.OffsetPhaseU_Curr;
    int32_t iv_raw = GET_MOTOR_CURRENT_V - ADCSampPare.OffsetPhaseV_Curr;
    ADCSampPare.PhaseU_Curr = iu_raw;
    ADCSampPare.PhaseV_Curr = iv_raw;
    ADCSampPare.BUS_Voltage = GET_BUS_VOL_ADC_RESULT - ADCSampPare.OffsetBUS_Curr;

    /* 电机运行标志处理 */
    if (!(AcimModa.State & 0x1)) {
        AcimModa.State |= 0x1;
        rotor_angle_rad = 0.0f;
        PWMOutputs(MCPWM0, ENABLE);
        voltage_pu = STARTUP_VOLTAGE_START_PU;
        freq_hz = STARTUP_FREQ_START_HZ;
        return;
    }

    if ((AcimModa.State & 0x1) && !AcimModa.ControlDelay) {
        /* 更新角度 */
        update_angle();

        /* 电压斜坡 */
        if (voltage_pu < STARTUP_VOLTAGE_MAX_PU)
            voltage_pu += STARTUP_VOLTAGE_STEP;
        if (voltage_pu > STARTUP_VOLTAGE_MAX_PU)
            voltage_pu = STARTUP_VOLTAGE_MAX_PU;

        /* 频率斜坡 */
        if (freq_hz < STARTUP_FREQ_MAX_HZ)
            freq_hz += STARTUP_FREQ_STEP_HZ;
        if (freq_hz > STARTUP_FREQ_MAX_HZ)
            freq_hz = STARTUP_FREQ_MAX_HZ;

        /* 计算目标绕组电压（标幺值，基值为 Vdc） */
        float amp = voltage_pu * MAX_MODULATION;   /* 绕组峰值 = voltage_pu * Vdc/√2 */
        u_uw = amp * sinf(rotor_angle_rad);        /* Vuw / Vdc */
        u_vw = amp * cosf(rotor_angle_rad);        /* Vvw / Vdc */

        /* 创新占空比计算 */
        compute_optimal_duty(u_uw, u_vw, duty);

        /* 转换为 Q15 格式并存储到全局结构体 */
        Svpwmdq.Ta_Q15 = (int32_t)(duty[0] * 32767.0f);
        Svpwmdq.Tb_Q15 = (int32_t)(duty[1] * 32767.0f);
        Svpwmdq.Tc_Q15 = (int32_t)(duty[2] * 32767.0f);

        /* 输出 PWM */
        Svpwm_Outpwm();
    }
    else {
        Svpwmdq.Ta_Q15 = 0;
        Svpwmdq.Tb_Q15 = 0;
        Svpwmdq.Tc_Q15 = 0;
        Svpwm_Outpwm();
    }
}

/*=============================================================================
 * 以下函数与原工程完全一致（未修改）
 *============================================================================*/
void AcimRetModaState(void)
{
    unsigned char state, SendBuf[10];
    state = AcimModa.State & 0x3;
    if (AcimModa.SlowStop) state |= 0x4;
    if (AcimModa.SlowStart) state |= 0x8;
    SendBuf[0] = 4;
    SendBuf[1] = 0xa0;
    SendBuf[2] = 0x81;
    SendBuf[3] = 0x1;
    SendBuf[4] = state;
    UsartDispatchAskOrder(defUSART1, SendBuf);
}

void AcimModaControlOrder(unsigned char ord)
{
    switch (ord) {
    case 1: AcimModa.State &= 0xFE;  break;
    case 2: AcimModa.State |= 0x1;  VF_AnglePare.Angle_theta_Q16 = 0; break;
    case 4: AcimModa.State ^= 0x2; break;
    case 0x10: if (AcimModa.State & 0x1) { AcimModa.SlowStop = 100; AcimModa.SlowStart = 0; } break;
    }
}

void AcimReceiveHandle(unsigned char* dat, unsigned char len)
{
    if (dat[0] == 0xa0) {
        switch (dat[1]) {
        case 0x1: AcimRetModaState(); break;
        case 0x2: AcimModaControlOrder(dat[2]); break;
        }
    }
}

void AcimModaControl(void)
{
    if (AcimModa.ControlDelay) {
        AcimModa.ControlDelay--;
    }
    else {
        if (AcimModa.State & 0x1) {
            if (!ModaIsRun) {
                VF_AnglePare.Angle_theta_Q16 = 0;
                MCPWM_EIF = BIT4 | BIT5;
                PWMOutputs(MCPWM0, ENABLE);
            }
        }
        else {
            PWMOutputs(MCPWM0, DISABLE);
        }
    }
}

unsigned char AcimRunOrder = 2;
void AcimControlProg(void)
{
    AcimModaControl();
    if (AcimRunOrder) {
        AcimModaControlOrder(AcimRunOrder);
        AcimRunOrder = 0;
    }
}

void VFAngle_init(void)
{
    VF_AnglePare.BASE_FREQ = 50;
    VF_AnglePare.step_anglemax = 500;
    VF_AnglePare.step_Speed = 3;
    VF_AnglePare.Angle_theta_Q16 = 0;
    VF_AnglePare.step_angle = 25;
    Speed_Ref = 3000;

    UsartTransmitLibInit(defUSART1, &AcimLibItem);
    UsartTransmitLibSetReceiveHandle(defUSART1, &AcimReceiveHandle);

    PWMOutputs(MCPWM0, ENABLE);
}