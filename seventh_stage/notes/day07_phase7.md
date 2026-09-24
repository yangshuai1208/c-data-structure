
# 第七阶段 Day07：综合训练与执行端DMA升级

## 一、今日完成内容

1. 完成18道嵌入式综合模拟面试题并复盘。
2. 完成20道综合笔试题并复盘。
3. 完成C语言UART流式帧解析器，编译成功。
4. 完成C++17超时重试状态机，编译成功。
5. 完成STM32F407 USART1_RX Circular DMA配置。
6. 将执行端接收方式由单字节中断升级为DMA + IDLE接收。
7. 接入256字节DMA缓冲区和512字节软件环形缓冲区。
8. 保留SEQ去重、ACK缓存、STOP抢占及非阻塞舵机状态机。
9. STM32工程Keil Rebuild成功。

## 二、C语言代码题：流式帧解析器

文件：day07/frame_stream_parser.c

协议格式：

HEAD | LEN | PAYLOAD | CHECKSUM | TAIL

- 帧头：0xAA
- 最大载荷：16字节
- 校验方式：LEN与载荷字节逐个异或
- 帧尾：0x55

使用enum表示解析状态，struct保存接收状态和载荷。

通过parser_feed()逐字节处理数据，完整合法帧才返回true。

重点纠错：

- enum与struct不能混淆。
- 赋值使用=，比较使用==。
- uint8_t不需要判断是否小于0。
- 必须限制载荷长度，防止数组越界。
- 错误帧需要重新同步。

## 三、C++代码题：超时重试状态机

文件：day07/retry_policy.cpp

核心成员：

- expected_seq_：当前请求SEQ
- last_send_ms_：最近一次发送时间
- retries_used_：已使用的重试次数
- state_：当前请求状态

核心接口：

- start()
- on_ack()
- on_timeout()

规则：

- 重传保持相同SEQ。
- IN_PROGRESS不是最终成功。
- OK将状态更新为SUCCEEDED。
- PREEMPTED、BUSY、ERROR将状态更新为FAILED。
- 最大重试次数为3次，首次发送不计入重试。
- 请求结束后忽略迟到ACK。

本题通过模拟时间进行测试，不依赖真实sleep。

## 四、STM32执行端DMA升级

原接收方式：

HAL_UART_Receive_IT()单字节中断接收。

升级后：

USART1 → Circular DMA → DMA接收缓冲区
→ 软件环形缓冲区 → 主循环解析完整命令。

关键API：

- MX_DMA_Init()
- HAL_UARTEx_ReceiveToIdle_DMA()
- HAL_UARTEx_RxEventCallback()
- HAL_DMA_IRQHandler()
- HAL_UART_IRQHandler()

DMA配置：

- DMA2 Stream2 / Channel4
- Peripheral to Memory
- Circular模式
- Memory Increment开启
- Byte数据宽度

DMA缓冲区：256字节。
软件环形缓冲区：512字节。

DMA回调中只搬运新增数据，不执行阻塞式ACK发送或舵机动作。

IDLE表示接收线路出现空闲，不直接等于业务帧完整。
当前业务协议仍通过换行符识别完整命令。

原有SEQ、ACK缓存、STOP抢占与非阻塞动作状态机保留。

## 五、今日错题与面试考点

1. 结构体对齐需要考虑成员填充和尾部填充。
2. memcpy不能用于源目标重叠场景，应使用memmove。
3. release/acquire用于建立跨线程同步关系。
4. HardFault应结合PC、LR、CFSR、HFSR定位。
5. FreeRTOS队列满且等待时间为0时立即返回失败。
6. DMA环形缓冲区必须计算回绕位置。
7. ACK丢失后应使用相同SEQ重传，执行端负责去重。
8. 超过重试次数表示结果未知，不能直接断言设备没有执行。

## 六、验证状态

- 两道代码题：编译成功。
- STM32 DMA升级：Keil Rebuild成功。
- STM32 DMA接收、ACK及STOP实机验证：尚未进行。
- ESP32 OTA构建、Release下载校验和真实升级：尚未完成。

## 七、第八天待办

1. STM32F407 DMA/IDLE板载通信验证。
2. 重复SEQ、连续命令及STOP抢占测试。
3. 恢复ESP-IDF环境，构建V1.0.0 OTA测试固件。
4. 验证V1.0.1 Release下载文件SHA256。
5. 进行V1.0.0 → V1.0.1真实HTTPS升级。
6. 检查OTA启动分区、版本、自检与回滚状态。