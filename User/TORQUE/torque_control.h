#ifndef __TORQUE_CONTROL_H
#define __TORQUE_CONTROL_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 初始化力矩控制器
 * @note  此函数在内部加载所有PID参数
 */
void TorqueControl_Init(void);

/**
 * @brief 重置力矩控制器状态（例如，电机失能时）
 */
void TorqueControl_Reset(void);

/**
 * @brief 计算力矩环
 * @param target_torque 目标力矩 (来自 Jetson)
 * @param actual_torque 实际力矩 (来自 ADC 传感器)
 * @return float 计算得到的电流指令 (A)
 */
float TorqueControl_Calculate(float target_torque, float actual_torque);


#ifdef __cplusplus
}
#endif

#endif // __TORQUE_CONTROL_H