#pragma once

class ThreadPool
{
public:
    size_t wait_time = 0; //Wait time in microseconds, if set to 0 it calls yield() instead of sleep()

    ThreadPool(unsigned int thread_count = std::thread::hardware_concurrency());
    ~ThreadPool();

    template<typename T, typename... Args>
    void submit(T&& function, Args&&... args)
    {
        ++running_tasks;
        {
            const std::scoped_lock lock(queue_mutex);
            tasks.emplace([function, args...]
                {
                    function(args...);
                });
        }
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
            const size_t invocations = count / threads.size() + (i < (count% threads.size()));
            //Calling func every invocation has small overhead for lots of small invocations (around 15% performance in the worst case), but I sacrificed it for ease of use
            submit([invocations, index, func] {
                for(size_t i = 0; i < invocations; ++i)
                    func(index + i);
                });
            index += invocations;
        }
        wait();
    }

    void wait() const;
    size_t runningTasks() const { return running_tasks; }
    size_t size() const { return threads.size(); }

private:
    void sleep() const;
    void work();

private:
    std::mutex queue_mutex;
    std::queue<std::function<void()>> tasks;
    std::vector<std::thread> threads;
    std::atomic<size_t> running_tasks;
    bool running = true;
};

