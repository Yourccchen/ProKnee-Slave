//
// jetson_comm.h
//
/**
 * @brief 负责与 Jetson (或其他上位机) 通过 USART3 进行通信
 * @note  使用 DMA + IDLE 中断 
 */

#ifndef __JETSON_COMM_H
#define __JETSON_COMM_H

#ifdef __cplusplus
extern "C" {
#endif

// 包含你的 HAL 库头文件，根据你的芯片型号可能需要修改
// (你的 debugc.h 使用了 f1xx，所以这里也用 f1xx)
#include "stm32f1xx_hal.h" 
#include "stdint.h"

// --- 可配置宏 ---
// 定义一个接收缓冲区大小，可以根据需要调整
#define JETSON_RX_BUFFER_SIZE 255  

// --- 公共函数声明 ---

/**
 * @brief 初始化 Jetson 通信 (USART3)
 * @note  此函数应在 main() 中的 MX_USART3_UART_Init() 之后调用
 */
void JETSON_UartInit(void);

/**
 * @brief USART3 的中断请求处理函数
 * @note  此函数应在 stm32f1xx_it.c 的 USART3_IRQHandler 中被调用
 */
void JETSON_UartIrqHandler(UART_HandleTypeDef* huart);

/**
 * @brief 检查并获取新接收到的数据
 * @param user_buffer [out] 用于存放新数据的缓冲区
 * @param length [out] 指向一个变量，用于存放接收到的数据长度
 * @return uint8_t 1: 有新数据, 0: 没有新数据
 * @note  此函数应在 main() 的 while(1) 循环中轮询调用
 */
uint8_t JETSON_GetData(uint8_t* user_buffer, uint16_t* length);


#ifdef __cplusplus
}
#endif

#endif //__JETSON_COMM_H