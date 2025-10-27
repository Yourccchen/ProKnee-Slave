# ProsSlave假肢控制下位机

该项目为动力假肢的下位机部分代码

**各串口用处**

- **USART1:**与ADC板通信
- **USART2:**用于DEBUG，将数据在电脑上可视化
- **USART3:**与Jetson板通信

用户自定义函数放在User文件夹

/DEBUG 存放用于DEBUG打印数据的相关代码

/Jetson 存放与Jetson通信的相关代码

/protocol.h 定义了通用消息结构体形式

/RS 存放电机控制的相关代码



#### 电机主要接口说明 Main API List

| 功能     | 接口名 / Function Name                                     | 简述 / Description     |
| ------ | ------------------------------------------------------- | -------------------- |
| 使能电机   | `Enable_Motor()`                                        | 进入运行状态               |
| 失能电机   | `Disenable_Motor(clear_err)`                            | 停止运行，`clear_err=1`清错 |
| 运控模式   | `RobStride_Motor_move_control(t, angle, speed, kp, kd)` | 力矩/速度/角度复合控制         |
| PP位置模式 | `RobStride_Motor_Pos_control(speed, angle)`             | 插补位置控制               |
| 速度模式   | `RobStride_Motor_Speed_control(speed, limit_cur)`       | 恒速控制                 |
| 电流模式   | `RobStride_Motor_current_control(current)`              | 恒流控制                 |
| 零点设置   | `Set_ZeroPos()`                                         | 设置当前角度为零点            |
| 读取参数   | `Get_RobStride_Motor_parameter(addr)`                   | 读取参数                 |
| 设置参数   | `Set_RobStride_Motor_parameter(addr, val, mode)`        | 设置参数                 |
| 协议切换   | `RobStride_Motor_MotorModeSet(F_CMD)`                   | 切换私有/Canopen/MIT协议   |

#### MIT协议相关

| 功能           | 接口名                                                                                                    |
| ------------ | ------------------------------------------------------------------------------------------------------ |
| MIT使能/失能     | `RobStride_Motor_MIT_Enable() / RobStride_Motor_MIT_Disable()`                                         |
| MIT综合控制      | `RobStride_Motor_MIT_Control(angle, speed, kp, kd, torque)`                                            |
| MIT位置/速度控制   | `RobStride_Motor_MIT_PositionControl(pos, speed)` / `RobStride_Motor_MIT_SpeedControl(speed, cur_lim)` |
| MIT清错        | `RobStride_Motor_MIT_ClearOrCheckError(cmd)`                                                           |
| MIT零点设置      | `RobStride_Motor_MIT_SetZeroPos()`                                                                     |
| MIT设置ID/运行模式 | `RobStride_Motor_MIT_SetMotorId(id)` / `RobStride_Motor_MIT_SetMotorType(type)`                        |
| MIT协议切换      | `RobStride_Motor_MIT_MotorModeSet(type)`                                                               |

---

#### 注意事项 Important Notes

- **协议切换**：如需切换协议，请首先通过 `RobStride_Motor_MotorModeSet(0x**)` 或 `RobStride_Motor_MIT_MotorModeSet(0x**)` 进行协议切换，并断电重启电机，方可使用。
- **MIT接口仅在 MIT 协议下有效**
- **标准帧 CAN ID 范围为 0x00\~0x7F**
- 控制参数如角度单位为弧度，速度单位为 rad/s，电流单位为 A。
