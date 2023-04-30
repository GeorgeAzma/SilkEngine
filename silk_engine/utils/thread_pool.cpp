#include "thread_pool.h"

ThreadPool::ThreadPool(uint thread_count)
    : threads(thread_count)
{
    for (auto& t : threads)
        t = std::thread(&ThreadPool::work, this);
}

ThreadPool::~ThreadPool()
{
    running = false;
    condition.notify_all();
    wait();
    for (auto& t : threads)
    {
        if (t.joinable())
            t.join();
    }
}

void ThreadPool::wait()
{
    while (running_tasks)
        std::this_thread::yield();
}

void ThreadPool::work()
{
    while (running)
    {
        std::function<void()> task = nullptr;
        {
            std::unique_lock lock(queue_mutex);
            condition.wait(lock, [this]()->bool { return tasks.size() || !running; });
            if (tasks.empty())
                break;
            task = std::move(tasks.front());
            tasks.pop();
        }
        if (task)
        {
            task();
            --running_tasks;
        }
    }
}