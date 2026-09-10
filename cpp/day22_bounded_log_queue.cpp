#include <cassert>
#include <condition_variable>
#include <cstddef>
#include <iostream>
#include <mutex>
#include <optional>
#include <queue>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

class BoundedLogQueue
{
public:
    explicit BoundedLogQueue(std::size_t capacity)
        : capacity_(capacity)
    {
        if (capacity_ == 0U)
        {
            throw std::invalid_argument(
                "capacity must be greater than zero"
            );
        }
    }

    bool try_push(std::string message)
    {
        /*
         * TODO 1：
         * 1. 加锁，保护全部共享状态。
         * 2. 已关闭或队列已满，返回 false。
         * 3. 将 message 移动进队列。
         * 4. 解锁后 notify_one()。
         * 5. 返回 true。
         */
        std::unique_lock<std::mutex>lock(mutex_);
        if(closed_||logs_.size()>=capacity_)
        {
            return false;
        }
        logs_.push(std::move(message));
        lock.unlock();
        cv_.notify_one();
        return true;
    }

    std::optional<std::string> pop()
    {
        /*
         * TODO 2：
         * 1. 使用 unique_lock 加锁。
         * 2. 等待：已关闭，或者队列非空。
         * 3. 已关闭且队列为空，返回 nullopt。
         * 4. 将队头移动到局部字符串。
         * 5. 删除队头，返回局部字符串。
         */
        std::unique_lock<std::mutex>lock(mutex_);
        cv_.wait(lock,[this](){return closed_||!logs_.empty();});
        if(closed_&&logs_.empty())
        {
            return std::nullopt;
        }
        std::string message=std::move(logs_.front());
        logs_.pop();
        return message;
    }

    void close()
    {
        /*
         * TODO 3：
         * 1. 在锁内设置 closed_ = true。
         * 2. 解锁后 notify_all()。
         */

         std::unique_lock<std::mutex>lock(mutex_);
         closed_=true;
         lock.unlock();
         cv_.notify_all();
    }

private:
    const std::size_t capacity_;

    std::mutex mutex_;
    std::condition_variable cv_;

    std::queue<std::string> logs_;
    bool closed_ = false;
};

int main()
{
    /* 检查容量限制和先进先出。 */
    BoundedLogQueue queue(2U);

    bool accepted = queue.try_push("OPEN");
    assert(accepted);

    accepted = queue.try_push("GRAB");
    assert(accepted);

    accepted = queue.try_push("STOP");
    assert(!accepted);

    auto message = queue.pop();
    assert(message.has_value());
    assert(*message == "OPEN");

    /* 出队后释放一个位置，可以继续入队。 */
    accepted = queue.try_push("STOP");
    assert(accepted);

    /* 关闭后拒绝新日志。 */
    queue.close();

    accepted = queue.try_push("RELEASE");
    assert(!accepted);

    /* 关闭后仍需按顺序取完已有日志。 */
    message = queue.pop();
    assert(message.has_value());
    assert(*message == "GRAB");

    message = queue.pop();
    assert(message.has_value());
    assert(*message == "STOP");

    message = queue.pop();
    assert(!message.has_value());

    /* 重复关闭应当安全。 */
    queue.close();

    /* 消费线程运行与关闭退出检查。 */
    BoundedLogQueue threaded_queue(2U);
    std::vector<std::string> consumed;

    std::thread worker([&]()
    {
        while (auto item = threaded_queue.pop())
        {
            consumed.push_back(std::move(*item));
        }
    });

    accepted = threaded_queue.try_push("FIRST");
    assert(accepted);

    accepted = threaded_queue.try_push("SECOND");
    assert(accepted);

    threaded_queue.close();
    worker.join();

    assert(consumed.size() == 2U);
    assert(consumed[0] == "FIRST");
    assert(consumed[1] == "SECOND");

    std::cout
        << "Day22 bounded log queue: "
        << "all checks passed.\n";

    return 0;
}