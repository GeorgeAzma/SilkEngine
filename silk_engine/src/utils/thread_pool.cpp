#include "thread_pool.h"

ThreadPool::ThreadPool(unsigned int thread_count)
    : threads(thread_count)
{
    for (auto& t : threads)
    {
        t = std::thread(&ThreadPool::work, this);
    }
}

ThreadPool::~ThreadPool()
{
    waitForTasks();

    running = false;
    for (auto& t : threads)
    {
        if (t.joinable())
            t.join();
    }
}

void ThreadPool::waitForTasks() const
{
    while (running_tasks)
    {
        std::this_thread::sleep_for(std::chrono::microseconds(1));
    }
}

bool ThreadPool::popTask(std::function<void()>& task)
{
    std::scoped_lock lock(tasks_mutex);

    if (tasks.empty())
        return false;

    task = std::move(tasks.front());
    tasks.pop();

    return true;
}

void ThreadPool::work()
{
    while (running)
    {
        std::function<void()> task;
        if (popTask(task))
        {
            task();
            --running_tasks;
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::microseconds(1));
        }
    }
}