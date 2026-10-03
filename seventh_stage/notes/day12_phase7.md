# 第七阶段 Day12：并发安全、CRC、ACK状态管理与异常恢复

日期：2026-10-03

## 一、今日完成

Day12围绕并发安全、通信完整性、ACK状态管理、ROS2/Linux Gateway异常恢复展开。

今日完成：

1. 20道嵌入式综合面试题
2. 16道笔试题
3. Day11顺延代码：BlockingQueue
4. Day12代码：CRC16-Modbus
5. Day12代码：ACK状态表
6. Day11顺延6道项目强化题
7. Day12四道真实工程场景题

笔试成绩：

14 / 16

三道代码题均已实际编译运行通过：

- blocking_queue.cpp
- crc16_modbus.c
- ack_table.cpp

实机测试仍统一安排到Day15。

---

## 二、数组与指针

函数参数：

```c
void func1(int a[10]);
void func2(int *a);
在函数参数位置基本等价。
数组作为函数参数时会调整为指针，因此函数内部：
sizeof(a)

得到的是指针大小，而不是调用者原数组的总大小。
因此工程中通常需要额外传入：
void func(int *data, size_t size);

面试回答
数组在函数参数位置会退化为指针，因此int a[10]和int *a在这里基本等价。函数内部不能通过sizeof(a)获得原数组长度，所以一般需要额外传入长度。
三、野指针和悬空指针
野指针
指针没有被正确初始化，指向未知或非法地址。
例如：
int *p;
*p = 10;

悬空指针
指针原来指向有效对象，但对象已经释放或生命周期结束。
例如：
int *p = malloc(sizeof(int));

free(p);

*p = 10;

属于：
Use After Free。
降低风险：
free(p);
p = NULL;

但需要注意：
如果还有其他指针副本指向同一块内存，它们仍可能悬空。
四、函数指针
定义：
typedef void (*command_handler_t)(void);

表示：
定义一个名为command_handler_t的函数指针类型。
它可以指向：
void func(void);

类型的函数。
例如：
command_handler_t handler = hand_stop;

handler();

嵌入式中常用于：
- 命令分发
- 驱动回调
- 状态机
- 中断回调
- 协议处理
例如：
CMD_OPEN -> open_handler
CMD_GRAB -> grab_handler
CMD_STOP -> stop_handler

相比大量switch-case，可以降低模块耦合。
五、大小端
假设：
uint32_t value = 0x12345678;

大端
高有效字节存放在低地址：
12 34 56 78

小端
低有效字节存放在低地址：
78 56 34 12

典型x86平台通常采用小端。
网络协议一般明确规定字节序。
TCP/IP网络字节序通常采用大端。
原因：
不同CPU可能具有不同本地字节序，如果协议不统一，多字节整数可能被错误解释。
六、atomic和mutex
例如：
std::atomic<int> count{0};

count++;

对于这个atomic变量本身：
count++是原子操作。
适合：
- 简单计数
- Flag
- 单个状态值
但是如果多个变量必须满足整体一致性：
a
b
状态之间存在约束

单独使用atomic不能自动保证整个临界区一致。
此时更适合：
std::mutex

面试回答
atomic适合简单单变量的原子操作，mutex适合保护一整个临界区以及多个变量之间的一致性。atomic不能自动让复杂数据结构整体线程安全。
七、多Mutex死锁
典型：
Thread A:
lock M1
lock M2

Thread B:
lock M2
lock M1

可能出现：
A持有M1等待M2
B持有M2等待M1

形成死锁。
工程上可以：
1. 统一加锁顺序
2. 减少嵌套锁
3. 使用std::lock
4. 使用std::scoped_lock
例如：
std::scoped_lock lock(m1, m2);

八、NVIC
NVIC：
Nested Vectored Interrupt Controller
负责Cortex-M异常和中断的：
- 使能
- 禁止
- 挂起
- 优先级
- 嵌套
抢占优先级
决定：
当前中断是否能被另一个更高优先级中断抢占。
子优先级/响应优先级
当多个具有相同抢占优先级的中断同时等待时，决定谁先被响应。
它本身通常不决定嵌套抢占。
九、HardFault随机发生
如果HardFault每次PC位置都不同，应优先怀疑：
- 栈溢出
- 数组越界
- 野指针
- 悬空指针
- 错误函数指针
- 内存越界写
- 并发访问破坏内存
这种问题经常表现为：
错误发生位置与真正造成内存破坏的位置不同。
因此需要：
PC / LR
CFSR
HFSR
BFAR
MMFAR
Stack
MAP
反汇编

综合定位。
十、UART循环DMA绕环问题
如果：
Buffer Size = 128

只保存：
last_pos
current_pos

如果DMA在两次检查之间已经完整绕环一次甚至多次：
仅凭这两个位置无法知道真正产生了多少数据。
例如：
last_pos = 20
current_pos = 20

可能表示：
没有新增数据

也可能：
已经完整绕了一圈或多圈

所以实际工程中需要：
- Half Transfer
- Transfer Complete
- IDLE
- 累计计数
- 环形缓冲
- 溢出检测
十一、I2C SDA被拉低
如果：
SDA一直为低

可能原因：
- 从设备状态机卡死
- MCU在一次I2C事务中途复位
- 从设备等待剩余Clock
- 总线短路
- 从设备故障
一种常见恢复方法：
暂时将SCL配置为GPIO
↓
手动产生若干个Clock
↓
让从设备继续完成残余事务
↓
尝试释放SDA
↓
产生STOP
↓
重新初始化I2C

常见会尝试约9个时钟。
如果仍不能恢复：
考虑复位或重新上电从设备。
十二、FreeRTOS任务栈
任务栈保存：
- 局部变量
- 函数调用现场
- 保存寄存器
- 返回地址
- 上下文切换现场
任务栈不存程序机器代码。
动态创建任务时：
xTaskCreate()

任务控制块和任务栈通常由FreeRTOS Heap分配。
静态创建时：
可以由用户提供内存。
栈溢出可能导致
- HardFault
- 随机崩溃
- 变量损坏
- 任务异常
- 系统行为不可预测
检测
uxTaskGetStackHighWaterMark()

以及：
configCHECK_FOR_STACK_OVERFLOW

和栈溢出Hook。
十三、FreeRTOS Queue
Queue既可以：
传递数据
也可以：
实现任务同步。
相比：
共享全局变量 + Mutex

Queue的优势：
- 生产者消费者解耦
- 数据所有权更清晰
- 支持阻塞等待
- 可唤醒任务
- 支持FromISR版本
- 降低共享变量直接访问
十四、portYIELD_FROM_ISR
假设：
低优先级任务正在运行。
ISR中：
xQueueSendFromISR()

唤醒了一个更高优先级任务。
如果一直等到下一个Tick再调度：
会增加高优先级任务响应延迟。
因此：
portYIELD_FROM_ISR()

用于请求：
ISR退出后尽快进行上下文切换。
面试回答
如果ISR唤醒了比当前任务优先级更高的任务，可以通过portYIELD_FROM_ISR()请求在中断退出后尽快进行任务切换，从而降低实时任务响应延迟。
十五、共享状态检查必须加锁
错误：
if (!queue_empty)
{
    pthread_mutex_lock(&mutex);

    pop();

    pthread_mutex_unlock(&mutex);
}

问题：
检查：
queue_empty

发生在锁外。
检查完成后到真正加锁之间：
另一个线程可能修改Queue。
这是典型：
TOCTOU
Time Of Check To Time Of Use。
正确：
lock
↓
检查queue
↓
修改queue
↓
unlock

也就是说：
检查共享状态和使用共享状态必须处在同一个同步范围中。
十六、VMIN和VTIME
termios中：
VMIN
控制读取返回前期望获得的最少字符数。
VTIME
参与串口读取超时控制。
单位一般为：
0.1秒。
特别情况：
VMIN = 0
VTIME = 0

一般表示：
read立即返回当前已有数据。
如果当前没有数据：
也可以立即返回0。
十七、select
select()会修改传入的：
fd_set

返回以后：
集合中通常只保留当前就绪FD。
因此下一次调用之前需要重新构造：
FD_ZERO()
FD_SET()

等集合。
select性能问题
大量FD时：
- 每次需要准备集合
- 用户态/内核态复制
- 线性扫描FD集合
- FD数量通常存在限制
因此大量连接时通常更适合：
epoll

十八、TCP与应用层CRC
TCP本身提供：
- 可靠字节流
- 顺序传输
- 重传
- TCP校验
如果业务数据只存在于一条纯TCP链路中：
额外CRC价值可能有限。
但如果数据经过：
TCP
↓
Linux Gateway
↓
UART
↓
STM32

或者：
Flash
RS485
其他链路

应用层CRC可以提供：
端到端的数据完整性检查。
十九、MQTT重连问题
MQTT断线后重新连接成功：
并不代表业务状态自动恢复。
可能出现：
- QoS1重复消息
- 离线期间消息丢失
- Retain消息重新接收
- 云端状态与本地状态不同步
- STM32仍然执行旧动作
工程处理：
- 业务SEQ
- 请求ID
- 幂等
- Session
- QoS
- Retain
- 状态查询
- 重连后重新同步
二十、ROS2 Topic、Service和Action
Topic
发布订阅。
特点：
- 异步
- 一对多/多对多
- 适合持续数据流
例如：
传感器数据。
Service
请求-响应。
适合：
查询当前执行状态

Action
更适合：
持续时间较长，并需要：
- 反馈
- 最终结果
- Cancel
的任务。
对于机器人机械手动作：
ROS2 Action在工程上通常比单纯Topic更适合长动作控制。
当前项目使用：
Topic发送动作
Service查询状态

也可以形成可工作的基本架构。
二十一、BlockingQueue
文件：
seventh_stage/day12/blocking_queue.cpp

已经实际运行通过。
核心结构：
std::deque<T> queue_;

std::mutex mutex_;

std::condition_variable not_empty_;
std::condition_variable not_full_;

bool stopped_;

push()
lock
↓
Queue满
↓
等待not_full
↓
检查stopped
↓
push_back
↓
notify not_empty

核心：
not_full_.wait(lock, [this] {
    return stopped_ ||
           queue_.size() < capacity_;
});

使用predicate是为了：
- 防止虚假唤醒
- 重新检查真实条件
二十二、为什么condition_variable使用unique_lock
std::unique_lock<std::mutex>

支持：
wait前释放Mutex
↓
线程睡眠
↓
唤醒
↓
重新获取Mutex

condition_variable::wait()需要这种灵活锁控制。
二十三、BlockingQueue stop()
停止时：
stopped_ = true;

并且：
not_empty_.notify_all();
not_full_.notify_all();

为什么两个都要通知？
因为可能存在：
Consumer等待Queue非空
Producer等待Queue非满

如果只唤醒一边：
另一边可能永远阻塞。
面试回答
我使用mutex保护Queue，用两个condition_variable分别等待非空和非满条件。wait使用predicate避免虚假唤醒。停止时设置stopped标志并notify_all唤醒所有Producer和Consumer，使所有线程能够安全退出。
二十四、CRC16-Modbus
文件：
seventh_stage/day12/crc16_modbus.c

已实际运行通过。
参数
初值：
0xFFFF

多项式：
0xA001

算法
对每个字节：
crc ^= data[i];

然后处理8个bit：
if (crc & 0x0001U)
{
    crc =
        (crc >> 1U) ^ 0xA001U;
}
else
{
    crc >>= 1U;
}

二十五、CRC是双层循环
外层：
for (size_t i = 0; i < size; ++i)

遍历数据字节。
内层：
for (int bit = 0; bit < 8; ++bit)

遍历当前字节对应的bit处理过程。
已知测试：
"123456789"

CRC16-Modbus：
0x4B37

二十六、Modbus CRC字节顺序
假设：
CRC = 0x4B37

帧中通常发送：
37 4B

即：
低字节在前。
恢复：
uint16_t expected_crc =
    frame[crc_pos] |
    ((uint16_t)frame[crc_pos + 1] << 8);

二十七、CRC能做什么，不能做什么
CRC可以：
检测偶然传输错误。
CRC不能：
- 加密
- 身份认证
- 防止攻击者修改
- 防止重复命令
即：
CRC != HMAC
CRC != Signature
CRC != SEQ

如果需要确认发送方可信：
应该使用：
- HMAC
- MAC
- 数字签名
- 其他认证机制
二十八、AckTable
文件：
seventh_stage/day12/ack_table.cpp

已实际运行通过。
核心：
struct AckEntry
{
    uint32_t seq;
    AckState state;
};

状态：
Waiting
Executing
Succeeded
Failed

二十九、insert()
逻辑：
查找是否重复SEQ
↓
判断容量
↓
插入

重复SEQ：
return false

容量满：
return false

否则：
entries_.push_back(...)

三十、update()
根据SEQ查找：
SEQ存在
→ 更新state

SEQ不存在
→ false

例如：
Waiting
↓
Executing
↓
Succeeded

对应：
发送
↓
IN_PROGRESS
↓
OK

三十一、find()
找到：
state = entry.state;
return true;

没找到：
return false;

并且：
找不到时不能修改输出参数。
这是很重要的API设计原则：
失败路径不要产生意外副作用。
三十二、erase()
根据SEQ找到记录：
entries_.erase(it);

然后：
return true;

不存在：
return false;

三十三、Tick回绕
推荐：
(uint32_t)(now - start) >= timeout

而不是：
now >= start + timeout

原因：
UINT32_MAX
↓
0

会发生回绕。
使用无符号减法：
能够利用模运算处理跨回绕时间差。
三十四、持有Mutex时不要长时间阻塞
错误设计：
lock
↓
blocking read
↓
recv
↓
delay
↓
unlock

问题：
其他任务无法获得Mutex。
可能导致：
- 响应延迟
- 优先级反转
- 模块卡死
- 系统吞吐下降
正确：
lock
↓
复制/修改必要共享数据
↓
unlock
↓
执行耗时操作

三十五、write/send成功不等于业务成功
Linux：
write()

成功。
只代表：
数据已被本地内核接受或处理到某一层。
并不能保证：
UART真正发送
STM32真正收到
协议解析成功
命令被接受
机械动作执行成功

因此需要：
SEQ
ACK
Timeout
Retry

形成业务闭环。
三十六、IN_PROGRESS为什么用更长超时
首次：
等待ACK

只确认：
设备是否收到命令。
通常很快。
收到：
IN_PROGRESS

说明进入实际执行阶段。
此时可能涉及：
- 舵机动作
- 状态机推进
- I2C通信
- PWM变化
所以：
Execution Timeout
>
First ACK Timeout

三十七、FAILED请求的幂等处理
假设：
SEQ=200
CMD=GRAB
FAILED

再次收到：
SEQ=200
CMD=GRAB

应该：
返回缓存FAILED。
不能重新执行。
原因：
同SEQ = 同一业务请求

必须保持幂等。
如果用户真的想重新执行：
应该生成：
新的SEQ

例如：
SEQ=201
CMD=GRAB

三十八、有界Queue与Backpressure
场景：
ROS2:
GRAB
OPEN
GRAB
STOP

STM32执行速度明显更慢。
Linux Gateway不能无限缓存。
应该使用：
Bounded Queue

例如：
capacity = 4

Queue满：
可以：
- 返回BUSY
- 拒绝新命令
- 根据业务丢弃未执行旧命令
- 上游限流
这就是：
Backpressure。
三十九、STOP优先级
STOP不能简单排在：
普通命令Queue尾部。
应该拥有：
更高软件优先级。
可以：
STOP插队

或者：
独立高优先级通道

并根据业务：
取消/清理待执行普通命令

注意：
软件STOP不是硬件急停。
四十、IN_PROGRESS后UART断线
场景：
SEQ301 GRAB
↓
IN_PROGRESS
↓
UART断开

不能立即认为：
FAILED

因为STM32可能仍然在执行。
正确：
连接恢复
↓
重新查询STM32
↓
获取当前SEQ和状态
↓
重新同步

如果无法确认最终状态：
更合理的是：
UNKNOWN
TIMEOUT

而不是直接认定物理动作失败。
四十一、MQTT重复消息
不能只根据：
"GRAB"

判断是否重复。
因为：
GRAB

可能是用户两次合法的新请求。
应该使用：
Business Request ID
或
SEQ

来识别同一个业务请求。
MQTT去重
解决：
消息传输层重复。
STM32幂等
解决：
机械执行层重复。
二者不是一回事。
四十二、Gateway崩溃恢复
场景：
Linux发送SEQ500
↓
STM32 IN_PROGRESS
↓
Linux进程崩溃
↓
Linux重启

Linux内存中的：
AckTable
RetryState

已经丢失。
但STM32可能仍然执行。
因此启动后应该：
UART连接
↓
HELLO / STATUS_QUERY
↓
查询STM32当前SEQ
↓
查询当前CMD
↓
查询执行状态
↓
重新构建Linux状态

还可以增加：
状态持久化
例如：
- 文件
- SQLite
- WAL
- Journal
Session ID
使用：
session_id + seq

防止设备/网关重启后的SEQ冲突。
最终原则：
本地持久化不能代替设备状态重新同步。

四十三、Day12核心工程认识
今天最重要的几个结论：
CRC正确
!=
设备可信

write成功
!=
STM32执行成功

Timeout
!=
命令没有执行

UART断线
!=
机械动作失败

Gateway重启
!=
STM32状态清零

相同命令字符串
!=
同一个业务请求

因此真实系统需要：
SEQ
ACK
CRC
Retry
Idempotency
Bounded Queue
Backpressure
State Query
Session
Resync

四十四、Day12代码验证
以下代码均已使用本地环境实际编译运行通过：
blocking_queue.cpp
crc16_modbus.c
ack_table.cpp

说明Day12代码训练已经完成。
四十五、Day12薄弱点
后续继续复习：
1. 数组参数退化
2. 大小端
3. atomic与mutex
4. NVIC抢占优先级与子优先级
5. FreeRTOS任务栈
6. portYIELD_FROM_ISR
7. I2C总线恢复
8. ROS2 Topic / Service / Action
9. Gateway状态恢复
10. Backpressure
四十六、Day12总结
Day12从单纯的协议可靠性继续提升到：
多线程并发安全
+
数据完整性
+
请求状态管理
+
断线恢复
+
业务幂等
+
状态重新同步

BlockingQueue解决线程间安全传递与退出；
CRC16解决通信数据错误检测；
AckTable解决SEQ与ACK状态映射；
而在工程层进一步加入：
Backpressure
状态查询
Session
Gateway恢复

使Linux Gateway、ROS2与STM32之间的可靠通信设计更加完整。