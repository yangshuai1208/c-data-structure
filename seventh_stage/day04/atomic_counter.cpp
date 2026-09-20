#include <atomic>
#include <thread>
#include <vector>
#include <cassert>
#include <iostream>

void worker(std::atomic<int> &counter, int count)
{
    for (int i = 0; i < count; ++i)
    {
        counter.fetch_add(1);
    }
}

int main()
{
    std::atomic<int> counter{0};

    const int thread_count = 4;
    const int increments_per_thread = 100000;

    std::vector<std::thread> threads;

    for (int i = 0; i < thread_count; ++i)
    {
        threads.emplace_back(
            worker,
            std::ref(counter),
            increments_per_thread);
    }

    for (auto &t : threads)
    {
        t.join();
    }

    std::cout << "counter = "
              << counter.load()
              << '\n';

    assert(counter.load() == 400000);

    std::cout << "All tests passed!\n";

    return 0;
}