#ifndef __PROTOCOL_H
#define __PROTOCOL_H

#include "stdint.h"

// --- 定义通信协议常量 ---
#define PACKET_HEADER 0xAA  // 我们的包头
#define PACKET_FOOTER 0xBB  // 我们的包尾

#pragma pack(1)
typedef struct {
    float   chf[10]; // 10个浮点数通道
    int16_t chs[5];  // 5个16位有符号整数通道
    uint8_t chb[3];  // 3个8位无符号整数通道
} CommDataStruct;
#pragma pack()
/* RxDataStruct
chf[0] : angle_target
chf[1] : speed_target
chf[2] : torque_target
********
chs[0] : 
********
chb[0] : mode
*/
/* TxDataStruct
chf[0] : Angle
chf[1] : Speed
chf[2] : Torque_Motor
chf[3] : Torque_Sensor
chf[4] : Temperature
chf[5] : Resilience
********
chs[0] : foot_pressure
********
chb[0] :
*/
#endif