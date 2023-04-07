#pragma once

class Thread
{
public:
	template<typename Fn, typename... Args>
	void run(Fn&& func, Args&&... args)
	{
		thread = std::thread(func, std::forward<Args>(args)...);
	}

	void join()
	{
		thread.join();
	}

private:
	std::thread thread;
};