#include <cassert>
#include <atomic>
#include <fstream>
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
    std::ofstream output(
        "build/day22_async.log",
        std::ios::out | std::ios::trunc
    );

    if (!output.is_open())
    {
        std::cerr << "Failed to open log file.\n";
        return 1;
    }

    BoundedLogQueue queue(8U);

    constexpr int messages_per_producer = 500;

    /* 两个生产线程共同修改，需要同步。 */
    std::atomic<std::size_t> accepted{0U};
    std::atomic<std::size_t> rejected{0U};

    /*
     * 仅消费线程修改；
     * 主线程在 consumer.join() 后读取。
     */
    std::size_t written = 0U;

    std::thread consumer([&]()
    {
        while (auto message = queue.pop())
        {
            output << *message << '\n';

            if (output)
            {
                ++written;
            }
        }
    });

    auto produce = [&](int producer_id)
    {
        for (int i = 0; i < messages_per_producer; ++i)
        {
            /* 字符串构造在队列锁外完成。 */
            std::string message =
                "producer=" + std::to_string(producer_id)
                + " seq=" + std::to_string(i);

            if (queue.try_push(std::move(message)))
            {
                accepted.fetch_add(1U);
            }
            else
            {
                rejected.fetch_add(1U);
            }
        }
    };

    std::thread first(produce, 1);
    std::thread second(produce, 2);

    /* 先等生产结束，确认不会再提交新日志。 */
    first.join();
    second.join();

    /* 关闭后，消费者继续处理队列中剩余日志。 */
    queue.close();

    /* 等待消费者排空队列并退出。 */
    consumer.join();

    /*
     * 此时没有其他线程访问 output，
     * 主线程可以刷新、关闭并检查文件状态。
     */
    output.flush();
    output.close();

    if (!output)
    {
        std::cerr << "Log file write or close failed.\n";
        return 1;
    }

    const std::size_t accepted_count = accepted.load();
    const std::size_t rejected_count = rejected.load();

    const std::size_t total =
        2U * static_cast<std::size_t>(
            messages_per_producer
        );

    assert(accepted_count + rejected_count == total);
    assert(written == accepted_count);

    std::cout
        << "Submitted: " << total << '\n'
        << "Accepted:  " << accepted_count << '\n'
        << "Rejected:  " << rejected_count << '\n'
        << "Written:   " << written << '\n'
        << "Day22 async logger: all checks passed.\n";

    return 0;
}