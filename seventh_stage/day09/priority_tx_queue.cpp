
#include <cstdint>
#include <cstddef>
#include <string>
#include <deque>
#include <mutex>
#include <condition_variable>
#include <algorithm>
#include <cassert>
#include <iostream>
#include <thread>

struct TxFrame
{
    std::uint32_t seq;
    std::string frame;
    bool is_stop;
};

class PriorityTxQueue
{
public:
    explicit PriorityTxQueue(std::size_t capacity)
        : capacity_(capacity)
    {
    }

    bool push(
        std::uint32_t seq,
        const std::string &frame,
        bool is_stop)
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);

            if (stopped_ || capacity_ == 0)
            {
                return false;
            }

            if (queue_.size() >= capacity_)
            {
                if (!is_stop)
                {
                    return false;
                }

                auto it = std::find_if(
                    queue_.begin(),
                    queue_.end(),
                    [](const TxFrame &item)
                    {
                        return !item.is_stop;
                    });

                if (it == queue_.end())
                {
                    return false;
                }

                queue_.erase(it);
            }

            TxFrame item{seq, frame, is_stop};

            if (is_stop)
            {
                queue_.push_front(item);
            }
            else
            {
                queue_.push_back(item);
            }
        }

        cv_.notify_one();

        return true;
    }

    bool wait_and_pop(
        std::uint32_t &seq,
        std::string &frame)
    {
        std::unique_lock<std::mutex> lock(mutex_);

        cv_.wait(
            lock,
            [this]()
            {
                return stopped_ || !queue_.empty();
            });

        if (stopped_)
        {
            return false;
        }

        const TxFrame item = queue_.front();

        queue_.pop_front();

        seq = item.seq;
        frame = item.frame;

        return true;
    }

    void stop()
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);

            stopped_ = true;

            queue_.clear();
        }

        cv_.notify_all();
    }

private:
    std::deque<TxFrame> queue_;

    std::mutex mutex_;

    std::condition_variable cv_;

    std::size_t capacity_;

    bool stopped_ = false;
};

int main()
{
    std::uint32_t seq = 0;
    std::string frame;

    PriorityTxQueue queue(3);

    assert(queue.push(1, "OPEN", false));
    assert(queue.push(2, "GRAB", false));
    assert(queue.push(3, "RELEASE", false));

    assert(!queue.push(5, "OPEN", false));

    assert(queue.push(4, "STOP", true));

    assert(queue.wait_and_pop(seq, frame));
    assert(seq == 4);
    assert(frame == "STOP");

    assert(queue.wait_and_pop(seq, frame));
    assert(seq == 2);
    assert(frame == "GRAB");

    assert(queue.wait_and_pop(seq, frame));
    assert(seq == 3);
    assert(frame == "RELEASE");

    PriorityTxQueue stop_queue(2);

    assert(stop_queue.push(10, "STOP", true));
    assert(stop_queue.push(11, "STOP", true));

    assert(!stop_queue.push(12, "STOP", true));

    PriorityTxQueue empty_queue(0);

    assert(!empty_queue.push(20, "OPEN", false));
    assert(!empty_queue.push(21, "STOP", true));

    PriorityTxQueue waiting_queue(2);

    bool pop_result = true;

    std::thread consumer(
        [&]()
        {
            std::uint32_t received_seq = 0;
            std::string received_frame;

            pop_result = waiting_queue.wait_and_pop(
                received_seq,
                received_frame);
        });

    waiting_queue.stop();

    consumer.join();

    assert(!pop_result);

    assert(!waiting_queue.push(30, "OPEN", false));

    std::cout << "priority_tx_queue passed\n";

    return 0;
}
