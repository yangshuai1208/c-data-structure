资料库
/
day21_cpp17_raii_stl.md


Day21：C++17 RAII、智能指针、STL 与异常安全
一、当天目标
Day21 的主线是：

引用与生命周期
→ RAII
→ 拷贝与移动语义
→ 智能指针
→ vector / find_if / optional
→ 异常安全
→ Linux 网关项目应用
当天完成的两份代码：

cpp/day21_raii_file.cpp：用 RAII 管理 FILE *，禁止拷贝并实现移动。

cpp/day21_pending_commands.cpp：用 vector、find_if、lambda、optional 管理待处理命令。

二、引用、值类别与生命周期
1. 左值引用与右值引用
int value = 10;

int &left_ref = value;       // 左值引用
const int &const_ref = 20;   // const 左值引用可绑定临时对象
int &&right_ref = 30;        // 右值引用
左值通常表示具有身份、可以取地址的对象。

右值通常表示临时值或即将被转移资源的对象。

引用不是独立拥有者，不负责销毁所引用的对象。

引用一旦绑定，不能改为引用另一个对象。

2. const 引用延长临时对象生命周期
const std::string &text = std::string("OPEN");
这里临时 std::string 直接绑定到局部 const 引用，其生命周期延长到 text 的生命周期结束。

但不能背成“只要有 const 引用就一定延长生命周期”。例如：

const std::string &bad()
{
    return std::string("OPEN"); // 错误：返回临时对象的引用
}
函数返回后临时对象已经销毁，调用者得到悬空引用。生命周期延长规则与具体绑定方式有关，不能通过再次传递引用无限延长。

3. 悬空指针与悬空引用
当指针或引用仍保存一个地址，但该地址对应对象的生命周期已经结束时，它们就是悬空的。它们的值不一定自动变成 nullptr，但不能再解引用。

const int *pointer = nullptr;

{
    int value = 10;
    pointer = &value;
}

// pointer通常仍非空，但*pointer属于未定义行为
三、RAII
1. 什么是 RAII
RAII 是 Resource Acquisition Is Initialization，即“资源获取即初始化”。核心思想是：

在对象构造时获取资源。

在对象析构时释放资源。

用对象的作用域管理资源生命周期。

正常返回和异常栈展开都会调用已构造局部对象的析构函数。

资源不只包括堆内存，还包括：

文件句柄。

Socket、串口文件描述符。

互斥锁。

线程。

数据库连接。

硬件或系统资源句柄。

2. RAII 为什么比手工清理可靠
手工管理容易遗漏提前返回和异常路径：

FILE *file = std::fopen(path, "w");

do_something_that_may_throw();

std::fclose(file); // 前面抛异常时不会执行
RAII 对象的析构函数统一负责清理，因此每个调用点不需要重复写清理逻辑。

3. explicit 与 RAII 没有直接等价关系
explicit LogFile(const char *path);
explicit 的作用是禁止不希望发生的隐式转换：

LogFile file("a.log"); // 正确：直接初始化
// LogFile file = "a.log"; // explicit时不允许
它有助于接口安全，但不是“获取资源”的语法，也不决定对象是否使用 RAII。

四、析构、拷贝与移动：Rule of Five
1. 五个特殊成员函数
~LogFile();                                   // 析构函数
LogFile(const LogFile &) = delete;            // 拷贝构造函数
LogFile &operator=(const LogFile &) = delete; // 拷贝赋值运算符
LogFile(LogFile &&) noexcept;                 // 移动构造函数
LogFile &operator=(LogFile &&) noexcept;      // 移动赋值运算符
它们分别处理：

对象销毁。

用旧对象创建新对象。

给已经存在的对象复制状态。

用旧对象创建新对象并转移资源。

给已经存在的对象转移资源。

2. 为什么资源类不能默认浅拷贝
如果两个 LogFile 只是复制同一个 FILE *：

first.file_ ─┐
             ├→ 同一个FILE对象
second.file_ ┘
两个析构函数都会尝试 fclose()，会造成重复释放，属于未定义行为。因此本练习选择：

LogFile(const LogFile &) = delete;
LogFile &operator=(const LogFile &) = delete;
3. 移动构造与移动赋值的区别
移动构造创建一个新对象：

LogFile second(std::move(first));
移动赋值的目标对象已经存在，可能已经持有资源，所以必须先释放旧资源：

LogFile third("old.log");
third = std::move(second);
4. std::move 自己并不移动资源
std::move(x) 本质上是类型转换，把 x 转成可匹配右值引用的表达式。真正的资源转移发生在移动构造函数或移动赋值运算符内部。

LogFile second(std::move(first));
执行流程是：

std::move(first) 允许选择 LogFile(LogFile &&)。

移动构造函数复制文件指针。

将 first.file_ 置为 nullptr。

所有权转移给 second。

5. 为什么移动操作常写 noexcept
LogFile(LogFile &&other) noexcept;
LogFile &operator=(LogFile &&other) noexcept;
表示该操作承诺不抛异常。

标准容器扩容时更愿意使用不会抛异常的移动操作。

资源句柄的简单转交通常不需要抛异常。

noexcept 是承诺，不应随意添加；如果函数内部确实可能抛异常却标记了 noexcept，异常逃出时程序会调用 std::terminate()。

6. Rule of Zero
如果类只由 std::string、std::vector、std::unique_ptr 等 RAII 成员组成，通常不需要自己编写析构、拷贝和移动操作，这叫 Rule of Zero。

优先级通常是：

优先Rule of Zero
→ 必须直接管理裸资源时再实现Rule of Five
五、完整代码一：RAII 文件类
#include <cassert>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <utility>

class LogFile
{
public:
    explicit LogFile(const char *path)
        : file_(std::fopen(path, "w"))
    {
        if (file_ == nullptr)
        {
            throw std::runtime_error("failed to open file");
        }
    }

    ~LogFile()
    {
        close();
    }

    LogFile(const LogFile &) = delete;
    LogFile &operator=(const LogFile &) = delete;

    LogFile(LogFile &&other) noexcept
        : file_(other.file_)
    {
        other.file_ = nullptr;
    }

    LogFile &operator=(LogFile &&other) noexcept
    {
        if (this != &other)
        {
            close();
            file_ = other.file_;
            other.file_ = nullptr;
        }

        return *this;
    }

    void write(const std::string &text)
    {
        if (file_ == nullptr)
        {
            throw std::logic_error("file is not open");
        }

        std::fputs(text.c_str(), file_);
        std::fputc('\n', file_);
    }

    bool is_open() const noexcept
    {
        return file_ != nullptr;
    }

private:
    void close() noexcept
    {
        if (file_ != nullptr)
        {
            std::fclose(file_);
            file_ = nullptr;
        }
    }

    std::FILE *file_;
};

int main()
{
    LogFile first("build/day21_first.log");
    first.write("OPEN");

    LogFile second(std::move(first));

    assert(!first.is_open());
    assert(second.is_open());

    LogFile third("build/day21_second.log");
    third = std::move(second);

    assert(!second.is_open());
    assert(third.is_open());

    third.write("STOP");

    std::puts("Day21 RAII file tests passed.");
    return 0;
}
代码中的关键不变量
一个有效的 FILE * 同一时刻只能由一个 LogFile 拥有。

移动后的源对象必须仍可安全析构，因此置为 nullptr。

close() 可重复调用：空指针状态下什么也不做。

移动赋值必须处理自移动：if (this != &other)。

移动赋值必须先释放目标对象原来拥有的资源。

析构函数不能让异常逃出。

更简洁的移动构造写法
LogFile(LogFile &&other) noexcept
    : file_(std::exchange(other.file_, nullptr))
{
}
std::exchange 返回旧值，同时把对象改为新值。

六、智能指针与所有权
1. std::unique_ptr
std::unique_ptr<T> 表示独占所有权：同一时刻只能有一个拥有者。

auto first = std::make_unique<Command>();
Command *observer = first.get();
auto second = std::move(first);
状态变化：

操作	所有者	观察指针
make_unique 后	first	尚无
observer = first.get()	first	observer非拥有
second = std::move(first)	second	observer仍指向同一堆对象
vector.push_back(std::move(second))	vector中的unique_ptr	仍有效
vector.clear()	无，堆对象被销毁	observer悬空
注意：

get() 不转移所有权。

不能对 get() 得到的观察指针执行 delete。

移动 unique_ptr 后，源智能指针通常为空。

所有者销毁后，观察指针不会自动变为 nullptr。

2. std::shared_ptr
std::shared_ptr<T> 使用引用计数表达共享所有权。最后一个拥有者销毁或 reset() 后，对象才会销毁。

auto first = std::make_shared<Node>();
auto second = first;
不能仅因为多个位置需要“访问”对象，就全部改成 shared_ptr。共享所有权必须是设计语义，而不是为了逃避生命周期分析。

3. shared_ptr 循环引用
struct Node
{
    std::shared_ptr<Node> peer;
};
两个对象互相使用 shared_ptr：

对象A → shared_ptr → 对象B
对象B → shared_ptr → 对象A
外部指针释放后，内部引用计数仍不为零，两个对象都不能析构，造成内存泄漏。

4. std::weak_ptr
观察关系应使用 weak_ptr 打破环：

struct Node
{
    std::weak_ptr<Node> peer;
};

if (auto peer = node.peer.lock())
{
    // lock成功期间，peer临时共享拥有对象
}
weak_ptr 不增加共享引用计数。

expired() 可检查对象是否已销毁。

lock() 成功返回 shared_ptr，失败返回空 shared_ptr。

多线程中不要先 expired() 再假设对象一直存活；直接用 lock() 获取临时所有权更可靠。

七、std::vector、迭代器与失效规则
1. size 与 capacity
size()：当前元素数量。

capacity()：不重新分配内存时最多可容纳的元素数量。

reserve(n)：请求容量至少为 n，不改变元素数量。

clear()：销毁全部元素，使 size() 变为零；不保证释放容量。

2. push_back 复杂度
摊销时间复杂度：O(1)。

发生扩容的单次操作：需要搬移或复制已有元素，通常是 O(n)。

“摊销 O(1)”不等于每次都严格 O(1)。

3. 扩容导致指针、引用和迭代器失效
当 vector 扩容并搬到新内存时，指向旧元素的：

指针。

引用。

迭代器。

都会失效。

可选方案：

返回对象副本。

保存索引或稳定 ID，并在使用时重新查找。

提前 reserve()，但超过保留容量后仍可能失效。

使用 std::vector<std::unique_ptr<T>>，vector 元素会移动，但堆上的 T 地址保持不变，直到其 unique_ptr 被销毁。

需求确实需要节点稳定性时选择合适的节点式容器。

4. operator[] 与 at()
commands_[index];     // 不进行边界检查，越界是未定义行为
commands_.at(index);  // 越界抛出std::out_of_range
不能说 operator[] 越界一定抛异常。

5. erase() 失效规则
commands_.erase(iterator);
删除位置及其后的迭代器、指针和引用会失效，因为后续元素通常需要前移。删除前保存的独立对象副本不受影响。

八、std::find_if、lambda 与 std::optional
1. std::find_if
const auto iterator = std::find_if(
    commands_.begin(),
    commands_.end(),
    [sequence](const Command &command)
    {
        return command.sequence == sequence;
    }
);
含义：

[begin, end) 是左闭右开的搜索区间。

lambda 对每个元素返回是否匹配。

找到时返回对应迭代器。

找不到时返回 commands_.end()。

end() 是尾后迭代器，不能解引用。

2. lambda 捕获
[sequence](const Command &command) // 按值捕获sequence
[&sequence](const Command &command) // 按引用捕获sequence
本题中两种都能工作。按值捕获更直接地表示查询条件不会被 lambda 修改，也避免异步保存 lambda 时引用悬空的问题。

3. std::optional<Command>
optional 表示“可能有一个值，也可能没有值”：

std::optional<Command> result;

return std::nullopt; // 没找到
return *iterator;    // 找到，复制Command到optional
常用接口：

result.has_value();
if (result) { /* 有值 */ }
result->sequence;
(*result).name;
result.value(); // 无值时抛std::bad_optional_access
本题返回副本，而不是返回 vector 内部元素的指针或引用，因此随后扩容、删除或清空容器都不会让已返回的 optional<Command> 悬空。

4. 成员名为什么写成 commands_
末尾下划线只是命名约定，不是 C++ 语法：

std::vector<Command> commands_;
它表示这是类的私有成员，便于和参数、局部变量区分。commands 使用复数，是因为它保存多条命令。

避免自行使用双下划线或“下划线加大写字母”开头的标识符，因为这些名称通常保留给实现。

九、完整代码二：待处理命令容器
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <optional>
#include <string>
#include <utility>
#include <vector>

struct Command
{
    unsigned int sequence;
    std::string name;
};

class PendingCommands
{
public:
    bool add(Command command)
    {
        const auto iterator = std::find_if(
            commands_.begin(),
            commands_.end(),
            [sequence = command.sequence](const Command &item)
            {
                return item.sequence == sequence;
            }
        );

        if (iterator != commands_.end())
        {
            return false;
        }

        commands_.push_back(std::move(command));
        return true;
    }

    std::optional<Command> find(unsigned int sequence) const
    {
        const auto iterator = std::find_if(
            commands_.begin(),
            commands_.end(),
            [sequence](const Command &item)
            {
                return item.sequence == sequence;
            }
        );

        if (iterator == commands_.end())
        {
            return std::nullopt;
        }

        return *iterator;
    }

    bool remove(unsigned int sequence)
    {
        const auto iterator = std::find_if(
            commands_.begin(),
            commands_.end(),
            [sequence](const Command &item)
            {
                return item.sequence == sequence;
            }
        );

        if (iterator == commands_.end())
        {
            return false;
        }

        commands_.erase(iterator);
        return true;
    }

    std::size_t size() const noexcept
    {
        return commands_.size();
    }

private:
    std::vector<Command> commands_;
};

int main()
{
    PendingCommands pending;

    assert(pending.size() == 0U);
    assert(!pending.find(1U).has_value());

    assert(pending.add({1U, "OPEN"}));
    assert(pending.add({2U, "GRAB"}));
    assert(pending.size() == 2U);

    assert(!pending.add({1U, "STOP"}));
    assert(pending.size() == 2U);

    const std::optional<Command> found = pending.find(2U);
    assert(found.has_value());
    assert(found->sequence == 2U);
    assert(found->name == "GRAB");

    const std::optional<Command> saved = pending.find(1U);
    assert(saved.has_value());

    assert(pending.add({3U, "RELEASE"}));
    assert(pending.add({4U, "STOP"}));
    assert(saved->sequence == 1U);
    assert(saved->name == "OPEN");

    assert(pending.remove(2U));
    assert(!pending.find(2U).has_value());
    assert(pending.size() == 3U);

    assert(!pending.remove(2U));
    assert(pending.size() == 3U);

    assert(pending.remove(1U));
    assert(saved->name == "OPEN");

    std::cout << "Day21 pending commands: all checks passed.\n";
    return 0;
}
接口语义
add()：按值接收命令，重复序号返回 false；成功时移动进容器。

find()：只读查找，找到后返回独立副本，找不到返回 nullopt。

remove()：找到后删除并返回 true；重复删除返回 false。

size()：不修改对象且不抛异常，所以声明为 const noexcept。

十、异常安全与锁的 RAII
1. 手工 lock() / unlock() 的问题
mutex_.lock();
std::string formatted = format_message(message); // 可能抛异常
cache_.push_back(std::move(formatted));           // 也可能抛异常
mutex_.unlock();
如果中间语句抛异常，控制流会跳过 unlock()。互斥锁保持锁定，其他线程可能永久阻塞，形成死锁。

2. 使用 std::lock_guard
void add_log(const std::string &message)
{
    std::string formatted = format_message(message);

    std::lock_guard<std::mutex> lock(mutex_);
    cache_.push_back(std::move(formatted));
}
lock_guard 构造时加锁。

离开作用域时析构并自动解锁。

正常返回与异常栈展开都能解锁。

格式化放在加锁前，可以缩短临界区。

3. 为什么先准备、后提交
推荐顺序：

锁外完成耗时或可能失败的准备
→ 加锁
→ 快速修改共享状态
→ 自动解锁
这能减少锁竞争，也避免在不必要的阶段持锁。

4. 常见异常安全保证
等级	含义
不抛保证	操作承诺不抛异常，常用 noexcept 表达
强保证	操作失败时，对象状态与调用前相同
基本保证	操作失败后对象仍有效、资源不泄漏，但状态可能改变
无保证	失败后连对象有效性或资源安全都不能保证
RAII 是异常安全的基础，但“用了 RAII”不自动等于整个操作具有强保证；还要分析状态修改顺序。

十一、项目口述标准答案
问题1：LogManager 为什么适合使用 C++ RAII 和 STL？
标准回答：

Linux 网关的 LogManager 同时管理文件、共享缓存和互斥锁，这些资源都有明确的获取与释放时机，因此适合用 RAII。std::ofstream 管理日志文件的打开与关闭，离开作用域时自动关闭；std::mutex 表示共享数据的互斥资源；std::lock_guard 在作用域内自动加锁和解锁，即使格式化或容器操作抛异常也不会漏解锁；std::vector 可以保存动态日志缓存。实际设计中我会先在锁外完成格式化，再进入较短的临界区写入缓存，并对缓存容量、刷盘策略和过载行为做限制。

各组件职责：

组件	解决的问题
std::ofstream	文件句柄的自动关闭与输出接口
std::mutex	防止多个线程同时修改共享状态产生数据竞争
std::lock_guard	保证所有退出路径都能解锁
std::vector	管理动态日志缓存及其元素生命周期
如果做异步日志，还应使用有界队列和独立写线程，明确队列满时阻塞、丢弃、降级或同步刷盘的策略，不能无限积压。

问题2：ProtocolParser 遇到错误帧应该抛异常吗？
标准回答：

串口或网络中的半包、校验失败、非法长度通常是可预期的输入状态，不适合把每一次解析失败都当成异常。若调用方只关心成功或失败，可以返回 std::optional<Command>；若需要区分等待更多数据、帧头错误、长度错误和校验错误，应返回详细的枚举或结果对象。解析成功时按值返回命令，避免返回指向临时对象或可失效缓冲区的指针。异常更适合表示无法在本层正常处理的故障，例如资源分配失败或内部不变量被破坏。

简单接口：

std::optional<Command> parse_frame(const Frame &frame);
详细接口：

enum class ParseResult
{
    Success,
    NeedMoreData,
    InvalidHeader,
    InvalidLength,
    ChecksumError
};
工程上还需要：先检查长度再访问字段、保留半包尾部、限制最大帧长、错误后重新同步帧头，并记录错误计数。

问题3：为什么 Linux 网关适合 C++，STM32 底层驱动仍可保留 C？
标准回答：

Linux 网关的 LogManager、ProtocolParser 和命令管理涉及文件、字符串、动态容器、并发锁以及复杂对象生命周期，C++ 的 RAII、智能指针、string、vector、optional 和标准算法能降低资源泄漏与生命周期错误。STM32 底层驱动通常与寄存器、HAL、ISR 和 C ABI 紧密结合，并要求可预测的内存、代码体积和实时行为，因此继续使用 C 往往更直接。C++ 不是任何场景都更好；应根据平台资源、工具链、团队能力和实时约束选择。在 MCU 上也可使用受控的 C++ 子集，同时按项目要求关闭或避免异常、RTTI 和不受控动态分配。

一句话总结：

高层业务用 C++ 管理复杂生命周期，底层驱动用 C 保持接口和资源行为可控；语言选择服务于约束，而不是互相替代。

十二、当天错题与代码错误
1. 判断题错题
题目	错误认识	正确结论
const引用与临时对象	只要用const引用就总能延长生命周期	只有符合规则的直接绑定才会延长；返回局部或临时对象引用仍会悬空
std::move	调用它就完成资源移动	它主要进行类型转换，真正转移由移动构造或移动赋值实现
vector::operator[]	越界会抛异常	operator[]不检查边界；at()才会在越界时抛异常
2. LogFile 代码错误
错误一：把赋值写成比较。

file_ == nullptr; // 只比较，没有修改file_
正确：

file_ = nullptr;
错误二：移动赋值只把 close() 放进自移动判断，却把资源转移放在判断外。自移动时会把自己的指针清空并丢失资源。

正确结构：

if (this != &other)
{
    close();
    file_ = other.file_;
    other.file_ = nullptr;
}
错误三：直接 std::fclose(file_) 没有先判断空指针。应集中调用可重复执行的 close()。

3. PendingCommands 代码错误
错误一：add() 的条件写反。

if (iterator == commands_.end())
{
    return false; // 错：end表示没找到，应允许添加
}
正确：

if (iterator != commands_.end())
{
    return false; // 找到了重复sequence
}
错误二：remove() 的条件写反。

if (iterator != commands_.end())
{
    return false; // 错：找到了反而返回失败
}
正确：

if (iterator == commands_.end())
{
    return false;
}

commands_.erase(iterator);
return true;
错误三：把 find_if() 直接放进 if。迭代器不是“找到/没找到”的布尔结果，必须保存返回值并与 end() 比较。

错误四：误用不存在的 command_、未声明的 iterator，以及把 make_error_code 当成查询结果。正确做法是返回 *iterator 或 std::nullopt。

4. 异常与锁的错误
错误写法：

mutex_.lock();
may_throw();
mutex_.unlock();
中间抛异常时 unlock() 被跳过。应使用 lock_guard，并尽量把可失败的准备工作移到锁外。

十三、常用 API 速查
API	作用	注意事项
std::move(x)	将表达式转换为可被移动操作接收的值类别	自己不执行资源转移
std::exchange(x, v)	返回x旧值并把x设为v	适合编写移动构造
std::make_unique<T>()	创建独占所有权对象	优先于裸new
unique_ptr::get()	取得非拥有裸指针	不可自行delete，所有者销毁后会悬空
unique_ptr::reset()	销毁当前对象并可接管新指针	会影响观察指针
std::make_shared<T>()	创建共享所有权对象	警惕循环引用
shared_ptr::reset()	释放当前共享所有权	引用计数为零时销毁对象
weak_ptr::lock()	尝试取得临时shared_ptr	失败时返回空指针
weak_ptr::expired()	检查被观察对象是否已销毁	并发场景优先直接使用lock()
vector::push_back()	在末尾添加元素	可能扩容并导致失效
vector::reserve()	预留容量	不改变size()，也不是永久地址保证
vector::erase()	删除指定位置元素	该位置及其后的迭代器等失效
vector::clear()	销毁全部元素	不保证释放容量
vector::at()	带边界检查访问	越界抛out_of_range
std::find_if()	按谓词搜索区间	未找到返回尾后迭代器
std::nullopt	表示optional无值	不等于默认构造一个Command
optional::has_value()	检查是否有值	访问前必须确认
std::lock_guard	作用域自动加锁/解锁	不支持手动提前解锁
std::runtime_error	表示运行时错误	继承自std::exception
std::logic_error	表示接口状态或逻辑使用错误	本例用于移动后文件不可写
十四、面试八股速答
1. 什么是 RAII？
RAII 把资源生命周期绑定到对象生命周期：构造时获取，析构时释放。正常返回和异常栈展开都会执行局部对象析构，因此可以统一管理内存、文件、锁、Socket 等资源。

2. std::move 做了什么？
std::move 主要进行类型转换，让表达式能够匹配右值引用；它本身不移动资源。真正的转移逻辑由移动构造或移动赋值函数实现。

3. 移动后对象还能不能使用？
移动后对象必须仍然有效并且可以安全析构或重新赋值，但其具体值通常只保证处于有效但未指定状态。本例人为规定移动后的 LogFile 为空。

4. 为什么移动赋值要检查自移动？
因为目标与源可能是同一对象。如果先释放自己的资源再从自己接管，就可能丢失资源。常用 if (this != &other) 保护整个释放与转移过程。

5. Rule of Five 与 Rule of Zero 是什么？
类直接管理资源并自定义其中一个析构、拷贝或移动操作时，通常要一起考虑五个特殊成员函数。若资源都由标准 RAII 类型成员管理，应优先使用 Rule of Zero，不手写这些函数。

6. unique_ptr 与 shared_ptr 如何选择？
所有权唯一时优先 unique_ptr；确实存在多个共同拥有者时才用 shared_ptr。仅观察对象时使用裸观察指针、引用或 weak_ptr，不要为了方便把所有关系都改成共享所有权。

7. 如何解决 shared_ptr 循环引用？
先明确拥有关系和观察关系，把至少一个非拥有方向改为 weak_ptr，使用时通过 lock() 临时获得 shared_ptr。

8. vector 为什么会使指针失效？
容量不足时 vector 会申请新内存并搬移元素，旧内存中的指针、引用和迭代器随即失效。删除元素还会使删除位置及之后的位置失效。

9. optional 解决什么问题？
optional<T> 显式表示可能存在一个 T，也可能没有，避免使用特殊值或返回可能为空的裸指针。本题返回命令副本，调用者不依赖容器内部对象的生命周期。

10. 为什么锁要使用 RAII？
手工 lock/unlock 遇到提前返回或异常容易漏解锁。lock_guard 在构造时加锁、析构时解锁，可以保证所有离开作用域的路径都释放锁。

11. 异常是否适合表示协议解析失败？
可预期的坏帧、半包和校验失败更适合用返回状态表达；异常用于本层难以正常处理的异常故障。需要错误原因时返回枚举或结果对象，只需成功/失败时可用 optional。

12. C++ 一定比 C 更适合嵌入式吗？
不一定。Linux 高层模块可利用 RAII、容器和类型系统管理复杂状态；MCU 底层驱动可能更重视 C ABI、可预测内存、代码尺寸和 ISR 约束。应按平台与模块选择语言或 C++ 子集。

十五、Developer PowerShell for VS 2022 编译命令
在项目根目录执行：

cd D:\43131\c-data-structure-git
New-Item -ItemType Directory -Force .\build | Out-Null
编译并运行 RAII 文件练习：

cl /nologo /std:c++17 /utf-8 /EHsc /W4 /WX /D_CRT_SECURE_NO_WARNINGS .\cpp\day21_raii_file.cpp /Fe:.\build\day21_raii_file.exe /Fo:.\build\day21_raii_file.obj
.\build\day21_raii_file.exe
编译并运行待处理命令练习：

cl /nologo /std:c++17 /utf-8 /EHsc /W4 /WX .\cpp\day21_pending_commands.cpp /Fe:.\build\day21_pending_commands.exe /Fo:.\build\day21_pending_commands.obj
.\build\day21_pending_commands.exe
预期输出：

Day21 RAII file tests passed.
Day21 pending commands: all checks passed.
十六、提交前检查
.gitignore 建议包含：

build/
*.exe
*.obj
*.pdb
*.ilk
*.log
检查并提交：

git status
git add .\cpp\day21_raii_file.cpp .\cpp\day21_pending_commands.cpp .\notes\day21_cpp17_raii_stl.md .\README.md
git commit -m "day21: review C++17 RAII, smart pointers and STL"
git push
Day21 最需要牢记的六句话：

RAII 用对象生命周期管理资源，异常路径也能自动清理。

std::move 只提供移动条件，真正移动由特殊成员函数完成。

独占所有权优先 unique_ptr，观察关系不应伪装成共享所有权。

shared_ptr 可能循环引用，观察方向用 weak_ptr。

vector 扩容和删除会导致相关指针、引用、迭代器失效。

可预期的协议错误用返回状态，真正异常故障再考虑抛异常。