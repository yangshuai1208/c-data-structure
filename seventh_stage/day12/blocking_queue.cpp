#include <cassert>
#include <condition_variable>
#include <cstddef>
#include <deque>
#include <iostream>
#include <mutex>
#include <thread>

template<typename T>
class BlockingQueue
{
public:
    explicit BlockingQueue(size_t capacity):capacity_(capacity)
    {

    }

    bool push(const T& value)
    {
        std::unique_lock<std::mutex> lock(mutex_);

        not_full_.wait(lock,[this]{
            return stopped_||queue_.size()<capacity_;
        });

        if(stopped_)
        {
            return false;
        }
        queue_.push_back(value);

        not_empty_.notify_one();

        return true;
    }
    
    bool pop(T& value)
    {
        std::unique_lock<std::mutex> lock(mutex_);

        not_empty_.wait(lock,[this]{
            return stopped_||!queue_.empty();
        });

        if(stopped_&&queue_.empty())
        {
            return false;
        }
        value=queue_.front();
        queue_.pop_front();

        not_full_.notify_one();

        return true;
    }

    void stop()
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            stopped_=true;
        }
        not_empty_.notify_all();
        not_full_.notify_all();
    }

private:
    std::deque<T> queue_;
    size_t capacity_;

    std::mutex mutex_;

    std::condition_variable not_empty_;
    std::condition_variable not_full_;

    bool stopped_{false};
};
int main()
{
    /* =========================
       1. FIFO测试
       ========================= */

    {
        BlockingQueue<int> queue(3);

        assert(queue.push(10));
        assert(queue.push(20));
        assert(queue.push(30));

        int value = 0;

        assert(queue.pop(value));
        assert(value == 10);

        assert(queue.pop(value));
        assert(value == 20);

        assert(queue.pop(value));
        assert(value == 30);

        queue.stop();

        assert(!queue.pop(value));
        assert(!queue.push(40));
    }

    /* =========================
       2. Consumer阻塞测试
       ========================= */

    {
        BlockingQueue<int> queue(2);

        int result = 0;

        std::thread consumer([&] {
            bool ok = queue.pop(result);
            assert(ok);
            assert(result == 100);
        });

        std::thread producer([&] {
            queue.push(100);
        });

        producer.join();
        consumer.join();

        queue.stop();
    }

    /* =========================
       3. Producer阻塞测试
       ========================= */

    {
        BlockingQueue<int> queue(1);

        assert(queue.push(1));

        std::thread producer([&] {
            bool ok = queue.push(2);
            assert(ok);
        });

        int value = 0;

        /*
         * pop释放一个位置，
         * producer才能继续push。
         */
        assert(queue.pop(value));
        assert(value == 1);

        producer.join();

        assert(queue.pop(value));
        assert(value == 2);

        queue.stop();
    }

    /* =========================
       4. stop唤醒Consumer
       ========================= */

    {
        BlockingQueue<int> queue(2);

        std::thread consumer([&] {
            int value = 0;

            bool ok = queue.pop(value);

            assert(!ok);
        });

        queue.stop();

        consumer.join();
    }

    /* =========================
       5. stop唤醒Producer
       ========================= */

    {
        BlockingQueue<int> queue(1);

        assert(queue.push(1));

        std::thread producer([&] {
            bool ok = queue.push(2);

            assert(!ok);
        });

        queue.stop();

        producer.join();
    }

    std::cout
        << "blocking_queue passed\n";

    return 0;
}