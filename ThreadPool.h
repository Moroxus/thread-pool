#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class ThreadPool
{
public:
    ThreadPool(unsigned int threadNumber = std::thread::hardware_concurrency()) {
        createThreads(std::max(1u, threadNumber));
    }

    ThreadPool(const ThreadPool &other) = delete;
    ThreadPool &operator=(const ThreadPool &other) = delete;

    ~ThreadPool() {
        shutdown = true;
        notifier.notify_all();
        for (auto &thread : threads) {
            thread.join();
        }
    }

    template <typename Function, typename... Args>
    auto push(Function &&function, Args &&... args) {
        using returnType = decltype (function(args...));
        std::packaged_task<returnType()> task([=]{ return function(args...); });
        std::future<returnType> future = task.get_future();
        {
            std::unique_lock<std::mutex> lock(mutex);
            taskQueue.emplace(std::packaged_task<void()>(std::move(task)));
        }
        notifier.notify_one();
        return future;
    }

private:

    void createThreads(unsigned int threadNumber) {
        auto threadFunction = [this]() {
            std::packaged_task<void()> task;
            while (true) {
                {
                    std::unique_lock<std::mutex> lock(mutex);
                    notifier.wait(lock);
                    if (shutdown) {
                       break;
                    }
                    if (taskQueue.empty()) {
                        continue;
                    }
                    task = std::move(taskQueue.front());
                    taskQueue.pop();
                }
                task();
            }
        };
        threads.reserve(threadNumber);
        for (unsigned int i = 0; i < threadNumber; ++i) {
            threads.emplace_back(std::thread(threadFunction));
        }
    }

private:
    std::mutex mutex;
    std::condition_variable notifier;
    std::vector<std::thread> threads;
    std::queue<std::packaged_task<void()>> taskQueue;
    std::atomic<bool> shutdown{false};
};

#endif // THREADPOOL_H
