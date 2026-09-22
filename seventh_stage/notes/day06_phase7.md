
# 第七阶段 Day06 学习与工程记录

日期：2026-09-22

## 一、今日结论

今日重点为 STM32F407 执行端 SEQ+ACK+去重升级，以及 ESP32-S3 HTTPS OTA 板载测试。

已完成两道 C/C++ 代码题，并推进 OTA 至 V1.0.0 实机运行、V1.0.1 固件构建和 GitHub Release 发布阶段。

真实 V1.0.0 → V1.0.1 无线升级和 rollback 尚未验证，不将编译通过等同于 OTA 实机成功。

## 二、面试与笔试复习

完成 18 道综合面试题和 20 道笔试题。

重点复习：

- GPIO：RCC、MODER、OTYPER、PUPDR、BSRR。
- UART：RXNE、中断入口、HAL回调、重新开启接收。
- DMA+IDLE：DMA负责搬运，IDLE用于判断串口空闲；不能直接等同于完整协议帧。
- Cortex-M：硬件自动压栈 R0-R3、R12、LR、PC、xPSR；PendSV完成任务切换相关的软件上下文保存。
- FreeRTOS：FromISR API、portYIELD_FROM_ISR、优先级反转与互斥锁。
- Linux：termios、部分写、EINTR/EAGAIN、epoll LT/ET。
- 工程可靠性：业务SEQ、ACK、去重、幂等、STOP抢占。

## 三、代码训练

### 1. seq_dedup.c

路径：seventh_stage/day6/seq_dedup.c

功能：判断SEQ是否重复。

关键点：

- 首次SEQ不能误判为重复。
- 相同SEQ返回重复。
- 新SEQ更新记录。
- 使用 valid 标志区分未初始化状态。

运行结果：通过。

### 2. ack_tracker.cpp

路径：seventh_stage/day6/ack_tracker.cpp

功能：线程安全地跟踪等待中的ACK。

关键点：

- start(seq)记录期望SEQ并进入等待。
- 不匹配的ACK不能清除等待状态。
- 匹配的ACK清除等待状态。
- 使用mutex保护共享状态。
- const成员函数中的mutex使用mutable。

运行结果：ack_tracker passed。

## 四、STM32F407执行端升级③

协议从普通字符串升级为：

Linux → STM32：
SEQ:12 CMD:HAND_GRAB\r\n

STM32 → Linux：
ACK:12 IN_PROGRESS\r\n
ACK:12 OK\r\n

设计内容：

- hand_protocol_parse_frame()解析SEQ与动作。
- ACK状态：IN_PROGRESS、OK、PREEMPTED、BUSY、ERROR。
- 固定长度ACK缓存保存近期处理记录。
- 重复SEQ不再次执行动作，而是返回缓存状态。
- STOP抢占时，旧命令标记PREEMPTED。
- 普通命令在执行端忙碌时返回BUSY。
- UART中断负责逐字节收包、拼行和置标志；协议解析放在主循环。
- 主循环持续调用hand_servo_update()推进非阻塞状态机。

验证状态：

- 协议解析模块：Keil编译通过。
- ACK缓存及main.c整合：已编写，完整版本仍需最终编译确认。
- SEQ/ACK实机串口交互：待验证。
- Linux端ACK联动：待完成。

注意：当前STOP为软件动作抢占，不是硬件急停。

## 五、ESP32-S3 HTTPS OTA

### 1. OTA软件模块

已有：

- ota_manager
- ota_writer
- ota_downloader

ota_downloader使用HTTPS客户端分块读取固件，并调用ota_writer写入下一OTA分区。

新增下载重定向处理，供GitHub Release固件下载使用；修改后的完整构建及实机下载仍待验证。

### 2. 实机烧录与调试

使用Windows usbipd将CH340透传给WSL。

设备：
Windows COM3 → WSL /dev/ttyUSB0

解决串口权限问题后，成功运行idf.py flash monitor。

确认运行版本：
V1.0.0

确认分区：
factory：0x020000
ota_0：0x220000
ota_1：0x620000

### 3. WiFi调试

最初出现持续WiFi重连。

处理手机热点频段后，ESP32获得IP，WiFi连接成功。

### 4. OLED调试

MPU6050初始化正常，但OLED初始化失败。

为隔离OTA测试，临时将OLED设为非关键模块，使程序可以继续运行主循环。

这仅是测试策略，后续应恢复或明确调整正式健康检查标准。

### 5. V1.0.1固件

将PROJECT_VER改为1.0.1并构建。

保存固件：
~/ota_artifacts/aiot-smart-glasses-v1.0.1.bin

已发布到GitHub Release：
v1.0.1

注意：V1.0.1尚未通过USB烧入开发板；板载基线仍是V1.0.0。

## 六、尚未完成的验证

- GitHub Release HTTPS下载和重定向实测。
- V1.0.0一次性OTA触发。
- V1.0.1写入ota_0并重启。
- PENDING_VERIFY及mark VALID日志。
- 故意自检失败后的rollback实验。
- Linux/ROS2端SEQ发送、ACK接收、超时重传。
- STM32执行端完整ACK链路实机测试。

## 七、今日调试经验与面试可讲点

1. 编译通过不等于实机验证通过。
2. WSL访问USB设备需要完成设备透传与串口权限配置。
3. OTA应先保存目标版本bin，再重新构建带升级触发功能的旧版本。
4. OTA升级必须通过版本号、运行分区、自检与回滚状态形成证据链。
5. 业务ACK不等于TCP ACK；相同SEQ重传不能导致执行端重复动作。
6. 健康检查需要区分关键模块和可降级模块，不能把临时测试绕过描述成正式容错方案。