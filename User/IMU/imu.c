#include "imu.h"
#include "usart.h"      // 提供 huart5 句柄
#include "string.h"     // 提供 memcpy

// --- 内部静态函数 (WIT-SDK 依赖) ---
static void SensorUartSend(uint8_t *p_data, uint32_t uiSize);
static void SensorDataUpdata(uint32_t uiReg, uint32_t uiRegNum);
static void Delayms(uint16_t ucMs);

// --- 内部变量 ---
static uint8_t s_ucRxBuffer[1]; // 用于 UART5 接收中断的1字节缓冲区

/**
 * @brief 全局角度变量定义
 * @note  在此文件中实际定义，在 imu.h 中声明
 */
volatile AngleData g_sAngle;

// --- 外部句柄 (来自 usart.c) ---
extern UART_HandleTypeDef huart5;


/**
 * @brief (公共) 初始化 IMU 模块
 */
void IMU_Init(void)
{
    // 1. 注册 SDK 回调函数
    //    WitInit(协议, 设备地址)
    WitInit(WIT_PROTOCOL_NORMAL, 0x50);
    //    注册串口发送函数
    WitSerialWriteRegister(SensorUartSend);
    //    注册数据更新回调函数
    WitRegisterCallBack(SensorDataUpdata);
    //    注册延时函数
    WitDelayMsRegister(Delayms);

    // 2. 解锁并配置 IMU (设置回传内容为角度，回传速率 10Hz)
    //    (如果配置失败，它将继续使用模块上一次保存的配置)
    Delayms(10);
    WitWriteReg(KEY, KEY_UNLOCK); // 解锁
    Delayms(10);
    WitSetContent(RSW_ANGLE); // 设置回传内容为 角度
    Delayms(10);
    WitSetOutputRate(RRATE_10HZ); // 设置回传速率 10Hz
    Delayms(10);
    WitWriteReg(SAVE, SAVE_PARAM); // 保存配置
    Delayms(10);
    
    // 3. 启动 UART5 的 1 字节中断接收
    //    数据将在 HAL_UART_RxCpltCallback 中被处理
    HAL_UART_Receive_IT(&huart5, s_ucRxBuffer, 1);
}

/**
 * @brief (公共) 获取最新的 IMU 角度数据
 */
AngleData IMU_GetAngle(void)
{
    return g_sAngle;
}


// =================================================================
// =           (私有) WIT-SDK 依赖函数实现                    =
// =================================================================

/**
 * @brief WIT-SDK 的串口发送函数
 * @note  我们使用 HAL 库的阻塞式发送，因为配置命令不常发送
 */
static void SensorUartSend(uint8_t *p_data, uint32_t uiSize)
{
    // 使用 huart5 发送数据，超时时间 1000ms
    HAL_UART_Transmit(&huart5, p_data, uiSize, 1000);
}

/**
 * @brief WIT-SDK 的延时函数
 * @note  直接调用 HAL_Delay
 */
static void Delayms(uint16_t ucMs)
{
    HAL_Delay(ucMs);
}

/**
 * @brief WIT-SDK 的数据更新回调函数
 * @note  当 SDK 成功解析到一个完整的数据包时，会调用此函数
 */
static void SensorDataUpdata(uint32_t uiReg, uint32_t uiRegNum)
{
    // 我们只关心角度数据 (寄存器地址 0x3d, 0x3e, 0x3f)
    if (uiReg >= Roll && uiReg <= Yaw)
    {
        // 1. 从 SDK 的 sReg 数组中拷贝数据
        //    (注意: sReg 是 int16_t 类型)
        //    sReg[Roll]  (0x3d)
        //    sReg[Pitch] (0x3e)
        //    sReg[Yaw]   (0x3f)
        
        // 2. 转换为浮点数并存入我们的全局结构体
        //    转换公式: 实际角度 = 寄存器值 / 32768 * 180
        g_sAngle.fAngle[0] = (float)sReg[Roll] / 32768.0f * 180.0f;
        g_sAngle.fAngle[1] = (float)sReg[Pitch] / 32768.0f * 180.0f;
        g_sAngle.fAngle[2] = (float)sReg[Yaw] / 32768.0f * 180.0f;
    }
}


// =================================================================
// =                   (公共) HAL 库回调函数                     =
// =================================================================

/**
 * @brief UART 接收完成回调函数
 * @note  此函数是一个弱定义 (weak) 函数，我们在这里重写它。
 * 它由 stm32f1xx_it.c 中的 HAL_UART_IRQHandler 自动调用。
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == UART5) // 确保是 IMU 所在的 UART5
    {
        // 1. 将收到的 1 字节数据喂给 SDK 的解析器
        WitSerialDataIn(s_ucRxBuffer[0]);
        
        // 2. 重新启动 1 字节中断接收
        HAL_UART_Receive_IT(&huart5, s_ucRxBuffer, 1);
    }
    
    // (您可以添加 else if (huart->Instance == ...) 来处理其他 UART 的回调)
}