#pragma once

class ThreadPool
{
public:
    ThreadPool(unsigned int thread_count = std::thread::hardware_concurrency());
    ~ThreadPool();

    template<typename T, typename... Args>
    void submit(T&& function, Args&&... args)
    {
        {
            std::scoped_lock lock(tasks_mutex);
            tasks.emplace([function, args...]
                {
                    function(args...);
                });
        }
        ++running_tasks;
    }

    void waitForTasks() const;
    size_t runningTasks() const { return running_tasks; }

private:
    bool popTask(std::function<void()>& task);
    void work();

private:
    std::mutex tasks_mutex;
    std::queue<std::function<void()>> tasks;
    std::vector<std::thread> threads;
    std::atomic<size_t> running_tasks;
    bool running = true;
};

