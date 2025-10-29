//
// User/ADC/adc_comm.h
// 模块：用于从 UART1 (ADC板) 接收数据
//

#ifndef __ADC_COMM_H
#define __ADC_COMM_H

// 包含你的 HAL 库头文件 (与 debugc.h 和 jetson_comm.h 一致)
#include "stm32f1xx_hal.h" 
#include "stdint.h"

// --- 公共函数 ---

/**
 * @brief 初始化 ADC 通信 (启动 UART1 DMA 循环接收)
 * @note  此函数应在 main() 中的 MX_USART1_UART_Init() 之后调用
 */
void ADC_Init(void);

/**
 * @brief 轮询 ADC 接收器 (必须在 main 的 while(1) 中不断调用)
 * @note  此函数在内部处理"字节流"，并解析"行数据"
 */
void ADC_PollReceiver(void);

/**
 * @brief 获取最新解析到的 Resilience 值
 * @return float
 */
float ADC_GetResilience(void);

/**
 * @brief 获取最新解析到的 Torque 值
 * @return float
 */
float ADC_GetTorque(void);

#endif //__ADC_COMM_H