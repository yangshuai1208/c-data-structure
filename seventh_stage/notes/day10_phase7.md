# 第七阶段 Day10：底层高频强化与可靠通信复盘

日期：2026-09-28

## 一、今日目标

Day10继续围绕嵌入式秋招高频知识、底层原理和工程可靠性进行强化。

今日完成：

1. 20道综合嵌入式面试题
2. 16道笔试题
3. C语言UART环形缓冲区
4. C++ TCP长度字段拆包器
5. STM32执行端可靠通信逻辑复盘
6. ESP32 OTA流程复盘
7. ROS2可靠通信架构复盘
8. 项目综合小测

实机测试统一安排到Day15，Day11-Day14原计划保持不变。

---

## 二、面试题复盘

今日面试题覆盖：

- C语言
- C++
- Cortex-M
- STM32
- UART/DMA
- Timer/PWM
- I2C
- SPI
- CAN
- FreeRTOS
- Linux
- TCP
- MQTT
- ESP32 OTA
- ROS2
- 工程排障

### 1. volatile

volatile不能保证线程安全。

作用主要是告诉编译器：

该对象可能被正常程序流程之外的因素修改，
每次访问都应按照volatile语义进行实际读写。

典型场景：

- 硬件寄存器
- 中断与主程序共享的简单状态
- 某些异步修改对象

但volatile：

- 不保证原子性
- 不解决竞态条件
- 不能代替mutex
- 不能自动解决多核缓存一致性问题

---

### 2. const指针

```c
const int *p;
指向const int的指针，不能通过p修改对象。
int *const p;

常量指针，p本身不能重新指向其他对象。
const int *const p;

指针和所指对象都不能通过该声明修改。
3. struct内存对齐
结构体成员需要满足平台的对齐要求。
编译器可能：
- 在成员之间加入padding
- 在结构体末尾加入padding
结构体总大小通常是最大成员对齐要求的整数倍。
位域布局具有实现相关性，
因此不建议把普通C位域作为跨编译器通用的硬件寄存器映射方式。
4. unique_ptr / shared_ptr
unique_ptr：
- 独占所有权
- 不允许复制
- 支持移动
- 开销较小
shared_ptr：
- 共享所有权
- 使用引用计数
- 最后一个shared_ptr销毁时释放对象
循环引用：
两个对象相互持有shared_ptr可能导致引用计数无法归零。
解决方式：
使用weak_ptr打破循环。
5. condition_variable
condition_variable等待时需要配合谓词。
原因：
1. 可能发生虚假唤醒
2. 被唤醒并不代表业务条件一定成立
3. 多个线程之间可能竞争同一个条件
推荐：
cv.wait(lock, [] {
    return condition;
});

三、Cortex-M / STM32底层
1. STM32启动流程
典型流程：
上电 / Reset
↓
从向量表读取初始MSP
↓
读取Reset_Handler地址
↓
进入Reset_Handler
↓
初始化.data
↓
清零.bss
↓
SystemInit
↓
C/C++运行环境初始化
↓
main()
2. Cortex-M异常入栈
异常发生时，硬件基本异常栈帧自动保存：
- R0
- R1
- R2
- R3
- R12
- LR
- PC
- xPSR
R4-R11通常由软件在需要时保存。
异常返回后，处理器根据异常返回信息恢复现场。
3. UART循环DMA
循环DMA常见做法：
通过DMA剩余计数计算当前写位置：
current_pos = buffer_size - DMA_remaining

结合上次处理位置：
last_pos

判断本轮新收到的数据。
需要考虑：
- 回绕
- DMA覆盖未处理数据
- IDLE中断
- Half Transfer
- Transfer Complete
如果生产速度长期大于消费速度，
最终一定发生数据覆盖。
4. Timer / PWM
PSC：预分频
ARR：自动重装载值，决定周期
CCR：比较值，影响占空比
常见PWM频率：
PWM频率 =
定时器时钟 /
((PSC + 1) × (ARR + 1))

常见边沿对齐PWM模式下：
Duty ≈ CCR / (ARR + 1)

5. I2C和SPI
I2C：
- SDA + SCL
- 地址选择设备
- 一般需要上拉
- 半双工总线通信
SPI：
- SCLK
- MOSI
- MISO
- CS
- 通常支持全双工
- 不使用总线地址选择从设备
I2C出现NACK需要排查：
1. 设备地址
2. 7位/8位地址问题
3. 供电
4. 共地
5. SDA/SCL
6. 上拉
7. 总线时序
8. 从设备是否准备好
6. CAN仲裁
CAN通过显性位和隐性位进行无损仲裁。
显性0可以覆盖隐性1。
如果节点发送1，但读取总线得到0，
说明自己仲裁失败，退出本轮发送。
其他条件相同时：
标识符数值越小，优先级越高。
四、FreeRTOS
1. 阻塞态
任务可能因为以下原因进入Blocked：
- vTaskDelay
- Queue等待
- Semaphore等待
- EventGroup等待
- Notification等待
vTaskDelay主要等待时间。
Queue/Semaphore等主要等待事件或资源。
2. FromISR API
ISR中应该使用专门的FromISR接口，例如：
xQueueSendFromISR()

原因：
中断上下文不能像普通任务一样阻塞。
如果ISR唤醒了更高优先级任务，可以使用：
portYIELD_FROM_ISR()

请求退出中断后尽快进行任务切换。
3. Mutex和Binary Semaphore
Mutex：
- 保护共享资源
- 有所有权
- 通常支持优先级继承
Binary Semaphore：
- 常用于事件同步
- 不强调所有权
优先级继承主要缓解优先级反转。
五、Linux与网络
1. 进程和线程
进程通常拥有独立虚拟地址空间。
同一进程中的线程共享：
- 地址空间
- 全局变量
- 堆
- 文件描述符等资源
线程拥有自己的：
- 栈
- 寄存器上下文
- 调度状态
2. 阻塞 / 非阻塞 / IO多路复用
阻塞IO：
没有数据时线程可能睡眠等待。
非阻塞IO：
没有数据时立即返回，例如：
EAGAIN
EWOULDBLOCK

IO多路复用：
使用select、poll、epoll同时等待多个文件描述符。
3. TCP粘包和拆包
TCP是字节流协议。
因此：
一次send != 一次recv

可能出现：
- 半包
- 粘包
- 短写
- 短读
应用层必须自己定义消息边界。
常见方案：
- 固定长度
- 分隔符
- 长度字段
4. MQTT QoS
QoS 0：
最多一次。
QoS 1：
至少一次，可能重复。
QoS 2：
协议层提供恰好一次的消息交换流程。
即使使用QoS 1，
业务层仍然需要考虑重复执行问题。

六、代码题1：UART环形缓冲区
文件：
seventh_stage/day10/uart_ring_buffer.c

设计：
head = 下一个写位置
tail = 下一个读位置

空：
head == tail

满：
(head + 1) % capacity == tail

因为预留一个空槽：
数组容量 = 8
实际可保存 = 7字节

写入流程：
计算next_head
↓
判断是否满
↓
data[head] = value
↓
head = next_head

读取：
判断是否为空
↓
*out = data[tail]
↓
tail向后移动

同时增加：
overflow_count

记录写满导致的数据丢弃次数。
调试错误
最初错误包括：
rb->head = rb->data;

错误原因：
head是索引，不是指针。
正确：
rb->data[rb->head] = value;

读取时最初也出现了数组和指针方向写反的问题。
正确：
*out = rb->data[rb->tail];

七、代码题2：TCP长度字段拆包器
文件：
seventh_stage/day10/tcp_frame_decoder.cpp

协议：
[2字节大端Payload长度][Payload]

例如：
00 04 G R A B

长度：
0x0004 = 4

核心思路：
每次feed()收到的数据先加入：
buffer_

然后循环：
缓存是否至少有2字节？
↓
解析长度
↓
长度是否合法？
↓
完整帧是否到齐？
↓
提取Payload
↓
删除已消费数据
↓
继续解析下一帧

支持：
- 完整帧
- TCP半包
- TCP粘包
- 多帧连续解析
- 非法长度
- 错误后reset恢复
大端解析：
uint16_t payload_len =
    (static_cast<uint16_t>(buffer_[0]) << 8U) |
    static_cast<uint16_t>(buffer_[1]);

重点：
feed()中的size是本次TCP收到的数据量，
不能使用：
size > 32

判断协议非法。
真正限制的是：
Payload长度 <= 32

因为一次recv完全可能包含多个粘在一起的帧。
八、STM32执行端可靠通信复盘
执行链路：
UART DMA
→ 协议解析
→ SEQ检查
→ ACK
→ 非阻塞状态机
→ PCA9685
→ 舵机动作

新SEQ
第一次出现：
SEQ:101 CMD:HAND_GRAB

进入正常执行。
重复SEQ + 相同命令
不能再次执行动作。
应该：
读取缓存结果并返回ACK。
目的：
实现幂等。
重复SEQ + 不同命令
例如：
SEQ:101 HAND_GRAB
SEQ:101 HAND_OPEN

属于协议冲突。
不能作为正常重传处理。
STOP
STOP拥有更高软件优先级。
用于尽快终止或覆盖当前普通动作状态机。
注意：
软件STOP != 硬件安全急停。
九、ROS2可靠通信
已经通过PTY模拟验证：
ROS2命令
→ SEQ
→ UART发送
→ ACK模拟丢失
→ Timeout
→ 同SEQ重传
→ IN_PROGRESS
→ OK
→ SUCCEEDED

架构：
TX Worker
负责串口发送。
RX Worker
负责ACK接收和解析。
Retry Worker
负责超时判断及重传。
优势：
避免在ROS2订阅回调中阻塞等待串口ACK。
十、ESP32 OTA复盘
OTA流程：
V1.0.0
↓
检查V1.0.1
↓
选择备用OTA分区
↓
esp_ota_begin
↓
HTTPS下载
↓
esp_ota_write
↓
esp_ota_end
↓
esp_ota_set_boot_partition
↓
Restart
↓
启动V1.0.1
↓
自检
↓
确认新版本有效

双OTA分区的核心意义：
升级新固件时保留旧的可运行固件，
为升级失败和回滚保留恢复能力。
esp_ota_end()成功不代表整个OTA完成。
还需要：
1. 设置启动分区
2. 重启
3. 成功运行新版本
4. 必要自检
5. 启用回滚时确认新版本有效
十一、项目小测重点
为什么重传必须使用原SEQ？
如果重新生成SEQ：
执行端可能认为这是一个新命令，
造成机械动作重复执行。
ACK OK是否证明舵机真实到位？
不能。
ACK是软件协议状态。
真实物理动作需要：
- 位置反馈
- 编码器
- 电流检测
- 限位
- 其他传感器
进行闭环确认。
SEQ/ACK属于哪一层？
TCP提供可靠字节流。
MQTT提供消息传输和QoS。
项目中的SEQ/ACK属于应用层业务可靠机制。
用于：
- 命令唯一标识
- 超时重传
- 去重
- 状态跟踪
- 幂等执行
十二、Day10笔试错题
笔试成绩：
14 / 16

错题1：Cortex-M启动
向量表前两个字通常为：
1. 初始MSP
2. Reset_Handler地址
不是PSP。
错题2：CAN仲裁
其他条件相同时：
CAN Identifier越小
→ 仲裁优先级越高

原因：
显性0覆盖隐性1。
十三、今日薄弱点
需要继续复习：
1. Cortex-M启动流程
2. MSP / PSP
3. 异常自动入栈
4. CAN无损仲裁
5. Timer/PWM公式
6. OTA真正成功的判定
7. 应用层SEQ/ACK与TCP/MQTT的区别