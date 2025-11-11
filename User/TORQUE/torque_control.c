#include "torque_control.h"
#include "pid.h" // 包含我们刚创建的PID算法

// ==========================================================
//           !!! PID 参数在这里定义 !!!
// ==========================================================
// 您可以在这里修改参数，而无需触碰 main.c
// 它们是这个 .c 文件的“私有”参数
//
#define TORQUE_KP 3.0f     // 比例
#define TORQUE_KI 0.1f     // 积分
#define TORQUE_KD 0.0f     // 微分
#define TORQUE_INTEGRAL_MAX 5.0f  // 积分限幅
#define TORQUE_OUTPUT_MAX 10.0f // 输出限幅 (例如 10A)
// ==========================================================


// 定义一个“静态”的PID控制器实例。
// “static” 意味着它只在当前 .c 文件中可见。
static PID_Controller g_torque_pid;


// --- 公共函数实现 ---

void TorqueControl_Init(void)
{
    // 使用上面 #define 的参数来初始化PID
    PID_Init(&g_torque_pid, 
             TORQUE_KP, 
             TORQUE_KI, 
             TORQUE_KD, 
             TORQUE_INTEGRAL_MAX, 
             TORQUE_OUTPUT_MAX);
}

void TorqueControl_Reset(void)
{
    // 重置PID状态
    PID_Reset(&g_torque_pid);
}

float TorqueControl_Calculate(float target_torque, float actual_torque)
{
    // 调用PID算法
    return PID_Calculate(&g_torque_pid, target_torque, actual_torque);
}