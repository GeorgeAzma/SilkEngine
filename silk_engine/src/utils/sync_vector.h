#pragma once

template<typename T>
class SyncVector : protected std::vector<T>, NonCopyable
{
public:
	SyncVector() = default;
	virtual ~SyncVector()
	{
		clear();
	}

public:
	const T& front() const
	{
		std::scoped_lock lock(container_mutex);
		return front();
	}

	const T& back() const
	{
		std::scoped_lock lock(container_mutex);
		return back();
	}

	void push_back(const T& item)
	{
		std::scoped_lock lock(container_mutex);
		emplace_back(item);

		std::scoped_lock lock_blocking(blocking);
		condition.notify_one();
	}

	bool empty() const
	{
		std::scoped_lock lock(container_mutex);
		return empty();
	}

	size_t size() const
	{
		std::scoped_lock lock(container_mutex);
		return size();
	}

	void clear()
	{
		std::scoped_lock lock(container_mutex);
		return clear();
	}

	T pop_back()
	{
		std::scoped_lock lock(container_mutex);
		auto t = std::move(back());
		pop_back();
		return t;
	}

	void wait() const
	{
		while (empty())
		{
			std::unique_lock lock(blocking);
			condition.wait(lock);
		}
	}

protected:
	std::mutex container_mutex;
	std::condition_variable condition;
	std::mutex blocking;
};