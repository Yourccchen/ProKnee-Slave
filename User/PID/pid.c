#include "pid.h"

void PID_Init(PID_Controller* pid, float kp, float ki, float kd, float integral_max, float output_max)
{
    pid->Kp = kp;
    pid->Ki = ki;
    pid->Kd = kd;
    pid->integral_max = integral_max;
    pid->output_max = output_max;
    
    // 重置所有状态
    PID_Reset(pid);
}

void PID_Reset(PID_Controller* pid)
{
    pid->setpoint = 0.0f;
    pid->actual = 0.0f;
    pid->error = 0.0f;
    pid->last_error = 0.0f;
    pid->integral = 0.0f;
    pid->output = 0.0f;
}

float PID_Calculate(PID_Controller* pid, float setpoint, float actual)
{
    pid->setpoint = setpoint;
    pid->actual = actual;
    
    // 1. 计算误差
    pid->error = pid->setpoint - pid->actual;
    
    // 2. 计算积分项 (带抗饱和)
    pid->integral += pid->error;
    // 积分限幅
    if (pid->integral > pid->integral_max) {
        pid->integral = pid->integral_max;
    } else if (pid->integral < -pid->integral_max) {
        pid->integral = -pid->integral_max;
    }
    
    // 3. 计算微分项 (D)
    float derivative = pid->error - pid->last_error;
    
    // 4. 计算总输出 (P + I + D)
    //    P = Kp * error
    //    I = Ki * integral
    //    D = Kd * derivative
    pid->output = (pid->Kp * pid->error) + (pid->Ki * pid->integral) + (pid->Kd * derivative);
    
    // 5. 输出限幅
    if (pid->output > pid->output_max) {
        pid->output = pid->output_max;
    } else if (pid->output < -pid->output_max) {
        pid->output = -pid->output_max;
    }
    
    // 6. 更新 last_error
    pid->last_error = pid->error;
    
    return pid->output;
}