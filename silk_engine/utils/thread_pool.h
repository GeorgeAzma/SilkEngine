#pragma once

#include <future>

class ThreadPool
{
public:
    ThreadPool(uint thread_count = std::thread::hardware_concurrency());
    ~ThreadPool();

    template<typename T, typename... Args>
    void submit(T&& function, Args&&... args)
    {
        {
            std::scoped_lock lock(queue_mutex);
            tasks.emplace([function, args...]
                {
                    function(args...);
                });
        }
        ++running_tasks;
        condition.notify_one();
    }

    template<typename Fn, typename... Args, typename R = std::invoke_result_t<std::decay_t<Fn>, std::decay_t<Args>...>, typename = std::enable_if_t<!std::is_void_v<R>>>
    std::future<R> submitFuture(Fn&& function, Args&&... args)
    {
        shared<std::promise<R>> task_promise(new std::promise<R>);
        std::future<R> future = task_promise->get_future();
        {
            std::scoped_lock lock(queue_mutex);
            tasks.emplace([function, args..., task_promise]
                {
                    task_promise->set_value(function(args...));
                });
        }
        ++running_tasks;
        condition.notify_one();
        return future;
    }

    template<typename Fn>
    void forEach(size_t count, Fn&& func)
    {
        if (!count)
            return;
        size_t length = count / threads.size();
        size_t remain = count % threads.size();
        size_t index = 0u;
        for (size_t i = 0u; i < threads.size(); ++i)
        {
            size_t invocations = length + (i < remain);
            if (invocations)
            {
                submit([invocations, index, &func] {
                    for (size_t i = 0u; i < invocations; ++i)
                        func(index + i);
                    });
                index += invocations;
            }
        }
        wait();
    }

    void wait();
    size_t runningTasks() const { return running_tasks; }
    size_t size() const { return threads.size(); }

private:
    void work();

private:
    std::mutex queue_mutex;
    std::queue<std::function<void()>> tasks;
    std::vector<std::thread> threads;
    std::condition_variable_any condition;
    std::atomic_bool running = true;
    std::atomic<size_t> running_tasks = 0;
};

