//
// jetson_comm.c
//

#include "jetson_comm.h"
#include <string.h> // 用于 memcpy 和 memset

// --- 外部句柄声明 ---
// 这些变量由 CubeMX 在 main.c 中定义，我们在此处声明为 extern
extern UART_HandleTypeDef huart3;
extern DMA_HandleTypeDef hdma_usart3_rx;

// --- 私有变量 ---
// DMA 接收的原始缓冲区
static uint8_t jetson_dma_buffer[JETSON_RX_BUFFER_SIZE];
// 用于主循环处理的消息缓冲区
static uint8_t jetson_msg_buffer[JETSON_RX_BUFFER_SIZE];
// 接收到的消息长度 (volatile 关键字很重要)
static volatile uint16_t jetson_msg_len = 0;
// 新数据标志位 (volatile 关键字很重要)
static volatile uint8_t  jetson_new_data_flag = 0;


// --- 私有函数声明 ---
static void JETSON_UartIdleCallback(UART_HandleTypeDef* huart);


// --- 公共函数实现 ---

/**
 * @brief 初始化 Jetson 通信 (USART3)
 */
void JETSON_UartInit(void)
{
    // 1. 使能 USART3 的 IDLE 中断
    __HAL_UART_ENABLE_IT(&huart3, UART_IT_IDLE);

    // 2. 启动 DMA 循环接收
    HAL_UART_Receive_DMA(&huart3, (uint8_t*)jetson_dma_buffer, JETSON_RX_BUFFER_SIZE);
}

/**
 * @brief 检查并获取新接收到的数据
 */
uint8_t JETSON_GetData(uint8_t* user_buffer, uint16_t* length)
{
    // 检查标志位
    if (jetson_new_data_flag)
    {
        // **关键**：在复制数据前先清除标志位，防止竞态条件
        // (更安全的方法是进入临界区，即关中断，但对于简单应用，
        //  先清标志位通常可行)
        jetson_new_data_flag = 0; 
        
        // 复制数据到用户提供的缓冲区
        memcpy(user_buffer, (const void*)jetson_msg_buffer, jetson_msg_len);
        
        // 传出数据长度
        *length = jetson_msg_len;
        
        // (可选) 清理内部消息缓冲区
        // memset((void*)jetson_msg_buffer, 0, jetson_msg_len);
        jetson_msg_len = 0; // 重置长度

        return 1; // 返回 1 表示有新数据
    }
    
    // 没有新数据
    *length = 0;
    return 0; // 返回 0 表示没有新数据
}

/**
 * @brief USART3 的中断请求处理函数 (仿照 DEBUGC_UartIrqHandler)
 */
void JETSON_UartIrqHandler(UART_HandleTypeDef* huart)
{
    if (huart->Instance == USART3)                                 // 判断是否是 USART3
    {
        if (__HAL_UART_GET_FLAG(huart, UART_FLAG_IDLE) != RESET)   // 判断是否是空闲中断
        {
            __HAL_UART_CLEAR_IDLEFLAG(huart);                     // 清除空闲中断标志
            JETSON_UartIdleCallback(huart);                       // 调用中断处理函数
        } 
    }
}


// --- 私有函数实现 ---

/**
  * @brief  USART3 的空闲中断处理回调函数 (私有)
  */
static void JETSON_UartIdleCallback(UART_HandleTypeDef* huart)
{
    // 停止 DMA 传输，防止在处理数据时有新数据写入
    HAL_UART_DMAStop(huart);
    
    // 计算接收到的数据长度
    // 长度 = 缓冲区总大小 - DMA 剩余未传输的数据个数
    uint16_t data_length = JETSON_RX_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(huart->hdmarx);
    
    if (data_length > 0 && data_length < JETSON_RX_BUFFER_SIZE)
    {
        // 1. 将接收到的数据从 DMA 缓冲区复制到消息缓冲区
        memcpy((void*)jetson_msg_buffer, (const void*)jetson_dma_buffer, data_length);
        
        // 2. 记录消息长度
        jetson_msg_len = data_length;
        
        // 3. 设置新数据标志位，通知主循环(while(1))来处理
        jetson_new_data_flag = 1;
    }
    
    // 清空 DMA 缓冲区 (好习惯)
    memset((void*)jetson_dma_buffer, 0, JETSON_RX_BUFFER_SIZE);
    
    // 重新启动 DMA 接收
    // 注意：HAL_UART_Receive_DMA 会自动重置 DMA 计数器
    HAL_UART_Receive_DMA(huart, (uint8_t*)jetson_dma_buffer, JETSON_RX_BUFFER_SIZE);
}