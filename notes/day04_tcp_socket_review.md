## 算法练习

### 反转单链表

使用`prev`、`current`和`next`三个指针：

```c
next = current->next;
current->next = prev;
prev = current;
current = next;