//
// User/ADC/adc_comm.c
//

#include "adc_comm.h"
#include <stdio.h>    // 用于 sscanf
#include <string.h>   // 用于 memset

// --- 外部句柄 (由 CubeMX 在 main.c 中定义) ---
extern UART_HandleTypeDef huart1;
// 注意：我们通过 huart1.hdmarx 来访问 DMA 句柄，
// 因为它在 usart.c 的 HAL_UART_MspInit 中被链接了

// --- 私有 DMA 轮询变量 ---
#define ADC_DMA_BUFFER_SIZE 255
static uint8_t adc_dma_buffer[ADC_DMA_BUFFER_SIZE]; 
static uint32_t dma_adc_read_ptr = 0; 

// --- 私有行解析器变量 ---
#define ADC_LINE_BUFFER_SIZE 128
static char adc_line_buffer[ADC_LINE_BUFFER_SIZE];
static uint32_t adc_line_index = 0;

// --- 私有(静态)全局变量，用于存储解析结果 ---
static volatile float s_adc_resilience = 0.0f;
static volatile float s_adc_torque = 0.0f;


/**
 * @brief (私有) 处理从 UART1 DMA 来的一个新字节
 */
static void ProcessAdcByte(uint8_t byte)
{
    // 忽略 \r (回车符)
    if (byte == '\r')
    {
        return;
    }

    // \n (换行符) 是我们的 "消息结束符"
    if (byte == '\n')
    {
        if (adc_line_index > 0)
        {
            // 1. 在字符串末尾添加 null 终止符
            adc_line_buffer[adc_line_index] = '\0';
            
            // 2. 使用 sscanf 解析
            //    格式: %f,%f (resilience 在前, torque 在后)
            float temp_res = 0.0f;
            float temp_tor = 0.0f;
            
            if (sscanf(adc_line_buffer, "%f,%f", &temp_res, &temp_tor) == 2)
            {
                // 3. 成功！更新私有变量
                s_adc_resilience = temp_res;
                s_adc_torque = temp_tor;
            }
            
            // 4. 重置行缓冲区，准备接收下一行
            adc_line_index = 0;
        }
    }
    else
    {
        // 这是一个普通的数据字符，存入行缓冲区
        if (adc_line_index < (ADC_LINE_BUFFER_SIZE - 1))
        {
            adc_line_buffer[adc_line_index++] = (char)byte;
        }
        else
        {
            // 缓冲区溢出 (ADC板发送了超长行但没有换行)
            // 丢弃这一行，从头开始
            adc_line_index = 0;
        }
    }
}

// --- 公共函数实现 ---

void ADC_Init(void)
{
    // 启动 UART1 的 DMA 循环接收
    HAL_UART_Receive_DMA(&huart1, adc_dma_buffer, ADC_DMA_BUFFER_SIZE);
    
    // 初始化指针
    dma_adc_read_ptr = 0;
    adc_line_index = 0;
    s_adc_resilience = 0.0f;
    s_adc_torque = 0.0f;
}

void ADC_PollReceiver(void)
{
    // 1. 获取 DMA 当前写入到哪里了
    uint32_t dma_write_ptr = (ADC_DMA_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(huart1.hdmarx)) % ADC_DMA_BUFFER_SIZE;
    // 2. 循环处理所有新收到的字节
    while (dma_adc_read_ptr != dma_write_ptr)
    {
        // 3. 取出一个字节，交给 "行解析器" 处理
        ProcessAdcByte(adc_dma_buffer[dma_adc_read_ptr]);
        
        // 4. 更新 DMA 读取指针
        dma_adc_read_ptr = (dma_adc_read_ptr + 1) % ADC_DMA_BUFFER_SIZE;
    }
}

float ADC_GetResilience(void)
{
    return s_adc_resilience;
}

float ADC_GetTorque(void)
{
    return s_adc_torque;
}