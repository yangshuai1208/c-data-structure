# 第七阶段 Day11：并发、异常处理与可靠通信强化

日期：2026-09-30

## 一、今日完成

Day11围绕嵌入式底层、并发、Linux IO、可靠通信和异常处理展开。

今日完成：

1. 20道嵌入式综合面试题
2. 16道笔试题
3. C语言超时重试状态机
4. Day10薄弱知识复习
5. SEQ/ACK、Timeout、Retry工程逻辑强化

笔试成绩：

16 / 16

C代码：

retry_state_machine.c

已使用MSVC实际编译运行通过。

---

## 二、C语言重点

### 1. static局部变量

static局部变量具有静态存储期。

生命周期：

程序启动到程序结束。

作用域：

仍然局限在定义它的函数或代码块。

未初始化或初始化为0的static变量通常放在：

.bss

非零初始化static变量通常放在：

.data

static本身不能保证多线程安全。

---

### 2. 一级指针和二级指针

一级指针：

```c
char *p;
可以通过p访问或修改其指向的数据。
二级指针：
char **p;

常用于需要在函数内部修改调用者一级指针本身的场景。
例如：
- 动态分配内存
- 修改链表头指针
- 修改指针指向
3. 程序内存布局
.text：
程序机器指令。
.rodata：
只读常量、字符串字面量等。
.data：
已初始化且非零的全局变量和static变量。
.bss：
未初始化或零初始化的全局变量和static变量。
heap：
动态内存区域，由程序动态申请和释放。
stack：
局部变量、函数参数、返回地址、保存的寄存器和调用现场等。
.bss中的大量零数据通常不需要完整存储在固件镜像中，
启动阶段根据大小统一清零。
4. 宏副作用
例如：
#define MAX(a, b) ((a) > (b) ? (a) : (b))

如果调用：
MAX(x++, 10)

宏参数可能被多次求值。
因此：
带副作用的表达式不适合作为普通宏参数。
C语言中可以考虑：
- inline函数
C++中可以考虑：
- 模板
- std::max
三、C++与RAII
RAII：
Resource Acquisition Is Initialization
即：
资源获取即初始化。
核心思想：
将资源生命周期绑定到对象生命周期。
构造时获取资源，
析构时释放资源。
适合管理：
- mutex
- file descriptor
- socket
- memory
例如：
std::lock_guard<std::mutex> lock(mutex);

作用域结束后自动释放锁。
四、STM32 HardFault定位
发生HardFault后需要重点查看：
PC
异常发生时正在执行的指令地址。
可结合：
- map文件
- elf
- 反汇编
定位具体源码。
LR
帮助分析函数调用关系和异常返回状态。
CFSR
Configurable Fault Status Register。
包含：
- MemManage Fault
- BusFault
- UsageFault
等具体错误原因。
HFSR
HardFault Status Register。
可以判断是否由其他Fault升级成HardFault。
BFAR
Bus Fault Address Register。
当BFARVALID有效时，
可以提供导致BusFault的地址。
MMFAR
MemManage Fault Address Register。
当MMARVALID有效时，
可以提供内存管理异常访问地址。
五、中断ISR原则
ISR应该尽量短。
不推荐在ISR中进行：
- 长时间printf
- HAL_Delay
- JSON解析
- 大量计算
- 阻塞等待
原因：
ISR过长会：
- 增加中断延迟
- 影响系统实时性
- 延迟其他中断
- 影响RTOS任务调度
推荐方式：
ISR只完成：
1. 读取必要数据
2. 清除中断标志
3. 设置标志
4. 使用FromISR API通知任务
复杂处理放到任务上下文。
六、DMA并发访问
DMA传输时CPU可以访问同一块内存。
但存在风险：
- CPU和DMA同时修改数据
- CPU读取尚未完成的数据
- DMA覆盖未消费数据
- Cache一致性问题
- 数据所有权不清晰
工程上可以通过：
- 双缓冲
- 环形缓冲区
- head/tail
- DMA Half Transfer
- Transfer Complete
- IDLE
- 临界区
- Cache维护
进行处理。
不能简单认为：
CPU不能访问DMA缓冲区。
七、UART协议设计
常见串口帧：
Frame Header
Length
Payload
CRC
Frame Tail

作用：
Frame Header：
寻找帧起点。
Length：
确定Payload长度。
Payload：
实际业务数据。
CRC：
检查传输过程中是否出现数据损坏。
Frame Tail：
辅助确定帧结束。
如果Payload可能出现特殊帧头或帧尾字节，
不能直接丢弃数据。
可以使用：
- 长度字段
- 转义
- 字节填充
- COBS
- SLIP
等方式解决。
八、CRC
CRC主要用于：
数据传输错误检测。
CRC不能保证：
- 数据绝对正确
- 数据来自可信设备
- 防止恶意篡改
- 命令只执行一次
因此：
CRC != Authentication
CRC != Encryption
CRC != SEQ去重
九、FreeRTOS任务切换
典型发生场景：
1. Tick中断到来
2. 当前任务时间片结束
3. 当前任务调用阻塞API
4. 当前任务主动让出CPU
5. 更高优先级任务变为Ready
6. ISR唤醒更高优先级任务
FreeRTOS核心调度原则：
优先运行最高优先级的Ready任务。
十、死锁
典型情况：
Task A:
M1
M2

Task B:
M2
M1

A持有M1等待M2，
B持有M2等待M1，
形成循环等待。
工程上常见解决方案：
- 统一加锁顺序
- 减少嵌套锁
- 缩小临界区
- 必要时使用超时
- 简化共享资源设计
十一、优先级反转
典型情况：
Low持有Mutex
High等待Mutex
Medium持续抢占Low

Low无法及时运行释放锁，
最终High被间接阻塞。
Mutex的优先级继承：
临时提升Low的优先级，
使Low尽快完成临界区并释放Mutex。
优先级继承主要缓解优先级反转，
不能解决所有死锁问题。
十二、Linux mutex
pthread_mutex并不是直接“保护某个变量”。
它提供的是互斥机制。
程序员约定：
访问某组共享资源之前，
必须获取同一把mutex。
因此真正被保护的是：
共享资源对应的临界区。
十三、Linux read返回值
read() > 0

表示实际读取到的字节数。
read() == 0

对于TCP Socket通常表示：
对端已经有序关闭连接。
errno == EINTR

系统调用被信号中断。
需要根据操作语义决定是否重新调用。
errno == EAGAIN

或：
EWOULDBLOCK

对于非阻塞FD表示当前没有数据，
稍后再试。
十四、TCP短写
一次：
send(fd, buf, len, 0);

不保证全部发送完成。
如果返回：
0 < n < len

应该继续发送剩余内容：
buf += n
len -= n

直到：
- 全部发送
- 出现可处理的EINTR/EAGAIN
- 出现不可恢复错误
十五、epoll LT和ET
LT
Level Trigger。
只要FD仍然处于就绪状态，
就可能持续产生通知。
ET
Edge Trigger。
通常在状态发生变化时通知。
ET模式一般需要：
non-blocking fd
+
循环read
+
直到EAGAIN

否则可能留下未读取的数据，
却不能及时再次获得通知。
十六、SEQ与幂等
场景：
Linux发送：
SEQ:100 CMD:HAND_GRAB

STM32执行成功

ACK丢失

Linux超时重传：
SEQ:100 CMD:HAND_GRAB

执行端不能重新执行GRAB。
正确做法：
1. 使用原SEQ
2. 查询请求缓存
3. 发现相同SEQ和CMD
4. 不重复执行
5. 返回已有状态或最终ACK
这样实现业务幂等。
十七、OTA掉电安全
假设OTA写入备用分区到70%时断电。
合理的A/B升级流程：
旧固件仍然保留完整。
如果还没有修改boot partition，
设备重新启动时继续进入之前已经确认有效的旧固件。
正确流程：
下载新固件
↓
完整写入备用分区
↓
镜像检查成功
↓
修改Boot Partition
↓
Restart
↓
运行新固件
↓
Self Test
↓
Mark Valid

不能在新固件完整写入前就提前修改启动分区。
十八、Day11代码题：Retry State Machine
文件：
seventh_stage/day11/retry_state_machine.c

状态：
REQ_IDLE
REQ_WAIT_ACK
REQ_EXECUTING
REQ_SUCCEEDED
REQ_FAILED

核心流程：
首次发送
↓
WAIT_ACK
↓
Timeout
↓
Retry #1
↓
Timeout
↓
Retry #2
↓
Timeout
↓
FAILED

如果收到：
IN_PROGRESS

状态：
WAIT_ACK
→
EXECUTING

同时使用更长的执行阶段超时。
如果收到：
OK

状态：
SUCCEEDED

成功或失败状态都不能继续重试。
十九、Tick回绕
错误方式：
now_ms >= last_send_ms + timeout_ms

当uint32_t计时器接近UINT32_MAX时，
加法可能发生回绕。
推荐：
(uint32_t)(now_ms - last_send_ms)

再与timeout比较。
例如：
last = UINT32_MAX - 100
now  = 50

虽然now数值比last小，
但无符号减法仍可以得到正确的经过时间。
这是嵌入式系统Tick超时判断的重要写法。
二十、Day11代码验证
retry_state_machine.c已完成：
- 初始化测试
- 未超时测试
- 第一次重试
- 第二次重试
- 超过重试次数进入FAILED
- IN_PROGRESS
- OK
- 成功状态禁止重试
- uint32_t Tick回绕
- NULL参数
使用MSVC实际编译运行成功：
retry_state_machine passed

二十一、Day11笔试
成绩：
16 / 16

重点覆盖：
- .bss
- 数组越界
- static
- RAII
- HardFault
- ISR
- DMA
- CRC
- Deadlock
- Mutex
- TCP关闭
- EINTR
- send短写
- epoll ET
- SEQ幂等
- OTA掉电保护
二十二、今日薄弱点
后续继续复习：
1. HardFault寄存器定位
2. FreeRTOS任务切换条件
3. DMA与CPU并发访问
4. rodata / data / bss / stack
5. 宏参数副作用
6. epoll LT / ET
7. UART转义与协议成帧
二十三、顺延任务
以下内容顺延到Day12：
1. blocking_queue.cpp
2. Day11项目软件强化6题
Day12原计划保持不变，
以上两项作为额外补充任务加入Day12。
Day15继续统一进行：
- STM32 F407执行端实机
- SEQ/ACK去重
- STOP抢占
- PCA9685/MG90S动作
- ESP32-S3真实OTA
- ROS2/Linux/STM32三端联调