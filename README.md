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