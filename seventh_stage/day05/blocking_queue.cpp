#include <cassert>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class BlockingQueue
{
public:
    void push(int value)
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);

            queue_.push(value);
        }

        cv_.notify_one();
    }

    bool pop(int &value)
    {
        std::unique_lock<std::mutex> lock(mutex_);

        cv_.wait(
            lock,
            [this]()
            {
                return stopped_ || !queue_.empty();
            });

        if (stopped_ && queue_.empty())
        {
            return false;
        }

        value = queue_.front();

        queue_.pop();

        return true;
    }

    void stop()
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);

            stopped_ = true;
        }

        cv_.notify_all();
    }

private:
    std::queue<int> queue_;

    std::mutex mutex_;

    std::condition_variable cv_;

    bool stopped_ = false;
};
int main()
{
    BlockingQueue queue;

    int sum = 0;

    std::mutex sum_mutex;

    std::thread producer(
        [&queue]()
        {
            for (int i = 1; i <= 100; ++i)
            {
                queue.push(i);
            }

            queue.stop();
        });

    auto consumer =
        [&queue, &sum, &sum_mutex]()
        {
            int value = 0;

            while (queue.pop(value))
            {
                std::lock_guard<std::mutex> lock(sum_mutex);

                sum += value;
            }
        };

    std::thread consumer1(consumer);
    std::thread consumer2(consumer);

    producer.join();
    consumer1.join();
    consumer2.join();

    assert(sum == 5050);

    std::cout
        << "blocking_queue passed\n";

    return 0;
}