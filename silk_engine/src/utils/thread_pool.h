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
            std::scoped_lock lock(queue_mutex);
            tasks.emplace([function, args...]
                {
                    function(args...);
                });
        }
        ++running_tasks;
    }

    template<typename Fn, typename... Args, typename R = std::invoke_result_t<std::decay_t<Fn>, std::decay_t<Args>...>, typename = std::enable_if_t<!std::is_void_v<R>>>
    std::future<R> submitFuture(Fn&& function, Args&&... args)
    {
        std::shared_ptr<std::promise<R>> task_promise(new std::promise<R>);
        std::future<R> future = task_promise->get_future();
        {
            std::scoped_lock lock(queue_mutex);
            tasks.emplace([function, args..., task_promise]
                {
                    task_promise->set_value(function(args...));
                });
        }
        ++running_tasks;

        return future;
    }

    template<typename Fn>
    void forEach(size_t count, Fn func)
    {
        size_t index = 0;
        for (size_t i = 0; i < threads.size(); ++i)
        {
            size_t invocations = count / threads.size();
            size_t remainder = count % threads.size();
            invocations += (i < remainder);
            submit([invocations, index, func] {
                    func(index, index + invocations);
                });
            index += invocations;
        }
        waitForTasks();
    }

    void waitForTasks() const;
    size_t runningTasks() const { return running_tasks; }
    size_t threadCount() const { return threads.size(); }

private:
    void work();

private:
    std::mutex queue_mutex;
    std::queue<std::function<void()>> tasks;
    std::vector<std::thread> threads;
    std::atomic<size_t> running_tasks;
    bool running = true;
};

