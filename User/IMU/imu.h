#ifndef __IMU_H
#define __IMU_H

#ifdef __cplusplus
extern "C" {
#endif

#include "wit_c_sdk.h"
#include "stm32f1xx_hal.h"

/**
 * @brief 角度数据结构体
 * @note  确保这与您 main.c 中使用的 g_sAngle 变量定义一致
 */
typedef struct {
    float fAngle[3]; // 0:Roll, 1:Pitch, 2:Yaw
} AngleData;

/**
 * @brief 全局角度变量声明
 * @note  在 imu.c 中定义，供 main.c 外部访问
 */
extern volatile AngleData g_sAngle;

/**
 * @brief 初始化 IMU 模块
 * @note  (1) 注册 HAL 库函数 (UART发送/延时) 到 WIT-SDK
 * (2) 注册 SDK 的数据更新回调函数
 * (3) 配置 IMU 自动回传角度 (10Hz)
 * (4) 启动 UART5 的中断接收
 */
void IMU_Init(void);

/**
 * @brief (可选) 获取最新的 IMU 角度数据
 * @note  您也可以直接访问全局变量 g_sAngle
 * @return AngleData 结构体
 */
AngleData IMU_GetAngle(void);

#ifdef __cplusplus
}
#endif

#endif //__IMU_H