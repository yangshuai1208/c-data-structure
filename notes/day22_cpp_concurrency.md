Day22：C++并发与工程代码

1. 当天训练与结果

主线：C++17 thread、mutex、condition_variable、atomic与异步文件日志。

判断题10/15：第1、8、10、15题答错，第2题未答；已完成纠正。

完成两道代码分析：原子计数丢失更新、日志线程关闭退出。

day22_bounded_log_queue.cpp：用户运行通过。

day22_async_logger.cpp：用户运行通过，终端与文件行数截图核对一致。

day22_merge_sorted.cpp：将主比较逻辑的if改为while后，用户反馈运行正常。

完成3道项目书面复盘及批改；口述是否完成另行记录。

notes和README实际加入仓库、Git提交及岗位投递，不因笔记生成而视为已完成。

2. 线程与资源生命周期

std::thread成功构造后，新线程就可以执行任务，join不负责启动线程。

join等待线程结束，不会发出停止请求；成功join也建立线程结束与调用方后续执行之间的同步关系。

joinable表示线程对象仍关联一个可join的线程，不等于线程当前仍在执行。

joinable的std::thread对象析构会调用std::terminate，不能直接丢弃。

detach解除线程对象与线程的关联，不会延长线程引用的局部对象生命周期。

线程可能访问的队列、文件等对象必须保持存活，直到访问这些对象的线程结束。

3. mutex、RAII与条件变量

mutex保护共享状态，相关读写应遵循同一套同步规则。

lock_guard在构造时加锁，析构时解锁，适合短临界区。

unique_lock支持显式解锁和重新加锁，适合condition_variable::wait。

返回或异常栈展开时，持有锁的RAII对象析构会释放锁。

wait原子地释放互斥锁并进入等待；被唤醒后重新获得锁，再检查条件。

条件变量可能虚假唤醒，应使用谓词wait或while循环检查业务状态。

notify_one/notify_all不保存通知。真正持久存在的是受锁保护的状态。

本题等待条件：

cv_.wait(lock, [this]()
{
    return closed_ || !logs_.empty();
});

本题退出条件：

if (closed_ && logs_.empty())
{
    return std::nullopt;
}

等待用“关闭或非空”，退出用“关闭且为空”。状态修改在锁内完成，本题解锁后通知等待者。

4. atomic与丢失更新

普通int被多个线程无同步地递增，会产生数据竞争，属于未定义行为。
atomic的load和store各自是原子的，但组合起来不会自动成为原子读—改—写。

const int old_value = count.load();
count.store(old_value + 1);

两个线程都读到0，再分别写入1，就丢失一次更新。这里对count没有C++数据竞争，但存在逻辑竞态。

正确计数：

count.fetch_add(1);
// 或 ++count;

日志练习中，accepted和rejected由两个生产线程共同修改，因此使用atomic递增；written只由消费者修改，主线程在consumer.join之后读取，所以普通变量足够。written也可以声明成atomic，但这里没有必要。

5. 有界日志队列

接口与状态

行为

try_push：未关闭且未满

入队、通知消费者、返回true

try_push：已关闭或已满

返回false

pop：未关闭且为空

等待

pop：非空

返回最早一条日志，包括关闭后的剩余日志

pop：已关闭且为空

返回nullopt

close

锁内设置关闭状态，解锁后notify_all

检查容量和入队必须在同一个临界区。先移动队头到局部字符串，再pop删除队头。返回optional<string>不依赖队列内部元素生命周期。

当前限制是日志条数；每条字符串长度未限制，不能声称严格限制全部内存占用。try_push不会等待队列出现空位，但获取mutex仍可能等待，因此不是无锁或严格不阻塞的接口。返回false没有区分“满”和“关闭”。

6. 异步日志实测与边界

结构：两个生产线程提交日志，一个消费线程独占文件写入，队列容量8条，每个生产者尝试提交500条。

指标

本次运行

Submitted

1000

Accepted

233

Rejected

767

Written

233

文件实际行数

233

验证：233 + 767 = 1000，成功入队数 = 写入数 = 文件行数。其他运行的计数可能因调度变化。

安全退出顺序：

first.join();
second.join();
queue.close();
consumer.join();
output.flush();
output.close();

文件刷新、关闭后还需检查流状态。关闭前已接收的日志由消费者处理完，当前策略允许队列满时拒绝新日志。

如果先consumer.join再queue.close，消费者可能等待数据或关闭状态，主线程又等待消费者结束，从而永远无法执行close。即使队列为空也可能卡住。

本练习是正常运行和协作退出路径验证，不能据此声称覆盖全部并发交错、异常路径或长期稳定性。写入计数与最终流状态检查不等于断电持久化保证。未实现日志级别、轮转、关键日志专用队列、磁盘故障恢复，也未证明已接入Linux网关真实项目。

7. 合并有序数组

两个下标分别遍历两个非递减数组，每次取较小元素并推进对应下标；一组耗尽后追加另一组剩余元素。相等时先取任一侧均可保留重复值。

while (i < a.size() && j < b.size())
{
    if (a[i] < b[j])
    {
        result.push_back(a[i]);
        ++i;
    }
    else
    {
        result.push_back(b[j]);
        ++j;
    }
}

不能写成if，否则只比较一次就进入剩余元素追加，可能破坏顺序。时间O(n+m)，结果空间O(n+m)，除返回结果外辅助空间O(1)。测试涵盖空数组、负数、重复值和一组先耗尽。

8. 当天代码错题

将closed_误写成close_：成员变量名必须一致。

lambda内return末尾遗漏分号：return语句和整个wait调用分别需要分号。

初始化误写成==：初始化或赋值用=，比较用==。

合并数组主比较逻辑误用if：需要while持续比较。

9. 面试回答

为什么使用异步文件日志？

业务线程直接写文件可能受磁盘延迟影响。这里让业务线程提交日志到队列，由单个消费者统一写文件，降低文件I/O对业务执行的影响，简化文件访问同步。队列限制积压条数，队列共享状态仍使用互斥锁保护。异步不意味着磁盘本身写得更快。

队列拒绝大量日志怎么办？

当前练习选择队列满时拒绝新日志并计数，用于控制积压、避免等待空位。重要日志可考虑独立有界队列或预留容量，并定义再次满时的有限等待或备用输出策略；这些是改进方向，当前未实现。不能声称所有日志都不丢失。

如何安全退出？

先等待生产者结束，再关闭队列并唤醒消费者。消费者继续处理已接收日志，关闭且队列为空时退出。主线程join消费者后刷新、关闭文件并检查流状态。join不会主动停止线程，必须先提供可达的退出条件。

10. 构建与运行记录

环境：Windows、Developer PowerShell for VS 2022、MSVC、C++17。用户已配置VS Code运行入口。

在仓库根目录执行，build目录需存在：

cl /nologo /std:c++17 /utf-8 /EHsc /W4 /WX .\cpp\day22_bounded_log_queue.cpp /Fe:.\build\day22_bounded_log_queue.exe /Fo:.\build\day22_bounded_log_queue.obj
.\build\day22_bounded_log_queue.exe

cl /nologo /std:c++17 /utf-8 /EHsc /W4 /WX .\cpp\day22_async_logger.cpp /Fe:.\build\day22_async_logger.exe /Fo:.\build\day22_async_logger.obj
.\build\day22_async_logger.exe

cl /nologo /std:c++17 /utf-8 /EHsc /W4 /WX .\cpp\day22_merge_sorted.cpp /Fe:.\build\day22_merge_sorted.exe /Fo:.\build\day22_merge_sorted.obj
.\build\day22_merge_sorted.exe

(Get-Content .\build\day22_async.log | Measure-Object -Line).Lines

三个练习各有main，应分别构建。异步日志使用build/day22_async.log相对路径，运行工作目录需为仓库根目录。