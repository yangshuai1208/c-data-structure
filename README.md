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