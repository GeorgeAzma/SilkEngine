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
    wait();

    running = false;
    for (auto& t : threads)
    {
        if (t.joinable())
            t.join();
    }
}

void ThreadPool::wait() const
{
    while (running_tasks)
    {
        sleep();
    }
}

void ThreadPool::sleep() const
{ 
    if (wait_time) 
        std::this_thread::sleep_for(std::chrono::microseconds(wait_time)); 
    else 
        std::this_thread::yield();
}

void ThreadPool::work()
{
    while (running)
    {
        std::function<void()> task = nullptr;
        {
            std::unique_lock lock(queue_mutex);
            if (tasks.size())
            {
                task = std::move(tasks.front());
                tasks.pop();
            }
        }

        if (task)
        {
            task();
            --running_tasks;
        }
        else
        {
            sleep();
        }
    }
}