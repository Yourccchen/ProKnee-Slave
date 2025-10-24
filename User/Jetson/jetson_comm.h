//
// Core/Inc/jetson_comm.h
//

#ifndef __JETSON_COMM_H
#define __JETSON_COMM_H

#include "protocol.h" // 包含我们的协议
#include "stm32f1xx_hal.h"   // (根据你的芯片修改)
#include <stdint.h>

// --- 公共变量 ---
// 让 main.c 可以访问接收到的数据
extern CommDataStruct g_jetson_rx_data;
// 一个标志位，通知 main.c 有新数据
extern volatile uint8_t g_new_jetson_data_flag;

// --- 公共函数 ---

/**
 * @brief 初始化 Jetson 通信 (启动 DMA 循环接收)
 */
void JETSON_Init(void);

/**
 * @brief 轮询接收器 (必须在 main 的 while(1) 中不断调用)
 * @note  此函数在内部处理"字节流"，并解析"数据包"
 */
void JETSON_PollReceiver(void);

/**
 * @brief 发送一个数据包 (结构体) 给 Jetson
 * @param data 要发送的结构体指针
 * @return HAL_StatusTypeDef (HAL_OK, HAL_BUSY, HAL_ERROR)
 */
HAL_StatusTypeDef JETSON_SendData(CommDataStruct* data);

#endif //__JETSON_COMM_H