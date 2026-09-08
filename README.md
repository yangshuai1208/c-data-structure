# embedded-learning
## 第六阶段复习记录

### Day 2：栈与有效括号

新增文件：

- `leetcode/day02_valid_parentheses.c`
- `notes/day02_stack_parentheses.md`

本次复习内容：

- 栈的后进先出特性
- 括号匹配算法
- 栈上溢和栈下溢
- `malloc` 和 `free`
- 空指针与数组越界检查
- `assert` 单元测试
- 时间复杂度和空间复杂度分析

实现思路：

1. 遇到左括号时入栈。
2. 遇到右括号时弹出栈顶元素。
3. 判断左右括号是否匹配。
4. 扫描结束后检查栈是否为空。

验证结果：

```text
valid parentheses tests passed
```
### Day 6：二分查找与有序数组原地去重

完成两道C语言数组代码练习：

1. 二分查找
   - 使用左闭右开区间 `[left, right)`。
   - 中点使用 `left + (right - left) / 2U`。
   - 避免 `size_t` 执行 `0U - 1U` 产生无符号下溢。
   - 时间复杂度为 `O(log n)`，空间复杂度为 `O(1)`。

2. 有序数组原地去重
   - 使用快慢指针。
   - 快指针扫描输入数组。
   - 慢指针维护已经去重的有效区域。
   - 时间复杂度为 `O(n)`，空间复杂度为 `O(1)`。

测试结果：

```text
binary search tests passed
remove duplicates tests passed

## Day20：数据结构与算法综合训练

本日完成链表、环形队列、二分和数组去重综合复习。

### 代码练习

- 复查并完善`leetcode/day04_merge_sorted_lists.c`。
- 增加空链表、重复值和不等长链表测试。
- 新增`data-structure/day20_ring_queue.c`。
- 完成空队列、满队列、回绕、空指针和FIFO顺序测试。

### 核心结论

- 合并两个有序链表：`O(m+n)`时间、`O(1)`额外空间。
- 环形队列的`push/pop/peek`均为`O(1)`。
- 使用`count`可以区分`head == tail`时的队空和队满。
- UART 115200、8N1理论吞吐量约为11520字节/秒。
- 任务阻塞20ms可能积累约231字节，128字节缓冲区不足。
- `volatile`不能替代临界区、原子变量或RTOS同步机制。

### 工程应用

固定数组环形缓冲区具有容量可控、访问时间确定、无动态内存碎片等特点，更适合UART中断接收。发生溢出时应记录错误、丢弃异常帧并重新寻找帧头，不能静默覆盖未解析字节。