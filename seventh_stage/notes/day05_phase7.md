# 第七阶段 Day05 总结

## 一、今日完成

### 1. 代码题：命令抢占
完成 `command_preemption.c`。

核心规则：

- `STOP` 可以无条件抢占当前动作。
- `OPEN / GRAB / RELEASE` 只能在空闲状态启动。
- 使用 `action_is_stop()`、`state_is_busy()`、`action_to_state()` 拆分判断逻辑。

核心思想：

> 不针对每种状态组合硬编码，而是抽象“特殊命令、busy状态、状态转换”。

---

### 2. 代码题：BlockingQueue
完成 `blocking_queue.cpp`。

涉及：

- `std::mutex`
- `std::lock_guard`
- `std::unique_lock`
- `std::condition_variable`
- predicate
- `notify_one()`
- `notify_all()`
- 线程安全退出

核心流程：

`producer -> queue -> condition_variable -> consumer`

关键点：

- `lock_guard`：RAII自动加锁/解锁。
- `unique_lock`：支持 `condition_variable::wait()` 临时释放并重新获取锁。
- `stop + queue empty` 时 consumer 才退出。
- `notify_one()` 用于新数据，`notify_all()` 用于停止所有等待线程。

---

## 二、STM32执行端升级②：STOP抢占

Day04已经完成非阻塞动作状态机。

今天修改调度逻辑：

```text
普通动作
→ busy时拒绝

STOP
→ busy时仍允许执行
→ 覆盖当前target
→ motion_state切换为STOPPING