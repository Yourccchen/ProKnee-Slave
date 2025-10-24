//
// Core/Src/jetson_comm.c
//

#include "jetson_comm.h"
#include <string.h> // for memcpy

// --- 外部句柄 ---
extern UART_HandleTypeDef huart3;
extern DMA_HandleTypeDef hdma_usart3_rx;

// --- 私有 DMA 轮询变量 ---
#define JETSON_DMA_BUFFER_SIZE 255
static uint8_t jetson_dma_buffer[JETSON_DMA_BUFFER_SIZE]; 
static uint32_t dma_read_ptr = 0; 

// --- 协议解析状态机 ---
typedef enum {
    WAIT_HEADER,
    WAIT_PAYLOAD,
    WAIT_FOOTER
} ParseState;

static ParseState parse_state = WAIT_HEADER;
static uint8_t struct_buffer[sizeof(CommDataStruct)]; // 临时存放结构体字节
static uint32_t struct_index = 0;

// --- 公共变量定义 ---
CommDataStruct g_jetson_rx_data; // 接收到的数据结构体
volatile uint8_t g_new_jetson_data_flag = 0;// 接收完数据标志位


/**
 * @brief (私有) 内部调用的数据处理函数
 */
static void ProcessNewByte(uint8_t byte)
{
    switch (parse_state)
    {
        case WAIT_HEADER:
            if (byte == PACKET_HEADER)
            {
                // 找到了包头！切换到 "等待数据" 状态
                parse_state = WAIT_PAYLOAD;
                struct_index = 0; // 重置结构体缓冲区索引
            }
            break;

        case WAIT_PAYLOAD:
            // 往结构体缓冲区里塞数据
            if (struct_index < sizeof(CommDataStruct))
            {
                struct_buffer[struct_index++] = byte;
            }
            
            // 检查是否收满了
            if (struct_index >= sizeof(CommDataStruct))
            {
                // 结构体收满了，切换到 "等待包尾" 状态
                parse_state = WAIT_FOOTER;
            }
            break;
            
        case WAIT_FOOTER:
            if (byte == PACKET_FOOTER)
            {
                // 包尾也正确！我们收到了一个完整的、有效的数据包
                
                // 1. 把临时缓冲区的数据 "拷贝" 到全局变量中
                memcpy(&g_jetson_rx_data, struct_buffer, sizeof(CommDataStruct));
                
                // 2. 设置标志位，通知 main()
                g_new_jetson_data_flag = 1; 
            }
            
            // 无论包尾是否正确，都必须回到 "等待包头" 状态
            // (如果包尾错了，就丢弃这个包)
            parse_state = WAIT_HEADER;
            break;
    }
}

// --- 公共函数实现 ---

void JETSON_Init(void)
{
    // 启动 USART3 的 DMA 循环接收
    HAL_UART_Receive_DMA(&huart3, jetson_dma_buffer, JETSON_DMA_BUFFER_SIZE);
    
    // 初始化状态机和指针
    dma_read_ptr = 0;
    parse_state = WAIT_HEADER;
    g_new_jetson_data_flag = 0;
}

void JETSON_PollReceiver(void)
{
    // 1. 获取 DMA 当前写入到哪里了
    uint32_t dma_write_ptr = (JETSON_DMA_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(&hdma_usart3_rx)) % JETSON_DMA_BUFFER_SIZE;

    // 2. 循环处理所有新收到的字节
    while (dma_read_ptr != dma_write_ptr)
    {
        // 3. 取出一个字节，交给状态机处理
        ProcessNewByte(jetson_dma_buffer[dma_read_ptr]);
        
        // 4. 更新 DMA 读取指针
        dma_read_ptr = (dma_read_ptr + 1) % JETSON_DMA_BUFFER_SIZE;
    }
}

/**
 * @brief 发送一个数据包 (结构体) 给 Jetson
 */
HAL_StatusTypeDef JETSON_SendData(CommDataStruct* data)
{
    // 检查 huart3 的 TX 是否正忙
    if (huart3.gState != HAL_UART_STATE_READY)
    {
        // 上一次的 DMA 传输还没完成，返回 BUSY
        return HAL_BUSY;
    }

    // 1. 构建要发送的完整数据包
    //    [包头] + [结构体] + [包尾]
    uint16_t data_len = sizeof(CommDataStruct);
    uint16_t packet_len = 1 + data_len + 1;
    
    // 使用一个静态缓冲区来构建包，避免堆栈溢出
    static uint8_t tx_packet_buffer[256]; // 确保足够大
    
    if(packet_len > 256) return HAL_ERROR; // 结构体太大了

    // 2. 填充数据包
    tx_packet_buffer[0] = PACKET_HEADER;              // 包头
    memcpy(&tx_packet_buffer[1], data, data_len);     // 数据 (结构体)
    tx_packet_buffer[1 + data_len] = PACKET_FOOTER; // 包尾

    // 3. 启动 DMA 异步发送
    //    这是 "非阻塞" 的，函数会立刻返回
    return HAL_UART_Transmit_DMA(&huart3, tx_packet_buffer, packet_len);
}