#ifndef __PID_H
#define __PID_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float Kp;
    float Ki;
    float Kd;

    float setpoint; // 目标值
    float actual;   // 实际值

    float error;      // 当前误差
    float last_error; // 上一次误差
    
    float integral;   // 积分项
    float integral_max; // 积分限幅
    
    float output;     // PID 输出
    float output_max; // PID 输出限幅

} PID_Controller;

/**
 * @brief 初始化PID控制器
 * @param pid 要初始化的PID结构体指针
 * @param kp Kp参数
 * @param ki Ki参数
 * @param kd Kd参数
 * @param integral_max 积分项最大值 (用于抗积分饱和)
 * @param output_max 输出最大值 (用于限制最终输出)
 */
void PID_Init(PID_Controller* pid, float kp, float ki, float kd, float integral_max, float output_max);

/**
 * @brief 计算PID输出
 * @param pid PID结构体指针
 * @param setpoint 目标值
 * @param actual 实际值
 * @return PID计算得到的输出值
 */
float PID_Calculate(PID_Controller* pid, float setpoint, float actual);

/**
 * @brief 重置PID控制器（清除积分项等）
 * @param pid PID结构体指针
 */
void PID_Reset(PID_Controller* pid);


#ifdef __cplusplus
}
#endif

#endif // __PID_H